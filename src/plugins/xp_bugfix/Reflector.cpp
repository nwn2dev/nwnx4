#include <winsock2.h>
#include <windows.h>
#include <stdlib.h>
#include <stddef.h>
#include <vector>
#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>
#include "MD5.h"
#include "NetLayer.h"
#include "Reflector.h"

#include <pshpack1.h>
typedef struct _ENCAP_HEADER {
	UINT8 Magic; // ENCAP_MAGIC
	UINT8 Flags;
	UINT16 FromPort;
	UINT32 FromIP;
	UINT32 Hash[4];
	UINT16 LocalPort;
	UINT16 Reserved;
} ENCAP_HEADER, * PENCAP_HEADER;
#include <poppack.h>

C_ASSERT( sizeof( ENCAP_HEADER) == 0x1C );

const UINT8 ENCAP_FLAG_DATA                = 0x01;
const UINT8 ENCAP_FLAG_DEFAULT_GW          = 0x02;
const UINT8 ENCAP_FLAG_OVERRIDE_LOCAL_PORT = 0x04;
const UINT8 ENCAP_FLAG_SHUTDOWN            = 0x08;
const UINT8 ENCAP_FLAG_HIGH_LOAD           = 0x10;

const UINT32 GAME_FRAME_DATA_SIZE = 0x3C0;
const UINT32 CONNECTIONS_MAX = 0x400;

const UINT32 ALIVENESS_PERIOD = 30000; // Milliseconds before recycling table entry
const UINT32 EXPIRE_PERIOD = 1000;     // Check for expired connections every X milliseconds
const UINT32 DEFAULT_GATEWAY_TIMEOUT = 8000; // Time to unsticky a dead default gateway

typedef struct _CONNECTION_ENTRY {

	bool operator<(__in const _CONNECTION_ENTRY & other) const
	{
		if (other.IPAddress == IPAddress)
		{
			return Port < other.Port;
		}
		else
		{
			return IPAddress < other.IPAddress;
		}
	}

	bool operator==(__in const _CONNECTION_ENTRY & other) const
	{
		return IPAddress == other.IPAddress && Port == other.Port;
	}

	UINT32 IPAddress;    // Client IPv4
	UINT16 Port;         // Client port
	UINT32 GatewayPort;  // Encapsulation frontend port to forward to (preferred)
	UINT32 Gateway;      // Encapsulation frontend to forward to (preferred)
	UINT32 ActivityTick; // When entry was last sent or received on
	UINT32 RecvTick;     // When entry last received successfully
} CONNECTION_ENTRY, * PCONNECTION_ENTRY;

typedef struct _GATEWAY_INFO {
	UINT32 ClientIP;
	UINT16 ClientPort;
	UINT16 GatewayPort;
	UINT32 GatewayIP;
	UINT32 PlayerId;
} GATEWAY_INFO, * PGATEWAY_INFO;

#define GATEWAY_FLAG_DEFAULT   0x0001
#define GATEWAY_FLAG_SHUTDOWN  0x0002
#define GATEWAY_FLAG_HIGH_LOAD 0x0004


#define GATEWAY_FLAG_FROM_PKT  (GATEWAY_FLAG_SHUTDOWN | \
                                GATEWAY_FLAG_HIGH_LOAD)

typedef struct _GATEWAY_ADDRESS {
	UINT32 GatewayIP;
	UINT16 GatewayPort;
	UINT16 GatewayFlags;
	UINT16 GatewayLocalPort;
	UINT16 Reserved;
	UINT32 ActivityTick;
} GATEWAY_ADDRESS, * PGATEWAY_ADDRESS;

volatile UINT32 ConnectionTableGuard0;
CONNECTION_ENTRY ConnectionTable[CONNECTIONS_MAX];
volatile UINT32 ConnectionTableGuard1;
UINT32 ConnectionTableSize;
UINT32 DefaultGateway;
UINT16 DefaultGatewayPort;
UINT32 DefaultGatewayTick;
UINT32 ExpireTick;
CHAR TargetSecret[1024];
UINT32 TargetSecretLen;

