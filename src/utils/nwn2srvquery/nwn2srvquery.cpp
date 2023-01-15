#include <cstdio>
#include <string>

#ifdef _WIN32

#include <windows.h>
#include <winsock2.h>

typedef int socklen_t;

#pragma comment(lib, "ws2_32.lib")

typedef struct _TIME_STATE {
	LARGE_INTEGER freq;
	LARGE_INTEGER time;
} TIME_STATE, *PTIME_STATE;

typedef const struct _TIME_STATE* PCTIME_STATE;

#else

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <poll.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#define closesocket(s) close(s)

typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;

#define INVALID_SOCKET ((SOCKET)-1)

#define __in_ecount(x)
#define __inout
#define __inout_ecount(x)
#define __in
#define __out
#define __out_opt
#define __in_opt

typedef struct timeval TIME_STATE;
typedef struct timeval* PTIME_STATE;
typedef const struct timeval* PCTIME_STATE;

#endif

//
// Time differencing, returns count of milliseconds.
//

void record_time(__out PTIME_STATE timestate);
unsigned long diff_time(__in PCTIME_STATE then);

#ifdef _WIN32

void record_time(__out PTIME_STATE timestate)
{
	QueryPerformanceFrequency(&timestate->freq);
	QueryPerformanceCounter(&timestate->time);
}

unsigned long diff_time(__in PCTIME_STATE then)
{
	LARGE_INTEGER now;

	QueryPerformanceCounter(&now);

	return (now.QuadPart - then->time.QuadPart) / (then->freq.QuadPart / 1000);
}

#else

void record_time(__out PTIME_STATE timestate) { gettimeofday(timestate, NULL); }

unsigned long diff_time(__in PCTIME_STATE then)
{
	struct timeval now;

	gettimeofday(&now, NULL);

	return ((now.tv_sec - then->tv_sec) * 1000000 + now.tv_usec - then->tv_usec) / 1000;
}

#endif

#define QUERY_TIMEOUT (10000) /* Milliseconds to wait for a response */

template <class T>
bool GetPktField(__inout_ecount(Length) const unsigned char*& Data,
                 __inout size_t& Length,
                 __out T& Field)
{
	if (Length < sizeof(T))
		return false;

	Field = *reinterpret_cast<const T*>(Data);

	Length -= sizeof(T);
	Data += sizeof(T);

	return true;
}

bool GetPktString(__inout_ecount(Length) const unsigned char*& Data,
                  __inout size_t& Length,
                  __out std::string& String)
{
	//
	// Standard strings are a composite of a 32-bit length prefix and a string
	// buffer, not null terminated.
	//

	//
	// First, we'll get the string length field.
	//

	unsigned long StringLength;

	if (!GetPktField(Data, Length, StringLength))
		return false;

	//
	// Ensure the length is sane...
	//

	if (Length < static_cast<size_t>(StringLength))
		return false;

	//
	// Now, we'll initialize the std::string from the buffer.
	//

	try {
		String.assign(reinterpret_cast<const char*>(Data), static_cast<size_t>(StringLength));
	} catch (std::bad_alloc) {
		return false;
	}

	Data += static_cast<size_t>(StringLength);
	Length -= static_cast<size_t>(StringLength);

	return true;
}

bool GetPktData(__inout_ecount(Length) const unsigned char*& Data,
                __inout size_t& Length,
                __in size_t FieldLength,
                __out_opt const void** Field)
{
	if (Length < FieldLength)
		return false;

	//
	// If we have an output pointer, update it to point into the packet.
	// Otherwise, we'll just advance the buffer pointer and discard the data.
	//

	if (Field)
		*Field = reinterpret_cast<const void*>(Data);

	Length -= FieldLength;
	Data += FieldLength;

	return true;
}

template <class T>
bool AddPktField(__inout_ecount(Length) unsigned char*& Data,
                 __inout size_t& Length,
                 __in const T& Field)
{
	if (Length < sizeof(T))
		return false;

	*reinterpret_cast<T*>(Data) = Field;

	Length -= sizeof(T);
	Data += sizeof(T);

	return true;
}

bool AddPktData(__inout_ecount(Length) unsigned char*& Data,
                __inout size_t& Length,
                __in size_t FieldLength,
                __in const void* Field)
{
	if (Length < FieldLength)
		return false;

	//
	// Copy data directly into the buffer.
	//

	memcpy(Data, Field, FieldLength);

	//
	// Update our lengths appropriate.
	//

	Length -= FieldLength;
	Data += FieldLength;

	return true;
}

bool AddPktString(__inout_ecount(Length) unsigned char*& Data,
                  __inout size_t& Length,
                  __in const std::string& String)
{
	//
	// Standard strings are a composite of a 32-bit length prefix and a string
	// buffer, not null terminated.
	//

	//
	// Ensure the length is sane...
	//

	if (Length < String.size() + 4)
		return false;

	//
	// First, we'll add the string length field.
	//

	if (!AddPktField(Data, Length, static_cast<unsigned long>(String.size())))
		return false;

	//
	// Now, we'll add the string data from the std::string object.
	//

	if (!AddPktData(Data, Length, String.size(), String.data()))
		return false;

	return true;
}

