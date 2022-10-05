/***************************************************************************
    NWNX BugFix - NWN2Server bugfixes and patches plugin
    Copyright (C) 2008 Skywing (skywing@valhallalegends.com) 

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
#if !defined(BUGFIX_H_INCLUDED)
#define BUGFIX_H_INCLUDED

#define DLLEXPORT extern "C" __declspec(dllexport)

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0502
#endif

#include "windows.h"
#include <dbghelp.h>
#include <set>

#include "../plugin.h"
#include "../../misc/log.h"
#include "../../nwn2lib/NWN2Lib.h"

#define XP_BUGFIX_GAMEOBJARRAY_HOOK
#define XP_BUGFIX_SERVERAI_USES_POINTERS
#define XP_BUGFIX_LAZY_ITEM_AI_HANDLING
#define XP_BUGFIX_AIUPDATE_THROTTLING
//#define XP_BUGFIX_SSE2_CALC_CONTACT
// Note the below instrument define requires XP_BUGFIX_GAMEOBJARRAY_HOOK and XP_BUGFIX_SERVERAIUSES_POINTERS to be effective!
#define XP_BUGFIX_AIUPDATE_INSTRUMENT
#define XP_BUGFIX_NETLAYER_INSTRUMENT
//#define XP_BUGFIX_GAMEOBJ_CACHE
//#define XP_BUGFIX_GAMEOBJ_CACHE_STATS
//#define XP_BUGFIX_USE_SYMBOLS

//
// AIUpdate throttling needs the below hooks to work.
//

#ifdef XP_BUGFIX_AIUPDATE_THROTTLING
#define XP_BUGFIX_SERVERAI_USES_POINTERS
#define XP_BUGFIX_GAMEOBJARRAY_HOOK
#endif


/*

.text:006FB5EF                 mov     ecx, [esi+28h]
.text:006FB5F2                 mov     eax, [ecx]
.text:006FB5F4
.text:006FB5F4 loc_6FB5F4:                             ; CODE XREF: sub_6FB1E0+41Ej
.text:006FB5F4                 mov     [eax+4], ecx
.text:006FB5F7                 mov     ecx, eax
.text:006FB5F9                 cmp     ecx, [esi+24h]
.text:006FB5FC                 mov     eax, [eax]
.text:006FB5FE                 jnz     short loc_6FB5F4
.text:006FB600
.text:006FB600 loc_6FB600:                             ; CODE XREF: sub_6FB1E0+19j
.text:006FB600                                         ; sub_6FB1E0+3EEj ...
.text:006FB600                 pop     edi

.text:006FB23D                 mov     ecx, [esi+28h]
.text:006FB240                 mov     eax, [ecx]
.text:006FB242
.text:006FB242 loc_6FB242:                             ; CODE XREF: sub_6FB1E0+6Cj
.text:006FB242                 mov     [eax+4], ecx
.text:006FB245                 mov     ecx, eax
.text:006FB247                 cmp     ecx, [esi+24h]
.text:006FB24A                 mov     eax, [eax]
.text:006FB24C                 jnz     short loc_6FB242
.text:006FB24E
.text:006FB24E loc_6FB24E:                             ; CODE XREF: sub_6FB1E0+53j
.text:006FB24E                 pop     ebp
.text:006FB24F                 mov     dword ptr [esi+58h], 2
*/



class BugFix : public Plugin
{
public:
	BugFix();
	~BugFix();

	bool Init(char* nwnxhome);

	int GetInt(char* sFunction, char* sParam1, int nParam2);
	void SetInt(char* sFunction, char* sParam1, int nParam2, int nValue);
	float GetFloat(char* sFunction, char* sParam1, int nParam2) { return 0.0; }
	void SetFloat(char* sFunction, char* sParam1, int nParam2, float fValue) {};
	void SetString(char* sFunction, char* sParam1, int nParam2, char* sValue);
	char* GetString(char* sFunction, char* sParam1, int nParam2);
	void GetFunctionClass(char* fClass);

	static NWN::CNWSPlayer * GetClientObjectByPlayerId(__in NWN::PLAYERID PlayerId);

	static NWN::CGameObject * __fastcall GetGameObject(__in NWN::OBJECTID ObjectId);