typedef std::vector< GATEWAY_ADDRESS > GatewayVec;
GatewayVec GatewayList;

//
// Debug counters.
//

volatile UINT32 ReflectorBadEncapPackets = 0;
volatile UINT32 ReflectorBadChecksums = 0;
volatile UINT32 ReflectorBadSendPackets = 0;
volatile UINT32 ReflectorNoRouteToHost = 0;
volatile UINT32 ReflectorDefaultGatewayTimeouts = 0;
volatile UINT32 ReflectorConnectionTableFull = 0;
volatile UINT32 ReflectorMasterServerRequestsDropped = 0;

VOID
ReflectorSetTargetSecret(
	__in CONST CHAR * TargetSecretIn
	)
{
	StringCbCopyA( TargetSecret, sizeof( TargetSecret ), TargetSecretIn );
	TargetSecretLen = (UINT32) strlen( TargetSecret );

	return;
}

VOID
VerifyConnectionTable(
	VOID
	)
{
#if 0
	CONNECTION_ENTRY Entry;
	UINT32 Index;

	if (ConnectionTableSize > CONNECTIONS_MAX)
	{
		OutputDebugStringA( "ConnectionTableSize overflow!\n" );
		__debugbreak( );
	}

	if (ConnectionTableGuard0 != 0)
	{
		OutputDebugStringA( "ConnectionTableGuard0 corrupted!\n" );
		__debugbreak( );
	}

	if (ConnectionTableGuard1 != 0)
	{
		OutputDebugStringA( "ConnectionTableGuard1 corrupted!\n" );
		__debugbreak( );
	}

	ZeroMemory( &Entry, sizeof( CONNECTION_ENTRY ) );

	for (Index = 0; Index < ConnectionTableSize; Index += 1)
	{
		Entry = ConnectionTable[ Index ];
		if (Index != 0)
		{
			if (Entry == ConnectionTable[ Index - 1 ])
			{
				OutputDebugStringA( "ConnectionTable duplicate entries\n" );
				__debugbreak( );
			}
			else if (Entry < ConnectionTable[ Index - 1 ])
			{
				OutputDebugStringA( "ConnectionTable not sorted in ascending order\n" );
				__debugbreak( );
			}
		}
	}
#endif
}

PCONNECTION_ENTRY
LocateConnection(
	__in CONNECTION_ENTRY & Entry,
	__in BOOLEAN AllowCreate,
	__in BOOLEAN SetGateway
	)
{
	INT32 High;
	INT32 Low;
	INT32 Middle;

	//
	// Locate (and create if desired) a connection entry.  The connection entry
	// table is a binary sorted routing table used to find the gateway information
	// to transmit to for a given remote endpoint.
	//

	Low = 0;
	High = (INT32) (ConnectionTableSize - 1);

	while (High >= Low)
	{
		Middle = (Low + High) >> 1;
		if (Entry < ConnectionTable[ Middle ])
		{
			if (Middle == 0)
			{
				//
				// Create new entry and return entry.
				//

				break;
			}

			High = Middle - 1;
		}
		else if (!(Entry == ConnectionTable[ Middle ])) // If greater than
		{
			Low = Middle + 1;
		}
		else
		{
			//
			// Found an entry - update the gateway and activity tick, and
			// return it to the caller.
			//

			if (SetGateway != FALSE)
			{
				ConnectionTable[ Middle ].Gateway = Entry.Gateway;
				ConnectionTable[ Middle ].GatewayPort = Entry.GatewayPort;
				ConnectionTable[ Middle ].RecvTick = Entry.RecvTick;
			}

			ConnectionTable[ Middle ].ActivityTick = Entry.ActivityTick;
			return &ConnectionTable[ Middle ];
		}
	}

	if (AllowCreate == FALSE)
	{
		return NULL;
	}

	if (High < Low)
	{
		Low = 0;
		High = (INT32) (ConnectionTableSize - 1);

		if (ConnectionTableSize == 0)
		{
			ConnectionTable[ 0 ] = Entry;
			ConnectionTableSize += 1;

			VerifyConnectionTable( );

			return &ConnectionTable[ 0 ];
		}
	}

	for (Middle = max( 0, Low ); Middle < High + 1; Middle += 1)
	{
		if (!(ConnectionTable[ Middle ] < Entry) && !(ConnectionTable[ Middle ] == Entry))
		{
			break;
		}
	}

	//
	// Can't create a new entry if the table is full.
	//

	if (ConnectionTableSize == CONNECTIONS_MAX)
	{
		return NULL;
	}

	//
	// Insert a new entry at the desired insertion position.
	//

	memmove( &ConnectionTable[ Middle + 1 ],
		     &ConnectionTable[ Middle ],
			 (ConnectionTableSize - Middle) * sizeof( CONNECTION_ENTRY ) );
	ConnectionTableSize += 1;
	ConnectionTable[ Middle ] = Entry;

	VerifyConnectionTable( );

	return &ConnectionTable[ Middle ];
}


