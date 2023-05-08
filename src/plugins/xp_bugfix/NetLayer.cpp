/***************************************************************************
    NetLayer - Wrapper for AuroraServerNetLayer.dll and hooks into NWN2 to
	use it.

    Copyright (C) 2009 Skywing (skywing@valhallalegends.com).  This instance
    of NetLayerWindow is licensed under the GPLv2 for the usage of the NWNX4
    project, nonwithstanding other licenses granted by the copyright holder.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

***************************************************************************/

#include "bugfix.h"
#include "..\..\misc\Patch.h"
#include "ServerNetLayer.h"
#include <vector>
#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>
#include "NetLayer.h"
#include "Reflector.h"

extern std::unique_ptr<LogNWNX> logger;

#define RECV_RECONNECT_TIMEOUT 10000

extern long GameObjUpdateBurstSize;
extern CHAR NWNXHome[ MAX_PATH + 1 ];

bool ReplaceNetLayer();
bool EnableTls();
void SetDebugInfoPermission(int pDebugPermission);

CNetLayerInternal * NetLayerInternal;
bool                TlsActive;
bool                WindowExtensions;
WCHAR               TlsCert[ MAX_PATH + 1 ];
int	    			DebugInfoPermission;		


HMODULE AuroraServerNetLayer;
HMODULE AuroraServerMsgCheck;

struct PlayerStateInfo
{
	bool  AreaLoadPending;
	bool  BlockGameObjUpdate;
	bool  DropMessages;
	bool  LoginConfirmed;
	bool  InGameWorld;
	bool  TlsNegotiated; // +14 bytes into BNCS - ApplicationId - check at HandleBNCSMessage and set this accordingly
	bool  TlsActive;
	bool  BNVSHandled;
	ULONG RecvTick;
};

NETLAYER_HANDLE Connections[MAX_PLAYERS];
PlayerStateInfo PlayerState[MAX_PLAYERS];

typedef
void
(__stdcall * OnPlayerConnectionCloseProc)(
	__in unsigned long PlayerId,
	__in void * Context
	);

typedef
BOOL
(__stdcall * OnPlayerConnectionReceiveProc)(
	__in unsigned long PlayerId,
	__in_bcount( Length ) const unsigned char * Data,
	__in size_t Length,
	__in void * Context
	);

typedef
BOOL
(__stdcall * OnPlayerConnectionSendProc)(
	__in unsigned long PlayerId,
	__in_bcount( Size ) unsigned char * Data,
	__in unsigned long Size,
	__in unsigned long Flags,
	__in void * Context
	);

typedef
BOOL
(__stdcall * ValidateReceiveProtocolMessageProc)(
	__in unsigned long PlayerId,
	__in_bcount( Length ) const unsigned char * Data,
	__in size_t Length
	);

typedef
BOOL
(__stdcall * ValidateReceiveDatagramProc)(
	__in_bcount( Length ) const unsigned char * Data,
	__in size_t Length
	);

typedef struct _PLAYER_CONNECTION_CALLBACKS {
	OnPlayerConnectionCloseProc     OnClose;
	OnPlayerConnectionReceiveProc   OnReceive;
	OnPlayerConnectionSendProc      OnSend;
	void                          * Context;
} PLAYER_CONNECTION_CALLBACKS, * PPLAYER_CONNECTION_CALLBACKS;

typedef std::vector< PLAYER_CONNECTION_CALLBACKS > ConnectionCallbackVec;

//
// Nonstandard extensions for ApplicationId.
//

const ULONG APPLICATIONID_NEGOTIATE      = 0xFFFFFF00;
const ULONG APPLICATIONID_NEGOTIATE_MASK = 0xFFFFFF00;

//
// TLS is requested (or confirmed for server response).
//

const ULONG APPLICATIONID_TLS            = 0x00000001;



AuroraServerNetLayerCreateProc     AuroraServerNetLayerCreate_;
AuroraServerNetLayerSendProc       AuroraServerNetLayerSend_;
AuroraServerNetLayerReceiveProc    AuroraServerNetLayerReceive_;
AuroraServerNetLayerTimeoutProc    AuroraServerNetLayerTimeout_;
AuroraServerNetLayerDestroyProc    AuroraServerNetLayerDestroy_;
AuroraServerNetLayerQueryProc      AuroraServerNetLayerQuery_;

//
// Optional TLS callouts.

AuroraServerNetLayerCreateTlsClientProc AuroraServerNetLayerCreateTlsClient_;
AuroraServerNetLayerCreateTlsServerProc AuroraServerNetLayerCreateTlsServer_;
AuroraServerNetLayerSendExProc          AuroraServerNetLayerSendEx_;

//
// Packet filtering callouts.
//

ConnectionCallbackVec              ConnectionCallbacks;

ValidateReceiveProtocolMessageProc ValidateReceiveProtocolMessage;
ValidateReceiveDatagramProc        ValidateReceiveDatagram;

//
// Tracking of last send and receive.
//

ULONG                              NetLayerLastSendTick;
ULONG                              NetLayerLastRecvTick;
ULONG                              NetLayerLastTimeoutTick;

/***************************************************************************
    Debug output to the debugger before we can use wx logging safely.
***************************************************************************/

void
DebugPrintV(
	__in const char *Format,
	__in va_list Ap
	)
{
	CHAR Message[ 4096 ];

	//
	// Let's not clutter up things if a user mode debugger isn't present.
	//

	if (!IsDebuggerPresent())
		return;

	StringCbVPrintfA(
		Message,
		sizeof( Message ),
		Format,
		Ap
		);

	OutputDebugStringA( Message );
}

void
DebugPrint(
	__in const char *Format,
	...
	)
{
	va_list Ap;

	va_start( Ap, Format );

	DebugPrintV( Format, Ap );

	va_end( Ap );
}

void
CallOnPlayerConnectionClose(
	__in unsigned long PlayerId
	)
{
	for (ConnectionCallbackVec::const_iterator it = ConnectionCallbacks.begin();
	     it != ConnectionCallbacks.end();
	     ++it)
	{
		if (!it->OnClose)
			continue;

		it->OnClose( PlayerId, it->Context );
	}

	return;
}

BOOL
CallOnPlayerConnectionReceive(
	__in unsigned long PlayerId,
	__in_bcount( Length ) const unsigned char * Data,
	__in size_t Length
	)
{
	for (ConnectionCallbackVec::const_iterator it = ConnectionCallbacks.begin();
	     it != ConnectionCallbacks.end();
	     ++it)
	{
		if (!it->OnReceive)
			continue;

		if (it->OnReceive(
			PlayerId,
			Data,
			Length,
			it->Context) == FALSE)
		{
			return FALSE;
		}
	}

	return TRUE;
}

BOOL
CallOnPlayerConnectionSend(
	__in unsigned long PlayerId,
	__in_bcount( Length ) unsigned char * Data,
	__in unsigned long Size,
	__in unsigned long Flags
	)
{
	for (ConnectionCallbackVec::const_iterator it = ConnectionCallbacks.begin();
	     it != ConnectionCallbacks.end();
	     ++it)
	{
		if (!it->OnSend)
			continue;

		if (it->OnSend(
			PlayerId,
			Data,
			Size,
			Flags,
			it->Context) == FALSE)
		{
			return FALSE;
		}
	}

	return TRUE;
}


void
__stdcall
SetPacketFilterCallouts(
	__in void * Context,
	__in OnPlayerConnectionCloseProc OnClose,
	__in OnPlayerConnectionReceiveProc OnReceive,
	__in OnPlayerConnectionSendProc OnSend
	)
{
	PLAYER_CONNECTION_CALLBACKS Callbacks;

	Callbacks.Context   = Context;
	Callbacks.OnClose   = OnClose;
	Callbacks.OnReceive = OnReceive;
	Callbacks.OnSend    = OnSend;

	ConnectionCallbacks.push_back( Callbacks );
}

