#ifndef XXNETLAYER_H
#define XXNETLAYER_H

#ifdef _MSC_VER
#pragma once
#endif

#ifdef __USE_WINE__
#include <iostream>
#include <fstream>
#include <string>
#include <cryptopp/sha.h>
#include <cryptopp/hex.h>
#endif

//
// Begin snipped from NetLayerWindow.h
//

//
// Define callback types.
//

typedef
bool
(__cdecl * OnReceiveProc)(
	__in_bcount( Length ) const unsigned char * Data,
	__in size_t Length,
	__in void * Context
	);

typedef
bool
(__cdecl * OnSendProc)(
	__in_bcount( Length ) const unsigned char * Data,
	__in size_t Length,
	__in void * Context
	);

typedef
bool
(__cdecl * OnStreamErrorProc)(
	__in bool Fatal,
	__in unsigned long ErrorCode,
	__in void * Context
	);

typedef struct _CONNECTION_CALLBACKS
{
	//
	// Define the arbitrary user context value that is passed to each
	// callback invocation.
	//

	void              * Context;

	//
	// Define the high level receive procedure.  It is invoked each
	// time a completed high level frame has been received.
	//

	OnReceiveProc       OnReceive;

	//
	// Define the low level send procedure.  It is invoked each time
	// a subframe datagram is ready to be written out on the wire.
	//

	OnSendProc          OnSend;

	//
	// Define the routine that is invoked when an unrecoverable stream
	// error has occurred.  This typically indicates that the
	// connection should be thrown away.  If Fatal is not set then it
	// is possible that the operation could be retried, else there has
	// been a fatal state error (i.e. dropped compressed frame).
	//
	// N.B.  This is necessary to deal with the fact that the BioWare
	//       implementation has serious bugs that will eventually
	//       result in a stream being rendered unusable.
	//

	OnStreamErrorProc   OnStreamError;

} CONNECTION_CALLBACKS, * PCONNECTION_CALLBACKS;

typedef const struct _CONNECTION_CALLBACKS * PCCONNECTION_CALLBACKS;

//
// End snipped from NetLayerWindow.h
//


#define MAX_PLAYERS               0x60
#define PLAYERID_ALL_CLIENTS      0xFFFFFFFF
#define PLAYERID_INVALIDID        0xFFFFFFFE
#define PLAYERID_SERVER           0xFFFFFFFD
#define PLAYERID_ALL_PLAYERS      0xFFFFFFF7
#define PLAYERID_ALL_GAMEMASTERS  0xFFFFFFF6
#define PLAYERID_ALL_SERVERADMINS 0xFFFFFFF5

#define SEND_COMPRESSED_DATA        0x01
#define SEND_FLUSH_CACHE            0x02
#define SEND_LOW_PRIORITY           0x04
#define SEND_COMPRESSED_BY_NETLAYER 0x10
#define SEND_SINGLE_RECIPIENT       0x20

#include <pshpack1.h>

struct CExoString
{
	char * m_sString;
	int    m_nBufferLength;
};

struct PlayerInfo // sizeof = 0x78, CNetLayerPlayerInfo
{
	int            m_bPlayerInUse;              // 00
	CExoString     m_sPlayerName;               // 04
	char           skip0[0x04];                 // 0c
	unsigned long  m_nSlidingWindowId;          // 10
	int            m_bPlayerPrivileges;         // 14
	int            m_bGameMasterPrivileges;     // 18
	int            m_bServerAdminPrivileges;    // 1c
	char           skip1[0x58];                 // 20
	// +0
	// +120
	// +240

	// ((n<<4)-n)*8

#if 0

	0:000> dt nwn2server!CNetLayerPlayerInfo
   =00400000 MAX_BUFFER_LENGTH : Int4B
   =00400000 MASTER_SERVER_TIMEOUT : Uint4B
   +0x000 m_bPlayerInUse   : Int4B
   +0x004 m_sPlayerName    : CExoString
   +0x00c m_nPlayerLanguage : Int4B
   +0x010 m_nSlidingWindowId : Uint4B
   +0x014 m_bPlayerPrivileges : Int4B
   +0x018 m_bGameMasterPrivileges : Int4B
   +0x01c m_bServerAdminPrivileges : Int4B
   +0x020 m_szMstServerChallenge : CExoString
   +0x028 m_nMstServerTimeout : Uint8B
   +0x030 m_nMstServerTimeStamp : Uint8B
   +0x038 m_sGamePasswordChallenge : CExoString
   +0x040 m_sCDKeyChallenge : CExoString
   +0x048 m_sMstPasswordChallenge : CExoString
   +0x050 m_nConnectionType : UChar
   +0x054 m_bIsPrimaryPlayer : Int4B
   +0x058 m_lstKeys        : CExoArrayList<CNetLayerPlayerCDKeyInfo>
   +0x064 m_sMstPasswordResponse : CExoString
   +0x06c m_bCDKeyAuthorized : Int4B
   +0x070 m_bMstPasswordAuthorized : Int4B
   +0x074 m_nExpansionPacks : Uint2B

#endif
};