BOOL
__stdcall
GetGatewayByPlayer(
	__in CONST CHAR * PlayerName,
	__out PGATEWAY_INFO GwInfo
	)
{
	unsigned long     PlayerId;
	SOCKADDR_IN       Sin;
	PCONNECTION_ENTRY ConnEntry;
	CONNECTION_ENTRY  SearchEntry;

	if (ReflectorIsEnabled( ) == FALSE)
		return FALSE;

	for (PlayerId = 0; PlayerId < MAX_PLAYERS; PlayerId += 1)
	{
		const char * Name = GetPlayerAccountName( PlayerId );
		if (Name == NULL || strcmp( Name, PlayerName ) != 0)
			continue;

		break;
	}

	if (PlayerId == MAX_PLAYERS)
		return FALSE;

	if (GetPlayerConnectionInfo( PlayerId, &Sin ) == FALSE)
		return FALSE;

	ZeroMemory( &SearchEntry, sizeof( CONNECTION_ENTRY ) );
	SearchEntry.IPAddress = Sin.sin_addr.s_addr;
	SearchEntry.Port      = Sin.sin_port;

	ConnEntry = LocateConnection( SearchEntry, FALSE, FALSE );
	if (ConnEntry == NULL)
		return FALSE;

	GwInfo->ClientIP    = Sin.sin_addr.s_addr;
	GwInfo->ClientPort  = Sin.sin_port;
	GwInfo->GatewayPort = ConnEntry->GatewayPort;
	GwInfo->GatewayIP   = ConnEntry->Gateway;
	GwInfo->PlayerId    = PlayerId;
	return TRUE;
}

VOID
ManageGatewayList(
	__in PSOCKADDR_IN Sin,
	__in PENCAP_HEADER EncapHeader,
	__in UINT32 Now
	)
{
	UINT16 GatewayFlags = 0;

	if ((EncapHeader->Flags & ENCAP_FLAG_SHUTDOWN) != 0)
	{
		GatewayFlags |= GATEWAY_FLAG_SHUTDOWN;
	}

	if ((EncapHeader->Flags & ENCAP_FLAG_HIGH_LOAD) != 0)
	{
		GatewayFlags |= GATEWAY_FLAG_HIGH_LOAD;
	}

	try
	{
		for (GatewayVec::iterator it = GatewayList.begin( );
			 it != GatewayList.end( );
			 )
		 {
			//
			// Interested only in the current gateway.
			//

			if ((Sin->sin_addr.s_addr != it->GatewayIP) ||
			    (Sin->sin_port != it->GatewayPort))
			{
				++it;
				continue;
			}

			//
			// Record activity and continue.
			//

			it->ActivityTick = Now;
			it->GatewayFlags &= ~(GATEWAY_FLAG_FROM_PKT);
			it->GatewayFlags |= GatewayFlags;

			return;
		}

		//
		// This gateway doesn't exist yet in the tracking list; create a new
		// entry to describe it.
		//

		GATEWAY_ADDRESS GwInfo;

		GwInfo.GatewayIP = Sin->sin_addr.s_addr;
		GwInfo.GatewayPort = Sin->sin_port;
		GwInfo.GatewayFlags = GatewayFlags;
		GwInfo.GatewayLocalPort = EncapHeader->LocalPort;
		GwInfo.Reserved = 0;
		GwInfo.ActivityTick = Now;

		GatewayList.push_back( GwInfo );
		return;
	}
	catch (std::bad_alloc)
	{
		return;
	}
}