SOCKET
GetServerNetLayerSocket(
	)
{
	CExoNetInternal * NetI;

	if (NetLayerInternal == NULL)
		return INVALID_SOCKET;

	if (NetLayerInternal->Net == NULL)
		return INVALID_SOCKET;

	NetI = NetLayerInternal->Net->Internal;

	if (NetI == NULL)
		return INVALID_SOCKET;

	return NetI->Socket;
}

struct CNetLayerInternal *
GetServerNetLayer(
	)
{
	return NetLayerInternal;
}

const char *
__stdcall
GetPlayerAccountName(
	__in unsigned long PlayerId
	)
{
	if (PlayerId > MAX_PLAYERS)
		return NULL;

	if (!NetLayerInternal->Players[PlayerId].m_bPlayerInUse)
		return NULL;

	if (NetLayerInternal->Players[PlayerId].m_sPlayerName.m_sString == NULL)
		return "";
	else
		return NetLayerInternal->Players[PlayerId].m_sPlayerName.m_sString;
}


bool PlayerIdToConnectionId(unsigned long PlayerId, unsigned long *ConnectionId)
{
	unsigned long idx;

	if (PlayerId > MAX_PLAYERS)
		return false;

	if (!NetLayerInternal->Players[PlayerId].m_bPlayerInUse)
		return false;

	idx = NetLayerInternal->Players[PlayerId].m_nSlidingWindowId;

	if (!NetLayerInternal->Windows[idx].m_WindowInUse)
		return false;

	*ConnectionId = NetLayerInternal->Windows[idx].m_ReceiveConnectionId;

	return true;
}

bool PlayerIdToSlidingWindow(unsigned long PlayerId, unsigned long *SlidingWindowId)
{
	if (PlayerId > MAX_PLAYERS)
		return false;

	for (unsigned long i = 0; i < MAX_PLAYERS; i += 1)
	{
		if (NetLayerInternal->Windows[i].m_WindowInUse != 1)
			continue;

		if (NetLayerInternal->Windows[i].m_PlayerId == PlayerId)
		{
			*SlidingWindowId = i;
			return true;
		}
	}

	return false;
}

bool
__stdcall
GetPlayerConnectionInfo(
	__in unsigned long PlayerId,
	__out PSOCKADDR_IN Sin
	)
{
	SlidingWindow   * Winfo;
	CExoNetInternal * NetI;

	if (PlayerId > MAX_PLAYERS || NetLayerInternal == NULL)
		return false;

	Winfo = NULL;
	for (unsigned long i = 0; i < MAX_PLAYERS; i += 1)
	{
		if (NetLayerInternal->Windows[i].m_WindowInUse != 1)
			continue;

		if (NetLayerInternal->Windows[i].m_PlayerId == PlayerId)
		{
			Winfo = &NetLayerInternal->Windows[i];
			break;
		}
	}

	if (Winfo == NULL)
		return false;

	NetI = NetLayerInternal->Net->Internal;
	if (Winfo->m_ConnectionId >= NetI->NumConnections)
	{
		if (IsDebuggerPresent( ))
			__debugbreak( );

		return false;
	}

	ZeroMemory( Sin, sizeof( *Sin ) );

	Sin->sin_family      = AF_INET;
	Sin->sin_port        = htons( NetI->ConnectionAddresses[ Winfo->m_ConnectionId ].Port );
	Sin->sin_addr.s_addr = NetI->ConnectionAddresses[ Winfo->m_ConnectionId ].Address;

	return true;
}

/*

0:000> u 004ff8a0 
nwn2server!CNetLayerInternal::PlayerIdToConnectionId:
004ff8a0 8b442404        mov     eax,dword ptr [esp+4]
004ff8a4 8bd0            mov     edx,eax
004ff8a6 c1e204          shl     edx,4
004ff8a9 2bd0            sub     edx,eax
004ff8ab 83bcd19076030000 cmp     dword ptr [ecx+edx*8+37690h],0
004ff8b3 8d04d1          lea     eax,[ecx+edx*8]
004ff8b6 7425            je      nwn2server!CNetLayerInternal::PlayerIdToConnectionId+0x3d (004ff8dd)
004ff8b8 8b80a0760300    mov     eax,dword ptr [eax+376A0h]
0:000> u
nwn2server!CNetLayerInternal::PlayerIdToConnectionId+0x1e:
004ff8be 69c03c090000    imul    eax,eax,93Ch
004ff8c4 03c1            add     eax,ecx
004ff8c6 83781400        cmp     dword ptr [eax+14h],0
004ff8ca 7411            je      nwn2server!CNetLayerInternal::PlayerIdToConnectionId+0x3d (004ff8dd)
004ff8cc 8b401c          mov     eax,dword ptr [eax+1Ch]
004ff8cf 8b4c2408        mov     ecx,dword ptr [esp+8]
004ff8d3 8901            mov     dword ptr [ecx],eax
004ff8d5 b801000000      mov     eax,1
0:000> u
nwn2server!CNetLayerInternal::PlayerIdToConnectionId+0x3a:
004ff8da c20800          ret     8
004ff8dd 33c0            xor     eax,eax
004ff8df c20800          ret     8
004ff8e2 cc              int     3
004ff8e3 cc              int     3
004ff8e4 cc              int     3
004ff8e5 cc              int     3
004ff8e6 cc              int     3
*/


BOOL
__stdcall
SendMessageToPlayer(
	__in unsigned long PlayerId,
	__in_bcount( Size ) unsigned char * Data,
	__in unsigned long Size,
	__in unsigned long Flags
	);

BOOL
__stdcall
SendMessageToPlayer2(
	__in unsigned long PlayerId,
	__in_bcount( Size ) unsigned char * Data,
	__in unsigned long Size,
	__in unsigned long Flags
	);

BOOL
__stdcall
FrameTimeout2(
	__in unsigned long Unused
	);

BOOL
__stdcall
FrameReceive2(
	__in_bcount( Size ) unsigned char *Data,
	__in unsigned long Size
	);

void
SetGameObjUpdateSize(
	);

void
InitializePlayerInfoForBNCS(
	);

Patch _patches2[] =
{
	Patch( OFFS_SendMessageToPlayer, "\xe9", 1 ),
	Patch( OFFS_SendMessageToPlayer+1, (relativefunc)SendMessageToPlayer2 ),
	Patch( OFFS_CallFrameTimeout, (relativefunc)FrameTimeout2 ),
	Patch( OFFS_FrameReceive, "\xe9", 1 ),
	Patch( OFFS_FrameReceive+1, (relativefunc)FrameReceive2 ),
	Patch( OFFS_FrameSend, "\xc2\x0c\x00", 3 ), // Disable all outbound sends (from FrameTimeout)
	Patch( OFFS_GameObjUpdateSizeLimit1, "\xe9", 1 ),
	Patch( OFFS_GameObjUpdateSizeLimit1+1, (relativefunc) SetGameObjUpdateSize ),
	Patch( OFFS_CNetLayerInternal_HandlesBNCSMessage_CallInitializePlayerInfo, (relativefunc) InitializePlayerInfoForBNCS ),

	Patch( )
};

Patch *patches2 = _patches2;

bool
__cdecl
OnNetLayerWindowReceive(
	__in_bcount( Length ) const unsigned char * Data,
	__in size_t Length,
	__in void * Context
	);

bool
__cdecl
OnNetLayerWindowSend(
	__in_bcount( Length ) const unsigned char * Data,
	__in size_t Length,
	__in void * Context
	);

bool
__cdecl
OnNetLayerWindowStreamError(
	__in bool Fatal,
	__in unsigned long ErrorCode,
	__in void * Context
	);