	static void CalcPositionsLoop0Fix();
	static void CalcPositionsLoop1Fix();
	static void NullDerefCrash0Fix();
	static void NullDerefCrash1Fix();
	static void NullDerefCrash2Fix();
	static void NullDerefCrash3Fix();
	static void NullDerefCrash4Fix();
	static void Crash5Fix();
	static void NullDerefCrash6Fix();
	static void NullDerefCrash7Fix();
	static void NullDerefCrash8Fix();
	static void NullDerefCrash9Fix();
	static void NullDerefCrash10Fix();
	static void NullDerefCrash11Fix();
	static void NullDerefCrash12Fix();
	static void Uncompress0Fix();
	static void Uncompress1Fix();
	static void CGameEffectDtorLogger();
	static void SendCompressionHook();
	static ULONG64 __cdecl GetHighResolutionTimerFix();
	static void AddInternalObjectHook();
	static void AddObjectAtPosHook();
	static void DeleteAllHook();
	static void GetGameObjectHook();
	static void DeleteHook();
	static void AIMasterUpdateState_GetObjectHook();
	static void AIMasterUpdateState_GetObject2Hook();
	static void SetAreaTransitionBMPHook();
	static void AddItemToAIMasterHook();
	static void AddItemToAIMasterHook2();
	static void ApplyEffectHook();
	static void SetLastUpdateObjectHook();
	static void AIMasterUpdateState_PrologueHook();
	static void AIMasterUpdateState_GetObjectForThrottlingHook();
	static void AddObjectToAreaHook();
	static void RemoveObjectFromAreaHook();
	// static void UpdatePositionInObjectsArrayHook();
	static void UpdatePositionInObjectsArray_SwapUpHook();
	static void UpdatePositionInObjectsArray_SwapDownHook();
	static void SortObjectsForGameObjectUpdate_GetObjectHook();
	static void GetFirstObjectIndiceByX_GetObjectHook();
	static void DeleteLastUpdateObjectsInOtherAreas_Hook();
	static void CLastUpdateObject_DestructorHook();
	static void CreateNewLastUpdateObjectHook();
	static void GetLastUpdateObjectHook();
	static void TestObjectUpdateDifferences_GetLUOHook();
	static void CallNWN2_SurfaceMesh_FindFace_Hook();
	static void Calc_Contact_0(__in NWN::QuickRay *Ray, __in float DistReq, __out float * IntersectDistance, __out NWN::Vector3 * IntersectFaceNormal);
	static void CalcPathFindFaceHook1();
	static void ResolveSafeProjectileItemPatch();

	static int __stdcall sendtoMstHook(__in SOCKET s, __in const char *buf, __in int len, __in int flags, __in struct sockaddr_in *to, __in int tolen);
	static int __stdcall sendtoGlobalHook(__in SOCKET s, __in const char *buf, __in int len, __in int flags, __in struct sockaddr_in *to, __in int tolen);
	static int __stdcall sendtoOut(__in SOCKET s, __in const char *buf, __in int len, __in int flags, __in struct sockaddr_in *to, __in int tolen);
	
	static NWN::CLastUpdateObject * __fastcall GetLastUpdateObject(__in NWN::CNWSPlayer * Player, __in NWN::OBJECTID ObjectId);
	static BOOL __stdcall CheckAreaSurfaceMeshFaceIndex(__in ULONG Width, __in ULONG Height, __in ULONG Index);

private:

	typedef struct _FIND_SERVER_GUI_WINDOW_PARAM
	{
		HWND hwnd;
		ULONG ProcessId;
	} FIND_SERVER_GUI_WINDOW_PARAM, * PFIND_SERVER_GUI_WINDOW_PARAM;

	bool Check();
	static void __stdcall SafeInitPositionList(NWN2::somestruc *struc);
	static void __stdcall LogNullDerefCrash0();
	static void __stdcall LogNullDerefCrash1();
	static void __stdcall LogNullDerefCrash2();
	static void __stdcall LogNullDerefCrash3();
	static void __stdcall LogNullDerefCrash4();
	static void __stdcall LogCrash5();
	static void __stdcall LogNullDerefCrash6();
	static void __stdcall LogNullDerefCrash7();
	static void __stdcall LogNullDerefCrash8();
	static void __stdcall LogNullDerefCrash9();
	static void __stdcall LogNullDerefCrash10();
	static void __stdcall LogNullDerefCrash11();
	static void __stdcall LogNullDerefCrash12();
	static void __stdcall LogUncompress0Fix();
	static void __stdcall LogUncompress1Fix();