C_ASSERT( sizeof( PlayerInfo ) == 0x78 );

struct NetLayerInternal;

struct SlidingWindow // sizeof = 0x93c  CNetLayerWindow
{
#if 0
	char             skip0[0x04];   // 00
	int              connected;     // 04
	unsigned long    ConnectionId;  // 08
	char             skip1[0x930];  // 0c
#endif
	void             *vftable;               // 00
	NetLayerInternal *m_NetLayerInternal;    // 04
	int               m_WindowInUse;         // 08
	unsigned long     m_PlayerId;            // 0c
	unsigned long     m_ReceiveConnectionId; // 10
	unsigned long     m_ConnectionId;        // 14  (SEND connection id)
	unsigned long     m_LastSeqAcked;        // 18
	unsigned long     m_Seq;                 // 1c  (2019-09-08:  This is probably connection ID?)
	unsigned long     m_RemoteSeq;           // 20
	unsigned long     field_24;              // 24
	unsigned long     m_FramesSent;          // 28
	unsigned long     field_2c;              // 2c
	unsigned long     field_30;              // 30
	unsigned long     m_Timer1;              // 34
	unsigned long     field_38;              // 38
	unsigned long     field_3c;              // 3c
	unsigned long     m_Timer2;              // 40
	unsigned long     field_44;              // 44
	unsigned long     field_48;              // 48
	unsigned long     field_4c;              // 4c
	unsigned long     field_50;              // 50
	unsigned long     field_54;              // 54
	unsigned long     field_58;              // 58
	unsigned long     field_5c;              // 5c
	unsigned long     m_SubframeIndicies;    // 60
	char              skip0[0x8d8];          // 64
};

C_ASSERT( sizeof( SlidingWindow ) == 0x93c );

struct SocketInfo // sizeof = 0x10, probably sockaddr
{
	char           skip0[0x2]; // 00
	unsigned short Port;       // 02 host byte order
	unsigned long  Address;    // 04 network byte order
	char           skip1[0x8]; // 08
};

C_ASSERT( sizeof( SocketInfo ) == 16 );

struct CExoNetInternal
{
	char         skip0[0x10];         // 00
	SOCKET       Socket;              // 10
	char         skip1[0x24];         // 18
	unsigned     NumConnections;      // 38
	char         skip2[0x04];         // 3c
	ULONG      * RecvfromTimes;       // 40
	SocketInfo * ConnectionAddresses; // 44
	char         skip3[0x1358];       // 48
	sockaddr_in  MasterServerAddress;
};

C_ASSERT( offsetof( CExoNetInternal, ConnectionAddresses ) == 0x44 );

struct CExoNet
{
	CExoNetInternal * Internal;

	enum
	{
		PROTOCOL_UDP = 1,
		PROTOCOL_IPX = 2
	};
};

struct CNetLayerInternal
{
	void         *ServerApp;                 // 00000
	CExoNet      *Net;                       // 00004
	char          skip0[0x04];               // 00008
	SlidingWindow Windows[MAX_PLAYERS];      // 0000c
	char          skip1[0x04];               // 3768C
	PlayerInfo    Players[MAX_PLAYERS];      // 37690
	// CExoNetExtendableBuffer FrameStorage; // 3A390
};



C_ASSERT( offsetof( CNetLayerInternal, Players ) == 0x37690 );

#include <poppack.h>