void ResetWindow(unsigned long PlayerId)
{
	CONNECTION_CALLBACKS Callbacks;
	NETLAYER_HANDLE      Handle;

	Callbacks.Context       = (void *) (ULONG_PTR) PlayerId;
	Callbacks.OnReceive     = OnNetLayerWindowReceive;
	Callbacks.OnSend        = OnNetLayerWindowSend;
	Callbacks.OnStreamError = OnNetLayerWindowStreamError;

	PlayerState[PlayerId].AreaLoadPending    = true;
	PlayerState[PlayerId].BlockGameObjUpdate = true;
	PlayerState[PlayerId].DropMessages       = false;
	PlayerState[PlayerId].LoginConfirmed     = false;
	PlayerState[PlayerId].InGameWorld        = false;
	PlayerState[PlayerId].TlsActive          = false;
	PlayerState[PlayerId].RecvTick           = GetTickCount( );

	//
	// Note BNVSHandled is deliberately not reset here.  It is cleared when a
	// successful BNCS reply (i.e, BNCR) is sent.
	//

	//
	// Create the window (the first time around), else simply reset its
	// internal state if we have already set it up.  We do not need to allocate
	// a new instance to reset its state and instead pass in the previous
	// instance handle.
	//

	if ((TlsActive == false) || (PlayerState[PlayerId].TlsNegotiated == false))
	{
		Handle = AuroraServerNetLayerCreate_(Connections[PlayerId], &Callbacks);
	}
	else
	{
		AURORA_SERVER_QUERY_SET_WINDOW_EXTENSIONS SetWindowExtensions;

		if (Connections[PlayerId] == NULL)
		{
			Handle = AuroraServerNetLayerCreate_(Connections[PlayerId], &Callbacks);

			if (Handle == NULL)
				return;
		}

		SetWindowExtensions.WindowExtensionsEnabled = (WindowExtensions != false);

		if (!AuroraServerNetLayerQuery_(
			Connections[ PlayerId ],
			AuroraServerQuerySetWindowExtensions,
			sizeof( SetWindowExtensions ),
			NULL,
			&SetWindowExtensions))
		{
			logger->Err("! Failed to configure window extensions.  AuroraServerNetLayer.dll may be out of date.");
		}

		Handle = AuroraServerNetLayerCreateTlsServer_(Connections[PlayerId], &Callbacks, TlsCert);

		if (Handle != NULL)
			PlayerState[PlayerId].TlsActive = true;
	}

	if (Handle == NULL)
		return;

	Connections[PlayerId] = Handle;
}

bool ReplaceNetLayer()
{
	//
	// Wire up the dllimports.
	//

	struct { bool Required; const char *Name; void **Import; } DllImports[] =
	{
		{ true , "AuroraServerNetLayerCreate",          (void **) &AuroraServerNetLayerCreate_          },
		{ true , "AuroraServerNetLayerSend",            (void **) &AuroraServerNetLayerSend_            },
		{ true , "AuroraServerNetLayerReceive",         (void **) &AuroraServerNetLayerReceive_         },
		{ true , "AuroraServerNetLayerTimeout",         (void **) &AuroraServerNetLayerTimeout_         },
		{ true , "AuroraServerNetLayerDestroy",         (void **) &AuroraServerNetLayerDestroy_         },
		{ false, "AuroraServerNetLayerQuery",           (void **) &AuroraServerNetLayerQuery_           },
		{ false, "AuroraServerNetLayerCreateTlsClient", (void **) &AuroraServerNetLayerCreateTlsClient_ },
		{ false, "AuroraServerNetLayerCreateTlsServer", (void **) &AuroraServerNetLayerCreateTlsServer_ },
		{ false, "AuroraServerNetLayerSendEx",          (void **) &AuroraServerNetLayerSendEx_          },
	};
	AuroraServerNetLayer = LoadLibraryA("AuroraServerNetLayer.dll");

	if (!AuroraServerNetLayer)
	{
		logger->Err("* Failed to load AuroraServerNetLayer.dll");
		return false;
	}

	for (int i = 0; i < sizeof(DllImports)/sizeof(DllImports[0]); i += 1)
	{
		*DllImports[i].Import = (void *)GetProcAddress(AuroraServerNetLayer, DllImports[i].Name);

		if (!*DllImports[i].Import)
		{
			if (!DllImports[i].Required)
			{
				logger->Warn(
					"* Warning: You need to update your AuroraServerNetLayer.dll; missing optional entrypoint AuroraServerNetLayer!%s",
					DllImports[i].Name);
				continue;
			}
			logger->Err("* Unable to resolve AuroraServerNetLayer!%s", DllImports[i].Name);
			return false;
		}
	}

	AuroraServerMsgCheck = LoadLibraryA("AuroraServerMsgCheck.dll");
	if (!AuroraServerMsgCheck)
	{
		logger->Err("* Failed to load AuroraServerMsgCheck.dll; is it located in the same directory as AuroraServerNetLayer.dll?");
	}
	else
	{
		ValidateReceiveProtocolMessage = (ValidateReceiveProtocolMessageProc)GetProcAddress(AuroraServerMsgCheck, "ValidateReceiveProtocolMessage");
		if (ValidateReceiveProtocolMessage == NULL)
		{
			logger->Err("* Failed to resolve ValidateReceiveProtocolMessage in AuroraServerMsgCheck.dll");
		}

		ValidateReceiveDatagram = (ValidateReceiveDatagramProc)GetProcAddress(AuroraServerMsgCheck, "ValidateReceiveDatagram");
		if (ValidateReceiveDatagram == NULL)
		{
			logger->Err("* Failed to resolve ValidateReceiveDatagram in AuroraServerMsgCheck.dll");
		}
	}

	int i = 0;
	while(patches2[i].Apply()) {
		i++;
	}

	//
	// Set each window into a good initial state.
	//

	for (unsigned long i = 0; i < MAX_PLAYERS; i += 1)
		ResetWindow( i );

	//
	// All done.
	//

	return true;
}

enum { SHA512_HASH_LENGTH = 64 };

void
TlsCertificateHashToString(
	__in_bcount( SHA512_HASH_LENGTH ) const void * Hash,
	__out_bcount( SizeBytes ) char * HashStr,
	__in size_t SizeBytes
	)
{
	const UCHAR * HashBytes;
	CHAR          HashChunk[ 8 ];
	size_t        i;

	HashStr[ 0 ] = '\0';

	HashBytes = (const UCHAR *) Hash;

	for (i = 0; i < 64; i += 1)
	{
		StringCbPrintfA(
			HashChunk,
			sizeof( HashChunk ),
			"%02X",
			HashBytes[ i ] );
		StringCbCatA( HashStr, SizeBytes, HashChunk );

		if (i + 1 < SHA512_HASH_LENGTH)
			StringCbCatA( HashStr, SizeBytes, ":" );
	}

	return;
}