	static void __stdcall FreeNwn2Heap(void *p);

	static void __stdcall LogStackTrace(
		__in const CONTEXT * Context,
		__in ULONG_PTR TraceContext
		);

	static void __stdcall HandleAreaTransitionBMP(int LoadScreenId, struct CExoString * LoadScreenName, void * MessageObject);
	static void __fastcall WriteWORD(unsigned short Value, void * MessageObject);
	static void __fastcall WriteCExoString(struct CExoString * Value, void * MessageObject);

	static void __fastcall AddGameObject(__in NWN::OBJECTID ObjectId, __in NWN::CGameObject * Object);
	static void __fastcall AddGameObjectAtPos(__in NWN::CGameObjectArray * GameObjArray, __in NWN::OBJECTID ObjectId, __in NWN::CGameObject * Object);
	static void __fastcall RemoveGameObject(__in NWN::OBJECTID ObjectId);
	static void __fastcall DeleteAllGameObjects();

	static void __fastcall AddObjectToAIMaster(__in NWN::CNWSObject * Object, __in int AILevel);
	static void __fastcall CServerAIMaster_AddObject(__in NWN::CServerAIMaster * AIMaster, __in ULONG Unused, __in NWN::CNWSObject * Object, __in int AILevel);
	static void __fastcall RemoveObjectFromAIMaster(__in NWN::CNWSObject * Object);
	static void __fastcall CServerAIMaster_RemoveObject(__in NWN::CServerAIMaster * AIMaster, __in ULONG Unused, __in NWN::CNWSObject *Object);
	static NWN::CNWSObject * __fastcall CheckForSafeEffect(__in NWN::CNWSObject * Object, __in NWN::CGameEffect * Effect);

	static NWN::CServerAIMaster * __fastcall OnServerAIMaster_UpdateState(__in NWN::CServerAIMaster * AIMasterThis);
	static NWN::CGameObject * __fastcall OnServerAIMaster_UpdateState_PreUpdateObject(__in NWN::CGameObject * Object);
	static void OnServerCreatedObject(__in NWN::CGameObject * Object);
	static int __cdecl SortGameObjectsByAISortingLevel(__in void * Context, __in const void * Object0, __in const void * Object1);

	static void CheckAreaObjectLists(__in NWN::CNWSArea * Area);
	static void __fastcall AddObjectToArea(__in NWN::CNWSArea * Area, __in NWN::CGameObject * Object);
	static void __fastcall RemoveObjectFromArea(__in NWN::CNWSArea * Area, __in NWN::OBJECTID ObjectId);
	static void __fastcall UpdatePositionInObjectsArray_SwapUp(__in NWN::CNWSArea * Area, __in int Index);
	static void __fastcall UpdatePositionInObjectsArray_SwapDown(__in NWN::CNWSArea * Area, __in int Index);
	// static void __fastcall UpdatePositionInObjectsArray(__in NWN::CNWSArea * Area, __in NWN::CGameObject * GameObject);

	static void CheckLUOTable(__in NWN::CNWSPlayer * Player);
	static void __fastcall OnAddLUO(__in NWN::CNWSPlayer * Player, __in NWN::CLastUpdateObject * LUO);
	static void __fastcall OnDeleteLUO(__in NWN::CLastUpdateObject * LUO);

	static BOOL __fastcall ShouldSkipOtherAreaLUODeleteCheck(__in NWN::CGameObject * PlayerCreature);

	static void PrintObjectInfo(__in const char * Header, __in NWN::CGameObject * Object, __in NWN::OBJECTID ObjectId);
	static void PrintObjectLists(__in NWN::CNWSArea * Area);
	static void LogServerDebugInfo();

	static BOOL CALLBACK BugFix::FindServerGuiWindowEnumProc(__in HWND hwnd, __in LPARAM lParam);
	static HWND FindServerGuiWindow();