UINT32
__stdcall
EnumGateways(
	__in UINT32 GatewayCount,
	__out_ecount( GatewayCount ) PGATEWAY_ADDRESS GatewayAddresses
	)
{
#if 0
	PCONNECTION_ENTRY ConnEntry;
	UINT32 ConnIndex;
	UINT32 GwIndex;
#endif

	UINT32 GwWritten;

	GwWritten = 0;
	if (GwWritten == GatewayCount)
	{
		return GwWritten;
	}

	for (GatewayVec::const_iterator it = GatewayList.begin( );
	     it != GatewayList.end( );
	     ++it)
	{
		GatewayAddresses[ GwWritten ] = *it;

		if ((it->GatewayIP == DefaultGateway) &&
		    (it->GatewayPort == DefaultGatewayPort))
		{
			GatewayAddresses[ GwWritten ].GatewayFlags |= GATEWAY_FLAG_DEFAULT;
		}

		GwWritten += 1;

		if (GwWritten == GatewayCount)
		{
			return GwWritten;
		}
	}

#if 0
	//
	// Enumerate all connections to find the list of known gateways and return
	// them to the caller.
	//

	for (ConnIndex = 0; ConnIndex < ConnectionTableSize; ConnIndex += 1)
	{
		ConnEntry = &ConnectionTable[ ConnIndex ];

		//
		// Has this gateway already been recorded?
		//

		for (GwIndex = 0; GwIndex < GwWritten; GwIndex += 1)
		{
			if ((GatewayAddresses[ GwIndex ].GatewayIP == ConnEntry->Gateway) &&
			    (GatewayAddresses[ GwIndex ].GatewayPort == ConnEntry->GatewayPort))
			{
				break;
			}
		}

		//
		// If the gateway hasn't been recorded yet, then return it now to the
		// caller.
		//

		if (GwIndex == GwWritten)
		{
			if (GwWritten >= GatewayCount)
			{
				return GwWritten;
			}

			GatewayAddresses[ GwWritten ].GatewayIP = ConnEntry->Gateway;
			GatewayAddresses[ GwWritten ].GatewayPort = ConnEntry->GatewayPort;
			GatewayAddresses[ GwWritten ].GatewayFlags = 0;

			if ((ConnEntry->Gateway == DefaultGateway) &&
			    (ConnEntry->GatewayPort == DefaultGatewayPort))
			{
				GatewayAddresses[ GwWritten ].GatewayFlags |= GATEWAY_FLAG_DEFAULT;
			}

			GwWritten += 1;
		}
	}
#endif

	return GwWritten;
}

VOID
ExpireConnections(
	VOID
	)
{
	UINT32 Index;
	UINT32 Now;
	//
	// Age out any entries that haven't been sent or received on lately.  These
	// will go to a default gateway for outbound or will be reconstituted as
	// desired on next inbound traffic submission.
	//

	Now = GetTickCount( );

	if (Now - ExpireTick >= EXPIRE_PERIOD)
	{

		//
		// Remove any connection entries that are already expired.
		//

		if (ConnectionTableSize != 0)
		{
			for (Index = ConnectionTableSize - 1; Index != (UINT32) (0 - 1); Index -= 1)
			{
				if (Now - ConnectionTable[ Index ].ActivityTick >= ALIVENESS_PERIOD)
				{
					memmove( &ConnectionTable[ Index ],
							 &ConnectionTable[ Index + 1 ],
							 (ConnectionTableSize - (Index + 1) ) * sizeof( CONNECTION_ENTRY ) );
					ConnectionTableSize -= 1;
					ZeroMemory( &ConnectionTable[ ConnectionTableSize ], sizeof( CONNECTION_ENTRY ) );
					VerifyConnectionTable( );
				}
			}
		}

		for (GatewayVec::iterator it = GatewayList.begin( );
		     it != GatewayList.end( );
		     )
		{
			if (Now - it->ActivityTick >= DEFAULT_GATEWAY_TIMEOUT)
			{
				it = GatewayList.erase( it );
			}
			else
			{
				++it;
			}
		}

		ExpireTick = Now;
	}

	return;
}