bool EnableTls(bool skipAuroraServerQueryCreateCertificate)
{
	AURORA_SERVER_QUERY_CREATE_CERTIFICATE Cert;
	CHAR                                   HostKey[ 256 ];

	ZeroMemory( &Cert, sizeof( Cert ) );

	if ((AuroraServerNetLayerCreateTlsServer_ == NULL) ||
	    (AuroraServerNetLayerQuery_ == NULL) ||
		(AuroraServerNetLayerSendEx_ == NULL))
	{
		logger->Warn("! EnableTls: Old version of AuroraServerNetLayer in use without TLS support.");
		return false;
	}

	if (AuroraServerNetLayerQuery_(
		NULL,
		AuroraServerQueryTlsInitialized,
		0,
		NULL,
		NULL ) == FALSE)
	{
		logger->Err("! EnableTls: Failed to initialize TLS.  Check that .NET Framework 4.7.2 or newer is enabled.  https://support.microsoft.com/en-us/help/4054530/microsoft-net-framework-4-7-2-offline-installer-for-windows");
		return false;
	}

	StringCbPrintfA(
		Cert.CertificatePath,
		sizeof( Cert.CertificatePath ),
		"%s\\NWNCertificate",
		NWNXHome );

	StringCbCopyA(
		Cert.Hostname,
		sizeof( Cert.Hostname ),
		"CN=Neverwinter Nights" );

	// Most Windows encryption algorithms are missing from Wine, so we must work around it
	if (skipAuroraServerQueryCreateCertificate) {
		// Assume that you created the PFX file externally
		std::string certificatePath = std::string(Cert.CertificatePath) + ".pfx";
		std::ifstream file(certificatePath);
		if (!file.is_open()) {
			logger->Err("! EnableTls: Expected a PFX file to be stored at '%s'.  File must be available with read access", certificatePath);

			return false;
		}

		// Read file content into a string
		std::string pfxFileContent((std::istreambuf_iterator<char>(file)),
		                           std::istreambuf_iterator<char>());

		// Calculate SHA-512 hash
		CryptoPP::SHA512 hash;
		hash.CalculateDigest(Cert.Sha512, reinterpret_cast<const byte*>(pfxFileContent.c_str()), pfxFileContent.length());
	} else if (AuroraServerNetLayerQuery_(
		NULL,
		AuroraServerQueryCreateCertificate,
		sizeof( Cert ),
		NULL,
		&Cert ) == FALSE) {
		logger->Err("! EnableTls: Failed to create server certificate and store it in directory '%s'.  Check that .NET Framework 4.7.2 or newer is enabled.  https://support.microsoft.com/en-us/help/4054530/microsoft-net-framework-4-7-2-offline-installer-for-windows",
		            NWNXHome);

		return false;
	}

	StringCbPrintfW(
		TlsCert,
		sizeof( TlsCert ),
		L"%S\\NWNCertificate.pfx",
		NWNXHome );

	TlsActive = true;

	TlsCertificateHashToString( Cert.Sha512, HostKey, sizeof( HostKey ) );

	logger->Info("* EnableTls: Enabled TLS with host key %s", HostKey);
	return true;
}

void SetDebugInfoPermission(int pDebugPermission)
{
	DebugInfoPermission = pDebugPermission;
	if(DebugInfoPermission & 0x1) 
		logger->Info("* SetDebugInfoPermission: print Debug Information is Autorized for Players");
	if(DebugInfoPermission & 0x2) 
		logger->Info("* SetDebugInfoPermission: print Debug Information is Autorized for DMs");
	if(DebugInfoPermission & 0x4) 
		logger->Info("* SetDebugInfoPermission: print Debug Information is Autorized for Admin");
}

bool
CheckDatagram(
	__in_bcount( DataSize ) const void * Data,
	__in unsigned long DataSize
	)
{
	//
	// Call the external validation library if present.  Otherwise, allow all
	// datagrams on through.
	//

	if (ValidateReceiveDatagram == NULL)
		return true;

	if (ValidateReceiveDatagram( (const unsigned char *) Data, DataSize ) == FALSE)
		return false;

	return true;
}

BOOL
__stdcall
SendMessageToPlayer(
	__in unsigned long PlayerId,
	__in_bcount( Size ) unsigned char * Data,
	__in unsigned long Size,
	__in unsigned long Flags
	)
{
	DebugPrint( "SendMessageToPlayer(%08X, %p, %08X, %08X)\n", PlayerId, Data, Size, Flags);

	//
	// Enqueue the outbound message to each applicable player's internal
	// NetLayerWindow object.
	//

	for (unsigned Player = 0; Player < MAX_PLAYERS; Player += 1)
	{
		BOOL StatefulCompress = TRUE;

		//
		// Ignore players that are not present.
		//

		if (!NetLayerInternal->Players[Player].m_bPlayerInUse)
			continue;

		//
		// Ignore players that have not had BNVS handled.
		//

		if (!PlayerState[Player].BNVSHandled)
		{
			DebugPrint( "SendMessageToPlayer: Player has not sent BNVS!\n" );
			continue;
		}

		//
		// Check if this player matches the given filter.  We will continue the
		// next loop iteration if it does not.
		//

		switch (PlayerId)
		{

		case PLAYERID_SERVER:
			DebugPrint("Send to server...\n");
			return TRUE;

		case PLAYERID_ALL_PLAYERS:
			if (!NetLayerInternal->Players[Player].m_bPlayerPrivileges)
				continue;
			break;

		case PLAYERID_ALL_GAMEMASTERS:
			if (!NetLayerInternal->Players[Player].m_bGameMasterPrivileges)
				continue;
			break;

		case PLAYERID_ALL_SERVERADMINS:
			if (!NetLayerInternal->Players[Player].m_bServerAdminPrivileges)
				continue;
			break;

		default: // Specific player id and not a symbolic filter
			if (PlayerId != Player)
				continue;

			//
			// Fallthrough.
			//

		case PLAYERID_ALL_CLIENTS: // Anyone, even if they are not a player yet
			break;

		}

		if (Size >= 0x03 && Data[0] == 0x50)
		{
			DebugPrint("Send %s.%02X to player %lu (%lu bytes)\n", GetMsgName(Data[1]), Data[2], Player, Size);

			//
			// Avoid compressing messages with user content in them with
			// stateful compression so as to avoid compression inference
			// attacks against TLS.  This applies only to TLS enabled sessions.
			//

			if ((Data[1] == CMD::Login) ||
				(Data[1] == CMD::Chat) ||
				(Data[1] == CMD::Dialog) ||
				(Data[1] == CMD::GuiHotBar) ||
				(Data[1] == CMD::DebugInfo) ||
				(Data[1] == CMD::PopUpGUIPanel) ||
				(Data[1] == CMD::Portal) ||
				(Data[1] == CMD::Character_Download) ||
				(Data[1] == CMD::CustomToken) ||
				(Data[1] == CMD::GuiRoster) ||
				(Data[1] == CMD::CustomAnim))
			{
				StatefulCompress = FALSE;
			}
			else if ((Data[1] == CMD::ClientSideMessage) &&
				     (Data[2] == 0x0B)) // TextMessage
			{
				//
				// Check filters on privileges to print DebugInformation or not
				//
				if (Data[1] == CMD::DebugInfo)
				{
					int currentPrivileges = NetLayerInternal->Players[Player].m_bServerAdminPrivileges << 2;
					currentPrivileges |= NetLayerInternal->Players[Player].m_bGameMasterPrivileges << 1;
					currentPrivileges |= NetLayerInternal->Players[Player].m_bPlayerPrivileges;
					
					if((DebugInfoPermission & currentPrivileges) == 0)
					{
						continue;
					}
				}
				StatefulCompress = FALSE;
			}

			switch (Data[1])
			{

			case CMD::Login:
			case CMD::Area:
			case CMD::Chat:

			//
			// Following three messages are important to not disable Nagle for
			// in the interests of good combat throughput..
			//


//			case CMD::ClientSideMessage:
//			case CMD::VoiceChat:
//			case CMD::QuickChat:
			case CMD::CustomAnim:
			case CMD::DungeonMaster:
			case CMD::CharList:
				Flags |= SEND_FLUSH_CACHE;
				break;

			case CMD::GameObjUpdate:

				//
				// Only disable Nagle for GameObjUpdate.Update, and not other
				// types such as update visual effect that are frequently sent
				// in combat.
				//

				if (Data[2] == 0x01)
				{
					Flags |= SEND_FLUSH_CACHE;
				}
				break;

			}

			//
			// Record if this player is about to join an area, or if the first
			// GameObjUpdate was sent after joining the area.  This lets us
			// temporarily accelerate GameObjUpdates for players who are
			// downloading the initial area contents all at once, but only for
			// the first update -- even if their window was already full from
			// the already queued static area contents being sent via the
			// Area.ClientArea message.
			//

			if ((Data[1] == CMD::Area) &&
			    (Data[2] == 0x01)) // ClientArea
			{
				DebugPrint("Enabling accelerated area transfer to player %lu.\n", Player);

				//
				// Remember the player account name so that case can be made
				// canonical.
				//

				if (PlayerState[Player].InGameWorld == false)
				{
					const char * Name;

					Name = GetPlayerAccountName( Player );

					if (Name != NULL)
						TrackPlayerAccountName( Name );
				}

				//
				// Enable bursting for area load, and unblock GameObjUpdates.
				//

				PlayerState[Player].AreaLoadPending    = true;
				PlayerState[Player].BlockGameObjUpdate = false;
				PlayerState[Player].InGameWorld        = true;
			}
			else if ((Data[1] == CMD::GameObjUpdate) &&
				     (Data[2] == 0x01)) // Update
			{
				//
				// If GameObjUpdates are still being blocked, then drop the
				// message.  This prevents the client from being crashed by the
				// server erroneously emitting GameObjUpdates prior to the
				// transmission of an Area.ClientArea message.
				//
				// The server will once again retransmit the entire area
				// contents once Area.ClientArea is signaled, so it is not
				// harmful to eliminate these extraneous updates.
				//

				if (PlayerState[Player].BlockGameObjUpdate)
				{
					DebugPrint("Blocking premature GameObjUpdate.Update to player %lu.\n", Player);
					continue;
				}

				DebugPrint("Closing accelerated area transfer window for player %lu.\n", Player);
				PlayerState[Player].AreaLoadPending = false;
			}
			else if ((Data[1] == CMD::Login) &&
				     (Data[2] == 0x05)) // Confirm
			{
				if (PlayerState[Player].LoginConfirmed)
				{
					DebugPrint("Login confirmed twice for player %lu?\n", PlayerId);
					continue;
				}

				PlayerState[Player].LoginConfirmed = true;
			}
		}

		//
		// Call the packet filter, if one was registered.
		//

		if (CallOnPlayerConnectionSend(
			Player,
			Data,
			Size,
			Flags) == FALSE)
		{
			DebugPrint("Packet filter blocked message send.\n");
			return TRUE;
		}

		if (PlayerState[Player].TlsActive == false)
		{
			AuroraServerNetLayerSend_(
				Connections[Player],
				Data,
				Size,
				FALSE,
				(Flags & SEND_FLUSH_CACHE) ? TRUE : FALSE);
		}
		else
		{
			AuroraServerNetLayerSendEx_(
				Connections[Player],
				Data,
				Size,
				FALSE,
				(Flags & SEND_FLUSH_CACHE) ? TRUE : FALSE,
				StatefulCompress);
		}
	}

	return TRUE;
}