int main(int ac, char** av)
{
#ifdef _WIN32
	WSADATA wd;

	WSAStartup(0x0202, &wd);
#endif

	unsigned char users    = 0;
	unsigned char maxusers = 0;
	SOCKET s;
	SOCKADDR_IN sin;
	socklen_t sinlen = sizeof(sin);
	char buf[1500];
	socklen_t rlen;
	int status            = -1;
	u_short port          = 5121;
	unsigned long elapsed = 0;
	TIME_STATE timestate;

	for (;;) {
		if (ac != 2 && ac != 3) {
			fprintf(stderr, "Usage: %s <server-address> [server-port]\n", av[0]);
			break;
		}

		if (ac > 2)
			port = atoi(av[2]);

		s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

		if (s == INVALID_SOCKET) {
			fprintf(stderr, "Failed to create socket.\n");
			break;
		}

		memset(&sin, sizeof(sin), 0);

		sin.sin_family      = AF_INET;
		sin.sin_port        = htons(port);
		sin.sin_addr.s_addr = inet_addr(av[1]);

		if (sin.sin_addr.s_addr == INADDR_NONE) {
			hostent* he = gethostbyname(av[1]);

			if ((!he) || (he->h_addrtype != AF_INET)) {
				fprintf(stderr, "Failed to resolve hostname.\n");
				break;
			}

			memcpy(&sin.sin_addr.s_addr, he->h_addr, he->h_length);
		}

		memcpy(buf, "BNXI", 4);

		record_time(&timestate);

		if (sendto(s, buf, 4, 0, (sockaddr*)&sin, sizeof(sin)) < 1) {
			fprintf(stderr, "sendto failed.\n");
			break;
		}

#ifdef _WIN32
		HANDLE event = CreateEvent(NULL, TRUE, FALSE, NULL);

		if (!event) {
			fprintf(stderr, "Failed to create event.\n");
			break;
		}

		if (WSAEventSelect(s, event, FD_READ | FD_CLOSE)) {
			fprintf(stderr, "WSAEventSelect failed.\n");
			CloseHandle(event);
			break;
		}

		if (WaitForSingleObject(event, QUERY_TIMEOUT) != WAIT_OBJECT_0) {
			fprintf(stderr, "Timed out.\n");
			CloseHandle(event);
			break;
		}

		CloseHandle(event);
#else
		pollfd pfd;

		pfd.fd     = s;
		pfd.events = POLLIN | POLLHUP | POLLERR;

		if (!poll(&pfd, 1, QUERY_TIMEOUT)) {
			fprintf(stderr, "Timed out.\n");
			break;
		}
#endif

		rlen = recvfrom(s, buf, sizeof(buf), 0, (sockaddr*)&sin, &sinlen);

		elapsed = diff_time(&timestate);

		if (rlen < 1) {
			fprintf(stderr, "recvfrom failed.\n");
			break;
		}

		//
		//	CHAR   Signature[ 4 ]; // "BNXR"
		//	USHORT Port;
		//	UCHAR  UnkFC;
		//	UCHAR  Unk00;
		//	UCHAR  Unk01;
		//	UCHAR  Unk1E;
		//	UCHAR  NumPlayers;
		//	UCHAR  MaxPlayers;
		//	UCHAR  Unk00_01;
		//	UCHAR  Unk02;
		//	UCHAR  Unk00_02;
		//	UCHAR  Unk00_03;
		//	UCHAR  Unk00_04;
		//	UCHAR  Unk00_05;
		//	UCHAR  Unk00_06;
		//	UCHAR  NameLength;
		//	CHAR   Name[ NameLength ];
		//	UCHAR  VersionLength;
		//	CHAR   Version[ VersionLength ];

		const unsigned char* pkt = (unsigned char*)buf;
		size_t pktlen            = (size_t)rlen;
		const char* sig;

		if (!GetPktData(pkt, pktlen, 4, (const void**)&sig)) {
			fprintf(stderr, "Malformed packet (no signature)\n");
			break;
		}

		if (memcmp(sig, "BNXR", 4)) {
			fprintf(stderr, "Malformed packet (bad signature)\n");
			break;
		}

		if (!GetPktData(pkt, pktlen, 6, 0)) {
			fprintf(stderr, "Malformed packet (no port/other unknowns)\n");
			break;
		}

		if (!GetPktField(pkt, pktlen, users)) {
			fprintf(stderr, "Malformed packet (no user count)\n");
			break;
		}

		if (!GetPktField(pkt, pktlen, maxusers)) {
			fprintf(stderr, "Malformed packet (no max user count)\n");
			break;
		}

		break;
	}

	printf("users:%lu maxusers:%lu ping:%lu\n", (unsigned long)users, (unsigned long)maxusers,
	       elapsed);

	return status;
}