int __stdcall ReflectorSendto(__in SOCKET s, __in const char *buf, __in int len, __in int flags, __in struct sockaddr_in *to, __in int tolen)
{
	swutil::MD5_CTX Ctx;
	UINT8 DataBuf[ FRAME_DATA_SIZE ];
	PENCAP_HEADER EncapHeader;
	PCONNECTION_ENTRY Entry;
	CONNECTION_ENTRY SearchEntry;
	PSOCKADDR_IN Sin;
	UINT32 Now;
	BOOLEAN TryDefaultGateway;
	INT32 Slen;

	if ((UINT32)len > GAME_FRAME_DATA_SIZE)
	{
		ReflectorBadSendPackets += 1;

		ExpireConnections( );

		return len;
	}

	Sin = (PSOCKADDR_IN)to;
	Now = GetTickCount( );

	//
	// Look up an existing connection entry in the connection table so that the
	// traffic is routed to the proper outbound reflector.  Do not set a new
	// gateway for the send lookup and drop the message if there is no default
	// gateway established at all.
	//

	SearchEntry.IPAddress = Sin->sin_addr.s_addr;
	SearchEntry.Port = Sin->sin_port;
	SearchEntry.GatewayPort = DefaultGatewayPort;
	SearchEntry.Gateway = DefaultGateway;
	SearchEntry.ActivityTick = Now;
	SearchEntry.RecvTick = Now;

	Entry = LocateConnection( SearchEntry, TRUE, FALSE );
	if ((Entry == NULL) || (Entry->Gateway == INADDR_NONE))
	{
		//
		// No route to send, drop the message.
		//

		ReflectorNoRouteToHost += 1;

		ExpireConnections( );

		return len;
	}

	//
	// If traffic has not been received from the original gateway beyond the
	// gateway timeout, and there is a new default gateway, replicate a copy of
	// outbound traffic via the new default gateway too (if there is a valid
	// path via it, traffic will cut over once the client sends a packet through
	// the new default gateway).
	//

	if ((Now - Entry->RecvTick >= DEFAULT_GATEWAY_TIMEOUT) &&
		((Entry->Gateway != DefaultGateway) || (Entry->GatewayPort != DefaultGatewayPort)))
	{
		TryDefaultGateway = TRUE;
	}
	else
	{
		TryDefaultGateway = FALSE;
	}

	//
	// Build reflector encapsulated message.
	//

	EncapHeader = (PENCAP_HEADER)&DataBuf[ 0 ];

	RtlCopyMemory( EncapHeader + 1, buf, (size_t)len );
	EncapHeader->FromIP = Sin->sin_addr.s_addr;
	EncapHeader->FromPort = Sin->sin_port;
	EncapHeader->Magic = ENCAP_MAGIC;
	EncapHeader->Flags = ENCAP_FLAG_DATA;
	RtlZeroMemory( &EncapHeader->Hash, sizeof( EncapHeader->Hash ) );

	//
	// Calculate a hash except for the local case - which, to further optimize
	// CPU resources during potential attack scenarios (if a reflector is
	// local), is allowed to skip a checksum.
	//
	// N.B.  The networking stack is assumed to disallow remote loopback
	//       address traffic.
	//

	if (Entry->Gateway != INADDR_LOOPBACK_NET)
	{
		swutil::MD5Init( &Ctx );
		swutil::MD5Update( &Ctx, &len, sizeof( len ) );
		swutil::MD5Update( &Ctx, TargetSecret, TargetSecretLen );
		swutil::MD5Update( &Ctx, EncapHeader, (size_t)len + sizeof( ENCAP_HEADER ) );
		swutil::MD5Final( (unsigned char *)&EncapHeader->Hash, &Ctx );
	}

	Sin->sin_addr.s_addr = Entry->Gateway;
	Sin->sin_port = Entry->GatewayPort;

	//
	// Expire old connection route entries and transmit encapsulated data to
	// reflector server.  Attempt to route around a failed default gateway if
	// traffic from the old gateway on this link hasn't been received for some
	// time by transmitting a copy via the new default gateway.
	//

	ExpireConnections( );

	Slen = sendto( s, (char *) DataBuf, len + sizeof( ENCAP_HEADER ), flags, (PSOCKADDR)Sin, tolen );

	if (TryDefaultGateway != FALSE)
	{
		Sin->sin_addr.s_addr = DefaultGateway;
		Sin->sin_port = DefaultGatewayPort;
		sendto( s, (char *) DataBuf, len + sizeof( ENCAP_HEADER ), flags, (PSOCKADDR)Sin, tolen );
	}

	return Slen;
}