void
CheckForNewWindow(
	__in SlidingWindow *Winfo
	)
{
	//
	// Check if this CNetLayerWindow instance has not yet been tagged with our
	// magical SEQ/ACK values.  These are reset to zero on ShutDown/Initialize
	// as called by the game, but they should never increment with receive
	// processing never hit and the data send path also never hit.
	//

	if ((Winfo->m_LastSeqAcked == 0x0000) &&
		(Winfo->m_Seq == 0x0000)          &&
		(Winfo->m_RemoteSeq == 0x0000))
	{
		DebugPrint( "Reinitializing sliding window %p for player %lu.\n", Winfo, Winfo->m_PlayerId );

		if (!Winfo->m_WindowInUse)
		{
			DebugPrint( "!!! Window isn't in use!\n" );

			if (IsDebuggerPresent( ))
				__debugbreak( );
		}
		else if (Winfo->m_PlayerId >= MAX_PLAYERS)
		{
			DebugPrint( "!!! PlayerId is out of range for window!\n" );

			if (IsDebuggerPresent( ))
				__debugbreak( );

			return;
		}

		//
		// Call the packet filter, if one was registered.
		//

		CallOnPlayerConnectionClose( Winfo->m_PlayerId );

		//
		// Reinitialize our internal window so that it has the right seq/ack
		// states.
		//

		ResetWindow( Winfo->m_PlayerId );

		//
		// Tag this window as owned by us so that we do not try and throw away
		// our existing seq/ack state the next time around.  This also lets us
		// easily verify that nobody else is touching the window's internal
		// state as we should have disabled all of that logic.
		//

		Winfo->m_Seq = 0x4242;
	}
	else if ((Winfo->m_LastSeqAcked == 0x0000) &&
		     (Winfo->m_Seq == 0x4242)          &&
		     (Winfo->m_RemoteSeq == 0x0000))
	{
		//
		// Everything checks out as ok as a window that we have already set up
		// for our end, consider it good.
		//

		return;
	}
	else
	{
		//
		// With us having snipped out the code that would update seq/ack fields
		// this should not happen unless there's something we've missed, which
		// would be bad.  Catch this here and now so we can figure out just
		// what happened.
		//

		DebugPrint(
			"Window %p player %lu LastSeqAcked %04X Seq %04X RemoteSeq %04X has had its internal seq/ack state modified when it should not have!\n",
			Winfo,
			Winfo->m_PlayerId,
			Winfo->m_LastSeqAcked,
			Winfo->m_Seq,
			Winfo->m_RemoteSeq);

		if (IsDebuggerPresent( ))
			__debugbreak( );
	}
}

__declspec(naked)
BOOL
__stdcall
SendMessageToPlayer2(
	__in unsigned long PlayerId,
	__in_bcount( Size ) unsigned char * Data,
	__in unsigned long Size,
	__in unsigned long Flags
	)
{
	__asm
	{
		mov  [NetLayerInternal], ecx
		jmp  SendMessageToPlayer
	}
}

__declspec(naked)
BOOL
__stdcall
DisconnectPlayerThunk(
	__in unsigned long PlayerId,
	__in unsigned long ReasonId,
	__in int Unk0Typically1,
	__in int Unk1Typically1
	)
{
	__asm
	{
		mov  ecx, [NetLayerInternal]
		mov  eax, OFFS_DisconnectPlayer
		jmp  eax
	}
}

VOID
__stdcall
DisconnectPlayer(
	__in unsigned long PlayerId,
	__in unsigned long ReasonId
	)
{
	if (PlayerId > MAX_PLAYERS)
		return;

	DisconnectPlayerThunk(PlayerId, ReasonId, 1, 1);
}

__declspec(naked)
void
__fastcall
SetInFrameTimer(
	__in SlidingWindow * Winfo
	)
{
	__asm
	{
		; ecx already set by __fastcall
		mov   eax, OFFS_SetInFrameTimer
		jmp   eax
	}
}

__declspec(naked)
void
__stdcall
CallFrameTimeout(
	__in unsigned long Unused,
	__in SlidingWindow * Winfo
	)
{
	__asm
	{
		push    ebp
		mov     ebp, esp

		push    dword ptr [ebp+08h]
		mov     ecx, dword ptr [ebp+0ch]
		mov     eax, OFFS_FrameTimeout
		call    eax

		mov     esp, ebp
		pop     ebp
		ret     08h
	}
}