	static bool __fastcall CallCNetLayerInternal_MessageArrived(__in struct CNetLayerInternal * This, __in void * Unused, __in int ExoProtocol, __in SOCKET Socket, __in int Unk_always_zero, __in int Unk_always_1);
	static NWN::CNWSPlayer * __fastcall GetClientObjectByPlayerId2(__in struct NWN::CServerExoApp * This, __in void * Unused, __in NWN::PLAYERID PlayerId, __in UCHAR ExpansionMask);

	static int __stdcall recvfromHook(__in SOCKET s, __out char *buf, __in int len, __in int flags, __out struct sockaddr *from, __inout_opt int *fromlen);

	bool EnableRecvfromHook();

	ULONG         lastlog;
	HMODULE       nwn2mm;
	LARGE_INTEGER perffreq;
	bool          useGetTickCount;
	ULONG         tickCountDelta;
	bool          verboseLogging;
	static bool   rewriteClientUdpPort;
	static USHORT aiUpdateSortingLevel;
	static ULONG  badPacketCounter;
	static ULONG  aiUpdateThrottle; // Target max time in ms to spend each AIUpdate before throttling engaged.
	static bool   overrideNetRecv;
	static bool   directPollNetRecv;
	static bool   aiUpdateSortPlayersFirst;
	static bool   devMode; // If true, perform diagnostics checks & asserts
	static bool   reflectorEnabled; // If true, reflector is enabled
	static std::string reflectorSecret;

	typedef struct _BAD_PACKET_LOG {
		ULONG IPAddress;
		ULONG Count;
		time_t SeenTime;
	} BAD_PACKET_LOG, *PBAD_PACKET_LOG;

	enum
	{
		OBJARRAY_SIZE = 0x1000000,
		OBJARRAY_MASK = OBJARRAY_SIZE-1,

		LAST_OBJARRAY_CONST,

		GAME_OBJ_CACHE_SIZE = 8,    // Must be a power of 2  (4 was ok)
	};

	struct GameObjCacheEntry
	{
		NWN::OBJECTID ObjectId;
		NWN::CGameObject * Object;
	};

	static GameObjCacheEntry GameObjectCache[ 1 + GAME_OBJ_CACHE_SIZE ]; // [0] is always NWN::INVALIDOBJID -> NULL
	static ULONG GameObjectCacheIndex;
	static ULONG GameObjectCacheHits;
	static ULONG GameObjectCacheMisses;

	static NWN::CGameObjectArrayNode * GameObjectNodes[ OBJARRAY_SIZE ];

	static bool NextGameEffectIsSafe;
	static bool RebaselineAISortingLevel;
	static bool ResortAIListArray;
	static bool RecvfromSuccess;

	static HWND ServerGuiWindow;

	static ULONG SafeGameEffectCount;
	static ULONG UnsafeGameEffectCount;
	static ULONG ItemDynamicAddedToAIListCount;
	static ULONG ThisLoopThrottledObjectCount;
	static ULONG AIUpdateIterationsThrottledCount;
	static ULONG ThrottledObjectCount;
	static ULONG ForcedNPCUpdateObjectCount;
	static ULONG NonThrottledObjectCount;
	static ULONG ThrottleImmuneObjectCount;
	static ULONG AISortingLevelRebaselineCount;
	static ULONG AIListSortCount;
	static ULONG LUOAreaDeletionCheckSkipCount;
	static ULONG LUOAreaDeletionCheckRunCount;

	static ULONG AIUpdateStartTick;
	static ULONG LastRecvfromTick;
	static ULONG LastGoodRecvfromTick;
	static ULONG LastPostFakeRecvIndicationTick;
	static ULONG LastDirectNetRecvTick;

	//
	// Record the last event and update objects for display in the debugger.
	//

	static NWN::CGameObject * AIUpdate_LastEventObject;
	static NWN::CGameObject * AIUpdate_LastUpdateObject;
	static NWN::CServerAIMaster * AIMaster;

#ifdef XP_BUGFIX_USE_SYMBOLS

	class StackTracer* tracer;

#endif

};

VOID TrackPlayerAccountName(__in const char * Name);

#endif