struct CMD
{
	enum MAJORENUM
	{
		ServerStatus              = 0x01,
		Login                     = 0x02,
		Module                    = 0x03,
		Area                      = 0x04,
		GameObjUpdate             = 0x05,
		Input                     = 0x06,
		Store                     = 0x07,
		Gold                      = 0x08,
		Chat                      = 0x09,
		PlayerList                = 0x0A,
		UnusedB                   = 0x0B,
		Inventory                 = 0x0C,
		GuiInventory              = 0x0D,
		Party                     = 0x0E,
		Cheat                     = 0x0F,
		Camera                    = 0x10,
		CharList                  = 0x11,
		ClientSideMessage         = 0x12,
		Combat_Round              = 0x13,
		Dialog                    = 0x14,
		GuiCharacterSheet         = 0x15,
		QuickChat                 = 0x16,
		Sound                     = 0x17,
		Item_Property             = 0x18,
		GuiContainer              = 0x19,
		VoiceChat                 = 0x1A,
		GuiInfoPopup              = 0x1B,
		Journal                   = 0x1C,
		LevelUp                   = 0x1D, // Um?
		GuiHotBar                 = 0x1E,
		DungeonMaster             = 0x1F,
		MapPin                    = 0x20,
		DebugInfo                 = 0x21,
		SafeProjectile            = 0x22,
		Barter                    = 0x23,
		PopUpGUIPanel             = 0x24,
		Death                     = 0x25,
		GroupInput                = 0x26,
		DungeonMasterGroup        = 0x27,
		Ambient                   = 0x28,
		UnknownNameMsg            = 0x29, // They used the function pointer instead of the string in the debug print...
		Portal                    = 0x2A,
		Character_Download        = 0x2B,
		LoadBar                   = 0x2C,
		SaveLoad                  = 0x2D,
		GUIPartyBar               = 0x2E,
		ShutDownServer            = 0x2F,
		LevelUp2                  = 0x30, // Um?
		PlayModuleCharacterList   = 0x31,
		CustomToken               = 0x32,
		Cutscene                  = 0x33,
		GuiRoster                 = 0x34,
		WorldMap                  = 0x35,
		CustomAnim                = 0x36,

		MaximumMajorFunction
	};
};


/*
 * Helper/wrapper functions and hook trampoline functions.
 */

#define _MSGN(n) case CMD::##n: return #n;

inline
const char *
GetMsgName(
	__in UCHAR MsgId
	)
{
	switch (MsgId)
	{

	_MSGN(ServerStatus)
	_MSGN(Login)
	_MSGN(Module)
	_MSGN(Area)
	_MSGN(GameObjUpdate)
	_MSGN(Input)
	_MSGN(Store)
	_MSGN(Gold)
	_MSGN(Chat)
	_MSGN(PlayerList)
	_MSGN(UnusedB)
	_MSGN(Inventory)
	_MSGN(GuiInventory)
	_MSGN(Party)
	_MSGN(Cheat)
	_MSGN(Camera)
	_MSGN(CharList)
	_MSGN(ClientSideMessage)
	_MSGN(Combat_Round)
	_MSGN(Dialog)
	_MSGN(GuiCharacterSheet)
	_MSGN(QuickChat)
	_MSGN(Sound)
	_MSGN(Item_Property)
	_MSGN(GuiContainer)
	_MSGN(VoiceChat)
	_MSGN(GuiInfoPopup)
	_MSGN(Journal)
	_MSGN(LevelUp)
	_MSGN(GuiHotBar)
	_MSGN(DungeonMaster)
	_MSGN(MapPin)
	_MSGN(DebugInfo)
	_MSGN(SafeProjectile)
	_MSGN(Barter)
	_MSGN(PopUpGUIPanel)
	_MSGN(Death)
	_MSGN(GroupInput)
	_MSGN(DungeonMasterGroup)
	_MSGN(Ambient)
	_MSGN(UnknownNameMsg)
	_MSGN(Portal)
	_MSGN(Character_Download)
	_MSGN(LoadBar)
	_MSGN(SaveLoad)
	_MSGN(GUIPartyBar)
	_MSGN(ShutDownServer)
	_MSGN(LevelUp2)
	_MSGN(PlayModuleCharacterList)
	_MSGN(CustomToken)
	_MSGN(Cutscene)
	_MSGN(GuiRoster)
	_MSGN(WorldMap)
	_MSGN(CustomAnim)

	default:
		{
			static char UnkBuf[ 16 ];

			StringCbPrintfA(UnkBuf, sizeof( UnkBuf ), "%02X", MsgId);

			return UnkBuf;
		}
		break;

	}
}

#undef _MSGN

const char *
__stdcall
GetPlayerAccountName(
	__in unsigned long PlayerId
	);

bool
__stdcall
GetPlayerConnectionInfo(
	__in unsigned long PlayerId,
	__out PSOCKADDR_IN Sin
	);

bool
CheckDatagram(
	__in_bcount( DataSize ) const void * Data,
	__in unsigned long DataSize
	);

//#define MASTER_SERVER_HOSTNAME "nw2master.bioware.com"
#define MASTER_SERVER_HOSTNAME "nwn2.mst.valhallalegends.com"
#define MASTER_SERVER_PORT     6121

#endif