BOOL
__stdcall
FrameReceive(
	__in_bcount( Size ) unsigned char *Data,
	__in unsigned long Size,
	__in SlidingWindow *Winfo
	)
{
#ifdef XP_BUGFIX_NETLAYER_INSTRUMENT
	NetLayerLastRecvTick = GetTickCount( );
#endif

	DebugPrint(
		"FrameReceive: Recv from SlidingWindow %p player %lu\n",
		Winfo,
		Winfo->m_PlayerId);

	//
	// Drop the message if we haven't initialized fully.  It will be resent
	// anyway so this is ok.
	//

	if (NetLayerInternal == NULL)
		return TRUE;

	//
	// Reinitialize the window if appropriate.
	//

	CheckForNewWindow( Winfo );

	//
	// Update timeouts so that the server doesn't drop this player for timeout.
	//

	SetInFrameTimer( Winfo );

	PlayerState[Winfo->m_PlayerId].RecvTick = GetTickCount( );

	if (PlayerState[Winfo->m_PlayerId].BNVSHandled == false)
	{
		DebugPrint( "FrameReceive: Player has not sent BNVS!\n" );
		return TRUE;
	}

	//
	// Pass it on to the internal NetLayerWindow implementation to deal with.
	//

	return AuroraServerNetLayerReceive_(
		Connections[Winfo->m_PlayerId],
		Data,
		(size_t) Size);
}

__declspec(naked)
BOOL
__stdcall
FrameReceive2(
	__in_bcount( Size ) unsigned char *Data,
	__in unsigned long Size
	)
{
	__asm
	{
		push    ebp
		mov     ebp, esp

		push    ecx
		push    dword ptr [ebp+0ch]
		push    dword ptr [ebp+08h]
		call    FrameReceive

		mov     esp, ebp
		pop     ebp
		ret     08h
	}
}

BOOL
__stdcall
FrameTimeout(
	__in unsigned long Unused,
	__in SlidingWindow *Winfo
	)
{
#ifdef XP_BUGFIX_NETLAYER_INSTRUMENT
	NetLayerLastTimeoutTick = GetTickCount( );
#endif

	//DebugPrint(
	//	"FrameTimeout: Do timer processing for SlidingWindow %p player %lu\n",
	//	Winfo,
	//	Winfo->m_PlayerId);

	//
	// Reinitialize the window if appropriate.
	//

	CheckForNewWindow( Winfo );

	//
	// Perform timeout handling in the replacement NetLayerWindow.
	//

	AuroraServerNetLayerTimeout_(Connections[Winfo->m_PlayerId]);

	//
	// Let the server do its internal timeout handling too, so that it may drop
	// a player that has become unresponsive.  N.B.  We've neutered the
	// FrameSend API, so FrameTimeout's attempts to send a NAK/ACK will not be
	// harmful.  It will still be able to call DisconnectPlayer as appropriate.
	//

	CallFrameTimeout( Unused, Winfo );

	return TRUE;
}

__declspec(naked)
BOOL
__stdcall
FrameTimeout2(
	__in unsigned long Unused
	)
{
	__asm
	{
		push    ebp
		mov     ebp, esp

		push    ecx
		push    dword ptr [ebp+08h]
		call    FrameTimeout

		mov     esp, ebp
		pop     ebp
		ret     04h
	}
}



__declspec(naked)
BOOL
__stdcall
CallHandleMessage(
	__in_bcount( Length ) const unsigned char * Data,
	__in size_t Length,
	__in unsigned long PlayerId
	)
{
	__asm
	{
		;
		; BOOL
		; CServerExoApp::HandleMessage(
		;   __in unsigned long PlayerId,
		;   __in_bcount( Length ) const unsigned char * Data,
		;   __in size_t Size,
		;   __in bool NonClientMessage
		;   );
		;

		push    ebp
		mov     ebp, esp

		push    0h
		push    dword ptr [ebp+0ch]
		push    dword ptr [ebp+08h]
		push    dword ptr [ebp+10h]

		mov     ecx, dword ptr [NetLayerInternal]
		mov     ecx, dword ptr [ecx]
		mov     edx, dword ptr [ecx]
		call    dword ptr [edx+10h]

		mov     esp, ebp
		pop     ebp
		ret     0ch
	}
}

bool
__cdecl
OnNetLayerWindowReceive(
	__in_bcount( Length ) const unsigned char * Data,
	__in size_t Length,
	__in void * Context
	)
{
	unsigned long     PlayerId = (unsigned long) (ULONG_PTR) Context;
	PlayerInfo      * Pinfo;
	SlidingWindow   * Winfo;
	UCHAR             MajorFunction;
	UCHAR             MinorFunction;

	MajorFunction = (UCHAR) -1;
	MinorFunction = (UCHAR) -1;

	Pinfo = &NetLayerInternal->Players[ PlayerId ];
	Winfo = &NetLayerInternal->Windows[ Pinfo->m_nSlidingWindowId ];

	if (Length >= 0x03)
	{
		DebugPrint(
			"OnNetLayerWindowReceive: Recv %s.%02X from player %lu WindowId %lu\n",
			GetMsgName(Data[1]),
			Data[2],
			PlayerId,
			Pinfo->m_nSlidingWindowId);

		MajorFunction = Data[1];
		MinorFunction = Data[2];
	}

	if (!Pinfo->m_bPlayerInUse)
	{
		DebugPrint( "OnNetLayerWindowReceive: Player not active!\n" );
		return false;
	}

	if (Winfo->m_PlayerId != PlayerId)
	{
		logger->Err("OnNetLayerWindowReceive: *** SLIDING WINDOW PLAYER ID MISMATCH !! %lu, %lu, %lu (%p, %p)",
			PlayerId,
			Winfo->m_PlayerId,
			Pinfo->m_nSlidingWindowId,
			Pinfo,
			Winfo);
		DebugPrint(
			"OnNetLayerWindowReceive: *** SLIDING WINDOW PLAYER ID MISMATCH !! %lu, %lu, %lu (%p, %p)\n",
			PlayerId,
			Winfo->m_PlayerId,
			Pinfo->m_nSlidingWindowId,
			Pinfo,
			Winfo);

		if (IsDebuggerPresent( ))
			__debugbreak( );

		return false;
	}

	//
	// Call the message validator, if one was registered.
	//

	if (ValidateReceiveProtocolMessage != NULL)
	{
		if (ValidateReceiveProtocolMessage(
			PlayerId,
			Data,
			Length) == FALSE)
		{
			DebugPrint("Message validator blocked message receive.\n");
			return true;
		}
	}

	//
	// Call the packet filter, if one was registered.
	//

	if (CallOnPlayerConnectionReceive(
		PlayerId,
		Data,
		Length) == FALSE)
	{
		DebugPrint("Packet filter blocked message receive.\n");
		return true;
	}

	if (MajorFunction == CMD::Login)
	{
		if (MinorFunction == 0x0D) // Login.InitiateModuleForPlayer
		{
			//
			// Block InitiateModuleForPlayer if the player does not yet have a
			// creature.
			//

			NWN::CNWSPlayer * PlayerObject = BugFix::GetClientObjectByPlayerId( PlayerId );

			if (PlayerObject == NULL)
			{
				DebugPrint("No player for Login.InitiateModuleForPlayer?\n");
				if (IsDebuggerPresent( ))
					__debugbreak( );

				PlayerState[PlayerId].DropMessages = true;

				return true;
			}

			if (PlayerObject->m_oidPCObject == NWN::INVALIDOBJID)
			{
				DebugPrint("Player %lu attempting Login.InitiateModuleForPlayer without a PC object - blocking!\n", PlayerId);
				if (IsDebuggerPresent( ))
					__debugbreak( );

				PlayerState[PlayerId].DropMessages = true;

				return true;
			}

			if (PlayerState[PlayerId].LoginConfirmed == false)
			{
				DebugPrint("Player %lu attempting Login.InitiateModuleForPlayer without login confirmed - blocking!\n", PlayerId);
				if (IsDebuggerPresent( ))
					__debugbreak( );

				PlayerState[PlayerId].DropMessages = true;

				return true;
			}
		}
		else if (MinorFunction == 0x0B) // Login.LoadCharacterFinish
		{
			//
			// Block LoadCharacterFinish if the player does not yet have a
			// creature.
			//

			NWN::CNWSPlayer * PlayerObject = BugFix::GetClientObjectByPlayerId( PlayerId );

			if (PlayerObject == NULL)
			{
				DebugPrint("No player for Login.LoadCharacterFinish?\n");
				if (IsDebuggerPresent( ))
					__debugbreak( );

				PlayerState[PlayerId].DropMessages = true;

				return true;
			}

			if (PlayerObject->m_oidPCObject == NWN::INVALIDOBJID)
			{
				DebugPrint("Player %lu attempting Login.LoadCharacterFinish without a PC object - blocking!\n", PlayerId);
				if (IsDebuggerPresent( ))
					__debugbreak( );

				PlayerState[PlayerId].DropMessages = true;

				return true;
			}
		}
	}
	else if ((MajorFunction == CMD::Input) || (MajorFunction == CMD::GroupInput))
	{
		if (PlayerState[PlayerId].InGameWorld == false)
		{
			//
			// In this state, allow only run gui script.
			//
			// In this current state, the Input.RunScript function can only be utilized effectively in 
			// conjunction with other server plugins, as the player is currently represented as OBJECT_INVALID. 
			//
			// With the right processing, however, this makes it possible to obtain input from the client before
			// he enters the GameWorld.
			//
			if (!(MajorFunction == CMD::Input && MinorFunction == 0x30))
			{
				DebugPrint("Player %lu sent input before entering game world, dropped.\n", PlayerId);
				return true;
			}
		}

		if (MajorFunction == CMD::Input)
		{
			switch (MinorFunction)
			{

			case 0x0D: // Rest
				{
					NWN::CGameObject * CreatureObj;
					NWN::CNWSPlayer  * PlayerObject = BugFix::GetClientObjectByPlayerId( PlayerId );

					if (PlayerObject == NULL)
					{
						DebugPrint("No player for Input.Rest\n");
						if (IsDebuggerPresent( ))
							__debugbreak( );

						return true;
					}

					CreatureObj = BugFix::GetGameObject(PlayerObject->m_oidPCObject);
					if ((CreatureObj == NULL) || (CreatureObj->AsCreature() == NULL))
					{
						DebugPrint("No creature for Input.Rest\n");
						if (IsDebuggerPresent( ))
							__debugbreak( );

						return true;
					}
				}
				break;

			}
		}
	}

	if (PlayerState[PlayerId].DropMessages)
	{
		DebugPrint("Dropping messages from blocked player %lu\n", PlayerId);
		return true;
	}

	//
	// Pass the packet on to the actual handler, indicating it up to the game
	// for completion handling.
	//

	return CallHandleMessage( Data, Length, PlayerId ) ? TRUE : FALSE;
}