int __stdcall ReflectorRecvfrom(__in SOCKET s, __out char *buf, __in int len, __in int flags, __out struct sockaddr *from, __inout_opt int *fromlen)
{
	swutil::MD5_CTX Ctx;
	PENCAP_HEADER EncapHeader;
	UINT32 Hash[ 4 ];
	PSOCKADDR_IN Sin;
	PCONNECTION_ENTRY Entry;
	CONNECTION_ENTRY SearchEntry;
	UINT32 Now;

	//
	// Encapsulated message from reflector server - verify checksum and create
	// connection entry for routing, then de-encapsulate and indicate to the
	// game server for processing.
	//

	if ((UINT32)len < sizeof( ENCAP_HEADER ) || (UINT32)len > FRAME_DATA_SIZE)
	{
		ReflectorBadEncapPackets += 1;

		ExpireConnections( );
		WSASetLastError( WSAECONNRESET );
		return -1;
	}

	EncapHeader = (PENCAP_HEADER) buf;
	Sin = (PSOCKADDR_IN) from;

	memcpy( &Hash[ 0 ], &EncapHeader->Hash, sizeof( Hash ) );
	ZeroMemory( &EncapHeader->Hash, sizeof( EncapHeader->Hash ) );

	len = len - sizeof( ENCAP_HEADER );

	//
	// Calculate a hash except for the local case - which, to further optimize
	// CPU resources during potential attack scenarios (if a reflector is
	// local), is allowed to skip a checksum.
	//
	// N.B.  The networking stack is assumed to disallow remote loopback
	//       address traffic.
	//

	if (Sin->sin_addr.s_addr != INADDR_LOOPBACK_NET)
	{
		swutil::MD5Init( &Ctx );
		swutil::MD5Update( &Ctx, &len, sizeof( len ) );
		swutil::MD5Update( &Ctx, TargetSecret, TargetSecretLen );
		swutil::MD5Update( &Ctx, EncapHeader, len + sizeof( ENCAP_HEADER ) );
		swutil::MD5Final( (unsigned char *) &EncapHeader->Hash, &Ctx );

		if (RtlEqualMemory( &Hash, &EncapHeader->Hash, sizeof( Hash ) ) == FALSE)
		{
			ReflectorBadChecksums += 1;

			ExpireConnections( );
			WSASetLastError( WSAECONNRESET );
			return -1;
		}
	}

	//
	// Some load balancers may not provide a consistent local port for outbound
	// traffic.  A gateway can request that a specific return port be used; if
	// so, pick it up.
	//

	if ((EncapHeader->Flags & ENCAP_FLAG_OVERRIDE_LOCAL_PORT) != 0)
	{
		Sin->sin_port = EncapHeader->LocalPort;
	}

	Now = GetTickCount( );

	//
	// Handle default gateway assignment or reassignment.
	//
	// Leave the default gateway sticky until the default gateway goes down.
	// If the default gateway goes away, allow reassignment to any advertised
	// default gateway.
	//
	// N.B.  As gateways send regular heartbeats, don't bother scanning the
	//       gateway list if a gateway falls off for a default reselection.
	//

	if ((EncapHeader->Flags & ENCAP_FLAG_DEFAULT_GW) != 0)
	{
		if ((DefaultGateway == Sin->sin_addr.s_addr) &&
			(DefaultGatewayPort == Sin->sin_port))
		{
			DefaultGatewayTick = Now;
		}
		else if (Now - DefaultGatewayTick >= DEFAULT_GATEWAY_TIMEOUT)
		{
			DefaultGatewayTick = Now;
			DefaultGateway = Sin->sin_addr.s_addr;
			DefaultGatewayPort = Sin->sin_port;
			ReflectorDefaultGatewayTimeouts += 1;
		}
		else if (DefaultGateway == INADDR_NONE)
		{
			DefaultGatewayTick = Now;
			DefaultGateway = Sin->sin_addr.s_addr;
			DefaultGatewayPort = Sin->sin_port;
		}
	}

	//
	// Manage the list of known gateways.
	//

	ManageGatewayList( Sin, EncapHeader, Now );

	//
	// Do not create connection entries for aliveness notification messages.
	//
	// These have no data payload for the game server proper - they only serve
	// to establish default gateway parameters.
	//

	if ((EncapHeader->Flags & ENCAP_FLAG_DATA) == 0)
	{
		ExpireConnections( );
		WSASetLastError( WSAECONNRESET );
		return -1;
	}
	else if (((EncapHeader->Flags & ENCAP_FLAG_DEFAULT_GW) == 0) &&
	         (len == 6) &&
	         (memcmp( EncapHeader + 1, "BNXI", 4 ) == 0) &&
	         (*(PUINT16) ((PUINT8) ( EncapHeader + 1 ) + 4) == MASTER_SERVER_PORT))
	{
		//
		// Don't advertise to the master server through non-default gateways.
		// This would create multiple entries for the game server on the public
		// server list, which would not only pollute the server list, but the
		// non-default gateways are likely to be private and don't want public
		// listing as well.
		//
		// The master server first sends a BNXI request claiming its local port
		// is the master server port before listing a server.  Drop any such
		// inbound requests received through a non-default gateway such that
		// the master server never detects the presence of a game server at
		// such a reflector endpoint.
		//

		ReflectorMasterServerRequestsDropped += 1;

		ExpireConnections( );
		WSASetLastError( WSAECONNRESET );
		return -1;
	}

	//
	// Create a connection entry for routing back to the client (or find an
	// existing one), such that the client prefers to use the same gateway as
	// the traffic was originally sourced from.
	//

	SearchEntry.IPAddress = EncapHeader->FromIP;
	SearchEntry.Port = EncapHeader->FromPort;
	SearchEntry.GatewayPort = Sin->sin_port;
	SearchEntry.Gateway = Sin->sin_addr.s_addr;
	SearchEntry.ActivityTick = Now;
	SearchEntry.RecvTick = Now;

	Entry = LocateConnection( SearchEntry, TRUE, TRUE );
	if (Entry == NULL)
	{
		//
		// No space for new connections, drop the message.
		//

		ReflectorConnectionTableFull += 1;

		ExpireConnections( );
		WSASetLastError( WSAECONNRESET );
		return -1;
	}

	Sin->sin_addr.s_addr = EncapHeader->FromIP;
	Sin->sin_port = EncapHeader->FromPort;

	memmove( EncapHeader, EncapHeader + 1, (size_t) len );

	ExpireConnections( );

	return len;
}

BOOLEAN
ReflectorIsEnabled(
	VOID
	)
{
	return TargetSecret[0] != 0;
}