bool
__cdecl
OnNetLayerWindowSend(
	__in_bcount( Length ) const unsigned char * Data,
	__in size_t Length,
	__in void * Context
	)
{
	unsigned long     PlayerId = (unsigned long) (ULONG_PTR) Context;
	PlayerInfo      * Pinfo;
	SlidingWindow   * Winfo;
	CExoNetInternal * NetI;
	sockaddr_in       sin;
	int               slen;

#ifdef XP_BUGFIX_NETLAYER_INSTRUMENT
	NetLayerLastSendTick = GetTickCount( );
#endif

	//
	// Drop the message if we haven't initialized fully.  It will be resent
	// anyway so this is ok.
	//

	if (NetLayerInternal == NULL)
		return true;

	Pinfo = &NetLayerInternal->Players[ PlayerId ];
	Winfo = &NetLayerInternal->Windows[ Pinfo->m_nSlidingWindowId ];

	DebugPrint( "OnNetLayerWindowSend: Send to player %lu WindowId %lu %lu bytes\n", PlayerId, Pinfo->m_nSlidingWindowId, Length );
//	logger->Info( wxT("OnNetLayerWindowSend: Send to player %lu WindowId %lu %lu bytes\n"), PlayerId, Pinfo->m_nSlidingWindowId, Length );

	if (!Pinfo->m_bPlayerInUse)
	{
		DebugPrint( "OnNetLayerWindowSend: Player not active!\n" );
		return false;
	}

	if (Winfo->m_PlayerId != PlayerId)
	{
		logger->Err("OnNetLayerWindowSend: *** SLIDING WINDOW PLAYER ID MISMATCH !! %lu, %lu, %lu (%p, %p)",
			PlayerId,
			Winfo->m_PlayerId,
			Pinfo->m_nSlidingWindowId,
			Pinfo,
			Winfo);
		DebugPrint(
			"OnNetLayerWindowSend: *** SLIDING WINDOW PLAYER ID MISMATCH !! %lu, %lu, %lu (%p, %p)\n",
			PlayerId,
			Winfo->m_PlayerId,
			Pinfo->m_nSlidingWindowId,
			Pinfo,
			Winfo);

		if (IsDebuggerPresent( ))
			__debugbreak( );

		return false;
	}

	NetI = NetLayerInternal->Net->Internal;

	if (Winfo->m_ConnectionId >= NetI->NumConnections)
	{
		logger->Err("OnNetLayerWindowSend: *** ConnectionId for player %lu window %lu out of range !! %lu >= %lu (NetI %p)",
			PlayerId,
			Pinfo->m_nSlidingWindowId,
			Winfo->m_ConnectionId,
			NetI->NumConnections,
			NetI);
		DebugPrint("OnNetLayerWindowSend: *** ConnectionId for player %lu window %lu out of range !! %lu >= %lu (NetI %p)\n",
			PlayerId,
			Pinfo->m_nSlidingWindowId,
			Winfo->m_ConnectionId,
			NetI->NumConnections,
			NetI);

		if (IsDebuggerPresent( ))
			__debugbreak( );

		return false;
	}

	//
	// Write the raw message out using the game's sendto socket and the address
	// information for this player.
	//

	ZeroMemory( sin.sin_zero, sizeof( sin.sin_zero ) );
	sin.sin_family      = AF_INET;
	sin.sin_port        = htons( NetI->ConnectionAddresses[ Winfo->m_ConnectionId ].Port );
	sin.sin_addr.s_addr = NetI->ConnectionAddresses[ Winfo->m_ConnectionId ].Address;

	DebugPrint( "Send to %08X:%lu:\n", sin.sin_addr.s_addr, NetI->ConnectionAddresses[ Winfo->m_ConnectionId ].Port );

	if (ReflectorIsEnabled( ) && sin.sin_addr.s_addr != INADDR_LOOPBACK_NET)
	{
		slen = ReflectorSendto(
			NetI->Socket,
			(const char *) Data,
			(int) Length,
			0,
			(sockaddr_in *) &sin,
			sizeof( sin ));
	}
	else
	{
		slen = sendto(
			NetI->Socket,
			(const char *) Data,
			(int) Length,
			0,
			(sockaddr *) &sin,
			sizeof( sin ));
	}

	if (slen < 0)
	{
		DebugPrint( "sendto fails - %lu\n", WSAGetLastError( ) );
		return false;
	}

	return true;
}

bool
__cdecl
OnNetLayerWindowStreamError(
	__in bool Fatal,
	__in unsigned long ErrorCode,
	__in void * Context
	)
{
	unsigned long PlayerId;

	PlayerId = (unsigned long) (ULONG_PTR) Context;

	//
	// Just log a warning that something is broken.  We could disconnect the
	// player here too, but synchronizing that with the rest of the internal
	// state would be painful.  We could also just write a disconnect packet
	// out to the player and then let the server's timeout handling eventually
	// drop the server's player state for that player.
	//

	logger->Err("OnNetLayerWindowStreamError: Stream error %lu for player %lu...",
		ErrorCode,
		PlayerId);
	DebugPrint("OnNetLayerWindowStreamError: Stream error %lu for player %lu...\n",
		ErrorCode,
		PlayerId);

	DisconnectPlayer( PlayerId, 0 );

	return true;
}

unsigned long
__stdcall
SetGameObjUpdateSize2(
	__in unsigned long PlayerId
	)
{
	const unsigned long                  DefaultSize = 400;
	AURORA_SERVER_QUERY_SEND_QUEUE_DEPTH QueryDepth;

	if ((Connections[ PlayerId ] == NULL)     ||
	    (AuroraServerNetLayerQuery_ == NULL))
	{
		return DefaultSize;
	}

	//
	// Check the send queue depth.
	//

	if (!AuroraServerNetLayerQuery_(
		Connections[ PlayerId ],
		AuroraServerQuerySendQueueDepth,
		sizeof( QueryDepth ),
		NULL,
		&QueryDepth))
	{
		return DefaultSize;
	}

	//
	// Limit sends to default-sized send queues if this window is getting
	// behind, otherwise let it burst.
	//
	// As an exception, let the initial GameObjUpdate after joining an area
	// burst ahead even if the window wasn't empty -- which it would be
	// probably not, as we would have just sent a large chunk of data via the
	// Area.ClientArea message.
	//
	// This actually reduces the amount of data sent during area loading as it
	// allows a larger window of data to compress over !
	//

	if ((QueryDepth.SendQueueDepth >= 8) &&
	    (!PlayerState[ PlayerId ].AreaLoadPending))
		return DefaultSize;

	DebugPrint("Allowing burst GameObjUpdate transmission to %lu.\n", PlayerId);

	//
	// Otherwise, allow a large burst transmission to boost area loading
	// performance.
	//

	return (unsigned long) GameObjUpdateBurstSize;
}

__declspec(naked)
void
SetGameObjUpdateSize(
	)
{
	__asm
	{
		;
		; Calculate the actual update size to use.
		;

		push    ecx

		push    eax
		call    SetGameObjUpdateSize2

		pop     ecx

		;
		; Set the update size and return to normal program flow.
		;

		mov     esi, eax
		mov     eax, OFFS_GameObjUpdateSizeLimit1 + 6
		jmp     eax
	}
}

void
__stdcall
InitializePlayerInfoForBNCS2(
	__in unsigned long PlayerId,
	__in unsigned long PacketLength,
	__in const unsigned char * PacketBuffer
	)
{
	unsigned long ApplicationId;

	if (PlayerId > MAX_PLAYERS)
	{
		DebugPrint( "InitializePlayerInfoForBNCS2: Invalid player ID %08X\n", PlayerId );
		if (IsDebuggerPresent( ))
		{
			__debugbreak( );
		}

		return;
	}

	PlayerState[ PlayerId ].TlsNegotiated = false;
	PlayerState[ PlayerId ].RecvTick      = GetTickCount( );
	PlayerState[ PlayerId ].BNVSHandled   = false;

	if (PacketLength < 0x18)
		return;

	//
	// Get the ApplicationId from the BNCS message.  This is normally the tick
	// count of client startup, but with the Client Extension, it is set to
	// 0xFFFFFFxx where xx are requested features to negotiate.
	//

	ApplicationId = *(unsigned long *)(&PacketBuffer[0x0E]);
	if ((ApplicationId & APPLICATIONID_NEGOTIATE_MASK) != APPLICATIONID_NEGOTIATE)
		return;

	if (ApplicationId & APPLICATIONID_TLS) // Feature 0x01 : TLS
	{
		PlayerState[ PlayerId ].TlsNegotiated = true;
	}

	return;
}

__declspec(naked)
void
InitializePlayerInfoForBNCS(
	)
{
	__asm
	{
		push    ecx                    ; Save register

		push    dword ptr [ebp+0xc]    ; Push packet buffer
		push    dword ptr [ebp+0x10]   ; Push packet length
		push    ebx                    ; Push player ID

		call    InitializePlayerInfoForBNCS2

		pop     ecx                    ; Restore register

		mov     eax, OFFS_CNetLayerPlayerInfo_Initialize ; Set jump target
		jmp     eax                    ; Rejoin common program flow.
	}
}

int
NetLayerHandleBNCS(
	__in const char * buf,
	__in int len,
	__in struct sockaddr_in * from,
	__in int fromlen
	)
{
	ULONG       Now;
	ULONG       PlayerId;
	sockaddr_in sin;

	Now = GetTickCount( );

	for (PlayerId = 0; PlayerId < MAX_PLAYERS; PlayerId += 1)
	{
		if (!GetPlayerConnectionInfo(PlayerId, &sin))
			continue;

		if ((sin.sin_addr.s_addr == from->sin_addr.s_addr) &&
			(sin.sin_port == from->sin_port))
		{
			//
			// Do not allow BNCS from a connection that already has sent BNCS.
			// This causes the player to corrupt its internal view of which
			// players are associated with which connections.  Deal with this
			// by checking for a pre-existing connection from the same network
			// address, and if it has been active recently, dropping the BNCS
			// packet (otherwise forcibly disconnecting the stale connection).
			//

			if (Now - PlayerState[PlayerId].RecvTick >= RECV_RECONNECT_TIMEOUT)
			{
				DisconnectPlayer(PlayerId, 0);
				return len;
			}
			else
			{
				return -1;
			}
		}

		return len;
	}

	return len;
}

int
NetLayerHandleBNVS(
	__in const char * buf,
	__in int len,
	__in struct sockaddr_in * from,
	__in int fromlen
	)
{
	ULONG       Now;
	ULONG       PlayerId;
	sockaddr_in sin;

	Now = GetTickCount( );

	for (PlayerId = 0; PlayerId < MAX_PLAYERS; PlayerId += 1)
	{
		if (!GetPlayerConnectionInfo(PlayerId, &sin))
			continue;

		//
		// Do not allow BNVS from a connection that already has sent BNVS.
		// This causes the player to corrupt its internal view of which players
		// are associated with which connections.
		//

		if ((sin.sin_addr.s_addr == from->sin_addr.s_addr) &&
			(sin.sin_port == from->sin_port))
		{
			if (PlayerState[PlayerId].BNVSHandled != false)
			{
				return -1;
			}
			else
			{
				return len;
			}
		}

		return len;
	}

	return len;
}

void
NetLayerHandleBNVR(
	__in const char * buf,
	__in int len,
	__in struct sockaddr_in * to,
	__in int tolen
	)
{
	unsigned long ApplicationId;
	SOCKADDR_IN   sin;
	unsigned long PlayerId;

	if (len != 4 + 1 + 4)
		return;

	//
	// Patch the response ApplicationId to include negotiation status.
	//

	ApplicationId = APPLICATIONID_NEGOTIATE;

	for (PlayerId = 0; PlayerId < MAX_PLAYERS; PlayerId += 1)
	{
		if (!GetPlayerConnectionInfo(PlayerId, &sin))
			continue;

		if ((sin.sin_addr.s_addr == to->sin_addr.s_addr) &&
			(sin.sin_port == to->sin_port))
		{
			PlayerState[PlayerId].BNVSHandled = true;
			PlayerState[PlayerId].RecvTick = GetTickCount( );

			if ((TlsActive != false) &&
				(PlayerState[PlayerId].TlsNegotiated != false))
			{
				ApplicationId |= APPLICATIONID_TLS;
				break;
			}
		}
	}

	*(unsigned long*)(&buf[5]) = ApplicationId;
	return;
}

bool
NetLayerTlsOnForPlayer(
	__in unsigned long PlayerId
	)
{
	if (PlayerId >= MAX_PLAYERS)
		return false;

	if (TlsActive == false)
		return false;

	return PlayerState[PlayerId].TlsActive;
}
