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

#define STRSAFE_NO_DEPRECATE
#define _CRT_SECURE_NO_WARNINGS


#include "bugfix.h"
#include "..\..\misc\Patch.h"
#include "..\..\misc\IATHook.h"
#include "..\..\misc\ini.h"
#include "StackTracer.h"
#include <strsafe.h>
#include "NetLayer.h"
#include <xmmintrin.h>
#include "Reflector.h"
#include <cassert>
#include <map>

#define BUGFIX_VERSION "1.0.72"
#define __NWN2_VERSION_STR(X) #X
#define _NWN2_VERSION_STR(X) __NWN2_VERSION_STR(X)
#define NWN2_VERSION _NWN2_VERSION_STR(NWN2SERVER_VERSION)

#define BUGFIX_LOG_GAMEOBJACCESS 0

extern bool ReplaceNetLayer();
extern bool EnableTls();

extern bool TlsActive;
extern bool WindowExtensions;

extern int NetLayerHandleBNCS(__in const char * buf, __in int len, __in struct sockaddr_in * from, __in int fromlen);
extern int NetLayerHandleBNVS(__in const char * buf, __in int len, __in struct sockaddr_in * to, __in int tolen);
extern void NetLayerHandleBNVR(__in const char * buf, __in int len, __in struct sockaddr_in * to, __in int tolen);
extern bool NetLayerTlsOnForPlayer(__in unsigned long PlayerId);

/***************************************************************************
    NWNX and DLL specific functions
***************************************************************************/

typedef std::map< std::string, std::string > StringMap;

BugFix* plugin;
bool nocompress = true;
long GameObjUpdateBurstSize = 102400; // 100K
CHAR NWNXHome[ MAX_PATH + 1 ];
bool CanonicalizeAccountNames = true;
StringMap AccountNameMap;
LogNWNX* _logger;

#if defined (_M_IX86)

#undef PreFetchCacheLine
#define PreFetchCacheLine(l, a)  _mm_prefetch((CHAR CONST *) a, l)

#undef PF_TEMPORAL_LEVEL_1
#undef PF_NON_TEMPORAL_LEVEL_ALL

#define PF_TEMPORAL_LEVEL_1 _MM_HINT_T0
#define PF_TEMPORAL_LEVEL_2 _MM_HINT_T1
#define PF_TEMPORAL_LEVEL_3 _MM_HINT_T2
#define PF_NON_TEMPORAL_LEVEL_ALL _MM_HINT_NTA

#endif


typedef int (__stdcall * RecvfromCalloutProc)(__in SOCKET s, __out char *buf, __in int len, __in int flags, __out struct sockaddr *from, __inout_opt int *fromlen);

RecvfromCalloutProc RecvfromCallout;

extern
SOCKET
GetServerNetLayerSocket(
	);

extern
struct CNetLayerInternal *
GetServerNetLayer(
	);
bool
__cdecl
NWN2IntersectRayTri(
	__in const NWN::QuickRay * Ray,
	__in const NWN::Vector3 * Tri0,
	__in const NWN::Vector3 * Tri1,
	__in const NWN::Vector3 * Tri2,
	__out float & T
	);

void
BugFixDisconnectPlayer();

void *sendtoMstHookAddress = BugFix::sendtoMstHook;

const float Cc_PointZeroFive = 0.5f;
const ULONG Cc_maskAbsFloat = 0x7FFFFFFF;

Patch _patches[] =
{
	Patch( OFFS_CalcPositionLoop0, "\xe9", 1 ),
	Patch( OFFS_CalcPositionLoop0+1, (relativefunc)BugFix::CalcPositionsLoop0Fix ),
	Patch( OFFS_CalcPositionLoop1, "\xe9", 1 ),
	Patch( OFFS_CalcPositionLoop1+1, (relativefunc)BugFix::CalcPositionsLoop1Fix ),
#if NWN2SERVER_VERSION < 0x01211549
	Patch( OFFS_NullDerefCrash0, "\xe9", 1 ),
	Patch( OFFS_NullDerefCrash0+1, (relativefunc)BugFix::NullDerefCrash0Fix ),
	Patch( OFFS_NullDerefCrash1, "\xe9", 1 ),
	Patch( OFFS_NullDerefCrash1+1, (relativefunc)BugFix::NullDerefCrash1Fix ),
#endif
	Patch( OFFS_NullDerefCrash2, "\xe9", 1 ),
	Patch( OFFS_NullDerefCrash2+1, (relativefunc)BugFix::NullDerefCrash2Fix ),
#if NWN2SERVER_VERSION < 0x01211549
	Patch( OFFS_NullDerefCrash3, "\xe9", 1 ),
	Patch( OFFS_NullDerefCrash3+1, (relativefunc)BugFix::NullDerefCrash3Fix ),
#endif
#if NWN2SERVER_VERSION < 0x01231763
	Patch( OFFS_NullDerefCrash4, "\xe9", 1 ),
	Patch( OFFS_NullDerefCrash4+1, (relativefunc)BugFix::NullDerefCrash4Fix ),
	Patch( OFFS_Crash5, "\xe9", 1 ),
	Patch( OFFS_Crash5+1, (relativefunc)BugFix::Crash5Fix ),
#endif
	Patch( OFFS_NullDerefCrash6, "\xe9", 1 ),
	Patch( OFFS_NullDerefCrash6+1, (relativefunc)BugFix::NullDerefCrash6Fix ),
	Patch( OFFS_NullDerefCrash7, "\xe9", 1 ),
	Patch( OFFS_NullDerefCrash7+1, (relativefunc)BugFix::NullDerefCrash7Fix ),
	Patch( OFFS_NullDerefCrash8, "\xe9", 1 ),
	Patch( OFFS_NullDerefCrash8+1, (relativefunc)BugFix::NullDerefCrash8Fix ),
	Patch( OFFS_CheckUncompress0, "\xe9", 1 ),
	Patch( OFFS_CheckUncompress0+1, (relativefunc)BugFix::Uncompress0Fix ),
	Patch( OFFS_CheckUncompress1, "\xe9", 1 ),
	Patch( OFFS_CheckUncompress1+1, (relativefunc)BugFix::Uncompress1Fix ),
	Patch( OFFS_NullDerefCrash9, "\xe9", 1 ),
	Patch( OFFS_NullDerefCrash9+1, (relativefunc)BugFix::NullDerefCrash9Fix ),
#if NWN2SERVER_VERSION >= 0x01211549
	Patch( OFFS_NullDerefCrash10, "\xe9", 1 ),
	Patch( OFFS_NullDerefCrash10+1, (relativefunc)BugFix::NullDerefCrash10Fix ),
#endif
#if NWN2SERVER_VERSION == 0x01211549 && defined(XP_BUGFIX_USE_SYMBOLS)
	Patch( OFFS_CGameEffectDtor, "\xe9", 1 ),
	Patch( OFFS_CGameEffectDtor+1, (relativefunc)BugFix::CGameEffectDtorLogger ),
#endif
#if NWN2SERVER_VERSION >= 0x01231763
	Patch( OFFS_NullDerefCrash11, "\xe9", 1 ),
	Patch( OFFS_NullDerefCrash11+1, (relativefunc)BugFix::NullDerefCrash11Fix ),
	Patch( OFFS_SendCompressionHook, "\xe9", 1 ),
	Patch( OFFS_SendCompressionHook+1, (relativefunc)BugFix::SendCompressionHook ),
	Patch( OFFS_NullDerefCrash12, "\xe9", 1 ),
	Patch( OFFS_NullDerefCrash12+1, (relativefunc)BugFix::NullDerefCrash12Fix ),
//	Patch( OFFS_GetHighResolutionTimer,  "\xe9", 1 ),
//	Patch( OFFS_GetHighResolutionTimer+1, (relativefunc)BugFix::GetHighResolutionTimerFix ),

	//
	// The following patches replace the game object table with an internal
	// version.
	//

#ifdef XP_BUGFIX_GAMEOBJARRAY_HOOK

	Patch( OFFS_ObjArrayAddInternalObject, "\xe9", 1 ),
	Patch( OFFS_ObjArrayAddInternalObject+1, (relativefunc)BugFix::AddInternalObjectHook ),
	Patch( OFFS_ObjArrayAddObjectAtPos, "\xe9", 1 ),
	Patch( OFFS_ObjArrayAddObjectAtPos+1, (relativefunc)BugFix::AddObjectAtPosHook ),
	Patch( OFFS_ObjArrayDeleteAll, "\xe9", 1 ),
	Patch( OFFS_ObjArrayDeleteAll+1, (relativefunc)BugFix::DeleteAllHook ),
	Patch( OFFS_ObjArrayGetGameObject, "\xe9", 1 ),
	Patch( OFFS_ObjArrayGetGameObject+1, (relativefunc)BugFix::GetGameObjectHook ),
	Patch( OFFS_ObjArrayDelete, "\xe9", 1 ),
	Patch( OFFS_ObjArrayDelete+1, (relativefunc)BugFix::DeleteHook ),
	Patch( OFFS_AIMasterUpdateStateGetObj, "\xe9", 1 ), // Note this is overriden later if XP_BUGFIX_SERVERAI_USES_POINTERS or XP_BUGFIX_AIUPDATE_THROTTLING
	Patch( OFFS_AIMasterUpdateStateGetObj+1, (relativefunc)BugFix::AIMasterUpdateState_GetObjectHook ),
	Patch( OFFS_AIMasterUpdateStateGetOb2, "\xe9", 1 ),
	Patch( OFFS_AIMasterUpdateStateGetOb2+1, (relativefunc)BugFix::AIMasterUpdateState_GetObject2Hook ),

#endif

	//
	// The following patches store object pointers instead of object IDs in the
	// CServerAIMaster list.
	//
	// Note these patches must be declared AFTER XP_BUGFIX_GAMEOBJARRAY_HOOK
	// patches !!!
	//

#ifdef XP_BUGFIX_SERVERAI_USES_POINTERS

	Patch( OFFS_CServerAIMaster_AddObjectPtrPatch, "\x90\x90\x90\x90\x90\x90", 6 ),
	Patch( OFFS_CServerAIMaster_RemoveObjectPtrPatch, "\x89\xf0\x90\x90\x90\x90", 6 ), // mov eax, esi ; nop
	Patch( OFFS_AIMasterUpdateStateGetObj, "\xe9", 1 ),
//	Patch( OFFS_AIMasterUpdateStateGetObj+1, (relativefunc)OFFS_AIMasterUpdateStateGotObj ), // The below is better, skips some more useless work
	Patch( OFFS_AIMasterUpdateStateGetObj+1, (relativefunc)OFFS_AIMasterUpdateStateGotOba ), // Note this is overridden later if XP_BUGFIX_AIUPDATE_THROTTLING
	Patch( OFFS_CServerAIMaster_UpdateStatePostProcessGetObj, "\xe9", 1 ),
	Patch( OFFS_CServerAIMaster_UpdateStatePostProcessGetObj+1, (relativefunc)OFFS_CServerAIMaster_UpdateStatePostProcessGotObj ),

	//
	// These are for the CServerAIEventNode list (m_lEventQueue) and not the
	// object list.
	//

//	Patch( OFFS_AIMasterUpdateStateGetOb2, "\x89\xc7\xe9", 3 ), // mov edi, eax ; jmp
//	Patch( OFFS_AIMasterUpdateStateGetOb2+3, (relativefunc)OFFS_AIMasterUpdateStateGotOb2 ),

	//
	// Debugger instrumentation to save away the last object that is being
	// updated for display in the debugger.
	//

#ifdef XP_BUGFIX_AIUPDATE_INSTRUMENT
	Patch( OFFS_AIMasterUpdateStateGetObj+1, (relativefunc)BugFix::SetLastUpdateObjectHook ),
#endif

#endif

	Patch( OFFS_TransitionBMPFixPatch, (relativefunc)BugFix::SetAreaTransitionBMPHook ),
	Patch( OFFS_CExoNetInternal_SendMessageToMst, (absolutefunc)&sendtoMstHookAddress ),
	Patch( OFFS_NWN2_Pathfinder_Dtor_PatchJmp, "\x90\x90", 2),
	Patch( OFFS_NWN2_Pathfinder_ClearPath_PatchJmp, "\x90\x90", 2),

	//
	// The following skip some redundant game object array accesses in very hot
	// code paths, for improved performance.
	//

	Patch( OFFS_CServerExoAppInternal_GetClientObjectByClientId_Patch, "\x39\x5e\x34\x74\x1f", 5 ), // cmp dword ptr [esi+34h], ebx ; jz <Found>
	Patch( OFFS_CServerExoAppInternal_GetClientObjectByClientId_Patch+5, "\x39\x5e\x44\x74\x1a", 5 ), // cmp dword ptr [esi+44h], ebx ; jz <Found>
	Patch( OFFS_CServerExoAppInternal_GetClientObjectByClientId_Patch+10, "\xe9", 1 ),
	Patch( OFFS_CServerExoAppInternal_GetClientObjectByClientId_Patch+11, (relativefunc)OFFS_CServerExoAppInternal_GetClientObjectByClientId_CheckNext ),


//	Patch( OFFS_CServerExoAppInternal_GetClientObjectByClientId_RedundantGetObj, "\x90\x90\x90\x90\x90\x90\x90", 7 ),
	Patch( OFFS_CNWSCreatureStats_HasFeat_PatchJmp, "\x8b\x5c\x24\x10\xe9", 5 ), // mov ebx, dword ptr [esp+10h] ; jmp <real check>
	Patch( OFFS_CNWSCreatureStats_HasFeat_PatchJmp+5, (relativefunc)OFFS_CNWSCreatureStats_HasFeat_RealCheck ),

#ifdef XP_BUGFIX_LAZY_ITEM_AI_HANDLING

	//
	// Changes to only place an item in the AIMaster processuing list on the
	// first change requiring effect processing.
	//

	Patch( OFFS_CNWSItem_CNWSItem_NopAddObject, "\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90", 14 ),
	Patch( OFFS_CNWSObject_ApplyEffect_PatchAddItemToAIMaster, "\xe9", 1 ),
	Patch( OFFS_CNWSObject_ApplyEffect_PatchAddItemToAIMaster+1, (relativefunc)BugFix::AddItemToAIMasterHook ),
	Patch( OFFS_CNWSObject_UpdateEffectListEx_PatchAddItemToAIMaster, "\xe9", 1 ),
	Patch( OFFS_CNWSObject_UpdateEffectListEx_PatchAddItemToAIMaster+1, (relativefunc)BugFix::AddItemToAIMasterHook2 ),
	Patch( OFFS_CNWSObject_ApplyEffect, "\xe9", 1 ),
	Patch( OFFS_CNWSObject_ApplyEffect+1, (relativefunc)BugFix::ApplyEffectHook ),

#endif

#ifdef XP_BUGFIX_AIUPDATE_THROTTLING

	Patch( OFFS_CServerExoApp_Internal_MainLook_CallAIUpdate+1, (relativefunc)BugFix::AIMasterUpdateState_PrologueHook ),
	Patch( OFFS_AIMasterUpdateStateGetObj, "\xe9", 1 ),
	Patch( OFFS_AIMasterUpdateStateGetObj+1, (relativefunc)BugFix::AIMasterUpdateState_GetObjectForThrottlingHook ),

#endif

	//
	// The following are to track a list of object pointers (in addition to object
	// IDs) attached to each area.  Assumes that the game object array hooks
	// are in place.  The purpose of these hooks is to accelerate pointer
	// chasing code in hot code paths like last update object computation or
	// get object by position.
	//

	Patch( OFFS_CNWSArea_AddObjectToArea, "\xe9", 1 ),
	Patch( OFFS_CNWSArea_AddObjectToArea+1, (relativefunc)BugFix::AddObjectToAreaHook ),
	Patch( OFFS_CNWSArea_RemoveObjectFromArea, "\xe9", 1 ),
	Patch( OFFS_CNWSArea_RemoveObjectFromArea+1, (relativefunc)BugFix::RemoveObjectFromAreaHook ),
	Patch( OFFS_CNWSArea_UpdatePositionInObjectsArray_SwapUp, "\xe9", 1 ),
	Patch( OFFS_CNWSArea_UpdatePositionInObjectsArray_SwapUp+1, (relativefunc)BugFix::UpdatePositionInObjectsArray_SwapUpHook ),
	Patch( OFFS_CNWSArea_UpdatePositionInObjectsArray_SwapDown, "\xe9", 1 ),
	Patch( OFFS_CNWSArea_UpdatePositionInObjectsArray_SwapDown+1, (relativefunc)BugFix::UpdatePositionInObjectsArray_SwapDownHook ),
//	Patch( OFFS_CNWSArea_UpdatePositionInObjectsArray, "\xe9", 1 ),
//	Patch( OFFS_CNWSArea_UpdatePositionInObjectsArray+1, (relativefunc)BugFix::UpdatePositionInObjectsArrayHook ),

	//
	// The following patch is used to optimize walking the list of objects in
	// an area for last update object computation.  Instead of walking an array
	// of object IDs and resolving each to game object pointers, walk an array
	// of game object pointers and return each in turn.
	//

	Patch( OFFS_CNWSMessage_SortObjectsForGameObjectUpdate_GetObj, "\xe9", 1 ),
	Patch( OFFS_CNWSMessage_SortObjectsForGameObjectUpdate_GetObj+1, (relativefunc)BugFix::SortObjectsForGameObjectUpdate_GetObjectHook ),

	//
	// The following patch does the same for CNWSArea::GetFirstObjectIndiceByX.
	//

	Patch( OFSS_CNWSArea_GetFirstObjectIndiceByX_SetArrayOffset, (absolutefunc)(ArGameObjectBaseClassOffset+GoArObjList) ),
	Patch( OFFS_CNWSArea_GetFirstObjectIndiceByX_GetObj, "\xe9", 1 ),
	Patch( OFFS_CNWSArea_GetFirstObjectIndiceByX_GetObj+1, (relativefunc)BugFix::GetFirstObjectIndiceByX_GetObjectHook ),

	//
	// The following patch eliminates most calls to
	// CNWSMessage::DeleteLastUpdateObjectsInOtherAreas, except when there is
	// actually an object that has been removed from the area and a deletion
	// check is required.
	//

	Patch( OFFS_CNWSMessage_DeleteLastUpdateObjectsInOtherAreas_Hook, "\xe9", 1 ),
	Patch( OFFS_CNWSMessage_DeleteLastUpdateObjectsInOtherAreas_Hook+1, (relativefunc)BugFix::DeleteLastUpdateObjectsInOtherAreas_Hook ),

	//
	// The following patches maintain a fast lookup table of NWN::OBJECTID to
	// CLastUpdateObject.
	//

	Patch( OFFS_CLastUpdateObject_Destructor, "\xe9", 1 ),
	Patch( OFFS_CLastUpdateObject_Destructor+1, (relativefunc)BugFix::CLastUpdateObject_DestructorHook ),
	Patch( OFFS_CNWSMessage_CreateNewLastUpdateObject, "\xe9", 1 ),
	Patch( OFFS_CNWSMessage_CreateNewLastUpdateObject+1, (relativefunc)BugFix::CreateNewLastUpdateObjectHook ),

	//
	// The following patch adds a fast lookup routine for NWN::OBJECTID to
	// CLastUpdateObject.
	//

	Patch( OFFS_CNWSPlayer_GetLastUpdateObject, "\xe9", 1 ),
	Patch( OFFS_CNWSPlayer_GetLastUpdateObject+1, (relativefunc)BugFix::GetLastUpdateObjectHook ),

	//
	// The following patch adds a fast lookup path for NWN::OBJECTID to LUO for
	// CNWSMessage::TestObjectUpdateDifferences.
	//

	Patch( OFFS_CNWSMessage_TestObjectUpdateDifferences_GetLUO, "\xe9", 1 ),
	Patch( OFFS_CNWSMessage_TestObjectUpdateDifferences_GetLUO+1, (relativefunc)BugFix::TestObjectUpdateDifferences_GetLUOHook ),

#ifdef XP_BUGFIX_SSE2_CALC_CONTACT

	//
	// The following patch replaces the x87 FPU version of
	// NWN2_TileSurfaceMesh::Calc_Contact<0> with an SSE2 version.
	//

	Patch( OFFS_NWN2_TileSurfaceMesh_Calc_Contact_0, "\x83\xec\x0c\xf3\x0f\x10\x44\x24\x14\x33\xc0\x56\x8b\xf1\x39\x86\x94\x00\x00\x00\xf3\x0f\x11\x44\x24\x04\x88\x44\x24\x18\x89\x44\x24\x08\x0f\x8e\xcb\x02\x00\x00\x0f\x57\xff\x53\x8b\x5c\x24\x24\x55\x57\x33\xff\xeb\x0a\x8d\xa4\x24\x00\x00\x00\x00\x8d\x49\x00\x8b\x86\xa0\x00\x00\x00\x8b\x14\x38\x8b\x8e\x98\x00\x00\x00\x03\xc7\x8d\x14\x52\x8d\x2c\x91\x8b\x50\x04\x8b\x40\x08\x8d\x14\x52\xf3\x0f\x10\x54\x91\x08\x8d\x04\x40\xf3\x0f\x10\x44\x81\x08\x0f\x2f\xc2\x8d\x14\x91\x8d\x04\x81\x76\x05\x0f\x28\xca\xeb\x03\x0f\x28\xc8\xf3\x0f\x10\x75\x08\x0f\x2f\xce\x76\x05\x0f\x28\xce\xeb\x0d\x0f\x2f\xc2\x76\x05\x0f\x28\xca\xeb\x03\x0f\x28\xc8\x0f\x2f\xd0\x8b\x4c\x24\x20\xf3\x0f\x10\x59\x08\xf3\x0f\x10\x61\x40\xf3\x0f\x5c\xcb\xf3\x0f\x59\xcc\x76\x05\x0f\x28\xea\xeb\x03\x0f\x28\xe8\x0f\x2f\xf5\x76\x05\x0f\x28\xc6\xeb\x08\x0f\x2f\xd0\x76\x03\x0f\x28\xc2\x0f\x2f\xf9\xf3\x0f\x5c\xc3\xf3\x0f\x59\xc4\x76\x09\x0f\x2f\xf8\x0f\x87\xe9\x01\x00\x00\xf3\x0f\x10\x54\x24\x10\x0f\x2f\xca\x76\x09\x0f\x2f\xc2\x0f\x87\xd5\x01\x00\x00\xf3\x0f\x10\x12\xf3\x0f\x10\x00\x0f\x2f\xc2\x76\x05\x0f\x28\xca\xeb\x03\x0f\x28\xc8\xf3\x0f\x10\x75\x00\x0f\x2f\xce\x76\x05\x0f\x28\xce\xeb\x0d\x0f\x2f\xc2\x76\x05\x0f\x28\xca\xeb\x03\x0f\x28\xc8\x0f\x2f\xd0\xf3\x0f\x10\x19\xf3\x0f\x10\x61\x38\xf3\x0f\x5c\xcb\xf3\x0f\x59\xcc\x76\x05\x0f\x28\xea\xeb\x03\x0f\x28\xe8\x0f\x2f\xf5\x76\x05\x0f\x28\xc6\xeb\x08\x0f\x2f\xd0\x76\x03\x0f\x28\xc2\x0f\x2f\xf9\xf3\x0f\x5c\xc3\xf3\x0f\x59\xc4\xf3\x0f\x11\x44\x24\x2c\x76\x09\x0f\x2f\xf8\x0f\x87\x58\x01\x00\x00\xf3\x0f\x10\x54\x24\x10\x0f\x2f\xca\x76\x09\x0f\x2f\xc2\x0f\x87\x44\x01\x00\x00\xf3\x0f\x10\x62\x04\xf3\x0f\x10\x58\x04\x0f\x2f\xdc\x0f\x28\xd0\xf3\x0f\x58\xd1\xf3\x0f\x59\x15\x0c\x9e\x94\x00\x76\x05\x0f\x28\xc4\xeb\x03\x0f\x28\xc3\xf3\x0f\x10\x7d\x04\x0f\x2f\xc7\x76\x05\x0f\x28\xc7\xeb\x0d\x0f\x2f\xdc\x76\x05\x0f\x28\xc4\xeb\x03\x0f\x28\xc3\x0f\x2f\xe3\xf3\x0f\x10\x69\x04\xf3\x0f\x10\x71\x3c\xf3\x0f\x5c\xc5\xf3\x0f\x59\xc6\x76\x05\x0f\x28\xf4\xeb\x03\x0f\x28\xf3\x0f\x2f\xfe\x76\x05\x0f\x28\xdf\xeb\x08\x0f\x2f\xe3\x76\x03\x0f\x28\xdc\x0f\x57\xff\x0f\x2f\xf8\xf3\x0f\x10\x61\x3c\xf3\x0f\x5c\xdd\xf3\x0f\x59\xdc\x76\x09\x0f\x2f\xfb\x0f\x87\xb3\x00\x00\x00\xf3\x0f\x10\x64\x24\x10\x0f\x2f\xc4\x76\x09\x0f\x2f\xdc\x0f\x87\x9f\x00\x00\x00\xf3\x0f\x10\x2d\x0c\x9e\x94\x00\xf3\x0f\x5c\x4c\x24\x2c\x0f\x28\xe3\xf3\x0f\x58\xe0\xf3\x0f\x59\xe5\xf3\x0f\x5c\xd4\xf3\x0f\x10\x25\x40\x35\x95\x00\xf3\x0f\x5c\xc3\xf3\x0f\x59\xcd\xf3\x0f\x59\xc5\x0f\x54\xcc\x0f\x54\xc4\x0f\x54\xd4\xf3\x0f\x58\xc8\x0f\x2f\xd1\x77\x5c\x8d\x4c\x24\x18\x51\x50\x52\x8b\x54\x24\x2c\x55\x52\xe8\xd8\x5a\xf8\xff\x0f\x57\xff\x83\xc4\x14\x84\xc0\x74\x40\xf3\x0f\x10\x44\x24\x18\xf3\x0f\x10\x4c\x24\x10\x0f\x2f\xc8\x76\x2f\x8b\x44\x24\x28\xf3\x0f\x11\x00\x8b\x8e\xa0\x00\x00\x00\xd9\x44\x0f\x2c\x8d\x44\x0f\x2c\xd9\x1b\xc6\x44\x24\x24\x01\xd9\x40\x04\xf3\x0f\x11\x44\x24\x10\xd9\x5b\x04\xd9\x40\x08\xd9\x5b\x08\x8b\x44\x24\x14\x83\xc0\x01\x83\xc7\x40\x3b\x86\x94\x00\x00\x00\x89\x44\x24\x14\x0f\x8c\x54\xfd\xff\xff\x8a\x44\x24\x24\x5f\x5d\x5b\x5e\x83\xc4\x0c\xc2\x10\x00", 0x2fa ),
	Patch( OFFS_NWN2_TileSurfaceMesh_Calc_Contact_0+0x1a6, (absolutefunc)&Cc_PointZeroFive ),
	Patch( OFFS_NWN2_TileSurfaceMesh_Calc_Contact_0+0x237, (absolutefunc)&Cc_PointZeroFive ),
	Patch( OFFS_NWN2_TileSurfaceMesh_Calc_Contact_0+0x254, (absolutefunc)&Cc_maskAbsFloat ),
	Patch( OFFS_NWN2_TileSurfaceMesh_Calc_Contact_0+0x284, (relativefunc)NWN2IntersectRayTri ),

#endif

	//
	// The following patches hash tables to properly support hashing pointers
	// that are above 2GB without causing the hash table index to go negative.
	//

#ifdef OFFS_NWN2_HashTable_Hash1_Divide
    Patch(OFFS_NWN2_HashTable_Hash1_Divide, "\x31\xd2\xf7\x71\x04\x8b\xc2\xc2\x04\x00", 10),
#endif

#ifdef OFFS_NWN2_HashTable_Hash2_Divide
    Patch(OFFS_NWN2_HashTable_Hash2_Divide, "\x31\xd2\xf7\x71\x04\x8b\xc2\xc2\x04\x00", 10),
#endif

#ifdef OFFS_NWN2_HashTable_Hash3_Divide
    Patch(OFFS_NWN2_HashTable_Hash3_Divide, "\x31\xd2\xf7\x71\x04\x8b\xc2\xc2\x04\x00", 10),
#endif

#ifdef OFFS_NWN2_AreaSurfaceMesh_FindFace_CallFindFace
	Patch(OFFS_NWN2_AreaSurfaceMesh_FindFace_CallFindFace+1, (relativefunc)BugFix::CallNWN2_SurfaceMesh_FindFace_Hook),
		// Patch +1 relativefunc FindFaceHook
#endif

#ifdef OFFS_NWN2_AreaSurfaceMesh_IsValid_CallFindFace
	Patch(OFFS_NWN2_AreaSurfaceMesh_IsValid_CallFindFace+1, (relativefunc)BugFix::CallNWN2_SurfaceMesh_FindFace_Hook),
			// Patch +1 relativefunc FindFaceHook
#endif

#ifdef OFFS_NWN2_Pathfinder_CalcPath2_FindFaceHook1
	Patch(OFFS_NWN2_Pathfinder_CalcPath2_FindFaceHook1, "\xe9", 1),
	Patch(OFFS_NWN2_Pathfinder_CalcPath2_FindFaceHook1+1, (relativefunc)BugFix::CalcPathFindFaceHook1),
#endif

#ifdef OFFS_NWN2_CNWSCreature_ResolveSafeProjectileItemPatch
	Patch(OFFS_NWN2_CNWSCreature_ResolveSafeProjectileItemPatch, "\xe9", 1),
	Patch(OFFS_NWN2_CNWSCreature_ResolveSafeProjectileItemPatch+1, (relativefunc)BugFix::ResolveSafeProjectileItemPatch),
#endif

#ifdef OFFS_CNWSCreature_UnpossessFamiliar_DisableAutoMapCopy
	Patch(OFFS_CNWSCreature_UnpossessFamiliar_DisableAutoMapCopy, "\x90\xe9", 2),
#endif

#ifdef OFFS_CNWSMessage_HandlePlayerToServerInputMessage_RestPatch1
	Patch(OFFS_CNWSMessage_HandlePlayerToServerInputMessage_RestPatch1+1, "\x24", 1),
#endif

#ifdef OFFS_CNWSMessage_WriteGameObjUpdate_PlayerUpdate_KnownSpellsToRemove_Patch1
	//
	// Convert 8-bit loop counters to 32-bit loop counters.
	//

	// xor bl, bl -> xor ebx, ebx
	Patch(OFFS_CNWSMessage_WriteGameObjUpdate_PlayerUpdate_KnownSpellsToRemove_Patch1, "\x31\xdb", 2),
	// add bl, 1 -> inc ebx; nop; nop
	Patch(OFFS_CNWSMessage_WriteGameObjUpdate_PlayerUpdate_KnownSpellsToRemove_Patch2, "\x43\x90\x90", 3),
	// movzx eax, bl -> mov eax, ebx ; nop
	Patch(OFFS_CNWSMessage_WriteGameObjUpdate_PlayerUpdate_KnownSpellsToRemove_Patch3, "\x89\xd8\x90", 3),

	// xor bl, bl -> xor ebx, ebx
	Patch(OFFS_CNWSMessage_WriteGameObjUpdate_PlayerUpdate_KnownSpellsToAdd_Patch1, "\x31\xdb", 2),
	// add bl, 1 -> inc ebx; nop; nop
	Patch(OFFS_CNWSMessage_WriteGameObjUpdate_PlayerUpdate_KnownSpellsToAdd_Patch2, "\x43\x90\x90", 3),
	// movzx eax, bl -> mov eax, ebx ; nop
	Patch(OFFS_CNWSMessage_WriteGameObjUpdate_PlayerUpdate_KnownSpellsToAdd_Patch3, "\x89\xd8\x90", 3),

	// xor bl, bl -> xor ebx, ebx
	Patch(OFFS_CNWSMessage_WriteGameObjUpdate_PlayerUpdate_SlotsToRemove_Patch1, "\x31\xdb", 2),
	// add bl, 1 -> inc ebx; nop; nop
	Patch(OFFS_CNWSMessage_WriteGameObjUpdate_PlayerUpdate_SlotsToRemove_Patch2, "\x43\x90\x90", 3),
	// movzx edi, bl -> mov edi, ebx ; nop
	Patch(OFFS_CNWSMessage_WriteGameObjUpdate_PlayerUpdate_SlotsToRemove_Patch3, "\x89\xdf\x90", 3),

	// xor bl, bl -> xor ebx, ebx
	Patch(OFFS_CNWSMessage_WriteGameObjUpdate_PlayerUpdate_SlotsToAdd_Patch1, "\x31\xdb", 2),
	// add bl, 1 -> inc ebx; nop; nop
	Patch(OFFS_CNWSMessage_WriteGameObjUpdate_PlayerUpdate_SlotsToAdd_Patch2, "\x43\x90\x90", 3),
	// movzx edi, bl -> mov edi, ebx ; nop
	Patch(OFFS_CNWSMessage_WriteGameObjUpdate_PlayerUpdate_SlotsToAdd_Patch3, "\x89\xdf\x90", 3),
#endif

#ifdef OFFS_DisconnectPlayer
	Patch(OFFS_DisconnectPlayer, "\xe9", 1),
	Patch(OFFS_DisconnectPlayer+1, (relativefunc)BugFixDisconnectPlayer),
#endif

#endif

	Patch()
};

Patch *patches = _patches;

typedef void (__cdecl * NWN2Heap_Deallocate_Proc)(void *p);

NWN2Heap_Deallocate_Proc NWN2Heap_Deallocate;

namespace NWN
{
	NWN::CAppManager *& g_pAppManager = *(NWN::CAppManager **) OFFS_g_pAppManager;
}

DLLEXPORT Plugin* GetPluginPointerV2()
{
	return plugin;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		plugin = new BugFix();

		char szPath[MAX_PATH];
		GetModuleFileNameA(hModule, szPath, MAX_PATH);
		plugin->SetPluginFullPath(szPath);
	}
	else if (ul_reason_for_call == DLL_PROCESS_DETACH)
	{
		delete plugin;
	}
    return TRUE;
}


/***************************************************************************
    Implementation of BugFix Plugin
***************************************************************************/

BugFix::BugFix()
{
	description = "This plugin fixes some known bugs in nwn2server.";
	header = "NWNX BugFix Plugin V" BUGFIX_VERSION "\n" \
	         "(c) 2008-2012 by Skywing" \
	         "Visit NWNX at: http://www.nwnx.org" \
	         "Built for NWN2 version " NWN2_VERSION "\n";

	subClass = "BUGFIX";
	version  = BUGFIX_VERSION;

	lastlog  = GetTickCount();

	if (!QueryPerformanceFrequency( &perffreq ))
		perffreq.QuadPart = 1000;
	else
		perffreq.QuadPart /= 1000; // Split the difference

	useGetTickCount = false;
	verboseLogging = false;
	rewriteClientUdpPort = true;

	//
	// If we started the server while near to tick count wraparound, then back
	// us up a bit so as to avoid tripping wraparound problems with the server.
	//

	if (GetTickCount() >= 0xD0000000)
		tickCountDelta = 0xD0000000;
	else
		tickCountDelta = 0x00000000;

#ifdef XP_BUGFIX_GAMEOBJ_CACHE

	ZeroMemory( GameObjectCache, sizeof( GameObjectCache ) );

	for (ULONG Index = 0; Index < 1 + GAME_OBJ_CACHE_SIZE; Index += 1)
	{
		GameObjectCache[ Index ].ObjectId = NWN::INVALIDOBJID;
		GameObjectCache[ Index ].Object = NULL;
	}

#endif
}

BugFix::~BugFix()
{
	logger->Info("* Plugin unloaded.");
}

bool BugFix::Init(char* nwnxhome)
{
	bool DoReplaceNetLayer;
	long ThrottleValue;
	bool DisableCharacterCreation;
	bool DoEnableTls;
	bool DoWindowExtensions;
	long InitialAreaAABBCount;
	float FacingPrecision;
	long InitialAttachmentPointCount;
	std::string FacingPrecisionStr;
	std::string FacingPrecisionStrDefault = "0.999";
	std::string Secret = "";

	StringCbCopyA( NWNXHome, sizeof( NWNXHome ), nwnxhome );

	assert(GetPluginFileName());

	/* Log file */
	std::string logfile(nwnxhome);
	logfile.append("\\");
	logfile.append(GetPluginFileName());
	logfile.append(".txt");
	logger = new LogNWNX(logfile);
	_logger = logger;
	logger->Info(header.c_str());


	if (!Check())
	{
		logger->Info(  "* Wrong nwn2server version, patches not active."  );
		return true;
	}


	nwn2mm   = GetModuleHandleA("NWN2_MemoryMgr_amdxp.dll");

	if (nwn2mm)
		NWN2Heap_Deallocate = (NWN2Heap_Deallocate_Proc)GetProcAddress( nwn2mm, "?Deallocate@NWN2_Heap@@SAXPAX@Z" );

	if (!NWN2Heap_Deallocate)
		logger->Info(  "* WARNING: Failed to locate NWN2_Heap::Deallocate."  );

	int i = 0;
	while(patches[i].Apply()) {
		i++;
	}

	logger->Info("* Plugin initialized.");

	/* Ini file */
	std::string inifile(nwnxhome);
	inifile.append("\\");
	inifile.append(GetPluginFileName());
	inifile.append(".ini");

	logger->Info("* Reading inifile %s", inifile.c_str());

	SimpleIniConfig config(inifile);

	nocompress = false;

	config.Read( "DisableServerCompression", &nocompress, false );

	config.Read( "GameObjUpdateBurstSize", &GameObjUpdateBurstSize, GameObjUpdateBurstSize );

	config.Read( "CanonicalizeAccountNames", &CanonicalizeAccountNames, CanonicalizeAccountNames );

#ifdef XP_BUGFIX_AIUPDATE_THROTTLING
	config.Read( "AIUpdateThrottle", &ThrottleValue, 0L );
	aiUpdateThrottle = (ULONG) ThrottleValue;

	if (ThrottleValue)
	{
		logger->Info("* Throttling AIUpdate per main loop iteration if more than %lums is spent in AIUpdate processing for a single update cycle.", aiUpdateThrottle);
	}
#endif

	config.Read( "OverrideNetRecv", &overrideNetRecv, false );
	config.Read( "DirectPollNetRecv", &directPollNetRecv, false );
	config.Read( "AIUpdateSortPlayersFirst", &aiUpdateSortPlayersFirst, false );
	config.Read( "DevMode", &devMode, false );
	config.Read( "ReflectorSecret", &reflectorSecret, Secret );

	if (overrideNetRecv)
	{
		logger->Info("* Forcing server to poll for network data every main loop iteration via fake posted message.");
	}
	else if (directPollNetRecv)
	{
		logger->Info("* Forcing server to poll for network data every main loop iteration via direct call to CNetLayer::MessageArrived.");
	}

	if (nocompress)
	{
		logger->Info("* Disabling server to client compression.");
	}

	if (reflectorSecret.size() != 0)
	{
		logger->Info("* Reflector enabled with secret=%s", reflectorSecret.c_str( ) );
		reflectorEnabled = true;

		ReflectorSetTargetSecret(reflectorSecret.c_str());
	}

	config.Read( "ReplaceNetLayer", &DoReplaceNetLayer, true );
	config.Read( "EnableTls", &DoEnableTls, true );
	config.Read( "UseGetTickCount", &useGetTickCount, false );
	config.Read( "VerboseLogging", &verboseLogging, false );
	config.Read( "RewriteClientUdpPort", &rewriteClientUdpPort, true );
	config.Read( "DisableCharacterCreation", &DisableCharacterCreation, false );
	config.Read( "WindowExtensions", &DoWindowExtensions, true );
	config.Read( "FacingPrecision", &FacingPrecisionStr, FacingPrecisionStrDefault );
	config.Read( "InitialAreaAABBCount", &InitialAreaAABBCount, 100L );
	config.Read( "InitialAttachmentPointCount", &InitialAttachmentPointCount, 1L );

	FacingPrecision = atof(FacingPrecisionStr.c_str());

	{
		logger->Info("* Setting FacingPrecision %g (stock server default is 0.95).", FacingPrecision);

		Patch p( OFFS_CLastUpdateObjectFacingFactor_Float, (char *)&FacingPrecision, 4 );
		
		p.Apply( );
	}

	if (InitialAreaAABBCount != 0)
	{
		logger->Info("* Setting InitialAreaAABBCount to %lu (stock server default is 2000).", InitialAreaAABBCount);
		Patch p( OFFS_CNWSArea_CNWSArea_InitialPatchAABBCount, (char*)&InitialAreaAABBCount, 4 );

		p.Apply( );
	}

	if (InitialAttachmentPointCount != 0)
	{
		if (InitialAttachmentPointCount < 1 || InitialAttachmentPointCount > 15)
			InitialAttachmentPointCount = 16;

		logger->Info("* Setting InitialAttachmentPointCount to %lu (stock server default is 16).", InitialAttachmentPointCount);
		Patch p0( OFFS_NWN2_DynamicArray_NWN2_Object_NWN2_AttachmentPoint_add_DefaultCountPatch, (char*)&InitialAttachmentPointCount, 4 );
		Patch p1( OFFS_NWN2_Object_AddBaseAttachmentPoint_DefaultCountPatch, (char*)&InitialAttachmentPointCount, 4 );
		Patch p2( OFFS_NWN2_Object_DefineAttachmentPoint_DefaultCountPatch, (char*)&InitialAttachmentPointCount, 4 );

		p0.Apply( );
		p1.Apply( );
		p2.Apply( );
	}

	if (useGetTickCount)
	{
		logger->Info("* Using GetTickCount as server time source (instead of QueryPerformanceCounter).");
	}

	if (verboseLogging)
	{
		logger->Info("* Enabled verbose logging.");
	}

	if (rewriteClientUdpPort)
	{
		logger->Info("* Rewriting client UDP port number in packets.");
	}

	if (DisableCharacterCreation)
	{
		logger->Info("* Disabling character creation.");

		Patch p( OFFS_CNWSMessage_HandlePlayerToServerLoginMessage_CmpLoadCharacterStart4, "\x90\x90\x90\x90\x90\x90", 6 );
		p.Apply( );
	}

	EnableRecvfromHook();

	if (DoReplaceNetLayer)
	{
		logger->Info("* Replacing built-in CNetLayerWindow implementation.");

		if (ReplaceNetLayer())
		{
			logger->Info("* CNetLayerWindow replaced.");
			logger->Info("* GameObjUpdate burst size: %lu bytes (stock server default would be 400 bytes).", GameObjUpdateBurstSize);
		}
		else
		{
			logger->Info("* Failed to replace CNetLayerWindow.  Is AuroraServerNetLayer.dll present in the directory with nwn2server.exe?");
		}

		if (DoEnableTls)
		{
			logger->Info("* Attempting to enable TLS.");

			if (EnableTls())
			{
				logger->Info("* TLS enabled.");

				WindowExtensions = DoWindowExtensions;

				if (WindowExtensions)
					logger->Info("* TLS large window extensions enabled.");
			}
			else
			{
				logger->Info("* Failed to enable TLS.  Ensure that AuroraServerNetLayer.dll in the NWN2 installation directory is up to date and that .NET Framework v4.8 or newer is installed.");
			}
		}
	}

	int GameObjUpdateTime;

	if (config.Read( "GameObjUpdateTime", &GameObjUpdateTime, 0x30D40 ))
	{
		logger->Info("* Setting GameObjUpdate time to %lu microseconds.", GameObjUpdateTime);

		Patch p( OFFS_GameObjectUpdateTime1, (char *)&GameObjUpdateTime, 4 );
		
		p.Apply( );
	}

	int DatabaseBufferCount; // default 0xF000 (*0x8000 per buffer)

	config.Read ("DatabaseBufferCount", &DatabaseBufferCount, 1024 );

	{
		logger->Info("* Setting database buffer count to %d.", DatabaseBufferCount);

		//Patch p( code4initLow_BufferCountOffs, (char *)&DatabaseBufferCount, 4 );
		Patch p0( code4optRestart_SetBufferCount, "\xbd\x00\x00\x00\x00\x90\x90", 7 );
		Patch p1( code4optRestart_SetBufferCount+1, (char *)&DatabaseBufferCount, 4 );
		Patch p2( code4optRestart_SetBufferCount2, "\x90\x90", 2 );
		Patch p3( code4optRestart_SetBufferCount2+3, (char *)&DatabaseBufferCount, 4 );
		Patch p4( code4optRestart_SetBufferCount3, "\xeb\x20", 2 );
		p0.Apply( );
		p1.Apply( );
		p2.Apply( );
		p3.Apply( );
		p4.Apply( );
	}

#ifdef XP_BUGFIX_USE_SYMBOLS

	tracer = new StackTracer();

	std::string TraceLogFileName;

	if (config.Read( "StackTraceLogFile", &TraceLogFileName ))
	{
		int TraceCount = 0;

		logger->Info("* Trace log file: %s",
			TraceLogFileName.c_str());

		if (!config.Read( "StackTraceCount", &TraceCount ))
			TraceCount = 1024;

		if (!tracer->Initialize(
			(size_t)TraceCount,
			TraceLogFileName.wc_str(wxConvLibc).data()))
		{
			logger->Info("* Failed to initialize stack tracing for '%s' (%lu traces).",
				TraceLogFileName.c_str(), TraceCount);

			delete tracer;

			tracer = NULL;
		}
		else
		{
			logger->Info("* Initialized stack tracing to '%s' for %lu traces.",
				TraceLogFileName.c_str(), TraceCount);
		}
	}

#endif

	return true;
}

void BugFix::GetFunctionClass(char* fClass)
{
	strncpy_s(fClass, 128, "BUGFIX", 4);
}

int BugFix::GetInt(char* sFunction, char* sParam1, int nParam2)
{
	if (!strcmp(sFunction, "GET PLAYER TLS ENABLED"))
	{
		NWN::CGameObject * Object;
		NWN::CNWSCreature * Creature;

		Object = GetGameObject( (NWN::OBJECTID) nParam2 );
		if (Object == NULL)
			return 0;

		Creature = Object->AsCreature( );
		if (Creature == NULL)
			return 0;

		if (!Creature->GetIsPlayerCharacter( ))
			return 0;

		return NetLayerTlsOnForPlayer( Creature->GetControllingPlayerId( ) ) ? 1 : 0;
	}

	return 0;
}

void BugFix::SetInt(char* sFunction, char* sParam1, int nParam2, int nValue)
{
	if (!strcmp(sFunction, "GAMEOBJUPDATETIME"))
	{
		Patch p( OFFS_GameObjectUpdateTime1, (char *)&nValue, 4 );

		logger->Info("* Setting GameObjUpdateTime to %d microseconds", nValue);
		
		p.Apply( );
	}
}

void BugFix::SetString(char* sFunction, char* sParam1, int nParam2, char* sValue)
{
	logger->Trace("* Plugin SetString(0x%x, %s, %d, %s)", 0x0, sParam1, nParam2, sValue);
}

char* BugFix::GetString(char* sFunction, char* sParam1, int nParam2)
{
	logger->Trace("* Plugin GetString(0x%x, %s, %d)", 0x0, sParam1, nParam2);
	return NULL;
}

bool BugFix::Check()
{
	//
	// Let's make sure that the nwn2server version matches what we are compiled
	// against.
	//

	__try
	{
		if (*reinterpret_cast< unsigned long * >( OFFS_ProcessServerMessageHook ) != CHECK_ProcessServerMessageHook)
		{
			return false;
		}
	}
	__except( EXCEPTION_EXECUTE_HANDLER )
	{
		return false;
	}

	return true;
}

void __stdcall BugFix::FreeNwn2Heap(void *p)
{
	if (!p)
		return;

	if (NWN2Heap_Deallocate)
		NWN2Heap_Deallocate( p );
}

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4127) // conditional expression is constant
#endif

// http://people.csail.mit.edu/amy//papers/box-jgt.pdf

//
// "Fast, Minimum Storage Ray/Triangle Intersection"
// http://www.cs.virginia.edu/~gfx/Courses/2003/ImageSynthesis/papers/Acceleration/Fast%20MinimumStorage%20RayTriangle%20Intersection.pdf
//

#define EPSILON 1.1e-7f
#define CROSS(dest,v1,v2) \
          dest[0]=v1[1]*v2[2]-v1[2]*v2[1]; \
          dest[1]=v1[2]*v2[0]-v1[0]*v2[2]; \
          dest[2]=v1[0]*v2[1]-v1[1]*v2[0];
#define DOT(v1,v2) (v1[0]*v2[0]+v1[1]*v2[1]+v1[2]*v2[2])
#define SUB(dest,v1,v2) \
          dest[0]=v1[0]-v2[0]; \
          dest[1]=v1[1]-v2[1]; \
          dest[2]=v1[2]-v2[2]; 

template< bool TEST_CULL >
__forceinline
static
int
intersect_triangle(const float orig[3], const float dir[3],
                   const float vert0[3], const float vert1[3], const float vert2[3],
                   float *t, float *u, float *v)
{
   float edge1[3], edge2[3], tvec[3], pvec[3], qvec[3];
   float det,inv_det;

   /* find vectors for two edges sharing vert0 */
   SUB(edge1, vert1, vert0);
   SUB(edge2, vert2, vert0);

   /* begin calculating determinant - also used to calculate U parameter */
   CROSS(pvec, dir, edge2);

   /* if determinant is near zero, ray lies in plane of triangle */
   det = DOT(edge1, pvec);

   if (TEST_CULL)
   {
   if (det < EPSILON)
      return 0;

   /* calculate distance from vert0 to ray origin */
   SUB(tvec, orig, vert0);

   /* calculate U parameter and test bounds */
   *u = DOT(tvec, pvec);
   if (*u < 0.0 || *u > det)
      return 0;

   /* prepare to test V parameter */
   CROSS(qvec, tvec, edge1);

    /* calculate V parameter and test bounds */
   *v = DOT(dir, qvec);
   if (*v < 0.0 || *u + *v > det)
      return 0;

   /* calculate t, scale parameters, ray intersects triangle */
   *t = DOT(edge2, qvec);
   inv_det = 1.0f / det;
   *t *= inv_det;
   *u *= inv_det;
   *v *= inv_det;
   } else {                    /* the non-culling branch */
//   WriteText( "Det: %f\n", det );
   if (det > -EPSILON && det < EPSILON)
     return 0;
   inv_det = 1.0f / det;

   /* calculate distance from vert0 to ray origin */
   SUB(tvec, orig, vert0);

   /* calculate U parameter and test bounds */
   *u = DOT(tvec, pvec) * inv_det;
//   WriteText( "U: %f\n", *u );
   if (*u < 0.0 || *u > 1.0)
     return 0;

   /* prepare to test V parameter */
   CROSS(qvec, tvec, edge1);

   /* calculate V parameter and test bounds */
   *v = DOT(dir, qvec) * inv_det;
//   WriteText( "V: %f\n", *v );
   if (*v < 0.0 || *u + *v > 1.0)
     return 0;

   /* calculate t, ray intersects triangle */
   *t = DOT(edge2, qvec) * inv_det;
   }

   if (*t < 0.0)
      return 0;
   else
      return 1;
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#undef EPSILON
#undef CROSS
#undef SUB
#undef DOT

bool
IntersectRayTri(
	__in const NWN::Vector3 & Origin,
	__in const NWN::Vector3 & NormDir,
	__in_ecount(3) const NWN::Vector3 * Tri,
	__out float & T
	)
/*++

Routine Description:

	This routine performs an intersection test between a ray defined by an
	origin and a normalized direction, and a triangle defined by three
	verticies.

Arguments:

	Origin - Supplies the origin point of the ray.

	NormDir - Supplies the normalized direction of the ray.

	Tri - Supplies the triangle verticies defining the triangle to intersect.

	T - Receives the distance from the origin to the intersection point, should
	    the routine return true.

Return Value:

	Returns a Boolean value indicating true if the given ray intersects with
	the triangle in question, or false if no intersection occurred.

Environment:

	User mode.

--*/
{
	float U;
	float V;

	if (intersect_triangle< false >(
		(const float *) &Origin,
		(const float *) &NormDir,
		(const float *) &Tri[ 0 ],
		(const float *) &Tri[ 1 ],
		(const float *) &Tri[ 2 ],
		&T,
		&U,
		&V))
	{
		return true;
	}

	return false;
}

bool
__cdecl
NWN2IntersectRayTri(
	__in const NWN::QuickRay * Ray,
	__in const NWN::Vector3 * Tri0,
	__in const NWN::Vector3 * Tri1,
	__in const NWN::Vector3 * Tri2,
	__out float & T
	)
/*++

Routine Description:

	This routine performs an intersection test between a ray defined by an
	origin and a normalized direction, and a triangle defined by three
	verticies.

Arguments:

	Ray - Supplies the ray to test.

	Tri0 - Supplies the triangle verticies defining the triangle to intersect.

	Tri1 - Supplies the triangle verticies defining the triangle to intersect.

	Tri2 - Supplies the triangle verticies defining the triangle to intersect.

	T - Receives the distance from the origin to the intersection point, should
	    the routine return true.

Return Value:

	Returns a Boolean value indicating true if the given ray intersects with
	the triangle in question, or false if no intersection occurred.

Environment:

	User mode.

--*/
{
	float U;
	float V;

	if (intersect_triangle< false >(
		(const float *) &Ray->o,
		(const float *) &Ray->d,
		(const float *) Tri0,
		(const float *) Tri1,
		(const float *) Tri2,
		&T,
		&U,
		&V))
	{
		return true;
	}

	return false;
}

#ifdef XP_BUGFIX_USE_SYMBOLS

void __stdcall BugFix::LogStackTrace(
	__in const CONTEXT * Context,
	__in ULONG_PTR TraceContext
	)
{
	//
	// Call the class method.
	//

	if (!plugin->tracer)
		return;

	plugin->tracer->LogTrace( Context, TraceContext );
}

#endif

void __stdcall BugFix::SafeInitPositionList(NWN2::somestruc *struc)
{
	NWN2::position_list  *prev = struc->tail;
	NWN2::position_list  *cur  = prev->next;
	std::set< ULONG_PTR > visited;
	ULONG_PTR             key;
//	bool                  first = true;
//	float                 posY = 0.0f;
	bool                  loop = false;
	bool                  in_free_pool = false;

	try
	{
		for (;;)
		{
			key = reinterpret_cast< ULONG_PTR >( cur );

			//
			// If we see ``deadbeef'', then we're actually working on free list
			// nodes (bad).  The deadbeef marker signifies the end of the free
			// list for the NWN2_MemPool< NWN2_JStar::Node > slab allocator.
			//
			// Because the free list looks sufficiently like the
			// NWN2_JStar::Node::m_pParent linked list, we may traverse into it
			// if we get a node in the free list handed to us.  In this case we
			// should break out.  Note that scribbling over [+04h] in the free
			// list is not typically fatal, as it's 0x24 in size and only the
			// value at [+00h], which we only read and don't write, matters.
			//
			// However, this may interfere with other JStar nodes that have a
			// pointer to a freed node checked out, so it's not ideal.  We'll
			// try this though as it's better than crashing entirely.
			//
			// Eventually, after I have identified all the inlined calls to
			// NWN2_MemPool< NWN2_JStar::Node >::Free, I'll scrap this and just
			// instance the slab allocator per-NWN2_JStar object.
			//

			if (key == 0xDEADBEEF)
			{
				in_free_pool = true;
				loop         = true;
				break;
			}

//			logger->Info(wxT("%g %g\n"), cur->pos.y, posY);

			if (!key                                   ||
			    (visited.find( key ) != visited.end()))
			{
//				logger->Info( wxT( "* %g < %g, or loop at %p" ), cur->pos.y, posY, cur );
//				logger->Info("%d %d\n", cur->pos.y < posY, cur->pos.y > posY);
//				prev->next        = struc->head;
//				struc->head->prev = prev;
				loop              = true;
				break;
			}
			else
			{
				visited.insert( key );
			}

			cur->prev = prev;
			prev      = cur;
//			posY      = cur->pos.y;
//			first     = false;

			if (prev == struc->head)
				break;

			cur       = cur->next;
		}
	}
	catch (std::bad_alloc)
	{
//		prev->next        = struc->head;
//		struc->head->prev = prev;
		loop              = true;
	}

	if (loop)
	{
		ULONG now = GetTickCount();

		if (now - plugin->lastlog > 1000)
		{
			plugin->lastlog = now;

			_logger->Info(
				"* SafeInitPositionList: Fixed broken list links (%p, loop detected at @ %p).%s",
				struc,
				prev,
				in_free_pool ? " (WARNING: Node list extended into JStar slab allocator free pool!)" : ""
				);
		}
	}
}

void __stdcall BugFix::LogNullDerefCrash0()
{
	ULONG now = GetTickCount();

	if (now - plugin->lastlog > 1000)
	{
		plugin->lastlog = now;

		_logger->Info(  "LogNullDerefCrash0: Avoided null deference crash #0 (expired frame data?)."  );
	}
}

void __stdcall BugFix::LogNullDerefCrash1()
{
	ULONG now = GetTickCount();

	if (now - plugin->lastlog > 1000)
	{
		plugin->lastlog = now;

		_logger->Info(  "LogNullDerefCrash1: Avoided null deference crash #1 (party inviter invalid)."  );
	}
}

void __stdcall BugFix::LogNullDerefCrash2()
{
	ULONG now = GetTickCount();

	if (now - plugin->lastlog > 1000)
	{
		plugin->lastlog = now;

		_logger->Info(  "LogNullDerefCrash2: Avoided null deference crash #2 (no collider data - respawn while polymorphed race?)."  );
	}
}

void __stdcall BugFix::LogNullDerefCrash3()
{
	ULONG now = GetTickCount();

	if (now - plugin->lastlog > 1000)
	{
		plugin->lastlog = now;

		_logger->Info(  "LogNullDerefCrash3: Avoided null deference crash #3 (DM client toggle plot object)."  );
	}
}

void __stdcall BugFix::LogNullDerefCrash4()
{
	ULONG now = GetTickCount();

	if (now - plugin->lastlog > 1000)
	{
		plugin->lastlog = now;

		_logger->Info(  "LogNullDerefCrash4: Avoided null deference crash #4 (failed to insert player into game object array)."  );
	}
/*mov     edx, [eax]
mov     ecx, eax
mov     eax, [edx+68h]
*/
}

void __stdcall BugFix::LogCrash5()
{
	ULONG now = GetTickCount();

	if (now - plugin->lastlog > 1000)
	{
		plugin->lastlog = now;

		_logger->Info(  "LogCrash5: Avoided deference crash #5 (unpacketize expired frame data?)."  );
	}
/*mov     edx, [eax]
mov     ecx, eax
mov     eax, [edx+68h]
*/
}

void __stdcall BugFix::LogNullDerefCrash6()
{
	ULONG now = GetTickCount();

	if (now - plugin->lastlog > 1000)
	{
		plugin->lastlog = now;

		_logger->Info(  "LogNullDerefCrash6: Avoided null deference crash #6 (Inventory.UnequipItem while player not zoned in)."  );
	}
}

void __stdcall BugFix::LogNullDerefCrash7()
{
	ULONG now = GetTickCount();

	if (now - plugin->lastlog > 1000)
	{
		plugin->lastlog = now;

		_logger->Info(  "LogNullDerefCrash7: Avoided null deference crash #7 (ActionExchangeItem script function called on wrong object type)."  );
	}
}

void __stdcall BugFix::LogNullDerefCrash8()
{
	ULONG now = GetTickCount();

	if (now - plugin->lastlog > 1000)
	{
		plugin->lastlog = now;

		_logger->Info(  "LogNullDerefCrash8: Avoided null deference crash #8 (NULL CItemRepository in item acquisition)."  );
	}
}

void __stdcall BugFix::LogUncompress0Fix()
{
	ULONG now = GetTickCount();

	if (now - plugin->lastlog > 1000)
	{
		plugin->lastlog = now;

		_logger->Info(  "LogUncompress0Fix: Avoided crash due to invalid compressed data."  );
	}
}

void __stdcall BugFix::LogUncompress1Fix()
{
	ULONG now = GetTickCount();

	if (now - plugin->lastlog > 1000)
	{
		plugin->lastlog = now;

		_logger->Info(  "LogUncompress1Fix: Avoided crash due to invalid compressed data."  );
	}
}

void __stdcall BugFix::LogNullDerefCrash9()
{
	ULONG now = GetTickCount();

	if (now - plugin->lastlog > 1000)
	{
		plugin->lastlog = now;

		_logger->Info(  "LogNullDerefCrash9: Avoided null deference crash #9 (Nonexistant object in item repository)."  );
	}
}

void __stdcall BugFix::LogNullDerefCrash10()
{
	ULONG now = GetTickCount();

	if (now - plugin->lastlog > 1000)
	{
		plugin->lastlog = now;

		_logger->Info(  "LogNullDerefCrash10: Avoided null dereference crash #10 (Bogus feats during level-up processing)."  );
	}
}

void __stdcall BugFix::LogNullDerefCrash11()
{
	ULONG now = GetTickCount();

	if (now - plugin->lastlog > 1000)
	{
		plugin->lastlog = now;

		_logger->Info(  "LogNullDerefCrash11: Avoided null dereference crash #11 (Player with no automap module parting)."  );
	}
}

void __stdcall BugFix::LogNullDerefCrash12()
{
	ULONG now = GetTickCount();

	if (now - plugin->lastlog > 1000)
	{
		plugin->lastlog = now;

		_logger->Info(  "LogNullDerefCrash12: Avoided null dereference crash #12 (Level up with no player LUO)."  );
	}
}



unsigned long CalcPositionLoop0Ret      = OFFS_CalcPositionLoop0Ret;
unsigned long CalcPositionLoop1Ret      = OFFS_CalcPositionLoop1Ret;
#if NWN2SERVER_VERSION < 0x01211549
unsigned long NullDerefCrash0NormalRet  = OFFS_NullDerefCrash0RetNormal;
unsigned long NullDerefCrash0SkipRet    = OFFS_NullDerefCrash0RetSkip;
unsigned long NullDerefCrash1NormalRet  = OFFS_NullDerefCrash1RetNormal;
unsigned long NullDerefCrash1SkipRet    = OFFS_NullDerefCrash1RetSkip;
#endif
unsigned long NullDerefCrash2NormalRet  = OFFS_NullDerefCrash2RetNormal;
unsigned long NullDerefCrash2SkipRet    = OFFS_NullDerefCrash2RetSkip;
#if NWN2SERVER_VERSION < 0x01211549
unsigned long NullDerefCrash3NormalRet  = OFFS_NullDerefCrash3RetNormal;
unsigned long NullDerefCrash3SkipRet    = OFFS_NullDerefCrash3RetSkip;
#endif
#if NWN2SERVER_VERSION < 0x01231763
unsigned long NullDerefCrash4NormalRet  = OFFS_NullDerefCrash4RetNormal;
unsigned long NullDerefCrash4SkipRet    = OFFS_NullDerefCrash4RetSkip;
unsigned long Crash5NormalRet           = OFFS_Crash5RetNormal;
unsigned long Crash5SkipRet             = OFFS_Crash5RetSkip;
#endif
unsigned long NullDerefCrash6NormalRet  = OFFS_NullDerefCrash6RetNormal;
unsigned long NullDerefCrash6SkipRet    = OFFS_NullDerefCrash6RetSkip;
unsigned long NullDerefCrash7NormalRet  = OFFS_NullDerefCrash7RetNormal;
unsigned long NullDerefCrash7SkipRet    = OFFS_NullDerefCrash7RetSkip;
unsigned long NullDerefCrash8NormalRet  = OFFS_NullDerefCrash8RetNormal;
unsigned long NullDerefCrash8SkipRet    = OFFS_NullDerefCrash8RetSkip;
unsigned long CheckUncompress0NormalRet = OFFS_CheckUncompress0RetNormal;
unsigned long CheckUncompress0SkipRet   = OFFS_CheckUncompress0RetSkip;
unsigned long CheckUncompress1NormalRet = OFFS_CheckUncompress1RetNormal;
unsigned long CheckUncompress1SkipRet   = OFFS_CheckUncompress1RetSkip;
unsigned long UncompressMessage         = OFFS_UncompressMessage;
unsigned long NullDerefCrash9NormalRet  = OFFS_NullDerefCrash9RetNormal;
unsigned long NullDerefCrash9SkipRet    = OFFS_NullDerefCrash9RetSkip;
#if NWN2SERVER_VERSION >= 0x01211549
unsigned long NullDerefCrash10NormalRet = OFFS_NullDerefCrash10RetNormal;
unsigned long NullDerefCrash10SkipRet   = OFFS_NullDerefCrash10RetSkip;
#endif
#if NWN2SERVER_VERSION == 0x01211549
unsigned long CGameEffectDtorRet        = OFFS_CGameEffectDtorRet;
#endif
#if NWN2SERVER_VERSION >= 0x01231763
unsigned long NullDerefCrash11NormalRet = OFFS_NullDerefCrash11RetNormal;
unsigned long NullDerefCrash11SkipRet   = OFFS_NullDerefCrash11RetSkip;
unsigned long SendCompressionHookDoZlib = OFFS_SendCompressionHookDoZlib;
unsigned long SendCompressionHookNoZlib = OFFS_SendCompressionHookNoZlib;
unsigned long NullDerefCrash12Ret       = OFFS_NullDerefCrash12Ret;
#endif

/*
 * NWN2_JStar::SearchStep
 *
 * - There is a bug here somewhere where we return a NWN2_JStar::Node to the
 *   node allocator pool while it is still in use.  The way this typically
 *   fails is that we get a loop in the list because a node is inserted into
 *   the list twice after being freed and then checked out again for the same
 *   NWN2_JStar instance (failure mode I).  Other failure modes are possible
 *   (bizzare pathing behavior due to alias a "hot" NWN2_JStar::Node from
 *   another active NWN2_JStar instance (failure mode II), or taking a node in
 *   the free list and walking the free list ``Next'' pointers as
 *   NWN2_JStar::Node::m_pParent pointers until we reach the end of pool marker
 *   at 0xDEADBEEF (failure mode III).
 *
 * - N.B. I am considering instancing the allocator pool on a per-NWN2_JStar
 *   instance basis, disabling ``free'' calls, and then just throwing away the
 *   entire list of allocations made by that NWN2_JStar instance in the context
 *   of NWN2_JStar::~NWN2_JStar.  This should fix the problem for good at the
 *   cost of some minor additional memory usage while an NWN2_JStar instance is
 *   performing pathing stepping.  However, given the maximum pathing step
 *   length (300), this seems like it shouldn't be too much of an issue.
 *
 * - To find the actual problem, I recommend replacing the standard
 *   NWN2_MemPool< NWN2_JStar::Node > with a page heap style allocator that
 *   places each node on it's own page and reprotects that page to noaccess
 *   after it is freed to the pool.  This will catch the first use of a freed
 *   NWN2_JStar::Node object.  It may also be useful to log a stack trace with
 *   each allocation.  To use the system page heap to accomplish these tasks,
 *   simply replace NWN2_MemPool< NWN2_JStar::Node >::Free with HeapFree, and
 *   NWN2_MemPool< NWN2_JStar::Node >::Alloc with HeapAlloc, and enable page
 *   heap, which will also conveniently log stack traces as well NWN2 is
 *   rebuilt with /Oy- (so that EBP-based stack traces work).  Note that as
 *   Microsoft builds Windows with /Oy-, I believe that it would be a
 *   reasonable change to make for release builds for all modules, as it
 *   significantly improves debuggability of these issues in conjuction with
 *   tools such as page heap.
 */
__declspec(naked) void BugFix::CalcPositionsLoop0Fix()
{
	__asm
	{
		push    esi
		call    SafeInitPositionList

		jmp     dword ptr [CalcPositionLoop0Ret]
	}
}

/*
 * NWN2_JStar::SearchStep
 *
 * - There is a bug here somewhere where we return a NWN2_JStar::Node to the
 *   node allocator pool while it is still in use.  The way this typically
 *   fails is that we get a loop in the list because a node is inserted into
 *   the list twice after being freed and then checked out again for the same
 *   NWN2_JStar instance (failure mode I).  Other failure modes are possible
 *   (bizzare pathing behavior due to alias a "hot" NWN2_JStar::Node from
 *   another active NWN2_JStar instance (failure mode II), or taking a node in
 *   the free list and walking the free list ``Next'' pointers as
 *   NWN2_JStar::Node::m_pParent pointers until we reach the end of pool marker
 *   at 0xDEADBEEF (failure mode III).
 *
 * - N.B. I am considering instancing the allocator pool on a per-NWN2_JStar
 *   instance basis, disabling ``free'' calls, and then just throwing away the
 *   entire list of allocations made by that NWN2_JStar instance in the context
 *   of NWN2_JStar::~NWN2_JStar.  This should fix the problem for good at the
 *   cost of some minor additional memory usage while an NWN2_JStar instance is
 *   performing pathing stepping.  However, given the maximum pathing step
 *   length (300), this seems like it shouldn't be too much of an issue.
 *
 * - To find the actual problem, I recommend replacing the standard
 *   NWN2_MemPool< NWN2_JStar::Node > with a page heap style allocator that
 *   places each node on it's own page and reprotects that page to noaccess
 *   after it is freed to the pool.  This will catch the first use of a freed
 *   NWN2_JStar::Node object.  It may also be useful to log a stack trace with
 *   each allocation.  To use the system page heap to accomplish these tasks,
 *   simply replace NWN2_MemPool< NWN2_JStar::Node >::Free with HeapFree, and
 *   NWN2_MemPool< NWN2_JStar::Node >::Alloc with HeapAlloc, and enable page
 *   heap, which will also conveniently log stack traces as well NWN2 is
 *   rebuilt with /Oy- (so that EBP-based stack traces work).  Note that as
 *   Microsoft builds Windows with /Oy-, I believe that it would be a
 *   reasonable change to make for release builds for all modules, as it
 *   significantly improves debuggability of these issues in conjuction with
 *   tools such as page heap.
 */
__declspec(naked) void BugFix::CalcPositionsLoop1Fix()
{
	__asm
	{
		push    esi
		call    SafeInitPositionList

		jmp     dword ptr [CalcPositionLoop1Ret]
	}
}

/*
 * CNetLayerWindow::FrameTimeout
 *
 * - We call CExoNetExtendableBuffer::GetFrameData, which fails due to an
 *   expired frame or some other as of yet undetermined condition.  This
 *   routine has an [out] pointer to the frame data.  When the routine
 *   returns false, the [out] pointer is uninitialized, but
 *   CNetLayerWindow::FrameTimeout does not check the return value and will
 *   use an uninitialized buffer pointer (stack based) in this case.
 *
 * - The actual fix for this problem should be:
 *
 *   if (!pBuffer->GetFrameData( ..., &pFrameData, .... ))
 *     return false; // Or perform other error handling, e.g. drop player
 */
__declspec(naked) void BugFix::NullDerefCrash0Fix()
{
#if NWN2SERVER_VERSION < 0x01211549
	__asm
	{
		test    al, al
		jz      Skip

		mov     eax, dword ptr [esp+20h]

;		cmp     eax, 0ffffh
;		jbe     Skip

		movzx   cx, byte ptr [eax+03h]

		jmp     dword ptr [NullDerefCrash0NormalRet]

Skip:
		call    LogNullDerefCrash0
		jmp     dword ptr [NullDerefCrash0SkipRet]
	}
#endif
}

/*
 * CNWSMessage::WriteGameObjUpdate_UpdateObject
 *
 * - We cast a CGameObject as a CNWSCreature and the creature either has no
 *   CNWSCreatureStatus (failure mode I), or the object wasn't a CNWSCreature
 *   and the cast failed (failure mode II).  The object we are referencing is
 *   the object referenced by a player's CNWSCreature::m_oidInvitedToPartyBy,
 *   and the failure condition occurs when the inviter has left the server.
 *
 * - This typically happens when a player invites another player that is in a
 *   zone transfer, then the inviting player quits immediately after.
 *
 * - The actual fix for this problem should be to iterate through all
 *   CNWSCreature objects from CExoServerAppInternal::RemovePCFromWorld, and:
 *
 *   if (pCreature->GetInvitedToParty() && pCreature->GetInvitedToPartyBy()) ==
 *     pExitingPlayer->GetId())
 *   {
 *      pCreature->SetInvitedToParty( false );
 *      pCreature->SetInvitedToPartyBy( INVALID_OBJECT_ID );
 *   }
 *
 *   There may need to be a notification sent to the client to tell it that the
 *   party invitation is expired in a cleaner way as well.
 */
#if NWN2SERVER_VERSION == 0x00131409
__declspec(naked) void BugFix::NullDerefCrash1Fix()
{
	__asm
	{
		test	ebp, ebp
		jz      Skip

		mov     ecx, dword ptr [ebp+01fc4h]
		cmp     ecx, 0ffffh
		jbe     Skip

		jmp     dword ptr [NullDerefCrash1NormalRet]

Skip:
		call    LogNullDerefCrash1
		jmp     dword ptr [NullDerefCrash1SkipRet]
	}
}
#elif NWN2SERVER_VERSION == 0x00121295
__declspec(naked) void BugFix::NullDerefCrash1Fix()
{
	__asm
	{
		test	ebp, ebp
		jz      Skip

		mov     ecx, dword ptr [ebp+1each]
		cmp     ecx, 0ffffh
		jbe     Skip

		jmp     dword ptr [NullDerefCrash1NormalRet]

Skip:
		call    LogNullDerefCrash1
		jmp     dword ptr [NullDerefCrash1SkipRet]
	}
}
#else
__declspec(naked) void BugFix::NullDerefCrash1Fix()
{
	__asm
	{
	}
}
#endif

/*
 * NWN2_Collider::UpdateCollider
 *
 * - We touch NWN2_Collider::m_AABBInfo->m_pAABBMgr->m_pAABBs when in fact the
 *   NWN2_AABBMgr pointer (m_pAABBMgr) is null.
 *
 * - This tends to happen when a creature dies while polymorphed.  I assume
 *   that it's a race condition where the creature is pathed over before we
 *   have loaded collision detection data for the new form when unpolymorphing,
 *   but that isn't proven yet.
 *
 * - Haven't converted stack trace from 1.12 to 1.13 beta to determine the
 *   caller yet.
 */
__declspec(naked) void BugFix::NullDerefCrash2Fix()
{
	__asm
	{
		pop     edi
		test    ecx, ecx
		jz      Skip

		add     eax, dword ptr [ecx+0ch]
		fstp    dword ptr [eax]

		jmp     dword ptr [NullDerefCrash2NormalRet]

Skip:
		call    LogNullDerefCrash2
		jmp     dword ptr [NullDerefCrash2SkipRet]
	}
}

/*
 * CNWSMessage::HandlePlayerToServerDungeonMasterMessage
 *
 * - In the handler for the toggle plot object DM command, we receive an
 *   OBJECT_ID from a DM client, which is a valid CGameObject reference, but
 *   points to an object not derived from CNWSObject (such as a CNWSArea or
 *   CNWSModule).  The code assumes that if CGameObjectArray::GetGameObject
 *   returns for an OBJECT_ID, that CGameObject::AsNWSObject() will always
 *   cast successfully to a CNWSObject.
 *
 * - The actual fix for this problem should be along the lines of the
 *   following:
 *
 *   CNWSObject *pServerObject = pObject->AsNWSObject();
 *
 *   if (!pServerObject)
 *     return false; // Or handle the error appropriately.
 */
__declspec(naked) void BugFix::NullDerefCrash3Fix()
{
#if NWN2SERVER_VERSION < 0x01211549
	__asm
	{
		test    eax, eax
		jz      Skip

		mov     eax, dword ptr [eax+180h]

		jmp     dword ptr [NullDerefCrash3NormalRet]

Skip:
		call    LogNullDerefCrash3
		jmp     dword ptr [NullDerefCrash3SkipRet]
	}
#endif
}

#if NWN2SERVER_VERSION < 0x01231763
/*
 * CServerExoAppInternal::LoadCharacterStart
 *
 * - We call CNWSPlayer::GetGameObject() for a newly inserted CNWSPlayer
 *   object, but the player wasn't actually inserted into the game object array
 *   and so this the call returns null.  We then go on to reference the
 *   returned CGameObject pointer for a call to CGameObject::AsNWSCreature, for
 *   use in CServerExoAppInternal::ProcessBackwardsCompatibility, but as the
 *   CNWSPlayer had no CGameObject associated with it, we crash.
 *
 * - The real solution should be to handle the failure to insert the CNWSPlayer
 *   into the game object array and if we fail to insert the game object, send
 *   back a PlayerLogin_Fail message.  e.g.:
 *
 *   CGameObject *pGameObject = pPlayerObject->GetGameObject();
 *
 *   if (!pGameObject) // Suggested new check
 *     break; // Fallthrough handles via if (!pPlayerObject->GetGameObject())
 *
 *   CNWSCreature *pCreatureObject = pGameObject->AsNWSCreature();
 *
 *   if (!pCreatureObject) // Suggested new check
 *     break; // Fallthrough handles via if (!pPlayerObject->GetGameObject())
 *
 *   ProcessBackwardsCompatibility( pCreatureObject );
 *
 *   CNWSDungeonMaster *pDMObject = pPlayerObject->AsNWSDungeonMaster();
 *
 *   if (pDMObject)
 *   {
 *     // ... Additional processing ...
 *     break; // All done
 *   }
 *
 * - This may happen when we fail to process a .bic file.
 */
__declspec(naked) void BugFix::NullDerefCrash4Fix()
{
	__asm
	{
		test    eax, eax
		jz      Skip

		mov     edx, dword ptr [eax]
		mov     ecx, eax
		mov     eax, dword ptr [edx+68h]

		jmp     dword ptr [NullDerefCrash4NormalRet]

Skip:
		call    LogNullDerefCrash4

		;
		; Sends a login failure message and cleans up.
		;

		jmp     dword ptr [NullDerefCrash4SkipRet]
	}
}

/*
 * CNetLayerWindow::UnpacketizeFullMessages
 *
 * - We call CExoNetExtendableBuffer::GetFrameData, which fails due to an
 *   expired frame or some other as of yet undetermined condition.  This
 *   routine has an [out] pointer to the frame data.  When the routine
 *   returns false, the [out] pointer is uninitialized, but
 *   CNetLayerWindow::UnpacketizeFullMessages (incorrectly) assumes that
 *   CExoNetExtendableBuffer::GetFrameData signals a failure condition by
 *   setting the [out] pointer to NULL.  Thus, we use an uninitialized
 *   buffer pointer (stack based) in this case.
 *
 * - The actual fix for this problem should be:
 *
 *   if (!pBuffer->GetFrameData( ..., &pFrameData, .... ))
 *     return false; // Or perform other error handling, e.g. drop player
 */
__declspec(naked) void BugFix::Crash5Fix()
{
	__asm
	{
		test    al, al
		jz      Skip

		mov     ecx, dword ptr [esp+14h]

		;
		; This is redundant as the test al, al above will catch it.  However,
		; just to be clear, redoing it.
		;

		test    ecx, ecx

		jmp     dword ptr [Crash5NormalRet]

Skip:
		call    LogCrash5

		;
		; Set the output pointer to null so that the check for failure will
		; work as it was intended to.
		;

		xor     ecx, ecx
		mov     dword ptr [esp+14h], ecx

		test    ecx, ecx

		jmp     dword ptr [Crash5SkipRet]
	}
}
#else
void BugFix::NullDerefCrash4Fix()
{
}
void BugFix::Crash5Fix()
{
}
#endif

/*
 * CNWSMessage::HandlePlayerToServerInventoryMessage
 *
 * - We call CNWSPlayer::GetGameObject, but fail to handle the contingency
 *   where this returns NULL.
 *
 * - The actual fix to this problem should be:
 *
 *   CGameObject *pGameObject = pPlayer->GetGameObject();
 *
 *   if (!pGameObject) break; // Suggested new check
 *
 */
__declspec(naked) void BugFix::NullDerefCrash6Fix()
{
	__asm
	{
		test    eax, eax
		jz      Skip

		mov     edx, dword ptr [eax]
		mov     ecx, eax
		mov     eax, dword ptr [edx+068h]

		jmp     dword ptr [NullDerefCrash6NormalRet]

Skip:
		call    LogNullDerefCrash6

		;
		; Send an unequip failure message to the client.
		;

		jmp     dword ptr [NullDerefCrash6SkipRet]
	}
}


/*
 * CNWVirtualMachineCommands::ActionExchangeItem
 *
 * - We call CGameObject::AsNWSObject(), but do not handle the case where the
 *   cast to CNWSObject fails (e.g. an environmental object was provided).
 *
 * - The actual fix to this problem should be (note that there are two if
 *   branches that must be altered, or the AsNWSObject() cast should be moved
 *   before the if fork):
 *
 *   CNWSObject *pNWSObject = pGameObject->AsNWSObject();
 *
 *   if (!pNWSObject) break; // Suggested new check
 *
 */
__declspec(naked) void BugFix::NullDerefCrash7Fix()
{
	__asm
	{
		;
		; Must preserve ecx/eax across the call, edx is not read before it is
		; rewritten beyond this point.
		;

		pushfd
		push    ecx
		push    eax

		;
		; Cast to CNWSObject with a check for failure.  We will still run the
		; unchecked casts afterwards, but this pre-check is sufficient as we do
		; not support concurrency in the scripting environment.
		;

		mov     ecx, esi
		mov     edx, dword ptr [esi]
		call    dword ptr [edx+03ch]
		test    eax, eax
		pop     eax
		pop     ecx

		jz      Skip

		mov     cl, byte ptr [eax+03e0h]
		popfd

		jmp     dword ptr [NullDerefCrash7NormalRet]

Skip:

		call    LogNullDerefCrash7

		popfd

		;
		; Fail the script VM call.
		;

		jmp     dword ptr [NullDerefCrash7SkipRet]
	}
}


/*
 * CNWSItem::AcquireItem
 *
 * - We do not check that the object's CItemRepository exists.
 *
 * - The actual fix to this problem should be:
 *
 *   CItemRepository *pItemRepository = ...;
 *
 *   if (!pItemRepository) break; // Suggested new check
 *
 *   pItemRepository->FindBlankPosition( ... );
 *
 */
__declspec(naked) void BugFix::NullDerefCrash8Fix()
{
#if NWN2SERVER_VERSION > 0x01221588
	__asm
	{
		test    eax, eax
		jz      Skip

		mov     edx, dword ptr [edi]
		push    01h
		push    00h

		jmp     dword ptr [NullDerefCrash8NormalRet]

Skip:
		call    LogNullDerefCrash8

		;
		; Fail the item acquisition.
		;

		jmp     dword ptr [NullDerefCrash8SkipRet]
	}
#else
	__asm
	{
		test    ebp, ebp
		jz      Skip

		mov     edx, dword ptr [edi]
		push    01h
		push    00h

		jmp     dword ptr [NullDerefCrash8NormalRet]

Skip:
		call    LogNullDerefCrash8

		;
		; Fail the item acquisition.
		;

		jmp     dword ptr [NullDerefCrash8SkipRet]
	}
#endif
}

/*
 * CNetLayerWindow::UnpacketizeFullMessages
 *
 * - We don't check that the frame chunk length is within bounds.
 * - We don't check that zlib inflate succeeds.
 *
 * - The actual fix should check that the frame chunk length is within the
 *   length of the remaining packet buffer, and that the uncompress call
 *   succeeds.
 */
__declspec(naked) void BugFix::Uncompress0Fix()
{
	__asm
	{
		cmp     edi, ebx
		jnbe    Skip

		mov     ecx, dword ptr [esi+04h]
		push    edi
		push    eax
		mov     eax, dword ptr [esi+0ch]
		push    eax
		call    dword ptr [UncompressMessage]

		test    eax, eax
		jz      Skip

		jmp     dword ptr [CheckUncompress0NormalRet]

Skip:
		call    LogUncompress0Fix

		;
		; Drop the message.
		;

		jmp     dword ptr [CheckUncompress0SkipRet]
	}
}

/*
 * CNetLayerWindow::UnpacketizeFullMessages
 *
 * - We don't check that zlib inflate succeeds.
 *
 * - The actual fix should check that the uncompress call succeeds.
 */
__declspec(naked) void BugFix::Uncompress1Fix()
{
	__asm
	{
		mov     edx, dword ptr [esi+0ch]
		mov     ecx, dword ptr [esi+04h]
		push    ebx
		push    ebp
		push    edx
		call    dword ptr [UncompressMessage]

		test    eax, eax
		jz      Skip

		jmp     dword ptr [CheckUncompress1NormalRet]

Skip:
		push    ebp
		call    FreeNwn2Heap

		call    LogUncompress1Fix

		;
		; Drop the message.
		;

		jmp     dword ptr [CheckUncompress1SkipRet]
	}
}

/*
 * CItemRepository::GetItemPtrInRepository
 *
 * - We do not check that GetItemByGameObjectID returns a non-NULL CNWSItem*.
 *
 * - The actual fix to this problem should be:
 *
 *   CNWSItem *pItem = pServer->GetItemByGameObjectID( ObjectID );
 *
 *   if (!pItem) continue; // Suggested new check
 *
 * - Additionally, why the CItemRepository had an invalid object id in it
 *   should be looked into as well.  This is suspicious, perhaps there needs to
 *   be a check to remove an item from all item repositories when a CNWSItem is
 *   deleted.
 *
 */
__declspec(naked) void BugFix::NullDerefCrash9Fix()
{
	__asm
	{
		test    eax, eax
		jz      Skip

		mov     edi, eax
		mov     eax, dword ptr [edi+04h]

		jmp     dword ptr [NullDerefCrash9NormalRet]

Skip:
		call    LogNullDerefCrash9

		;
		; Continue on to the next item.
		;

		jmp     dword ptr [NullDerefCrash9SkipRet]
	}
}

/*
 * CNWSCreatureStats::ValidateLevelUp
 *
 * - We don't handle the case where CNWSFeat::GetFeat returns NULL.  This
 *   results in a crash during level-up processing (inside
 *   CNWSRules::IgnoreValidation) if a client specifies a bogus feat identifier
 *   in their level-up packet.
 *
 * - The actual fix to this problem should be:
 *
 *    CNWSFeat *pFeat = g_pRules->GetFeat( FeatId );
 *
 *    if (!pFeat) return LEVELUP_FAIL_BAD_FEAT;
 *
 */
__declspec(naked) void BugFix::NullDerefCrash10Fix()
{
	__asm
	{
		test    eax, eax
		jz      Skip

		cmp     dword ptr [esp+028h], ebx
		mov     ebp, eax

		jmp     dword ptr [NullDerefCrash10NormalRet]

Skip:
		call    LogNullDerefCrash10

		;
		; Fail the level-up.
		;

		jmp     dword ptr [NullDerefCrash10SkipRet]
	}
}

/*
 * CGameEffect::~CGameEffect
 *
 * - Log stack traces to catch who deletes a still-live CGameEffect.
 *
 */
__declspec(naked) void BugFix::CGameEffectDtorLogger()
{
#if NWN2SERVER_VERSION == 0x01211549
	__asm
	{
		sub     esp, SIZE CONTEXT

		mov     dword ptr [esp]CONTEXT.ContextFlags, CONTEXT_CONTROL | CONTEXT_INTEGER
		mov     word ptr [esp]CONTEXT.SegSs, ss
		mov     word ptr [esp]CONTEXT.SegCs, cs
		mov     dword ptr [esp]CONTEXT.Esp, esp
		add     dword ptr [esp]CONTEXT.Esp, SIZE CONTEXT
		mov     dword ptr [esp]CONTEXT.Ebp, ebp
		mov     dword ptr [esp]CONTEXT.Eax, eax
		mov     dword ptr [esp]CONTEXT.Ebx, ebx
		mov     dword ptr [esp]CONTEXT.Ecx, ecx
		mov     dword ptr [esp]CONTEXT.Edx, edx
		mov     dword ptr [esp]CONTEXT.Edi, edi
		mov     dword ptr [esp]CONTEXT.Esi, esi
		mov     dword ptr [esp]CONTEXT.Eip, OFFS_CGameEffectDtor
		pushfd
		pop     eax
		mov     dword ptr [esp]CONTEXT.EFlags, eax

		lea     eax, dword ptr [esp]
		push    ecx
		push    eax
		call    LogStackTrace

		mov     eax, OFFS_ms_iGameEffectCount
		dec     dword ptr [eax]

		mov     eax, dword ptr [esp]CONTEXT.Eax
		mov     ecx, dword ptr [esp]CONTEXT.Ecx

		add     esp, SIZE CONTEXT

		push    ecx

		jmp     dword ptr [CGameEffectDtorRet]
	}
#endif
}


/*
 * CNWSPlayer::DropTURD:
 *
 * - We don't handle the case where the player was not fully initialized and
 *   had not acquired a NWN2_SAutoMap::NWN2_SAutoMapModule yet.
 *
 * - The actual fix to this problem should be:
 *
 *    NWN2_SAutoMap::NWN2_SAutoMapModule *pMapModule = pCreature->m_cAutoMap.m_CurrentModuleAutoMap
 *
 *    if (pMapModule) {  CopyAutomapData( );  }
 *
 */
__declspec(naked) void BugFix::NullDerefCrash11Fix()
{
	__asm
	{
		test    eax, eax
		jz      Skip

		mov     edx, dword ptr [eax+030h]
		push    ecx
		lea     ecx, dword ptr [eax+020h]

		jmp     dword ptr [NullDerefCrash11NormalRet]

Skip:
		call    LogNullDerefCrash11

		;
		; Don't persist the nonexistant automap data.
		;

		jmp     dword ptr [NullDerefCrash11SkipRet]
	}
}

/*
 * CNWSMessage::HandlePlayerToServerLevelUpMessage
 *
 * - We don't handle the case of the player LUO being NULL.
 *
 * - The actual fix to this problem should be:
 *
 *    if (pPlayerLUO && pPlayerLUO->m_lstFeatUses.num) { pPlayerLUO->m_lstFeatUses[0].m_nUsedToday += 1; }
 *
 */
__declspec(naked) void BugFix::NullDerefCrash12Fix()
{
	__asm
	{
		test    eax, eax
		jz      Skip

		cmp     dword ptr [eax+03ch], 00h
		jle     NoFeats

		mov     ecx, dword ptr [eax+038h]
		mov     eax, dword ptr [ecx]
		add     byte ptr [eax+02h], 01h

NoFeats:
		jmp     dword ptr [NullDerefCrash12Ret]

Skip:
		call    LogNullDerefCrash12

		jmp     dword ptr [NullDerefCrash12Ret]
	}
}


/*
 * CNetLayerInternal::SendMessageToPlayer
 *
 * - Allow compression to be forced always off for server to client messages.
 *
 */
__declspec(naked) void BugFix::SendCompressionHook()
{
	__asm
	{
		cmp     byte ptr [nocompress], 0h
		je      DoCompress

		jmp     dword ptr [SendCompressionHookNoZlib]

DoCompress:

		jmp     dword ptr [SendCompressionHookDoZlib]
	}
}

/*
 * CExoTimersInternal::GetHighResolutionTimer
 *
 * - Use 64-bit integral path instead of 32-bit float path to calculate time
 *   from performance counter (so as to avoid overflow).
 *
 */
ULONG64 __cdecl BugFix::GetHighResolutionTimerFix()
{
	LARGE_INTEGER PerfCount;

	if (plugin->useGetTickCount || !QueryPerformanceCounter( &PerfCount ))
	{
		return ((GetTickCount( ) - plugin->tickCountDelta) * 1000);
	}

	//
	// We assume that the perf frequency is at least 1000 counts per second,
	// so we divide by that upfront.  This means that we have the extra 1000
	// factor to take care of here, which we do so.
	//
	// Splitting the difference like this allows the timer to still provide a
	// baseline level of functionality even if its resolution is lower than the
	// microsecond precision we require.
	//

	return (PerfCount.QuadPart * 1000) / plugin->perffreq.QuadPart;
}

/*
 * CGameObjectArray::AddInternalObject
 *
 * Add the object to our internal faster lookup table.
 *
 */
__declspec(naked) void BugFix::AddInternalObjectHook()
{
	__asm
	{

		push    ebp
		mov     ebp, esp

		;
		; Add the object into the game's array first.
		;

		push    dword ptr [ebp+10h] ; [in] BOOL IsPlayer
		push    dword ptr [ebp+0ch] ; [in] CGameObject * ObjectPtr
		push    dword ptr [ebp+08h] ; [out] NWN::ObjectId * ObjectId
		call    DoAddInternalObject

		;
		; Exit out if the add failed.
		;

		test    al, al
		jz      retpoint

		;
		; Now add it into our array.
		;

		mov     ecx, dword ptr [ebp+08h]  ; ObjectId
		mov     ecx, dword ptr [ecx]      ; *ObjectId
		mov     edx, dword ptr [ebp+0ch]  ; ObjectPtr
		call    BugFix::AddGameObject

		or      eax, 01h
		jmp     retpoint

DoAddInternalObject:
		;
		; Call the original logic to add the new object...
		;

		push    ebp
        mov     ebp, dword ptr [esp+0ch]
		mov     eax, OFFS_ObjArrayAddInternalObject+5
		jmp     eax

retpoint:
		pop     ebp
		ret     0ch
	}
}

/*
 * CGameObjectArray::AddObjectAtPos
 *
 * Add the object to our internal faster lookup table.
 *
 */
__declspec(naked) void BugFix::AddObjectAtPosHook()
{
	__asm
	{

		push    ebp
		mov     ebp, esp

		;
		; Add the object into the game's array first.
		;

		push    ecx ; save ecx
		push    dword ptr [ebp+0ch] ; [in] CGameObject * ObjectPtr
		push    dword ptr [ebp+08h] ; [in] NWN::ObjectId ObjectId
		call    DoAddObjectAtPos
		pop     ecx

		;
		; Exit out if the add failed.
		;

		test    al, al
		jz      retpoint

		;
		; Now add it into our array.
		;


		                                  ; [ecx = CGameObjectArray]
		push    dword ptr [ebp+0ch]       ; ObjectPtr
		mov     edx, dword ptr [ebp+08h]  ; ObjectId
		call    BugFix::AddGameObjectAtPos

		or      eax, 01h
		jmp     retpoint

DoAddObjectAtPos:
		;
		; Call the original logic to add the new object...
		;

		cmp     dword ptr [esp+08h], 0
		mov     eax, OFFS_ObjArrayAddObjectAtPos+5
		jmp     eax

retpoint:
		pop     ebp
		ret     08h
	}
}

/*
 * CGameObjectArray::DeleteAll
 *
 * Delete our internal object table.
 *
 */
__declspec(naked) void BugFix::DeleteAllHook()
{
	__asm
	{

		push    ecx
		push    ebx
		push    ebp
		push    esi
		push    edi

		push    ecx
		call    BugFix::DeleteAllGameObjects
		pop     ecx
		mov     eax, OFFS_ObjArrayDeleteAll+5
		jmp     eax
	}
}

/*
 * CGameObjectArray::GetGameObject
 *
 * Search our faster lookup table.
 *
 */
__declspec(naked) void BugFix::GetGameObjectHook()
{
	__asm
	{

		mov     ecx, dword ptr [esp+04h] ; [in] NWN::OBJECTID ObjectId

		;
		; Test for invalid object id
		;

		cmp     ecx, NWN::INVALIDOBJID
		jz      NoObjectFound_Before

		;
		; Search the internal table
		;

		push    dword ptr [esp+08h] ; save [out] CGameObject * * ObjectPtr

		call    BugFix::GetGameObject
		test    eax, eax
		jz      NoObjectFound_After

		pop     ecx
		mov     dword ptr [ecx], eax
		or      eax, 01h
		ret     08h

NoObjectFound_Before:
		xor     eax, eax
		mov     ecx, dword ptr [esp+08h] ; [out] CGameObject * * ObjectPtr
		mov     dword ptr [ecx], eax
		ret     08h

NoObjectFound_After:
		pop    ecx
		mov    dword ptr [ecx], eax
		ret    08h

	}
}

static const unsigned long DeleteHook_RetPoint = OFFS_ObjArrayDelete + 6;

/*
 * CGameObjectArray::DeleteHook
 *
 * Remove the object from our internal object table.
 *
 */
__declspec(naked) void BugFix::DeleteHook()
{
	__asm
	{

		push    ebp
		mov     ebp, esp

		;
		; Try and delete the object from the game's array first.
		;

		push    dword ptr [ebp+0ch] ; [out] CGameObject * * ObjectPtr
		push    dword ptr [ebp+08h] ; [in] NWN::ObjectId ObjectId
		call    DoDeleteObject
		test    al, al
		jz      NoObjectFound

		mov     ecx, dword ptr [ebp+08h] ; [in] NWN::OBJECTID ObjectId
		call    BugFix::RemoveGameObject

		or      eax, 01h
NoObjectFound:
		pop     ebp
		ret     08h

DoDeleteObject:

		mov     eax, dword ptr [esp+04h]
		mov     edx, eax
		jmp     dword ptr [DeleteHook_RetPoint]

	}
}

/*
 * CServerAIMaster::AIUpdate
 *
 * Search our faster lookup table.
 *
 */
__declspec(naked) void BugFix::AIMasterUpdateState_GetObjectHook()
{
	__asm
	{

		;
		; Note, safe to NOT save eax/ecx/edx here !
		;

		mov     ecx, eax
		call    BugFix::GetGameObject
		test    eax, eax
		jz      NoObjectFound

		mov     edx, OFFS_AIMasterUpdateStateGotObj
		jmp     edx

NoObjectFound:
		mov     eax, OFFS_AIMasterUpdateStateNoObj
		jmp     eax

	}
}

/*
 * CServerAIMaster::AIUpdate
 *
 * Search our faster lookup table.
 *
 */
__declspec(naked) void BugFix::AIMasterUpdateState_GetObject2Hook()
{
	__asm
	{

		;
		; Note, safe to NOT save eax/ecx/edx here !
		;

		mov     ecx, eax
		call    BugFix::GetGameObject

#ifdef XP_BUGFIX_AIUPDATE_INSTRUMENT
		mov     [BugFix::AIUpdate_LastEventObject], eax
#endif

		test    eax, eax
		jz      NoObjectFound

		mov     edi, eax
		mov     edx, OFFS_AIMasterUpdateStateGotOb2
		jmp     edx

NoObjectFound:
		mov     eax, OFFS_AIMasterUpdateStateNoObj2
		jmp     eax

	}
}

const unsigned long AreaTransitionBMP_ReturnSkip = OFFS_TransitionBMPFixResume;
const unsigned long AreaTransitionBMP_AddressOfSetAreaTransitionBMP = OFFS_SetAreaTransitionBMP;

/* 
 * CNWSMessage::SendServerToPlayerArea_ClientArea
 *
 * The server doesn't send the right load screen data to the client if there
 * was a custom load screen.  This sends the right data if need be.
 *
 */
__declspec(naked) void BugFix::SetAreaTransitionBMPHook()
{
	__asm
	{

		;
		; Check if the load screen is set to 0, i.e. use default.
		;

		mov     eax, dword ptr [ecx+018h]
		test    eax, eax
		jz      UseDefaultLoadScreen

		;
		; Not using default, custom handling.
		;

		pushad
		lea     ecx, dword ptr [ecx+01ch]

		push    esi
		push    ecx
		push    eax
		call    BugFix::HandleAreaTransitionBMP

		;
		; Return to after we wrote the load screen id in
		; SendServerToPlayerArea_ClientArea because we sent that part of the
		; message outselves.
		;
		; But still call CNWSPlayer::SetAreaTransitionBMP so the stack based
		; string object is destructed properly.
		;

		popad
		mov     dword ptr [esp], OFFS_TransitionBMPFixResume
		jmp     dword ptr [AreaTransitionBMP_AddressOfSetAreaTransitionBMP]

		;
		; Just jump to CNWSPlayer::SetAreaTransitionBMP and let everything fall
		; through as normal.
		;

UseDefaultLoadScreen:
		jmp     dword ptr [AreaTransitionBMP_AddressOfSetAreaTransitionBMP]

	}
}

/*
 * CNWSObject::ApplyEffect
 *
 * Dynamically register the item (already checked to be an item) with the
 * AIMaster list, so that items don't pollute the list with extraneous
 * unnecessary work.  The only AIUpdate processing for Item objects is for
 * effects and this is only enabled when ApplyEffect or UpdateEffectLIstEx
 * set the ShouldUpdateEffects flag.  To optimize this case, don't register the
 * item until the first effect object is applied (most items will never have
 * an effect applied).  This reduces the amount of load in the critical path
 * CServerAIMaster::UpdateState() loop.
 */
__declspec(naked) void BugFix::AddItemToAIMasterHook()
{
	__asm
	{

		;
		; Set m_bShouldUpdateEffects to 1 as this code was replaced with the
		; branch to this procedure, but only if this effect was not a safe one.
		;

		cmp     [BugFix::NextGameEffectIsSafe], 0
		jnz     DontRegisterObject

		mov     dword ptr [eax+ItShouldUpdateEffects], 1 ; set update effects

		;
		; Register the object with the AI list.
		;
		
		lea     ecx, [eax+ItGameObjectBaseClassOffset] ; set CNWSItem
		xor     edx, edx ; set AILevel to 0
		call    BugFix::AddObjectToAIMaster ; add to tracking list

DontRegisterObject:
		pop     edi
		pop     esi
		pop     ebp
		pop     ebx
		add     esp, 08h
		ret     0ch
	}
}

/*
 * CNWSObject::UpdateEffectListEx
 *
 * Dynamically register the item (already checked to be an item) with the
 * AIMaster list, so that items don't pollute the list with extraneous
 * unnecessary work.  The only AIUpdate processing for Item objects is for
 * effects and this is only enabled when ApplyEffect or UpdateEffectLIstEx
 * set the ShouldUpdateEffects flag.  To optimize this case, don't register the
 * item until the first effect object is applied (most items will never have
 * an effect applied).  This reduces the amount of load in the critical path
 * CServerAIMaster::UpdateState() loop.
 */
__declspec(naked) void BugFix::AddItemToAIMasterHook2()
{
	__asm
	{

		;
		; Set m_bShouldUpdateEffects to ebx as this code was replaced with the
		; branch to this procedure.
		;

		cmp     dword ptr [eax+ItShouldUpdateEffects], 0 ; test for already in AI update list
		mov     dword ptr [eax+ItShouldUpdateEffects], ebx ; set update effects
		jnz     RetPoint ; if already in list, no work to do.

		;
		; Register (or deregister) the object with the AI tracking list.
		;
		; Note duplicate additions are harmlessly handled internally and this
		; is important as we are only ADDING (not removing) the object from the
		; AI tracking list based on the previous value of the update effects
		; flag (as if the flag is nonzero the object must have been on the AI
		; list, else it may or may not be on the AIList but there is no work
		; to do for it if the flag is zero).
		;
		; Most items won't transition between these two states and it is much
		; simpler just to add once and never remove.
		;
		
		lea     ecx, [eax+ItGameObjectBaseClassOffset] ; set CNWSItem

		test    ebx, ebx  ; is effect processing enabled?
		jz      RemoveObj ; if no, remove object from list

		xor     edx, edx ; set AILevel to 0
		call    BugFix::AddObjectToAIMaster ; add to tracking list
		jmp     RetPoint ; rejoin common code

RemoveObj:

		;
		; Disabling object removal for now - this does not remove many objects
		; in practice and it may result in the next object in the AIMaster
		; array being skipped this UpdateState iteration, for which the
		; implications are unclear.
		;

;		call    BugFix::RemoveObjectFromAIMaster ; remove from tracking list

RetPoint:
		pop     esi
		pop     ebx
		add     esp, 014h
		ret     08h
	}
}


/*
 * CNWSObject::ApplyEffect
 *
 * Check whether the effect being applied is safe to ignore for purposes of
 * item AI processing to minimize the size of the server AI list.
 */
__declspec(naked) void BugFix::ApplyEffectHook()
{
	__asm
	{

		;
		; Set m_bShouldUpdateEffects to 1 as this code was replaced with the
		; branch to this procedure.
		;

		mov     edx, dword ptr [esp+04h] ; get CGameEffect pointer
		call    BugFix::CheckForSafeEffect ; mark effect safe or not in NextGameEffectIsSafe
		mov     ecx, eax ; recover CGameEffect pointer

		sub     esp, 08h ; re-run prologue code replaced with the patch to this code
		mov     eax, OFFS_g_pAppManager ; get app manager offset
		mov     eax, dword ptr [eax] ; get app manager pointer
		mov     edx, OFFS_CNWSObject_ApplyEffect_Resume ; get resume point
		jmp     edx ; rejoin common code in server
	}
}

/*
 * CServerAIMaster::UpdateState
 *
 * Save the current object being updated so that it can be opportunistically
 * displayed in the debugger.
 *
 * All nonvolatiles and eax must be preserved.
 */
__declspec(naked) void BugFix::SetLastUpdateObjectHook()
{
	__asm
	{
		mov     [BugFix::AIUpdate_LastUpdateObject], eax ; set last update object
		mov     edx, OFFS_AIMasterUpdateStateGotOba ; set jump target
		jmp     edx ; rejoin common code.
	}
}

/*
 * Called instead of CServerAIMaster::UpdateState by
 * CServerExoAppInternal::MainLoop so that NPC AIUpdate throttling can be
 * prepared for this round of AIUpdates.
 */
__declspec(naked) void BugFix::AIMasterUpdateState_PrologueHook()
{
	__asm
	{
		call    BugFix::OnServerAIMaster_UpdateState ; call pre-UpdateState hook
		mov     ecx, eax ; recover this pointer
		mov     edx, OFFS_CServerAIMaster_UpdateState ; set jump target
		jmp     edx ; rejoin common code.
	}
}

/*
 * CServerAIMaster::UpdateState
 *
 * Implement a throttling policy for AIUpdate() for NPCs.
 *
 * All nonvolatiles and eax must be preserved.
 */
__declspec(naked) void BugFix::AIMasterUpdateState_GetObjectForThrottlingHook()
{
	__asm
	{
		prefetcht0 byte ptr [eax+0x000] ; prefetch cache line
		prefetcht0 byte ptr [eax+0x020] ; prefetch cache line
		prefetcht0 byte ptr [eax+GoIdSelf] ; prefetch cache line

		mov     ecx, eax ; set object to attempt to update
		call    BugFix::OnServerAIMaster_UpdateState_PreUpdateObject ; give hook code a chance to veto update
		test    eax, eax ; check if update was vetoed
		jz      SkipThisObjectUpdate ; if z, don't update this object.

		mov     edx, OFFS_AIMasterUpdateStateGotOba ; set jump target
		jmp     edx ; rejoin common code.

SkipThisObjectUpdate:
		mov     edx, OFFS_CServerAIMaster_UpdateState_NextUpdateObject ; set jump target
		jmp     edx ; rejoin common code
	}
}

#if 0

//
// Start of function hook is not used - because AddObjectToArea does an
// internal RemoveObjectFromArea call before adding the area to the server's
// internal lists.  This causes the lists to get out of sync for an object that
// is already in the area and is being re-added (i.e., DM teleport within the
// same area).
//

/*
 * CNWSArea::AddObjectToArea
 *
 * Maintains a list of area object pointers in the area.
 *
 * All nonvolatiles and ecx must be preserved.
 */
__declspec(naked) void BugFix::AddObjectToAreaHook()
{
	__asm
	{
		push    ecx ; save this pointer
		mov     edx, dword ptr [esp+08h] ; set object to add
		call    BugFix::AddObjectToArea ; maintain area object pointer list

		;
		; N.B.  Server code expects to "push ecx" too so we just avoid undoing
		;       it here.
		;

		mov     ecx, dword ptr [esp] ; reload saved this pointer
		mov     eax, OFFS_g_pAppManager ; get app manager global variable offset
		mov     eax, dword ptr [eax] ; read app manager global variable
		mov     edx, OFFS_CNWSArea_AddObjectToArea_Resume ; set jump target
		jmp     edx ; rejoin common code
	}
}

#endif

/*
 * CNWSArea::AddObjectToArea
 *
 * Maintains a list of area object pointers in the area.
 *
 * All nonvolatiles must be preserved.
 */
__declspec(naked) void BugFix::AddObjectToAreaHook()
{
	__asm
	{
		mov     ecx, ebx ; set CNWSArray pointer
		mov     edx, edi ; set CGameObject pointer
		call    BugFix::AddObjectToArea ; maintain area object pointer list

		;
		; Complete the add object to object list array operation.
		;

		mov     ecx, dword ptr [esi+08h] ; get array used element count
		mov     edx, dword ptr [esi] ; get array base
		mov     dword ptr [edx+ecx*4], ebp ; initialize new array element
		add     dword ptr [esi+08h], 1 ; increment array used element count

		mov     ecx, OFFS_CNWSArea_AddObjectToArea_Resume ; set jump target
		jmp     ecx ; rejoin common code
	}
}

/*
 * CNWSArea::RemoveObjectFromArea
 *
 * Maintains a list of area object pointers in the area.
 *
 * All nonvolatiles and ecx must be preserved.
 */
__declspec(naked) void BugFix::RemoveObjectFromAreaHook()
{
	__asm
	{
		push    ecx ; save this pointer
		mov     edx, dword ptr [esp+08h] ; set object to remove
		call    BugFix::RemoveObjectFromArea ; maintain area object pointer list
		pop     ecx ; restore this pointer

		push    ebx ; save nonvolatile
		push    esi ; save nonvolatile
		mov     esi, ecx ; save this pointer
		mov     ecx, [esi+0x0a98] ; get count of objects in area

		mov     edx, OFFS_CNWSArea_RemoveObjectFromArea_Resume ; set jump target
		jmp     edx ; rejoin common code
	}
}


#if 0
/*
 * CNWSArea::UpdatePositionInObjectsArray
 *
 * Ensure that sorting of CNWSArea::m_cObjects matches that of the shadow array
 * that is maintained by xp_bugfix.
 *
 * All nonvolatile registers and parameter registers must be preserved.
 *
 */
__declspec(naked) void BugFix::UpdatePositionInObjectsArrayHook()
{
	__asm
	{

		push    ebp
		mov     ebp, esp

		;
		; Call the game to sort the object array first.
		;

		push    ecx ; save this pointer

		push    dword ptr [ebp+10h] ; [in] NWN::Vector3 &
		push    dword ptr [ebp+0ch] ; [in] NWN::Vector3 &
		push    dword ptr [ebp+08h] ; [in] CGameObject * Object
		call    DoUpdatePositionInObjectsArray

		pop     ecx ; restore this pointer
		test    eax, eax ; test if object exists
		jz      NoObjectFound ; if z, object does not exist

		mov     edx, dword ptr [ebp+08h] ; [in] CGameObject * Object
		call    BugFix::UpdatePositionInObjectsArray

		mov     eax, 01h
NoObjectFound:
		pop     ebp
		ret     0ch

DoUpdatePositionInObjectsArray:

		sub     esp, 28h ; allocate stack frame
		push    ebx ; save nonvolatile
		push    esi ; save nonvolatile
		mov     edx, OFFS_CNWSArea_UpdatePositionInObjectsArray_Resume ; set jump target
		jmp     edx ; rejoin common code

	}
}
#endif

/*
 * CNWSArea::UpdatePositionInObjectsArray_SwapUpHook
 *
 * Ensure that sorting of CNWSArea::m_cObjects matches that of the shadow array
 * that is maintained by xp_bugfix.
 *
 * On input :
 *
 * ebx = this
 * edx = ScaledIndex
 *
 * On output :
 *
 * Scratch : ecx, esi, eax
 */
__declspec(naked) void BugFix::UpdatePositionInObjectsArray_SwapUpHook()
{
	__asm
	{

		push    edx ; save volatile register

		;
		; Swap values in both object id and pointer arrays.
		;

		mov     ecx, ebx ; set CNWSArea this pointer
		shr     edx, 2 ; convert scaled index into unscaled index
		call    BugFix::UpdatePositionInObjectsArray_SwapUp ; swap array[index+1] with array[index]

		pop     edx ; restore volatile register

		mov     ecx, OFFS_CNWSArea_UpdatePositionInObjectsArray_SwapUp_Resume ; set jump target
		jmp     ecx ; rejoin common code
	}
}

/*
 * CNWSArea::UpdatePositionInObjectsArray_SwapDownHook
 *
 * Ensure that sorting of CNWSArea::m_cObjects matches that of the shadow array
 * that is maintained by xp_bugfix.
 *
 * On input :
 *
 * ebx = this
 * edx = UnscaledIndex
 *
 * On output :
 *
 * edx = UnscaledIndex - 1
 * eflags = <result of sub UnscaledIndex, 1>
 *
 * Scratch : ecx, esi, eax
 */
__declspec(naked) void BugFix::UpdatePositionInObjectsArray_SwapDownHook()
{
	__asm
	{

		push    edx ; save volatile register

		;
		; Swap values in both object id and pointer arrays.
		;

		mov     ecx, ebx ; set CNWSArea this pointer
		call    BugFix::UpdatePositionInObjectsArray_SwapDown ; swap array[index-1] with array[index]

		pop     edx ; restore volatile register
		sub     edx, 1 ; subtract one from unscaled index

		mov     ecx, OFFS_CNWSArea_UpdatePositionInObjectsArray_SwapDown_Resume ; set jump target
		jmp     ecx ; rejoin common code
	}
}

/*
 * CNWSMessage::SortObjectsForGameObjectUpdate.
 *
 * Use an array of game object pointers instead of an array of game objects to
 * avoid very expensive pointer chasing in a N*N manner repeated for each
 * player for each area for each object in that area.
 *
 * eax points to CNWSArea.
 * ebp is the object index.
 * esi receives the retrieved object pointer.
 * ecx, edx, eax, esi can be written to.
 */
__declspec(naked) void BugFix::SortObjectsForGameObjectUpdate_GetObjectHook()
{
	__asm
	{
		mov     ecx, [eax+ArGameObjectBaseClassOffset+GoArObjList] ; get area object list array
		mov     esi, [ecx+ebp*8] ; get object from area object list
		mov     ecx, OFFS_CNWSMessage_SortObjectsForGameObjectUpdate_GotObj ; set jump target
		jmp     ecx ; rejoin common code
	}
}

/*
 * CNWSArea::GetFirstObjectIndiceByX
 *
 * Use an array of game object pointers instead of an array of game objects to
 * avoid very expensive pointer chasing in a N*N manner repeated for each
 * player for each area for each object in that area.
 *
 * ecx points to CNWSArea.
 * edx is the object index.
 * edi points to CNWSArea area object pointer list element (i.e., BugFix_AreaObjectList->m_Array + ObjectIndex).
 * esi is [g_pAppManager]
 * eax receives the retrieved object pointer.
 * ecx, ebp can be written to.
 */
__declspec(naked) void BugFix::GetFirstObjectIndiceByX_GetObjectHook()
{
	__asm
	{
		mov     eax, [edi] ; get current object pointer from current element pointer
		add     edi, 04h   ; 4 more bytes in BugFix_ObjectIndo than in game's array increment.
		mov     ecx, OFFS_CNWSArea_GetFirstObjectIndiceByX_GotObj ; set jump target
		jmp     ecx ; rejoin common code
	}
}

/*
 * CNWSMessage::DeleteLastUpdateObjectsInOtherAreas
 *
 * Eliminate most actual processing if we are in the same area as the last
 * update and the area has removed no objects since the last update.
 *
 * This avoids very expensive N*N pointer chasing through the overloaded game
 * object hash table in the critical update path per player per turn (which
 * cannot be deferred like AIUpdate processing if we are getting behind).
 *
 * On input :
 *
 * eax points to CGameObject of the player to check for updates.
 * esi points to CNWSPlayer.
 *
 * On output :
 *
 * edi points to CGameObject of the player to check for updates.
 * eax is [g_pAppManager]
 * esi is [g_pAppManager]
 *
 * On early exit return (to skip) :
 *
 * pop edi
 * pop esi
 * add esp, 0ch
 * ret 04h
 */
__declspec(naked) void BugFix::DeleteLastUpdateObjectsInOtherAreas_Hook()
{
	__asm
	{
		xor     ecx, ecx ; generate zero
		cmp     dword ptr [esi+PlrActiveObjectsLastUpdate], ecx ; is last update object list initialized?
		je      SkipUpdate ; if e, skip update

		mov     ecx, eax ; get player's CGameObject
		push    eax ; save player's CGameObject
		call    BugFix::ShouldSkipOtherAreaLUODeleteCheck ; compute update check skippable
		pop     edi ; set edi to player's CGameObject
		test    eax, eax ; is update check skippable
		jz      NoSkip ; if z, not skippable

SkipUpdate:

		;
		; No need to do an update check here because no objects have been
		; deleted.  Skip all of the work & extremely slow cache thrashing that
		; occurs in this path this time around ...
		;

		pop     edi ; restore nonvolatile
		pop     esi ; restore nonvolatile
		add     esp, 0ch ; deallocate stack frame
		ret     04h ; return from CNWSMessage::DeleteLastUpdateObjectsInOtherArea

NoSkip:

		;
		; An object has been deleted (or we switched areas) since the last
		; check.  Run the full comparison logic;
		;

		mov     eax, OFFS_g_pAppManager ; get app manager offset
		mov     eax, dword ptr [eax] ; get app manager pointer
		mov     edx, OFFS_CNWSMessage_DeleteLastUpdateObjectsInOtherAreas_Resume ; set jump target
		jmp     edx ; rejoin common code
	}
}

/*
 * CLastUpdateObject::~CLastUpdateObject
 *
 * Remove mapping entries in a player for NWN::OBJECTID to CLastUpdateObject.
 *
 * All nonvolatile registers and parameter registers must be preserved.
 *
 */
__declspec(naked) void BugFix::CLastUpdateObject_DestructorHook()
{
	__asm
	{
		push    ecx ; save this pointer

		call    BugFix::OnDeleteLUO ; delete LUO from lookup table

		pop     ecx ; recover this pointer

		push    ebp ; save nonvolatile
		push    esi ; save nonvolatile
		push    edi ; save nonvolatile
		mov     esi, ecx ; set this pointer

		mov     edx, OFFS_CLastUpdateObject_Destructor_Resume ; set jump target
		jmp     edx ; rejoin common code
	}
}

/*
 * CNWSMessage::CreateNewLastUpdateObject
 *
 * Create mapping entries in a player for NWN::OBJECTID to CLastUpdateObject.
 *
 * Return value must be preserved.
 *
 */
__declspec(naked) void BugFix::CreateNewLastUpdateObjectHook()
{
	__asm
	{
		push    ebp ; save nonvolatile
		mov     ebp, esp ; establish stack frame

		;
		; Call the game to create the LUO first.
		;

		push    dword ptr [ebp+14h] ; ULONG &
		push    dword ptr [ebp+10h] ; ULONG &
		push    dword ptr [ebp+0ch] ; [in] CNWSObject *
		push    dword ptr [ebp+08h] ; [in] CNWSPlayer *
		call    DoCreateNewLastUpdateObject

		push    eax ; save return LUO pointer
		mov     ecx, dword ptr [ebp+08h] ; set CNWSPlayer pointer
		mov     edx, eax ; set LUO pointer
		call    BugFix::OnAddLUO ; add LUO to lookup table
		pop     eax ; recover return LUO pointer

		pop     ebp ; restore nonvolatile
		ret     010h ; return

DoCreateNewLastUpdateObject:

		push    ecx ; create local variable storage space
		push    ebx ; save nonvolatile
		push    ebp ; save nonvolatile
		push    esi ; save nonvolatile
		push    edi ; save nonvolatile

		mov     edx, OFFS_CNWSMessage_CreateNewLastUpdateObject_Resume ; set jump target
		jmp     edx ; rejoin common code
	}
}

/*
 * CNWSMessage::GetLastUpdateObject
 *
 * Perform an efficient search for the associated CLastUpdateObject for a
 * given NWN::OBJECTID.  This is a hot code path for the critical GameObjUpdate
 * path (which has can't be deferred like AI update) and the normal search
 * path requires highly expensive pointer chasing.
 *
 * Return value must be preserved.
 *
 */
__declspec(naked) void BugFix::GetLastUpdateObjectHook()
{
	__asm
	{
		mov     edx, dword ptr [esp+04h] ; set object id
		call    BugFix::GetLastUpdateObject ; perform efficient LUO lookup
		ret     04h ; return
	}
}

/*
 * CNWSMessage::TestObjectUpdateDifferences
 *
 * Perform an efficient search for the associated CLastUpdateObject for a
 * given NWN::OBJECTID.  This is a hot code path for the critical GameObjUpdate
 * path (which has can't be deferred like AI update) and the normal search
 * path requires highly expensive pointer chasing.
 *
 * If an LUO is found, [edi] and edx must be set to the LUO pointer.
 *
 */
__declspec(naked) void BugFix::TestObjectUpdateDifferences_GetLUOHook()
{
	__asm
	{
		mov     ecx, ebp ; get CNWSPlayer object pointer 
		mov     edx, dword ptr [esi+GoIdSelf] ; get object id to look up from CGameObject
		call    BugFix::GetLastUpdateObject ; perform efficient LUO lookup
		test    eax, eax ; found an LUO for this object?
		jz      NoLUOFound ; if z, no LUO found

		mov     dword ptr [edi], eax ; set return LUO pointer for caller
		mov     edx, eax ; set LUO pointer for remainder of test object update differences
		mov     ecx, OFFS_CNWSMessage_TestObjectUpdateDifferences_GotLUO ; set jump target
		jmp     ecx ; rejoin common code

NoLUOFound:

		mov     edx, OFFS_CNWSMessage_TestObjectUpdateDifferences_NewLUO ; set jump target
		jmp     edx ; rejoin common code
	}
}

/*
 * NWN2_AreaSurfaceMesh::FindFace -> NWN2_SurfaceMesh::FindFace
 * NWN2_AreaSurfaceMesh::IsValid -> NWN2_SurfaceMesh::FindFace
 *
 * Check if a tile mesh index out of range of the tile mesh array was supplied
 * and return -1 in this case.  This avoids floating point math imprecision
 * from causing the four adjacent tile surface mesh lookup path to walk off
 * the tile surface mesh array.
 *
 * Forwards on to NWN2_SurfaceMesh::FindFace if the surface mesh index was
 * valid.
 *
 * N.B.  This code assumes knowledge of the internal register state of *both*
 *       its callers !!!  esi == mesh index, edi == NWN2_AreaSurfaceMesh *
 *
 */
__declspec(naked) void BugFix::CallNWN2_SurfaceMesh_FindFace_Hook()
{
	__asm
	{
		push    ecx ; save register
		push    esi ; push mesh index
		push    dword ptr [edi+AsmTileGridHeight] ; push tile grid height
		push    dword ptr [edi+AsmTileGridWidth] ; push tile grid width
		call    BugFix::CheckAreaSurfaceMeshFaceIndex
		pop     ecx ; restore register
		test    eax, eax ; was mesh index in range?
		jz      return_invalid_face ; if z, mesh index out of range

		mov     eax, OFFS_NWN2_SurfaceMesh_FindFace ; set call target
		jmp     eax ; transfer control to NWN2_SurfaceMesh::FindFace

return_invalid_face:
		or      eax, -1 ; set return value to -1 (invalid face index)
		ret     4 ; return to caller
	}
}

const ULONG CalcPathFindFaceHook1_Return = OFFS_NWN2_Pathfinder_CalcPath2_FindFaceHook1 + 6;

/*
 * NWN2_Pathfinder::CalcPath2:  Check that tile grid index is in range.
 */

__declspec(naked) void BugFix::CalcPathFindFaceHook1()
{
	__asm
	{
		push    ecx ; save register
		push    eax ; save register
		push    edx ; save register
		movzx   eax, ax ; zero extend push index
		push    eax ; push mesh index
		push    dword ptr [esi+AsmTileGridHeight] ; push tile grid height
		push    dword ptr [esi+AsmTileGridWidth] ; push tile grid width
		call    BugFix::CheckAreaSurfaceMeshFaceIndex
		test    eax, eax ; was mesh index in range?
		pop     edx ; restore register
		pop     eax ; restore register
		pop     ecx ; restore register
		jz      return_invalid_face ; if z, mesh index out of range

		movzx   eax, ax ; zero extend mesh index
		movzx   edi, ax ; zero extend mesh index

		jmp     dword ptr [CalcPathFindFaceHook1_Return] ; rejoin common code

return_invalid_face:

		pop     edi ; restore register
		pop     esi ;
		pop     ebp ;
		pop     ebx ;
		add     esp, 40h ; de-establish stack frame
		ret     14h ; return
	}
}

/*
 * NWN2_Pathfinder::ResolveSafeProjectile:  Handle no ammo being
 * equipped.
 */

__declspec(naked) void BugFix::ResolveSafeProjectileItemPatch()
{
	__asm
	{
		test    eax, eax ; was an ammo item found?
		jz      return_no_ammo_equipped ; if z, no ammo equipped

		mov     edx, dword ptr [eax+7D0h] ; get equipped item ID
		mov     ecx, OFFS_NWN2_CNWSCreature_ResolveSafeProjectileItemPatch+6 ; set jump target
		jmp     ecx ; rejoin common code

return_no_ammo_equipped:

		pop     edi ; restore register
		pop     esi ;
		pop     ebp ;
		pop     ebx ;
		ret     08h ; return
	}
}


#if 0
/*
 * NWN2_TileSurfaceMesh::Calc_Contact<0>
 *
 * Replace the x87 version of Calc_Contact<0> from nwn2server with the SSE2
 * version of the same from nwn2main.
 *
 */
__declspec(naked) void BugFix::Calc_Contact_0(__in NWN::QuickRay *Ray, __in float DistReq, __out float * IntersectDistance, __out NWN::Vector3 * IntersectFaceNormal)
{
	__asm
	{
		sub     esp, 0ch
		movss   xmm0, dword ptr [esp+014h]
		xor     eax, eax
		push    esi
		mov     esi, ecx
		cmp     dword ptr [esi+94h], eax
		movss   dword ptr [esp+04h], xmm0
		mov     dword ptr [esp+08h], eax
		jle     Retpoint0

		xorps   xmm7, xmm7
		push    ebx
		mov     ebx, dword ptr [esp+024h]
		push    ebp
		push    edi
		xor     edi, edi
		jmp short Cc01

		lea     esp, dword ptr [esp+0h]
		lea     ecx, dword ptr [ecx+0h]

Cc01:

		mov     eax, dword ptr [esi+0a0h]
		mov     edx, dword ptr [eax+edi]
		mov     ecx, dword ptr [esi+098h]
		add     eax, edi
		lea     edx, dword ptr [edx+edx*2]
		lea     ebp, dword ptr [ecx+edx*4]
		mov     edx, dword ptr [eax+4]
		mov     eax, dword ptr [eax+8]
		lea     edx, dword ptr [edx+edx*2]
		movss   xmm2, dword ptr [ecx+edx*4+8]
		lea     eax, dword ptr [eax+eax*2]
		movss   xmm0, dword ptr [ecx+eax*4+8]
		comiss  xmm0, xmm2
		lea     edx, dword ptr [ecx+edx*4]
		lea     eax, dword ptr [ecx+eax*4]
		jbe short Cc10

		movaps  xmm1, xmm2
		jmp short Cc11

Cc10:   movaps  xmm1, xmm0
Cc11:   movss   xmm6, dword ptr [ebp+08h]
		comiss  xmm1, xmm6
		jbe short Cc12
		movaps  xmm1, xmm6
		jmp short Cc14
Cc12:   comiss  xmm0, xmm2
		jbe short Cc13
		movaps  xmm1, xmm2
		jmp short Cc14
Cc13:   movaps  xmm1, xmm0
Cc14:   comiss  xmm2, xmm0
		mov     ecx, dword ptr [esp+020h]
		movss   xmm3, dword ptr [ecx+08h]
		movss   xmm4, dword ptr [ecx+040h]
		subss   xmm1, xmm3
		mulss   xmm1, xmm4
		jbe short Cc20

		movaps  xmm5, xmm2
		jmp short Cc21
Cc20:   movaps  xmm5, xmm0
Cc21:   comiss  xmm6, xmm5
		jbe short Cc22
		movaps  xmm0, xmm6
		jmp short Cc23
Cc22:   comiss  xmm2, xmm0
		jbe short Cc23
		movaps  xmm0, xmm2
Cc23:   comiss  xmm7, xmm1
		subss   xmm2, xmm3
		mulss   xmm0, xmm4
		jbe short Cc24


Retpoint0:

		pop     esi
		add     esp, 0ch
		ret     010h
	}
}
#endif



void __stdcall BugFix::HandleAreaTransitionBMP(int LoadScreenId, struct CExoString *LoadScreenName, void *MessageObject)
{
	_logger->Info(  "HandleAreaTransitionBMP: Setting load screen id %d" , LoadScreenId );

	WriteWORD((unsigned short) LoadScreenId, MessageObject);

	if (LoadScreenId == 1)
	{
		WriteCExoString(LoadScreenName, MessageObject);
	}
}

__declspec(naked) void __fastcall BugFix::WriteWORD(unsigned short Value, void * MessageObject)
{
	__asm
	{
		push    10h ; bit length
		push    ecx ; value to write
		mov     ecx, edx ; set this to MessageObject
		mov     eax, OFFS_CNWSMessage_WriteWORD
		call    eax

		ret
	}
}


__declspec(naked) void __fastcall BugFix::WriteCExoString(struct CExoString * Value, void * MessageObject)
{
	__asm
	{
		push    20h ; bit length
		push    ecx ; value to write
		mov     ecx, edx ; set this to MessageObject
		mov     eax, OFFS_CNWSMessage_WriteCExoString
		call    eax

		ret
	}
}

void __fastcall BugFix::AddGameObject(__in NWN::OBJECTID ObjectId, __in NWN::CGameObject * Object)
{
	ULONG MaskObjId = ObjectId & OBJARRAY_MASK;
	NWN::CGameObjectArrayNode * SearchNode;
	NWN::CGameObjectArrayNode * NewNode = new NWN::CGameObjectArrayNode;

	OnServerCreatedObject( Object );

	NewNode->m_objectId = ObjectId;
	NewNode->m_objectPtr = Object;
	NewNode->m_nextNode = NULL;

	if ((SearchNode = GameObjectNodes[MaskObjId]) == NULL)
	{
		GameObjectNodes[MaskObjId] = NewNode;
#if BUGFIX_LOG_GAMEOBJACCESS
		logger->Info("AddGameObject(%08x, %p) @ Toplevel", ObjectId, Object);
#endif
		return;
	}

	while (SearchNode->m_nextNode != NULL)
		SearchNode = SearchNode->m_nextNode;

	SearchNode->m_nextNode = NewNode;

#if BUGFIX_LOG_GAMEOBJACCESS
	logger->Info("AddGameObject(%08x, %p) @ Subnode %p", ObjectId, Object, SearchNode);
#endif
}

void __fastcall BugFix::AddGameObjectAtPos(__in NWN::CGameObjectArray * GameObjArray, __in NWN::OBJECTID ObjectId, __in NWN::CGameObject * Object)
{
	ULONG MaskObjId;

	OnServerCreatedObject( Object );

	//
	// N.B.  The way the game handles the AddObjectAtPos case is a mess.  The
	//       object is temporarily entered into the object array at multiple
	//       lookup indicies !
	//
	//       Instead of recreating the messy algorithm for deciding the slot
	//       to add the duplicate index in, we'll just take the data from the
	//       game object array itself.
	//
	
	if (GetGameObject( ObjectId ) != NULL)
	{
		if (ObjectId & 0x7F000000)
			ObjectId =(GameObjArray->m_nNextCharArrayID[ 0 ] + 1);
		else
			ObjectId = (GameObjArray->m_nNextObjectArrayID[ 0 ] + 1);
	}
	
	MaskObjId = ObjectId & OBJARRAY_MASK;

	NWN::CGameObjectArrayNode * SearchNode;
	NWN::CGameObjectArrayNode * NewNode = new NWN::CGameObjectArrayNode;

	NewNode->m_objectId = ObjectId;
	NewNode->m_objectPtr = Object;
	NewNode->m_nextNode = NULL;

	if ((SearchNode = GameObjectNodes[MaskObjId]) == NULL)
	{
		GameObjectNodes[MaskObjId] = NewNode;
#if BUGFIX_LOG_GAMEOBJACCESS
		logger->Info("AddGameObjectAtPos(%08x, %p) @ Toplevel", ObjectId, Object);
#endif
		return;
	}

	while (SearchNode->m_nextNode != NULL)
		SearchNode = SearchNode->m_nextNode;

	SearchNode->m_nextNode = NewNode;

#if BUGFIX_LOG_GAMEOBJACCESS
	logger->Info("AddGameObjectAtPos(%08x, %p) @ Subnode %p", ObjectId, Object, SearchNode);
#endif
}

void __fastcall BugFix::RemoveGameObject(__in NWN::OBJECTID ObjectId)
{
	NWN::CNWSArea * Area;
	ULONG MaskObjId = ObjectId & OBJARRAY_MASK;
	NWN::CGameObjectArrayNode * SearchNode;
	NWN::CGameObjectArrayNode * * PrevNodeNext;
#if BUGFIX_LOG_GAMEOBJACCESS
	logger->Info("RemoveGameObject(%08x)", ObjectId);
#endif

	PrevNodeNext = &GameObjectNodes[MaskObjId];

	if ((SearchNode = *PrevNodeNext) == NULL)
	{
		if (plugin->verboseLogging)
			_logger->Info(  "RemoveGameObject: Removing unknown game object %08X (toplevel node unmatched)" , ObjectId );
		return;
	}

	while (SearchNode->m_objectId != ObjectId)
	{
		if (SearchNode->m_nextNode == NULL)
		{
			if (plugin->verboseLogging)
				_logger->Info(  "RemoveGameObject: Removing unknown game object %08X (overflow nodelist unmatched" , ObjectId );
			return;
		}

		PrevNodeNext = &SearchNode->m_nextNode;
		SearchNode = SearchNode->m_nextNode;
	}

	*PrevNodeNext = SearchNode->m_nextNode;

	//
	// Remove the xp_bugfix area object list attached to the area if an area is
	// being deleted.
	//

	if ((Area = SearchNode->m_objectPtr->AsArea( )) != NULL)
	{
		if (Area->BugFix_AreaObjectList.m_Array != NULL)
		{
			delete [] Area->BugFix_AreaObjectList.m_Array;
			Area->BugFix_AreaObjectList.m_Array = NULL;
			Area->BugFix_AreaObjectList.m_nAllocatedSize = 0;
			Area->BugFix_AreaObjectList.m_nUsedSize = 0;
		}
	}

	delete SearchNode;

#ifdef XP_BUGFIX_GAMEOBJ_CACHE
	for (ULONG Index = 1; Index < 1 + GAME_OBJ_CACHE_SIZE; Index += 1)
	{
		if (GameObjectCache[ Index ].ObjectId == ObjectId)
		{
			GameObjectCache[ Index ].ObjectId = NWN::INVALIDOBJID;
			GameObjectCache[ Index ].Object = NULL;
			break;
		}
	}
#endif
}

void __fastcall BugFix::DeleteAllGameObjects()
{
	for (int i = 0; i < OBJARRAY_SIZE; i += 1)
	{
		NWN::CGameObjectArrayNode * Next;

		Next = GameObjectNodes[i];

		while (Next != NULL)
		{
			NWN::CGameObjectArrayNode * Node;
			Node = Next;
			Next = Node->m_nextNode;

			delete Node;
		}

		GameObjectNodes[i] = NULL;
	}

#ifdef XP_BUGFIX_GAMEOBJ_CACHE
	for (ULONG Index = 1; Index < 1 + GAME_OBJ_CACHE_SIZE; Index += 1)
	{
		GameObjectCache[ Index ].ObjectId = NWN::INVALIDOBJID;
		GameObjectCache[ Index ].Object = NULL;
	}
#endif
}

NWN::CGameObject * __fastcall BugFix::GetGameObject(__in NWN::OBJECTID ObjectId)
{
#ifdef XP_BUGFIX_GAMEOBJ_CACHE

	for (ULONG Index = 0; Index < 1 + GAME_OBJ_CACHE_SIZE; Index += 1)
	{
		if (GameObjectCache[ Index ].ObjectId == ObjectId)
		{
#ifdef XP_BUGFIX_GAMEOBJ_CACHE_STATS
			GameObjectCacheHits += 1;
#endif

			return GameObjectCache[ Index ].Object;
		}
	}

#ifdef XP_BUGFIX_GAMEOBJ_CACHE_STATS
	GameObjectCacheMisses += 1;
#endif

#else // defined(XP_BUGFIX_GAMEOBJ_CACHE)

	if (ObjectId == NWN::INVALIDOBJID)
		return NULL;

#endif

	ULONG MaskObjId = ObjectId & OBJARRAY_MASK;
	NWN::CGameObjectArrayNode * SearchNode;

	if ((SearchNode = GameObjectNodes[MaskObjId]) == NULL)
		return NULL;

	while (SearchNode->m_objectId != ObjectId)
	{
		if (SearchNode->m_nextNode == NULL)
			return NULL;

		SearchNode = SearchNode->m_nextNode;
	}

	NWN::CGameObject * Object = SearchNode->m_objectPtr;
	PreFetchCacheLine( PF_TEMPORAL_LEVEL_1, Object );

#ifdef XP_BUGFIX_GAMEOBJ_CACHE
	ULONG CacheIndex = (GameObjectCacheIndex + 1) & (GAME_OBJ_CACHE_SIZE - 1);
	GameObjectCacheIndex = CacheIndex;
	CacheIndex += 1;
	GameObjectCache[ CacheIndex ].Object = Object;
	GameObjectCache[ CacheIndex ].ObjectId = ObjectId;
#endif

	return Object;
}

void __fastcall BugFix::AddObjectToAIMaster(__in NWN::CNWSObject * Object, __in int AILevel)
{
	//
	// Add the object with the default AILevel of 0.
	//

	CServerAIMaster_AddObject( NWN::g_pAppManager->m_pServerExoApp->m_pcExoAppInternal->AIMaster, 0, Object, AILevel );

#ifdef XP_BUGFIX_AIUPDATE_THROTTLING

	//
	// Rebaseline sorting levels because a new object is going in to the
	// AIMaster's list.  It's okay to blow away these counters at any time as
	// they are transient.
	//

	if (aiUpdateThrottle != 0)
	{
		//
		// Trigger a resort of the AI object processing list and set the new
		// object to one less than the current maximum sorting level so that
		// it receives update service soon.
		//

		Object->BugFix_AISortingLevel = (aiUpdateSortingLevel == 0) ? 0 : aiUpdateSortingLevel - 1;
		ResortAIListArray = true;
	}

#endif
	
#ifdef XP_BUGFIX_AIUPDATE_INSTRUMENT

	//
	// Save the CServerAIMaster pointer for easy debugging access.
	//

	if (AIMaster == NULL)
	{
		AIMaster = NWN::g_pAppManager->m_pServerExoApp->m_pcExoAppInternal->AIMaster;
	}

#endif
}

__declspec(naked) void __fastcall BugFix::CServerAIMaster_AddObject(__in NWN::CServerAIMaster * AIMaster, __in ULONG Unused, __in NWN::CNWSObject * Object, __in int AILevel)
{
	__asm
	{
		mov     eax, OFFS_CServerAIMaster_AddObject ; Load jump target
		jmp     eax                       ; Jump to server code
	}
}

void __fastcall BugFix::RemoveObjectFromAIMaster(__in NWN::CNWSObject * Object)
{
	CServerAIMaster_RemoveObject( NWN::g_pAppManager->m_pServerExoApp->m_pcExoAppInternal->AIMaster, 0, Object );
}

__declspec(naked) void __fastcall BugFix::CServerAIMaster_RemoveObject(__in NWN::CServerAIMaster * AIMaster, __in ULONG Unused, __in NWN::CNWSObject *Object)
{
	__asm
	{
		mov     eax, OFFS_CServerAIMaster_RemoveObject ; Load jump target
		jmp     eax                       ; Jump to server code
	}
}

NWN::CNWSObject * __fastcall BugFix::CheckForSafeEffect(NWN::CNWSObject *Object, NWN::CGameEffect * Effect)
{
	bool SafeEffect;

	SafeEffect = false;

	//
	// Only item property and damage resistance effects that have a duration
	// type that is not temporary are safe to skip effect processing for in the
	// CServerAIMaster AI update cycle.
	//

	if (((Effect->m_nType == NWN::EFFECT_ITEMPROPERTY) ||
		(Effect->m_nType == NWN::EFFECT_DAMAGE_RESISTANCE)) &&
		((Effect->m_nSubType & NWN::DURATION_TYPE_MASK) != NWN::DURATION_TYPE_TEMPORARY))
	{
		SafeEffect = true;
	}

	if (SafeEffect)
		SafeGameEffectCount += 1;
	else
		UnsafeGameEffectCount += 1;

	//
	// If an unsafe effect is being applied to an item, check if it is
	// necessary to move the item to the AI list now.  Not all paths reach the
	// ApplyEffect hook.
	//

	if (SafeEffect == false)
	{
		NWN::CNWSItem * Item;

		Item = Object->AsItem( );

		if ((Item != NULL) && (Item->GetShouldUpdateEffects( ) == FALSE))
		{
			Item->SetShouldUpdateEffects( TRUE );
			AddObjectToAIMaster( Object, 0 );

			ItemDynamicAddedToAIListCount += 1;
		}
	}

	NextGameEffectIsSafe = SafeEffect;
	return Object;
}

NWN::CServerAIMaster * __fastcall BugFix::OnServerAIMaster_UpdateState(__in NWN::CServerAIMaster * AIMasterThis)
{
	//
	// Called for AIUpdate throttling just before CServerAIMaster::UpdateState
	// runs.  Sorts the CServerAIMaster update list based on AISortingLevel,
	// and records the timestamp of the start of the AIUpdate operation.
	//
	// Objects are processed in-order by CServerAIMaster::UpdateState when this
	// routine returns.  AIUpdate calls are permitted to go through until the
	// elapsed quota for AIUpdate processing in a single call is reached, at
	// which time unless the object being updated is a PC-controlled object,
	// an area, or a module, processing is skipped (by
	// OnServerAIMaster_UpdateState_PreUpdateObject).
	//
	// Each time an object that isn't a PC-controlled creature or an area or a
	// module is updated, that object's AISortingLevel is incremented by
	// OnServerAIMaster_UpdateState_PreUpdateObject.  That ensures that next
	// time around, any objects that haven't yet been processed (because they
	// fell off the end of the quota) get updated with higher priority than
	// those objects that exhausted the quota next time around.
	//

	//
	// Record the time at which AIUpdate processing just began.
	//

	AIUpdateStartTick = GetTickCount( );

	//
	// Sort the object array by AISortingLevel so that the highest priority
	// objects are examined before quota limits are hit (if they are hit this
	// update round).
	//

	if (aiUpdateThrottle != 0)
	{
		//
		// If a new object was created or a counter wrapped, reset all of the
		// sorting level counters to zero.  Any relative quota state will be
		// quickly reaccumulated and all objects are typically only off by a
		// value of 1 except for throttle immune objects.
		//

		if (RebaselineAISortingLevel == true)
		{
			RebaselineAISortingLevel = false;
			ResortAIListArray = true;

			for (int Index = 0;
			     Index < AIMasterThis->m_aGameAIList.m_aoGameObjects.m_nUsedSize;
			     Index += 1)
			{
				AIMasterThis->m_aGameAIList.m_aoGameObjects.m_Array[Index]->BugFix_AISortingLevel = 0;
			}

			AISortingLevelRebaselineCount += 1;
			aiUpdateSortingLevel = 0;
		}

		if (ResortAIListArray)
		{
			ResortAIListArray = false;

			qsort_s( AIMasterThis->m_aGameAIList.m_aoGameObjects.m_Array,
					 (rsize_t) AIMasterThis->m_aGameAIList.m_aoGameObjects.m_nUsedSize,
					 sizeof( NWN::CGameObject * ),
					 SortGameObjectsByAISortingLevel,
					 NULL );

			AIListSortCount += 1;
		}
	}

	if (overrideNetRecv != false)
	{
		if (ServerGuiWindow == NULL)
		{
			ServerGuiWindow = FindServerGuiWindow( );
		}

		if (ServerGuiWindow != NULL)
		{
			SOCKET ServerSocket = GetServerNetLayerSocket( );

			if (ServerSocket != INVALID_SOCKET)
			{
				PostMessage( ServerGuiWindow,
				             0x0404,
				             (WPARAM) ServerSocket,
				             (LPARAM) (FD_READ) );

#ifdef XP_BUGFIX_NETLAYER_INSTRUMENT
				LastPostFakeRecvIndicationTick = GetTickCount( );
#endif
			}
		}
	}
	else if (directPollNetRecv != false)
	{
		SOCKET ServerSocket = GetServerNetLayerSocket( );

		if (ServerSocket != INVALID_SOCKET)
		{
			struct CNetLayerInternal * NetLayerInt;

			NetLayerInt = GetServerNetLayer( );

			if (NetLayerInt != NULL)
			{
#ifdef XP_BUGFIX_NETLAYER_INSTRUMENT
				LastDirectNetRecvTick = GetTickCount( );
#endif

				//
				// Push down calls to recvfrom until one fails or we reach the
				// limit on inline recvfrom calls this iteration.
				//

				for (ULONG Count = 0; Count < 50; Count += 1)
				{
					RecvfromSuccess = false;

					CallCNetLayerInternal_MessageArrived( NetLayerInt,
														  NULL, 
														  CExoNet::PROTOCOL_UDP,
														  ServerSocket,
														  0,
														  1 );

					if (RecvfromSuccess == false)
					{
						break;
					}
				}
			}
		}
	}

#ifdef XP_BUGFIX_AIUPDATE_INSTRUMENT

	//
	// Save the CServerAIMaster pointer for easy debugging access.
	//

	if (BugFix::AIMaster == NULL)
	{
		BugFix::AIMaster = AIMasterThis;
	}

	if (aiUpdateThrottle != 0)
	{
		if (ThisLoopThrottledObjectCount != 0)
		{
			AIUpdateIterationsThrottledCount += 1;

			ThisLoopThrottledObjectCount = 0;
		}
	}

#endif

	return AIMasterThis;
}

NWN::CGameObject * __fastcall BugFix::OnServerAIMaster_UpdateState_PreUpdateObject(__in NWN::CGameObject * Object)
{
	if (aiUpdateThrottle != 0)
	{
		bool CheckThrottling;
		ULONG DesiredSortingLevel;
		bool ForcedNPCUpdate;
		USHORT SortingLevel;

		CheckThrottling = false;
		ForcedNPCUpdate = false;
		DesiredSortingLevel = 0;
		SortingLevel = 0;
		switch (Object->GetObjectType( ))
		{

		case NWN::OBJECT_TYPE_AREA:
		case NWN::OBJECT_TYPE_MODULE:

			//
			// No throttling for areas or modules.
			//
			// N.B.  This code could probably be removed as the module and
			//       area objects appear to be updated separately through a
			//       direct call that CServerAIMaster::UpdateState makes to
			//       CNWSModule::AIUpdate().  That direct call is not subject
			//       to any throttling, which is OK as modules and areas are
			//       considered critical objects that are to be prioritized.
			//

			break;

		case NWN::OBJECT_TYPE_CREATURE:
			{
				NWN::CNWSCreature * Creature = Object->AsCreature( );

				if (Creature == NULL)
					break;

				if ((Creature->GetIsPlayerCharacter( ) != FALSE) ||
					(Creature->GetControllingPlayerId( ) != PLAYERID_INVALIDID))
				{
					//
					// No throttling for players or player controlled
					// characters.  However, player controlled characters that
					// are NPCs still get their priority increased so that they
					// remain in the same level band (roughly) as the rest of
					// the creature objects for quota accounting if control is
					// released via unpossess, etc.
					//

					if (Creature->GetIsPlayerCharacter( ) == FALSE)
					{
						ForcedNPCUpdate = true;
					}
					else
					{
						//
						// Compute and set (if necessary) the sorting level for
						// this player object.
						//

						if (aiUpdateSortPlayersFirst == false)
						{
							DesiredSortingLevel = (USHORT) ~0;
						}
						else
						{
							DesiredSortingLevel = 0;
						}

						if (Creature->BugFix_AISortingLevel != DesiredSortingLevel)
						{
							Creature->BugFix_AISortingLevel = DesiredSortingLevel;
							ResortAIListArray = true;
						}
					}

					break;
				}

				//
				// No throttling for creatures that are in a dialog as players
				// may be blocked on the dialog reply being processed in a
				// timely fashion.
				//

				if (Creature->AsNWSObject( )->GetDialogOwner( ) != NWN::INVALIDOBJID)
				{
					ForcedNPCUpdate = true;
					break;
				}

				CheckThrottling = true;

				break;
			}

		case NWN::OBJECT_TYPE_PLACEABLE:
			{
				NWN::CNWSObject * Placeable = Object->AsNWSObject( );

				//
				// No throttling for placeables that are in a dialog as players
				// may be blocked on the dialog reply being processed in a
				// timely fashion.
				//

				if (Placeable->GetDialogOwner( ) != NWN::INVALIDOBJID)
				{
					ForcedNPCUpdate = true;
					break;
				}

				CheckThrottling = true;

				break;
			}

		default:
			CheckThrottling = true;
			break;

		}

		//
		// If this object is eligible for throttling, then check whether the
		// quota has been exceeded.  If so, throttle updates for this object.
		// Otherwise, increment the sorting level to deprioritize it relative
		// to other objects that may be throttled.  Note that it is ok if this
		// counter wraps.
		//

		if (CheckThrottling)
		{
			ULONG Now = GetTickCount( );

			if ((Now - AIUpdateStartTick) >= aiUpdateThrottle)
			{
#ifdef XP_BUGFIX_AIUPDATE_INSTRUMENT
				ThrottledObjectCount += 1;
				ThisLoopThrottledObjectCount += 1;
#endif

				//
				// Because all counters didn't increment uniformly and there
				// is throttling going on, the AIList array has to be resorted
				// next loop iteration.
				//

				ResortAIListArray = true;

				Object = NULL;
			}
			else
			{
#ifdef XP_BUGFIX_AIUPDATE_INSTRUMENT
				NonThrottledObjectCount += 1;
#endif

				if ((Object->BugFix_AISortingLevel += 1) == 0)
				{
					//
					// Rebaseline sorting levels on counter wrap.  These are
					// transient state anyway so it's okay to blow the counts
					// away at will.
					//

					RebaselineAISortingLevel = true;
				}

				//
				// Update the higher watermark sorting level value.
				//
				// N.B.  The high watermark deliberately does not include
				//       objects that are immune to throttling.
				//

				SortingLevel = Object->BugFix_AISortingLevel;

				if (SortingLevel > aiUpdateSortingLevel)
				{
					aiUpdateSortingLevel = SortingLevel;
				}
			}
		}
		else
		{
			//
			// Clear the sorting level for objects that are not eligible for
			// throttling except for forced NPC updates.  Forced NPC updated
			// objects must maintain sorting levels for processing priority
			// correctness once the forced processing status is rescinded so
			// set them to the current high watermark value.
			//

			if (ForcedNPCUpdate)
			{
#ifdef XP_BUGFIX_AIUPDATE_INSTRUMENT
				ForcedNPCUpdateObjectCount += 1;
#endif

				Object->BugFix_AISortingLevel = aiUpdateSortingLevel;
			}
			else
			{
#ifdef XP_BUGFIX_AIUPDATE_INSTRUMENT
				ThrottleImmuneObjectCount += 1;
#endif

				Object->BugFix_AISortingLevel = DesiredSortingLevel;
			}
		}
	}

#ifdef XP_BUGFIX_AIUPDATE_INSTRUMENT

	//
	// Save away the object about to be updated for debugging purposes.
	//

	if (Object != NULL)
	{
		AIUpdate_LastUpdateObject = Object;
	}

#endif

	return Object;
}

void BugFix::OnServerCreatedObject(__in NWN::CGameObject * Object)
{
	NWN::CNWSArea * Area;
	NWN::CNWSObject * NWSObject;

	//
	// Called when XP_BUGFIX_GAMEOBJARRAY_HOOK is enabled and the server has
	// created a new CGameObject.  Used to initialize private fields stored in
	// the game's padding for the object.
	//

	//
	// Initialize the AISortingLevel variable used to throttle AIUpdate
	// processing for NPCs.
	//

#ifdef XP_BUGFIX_AIUPDATE_THROTTLING
	if (aiUpdateThrottle != 0)
	{
		//
		// Trigger a resort of the AI object processing list and set the new
		// object to one less than the current maximum sorting level so that
		// it receives update service soon.
		//

		Object->BugFix_AISortingLevel = (aiUpdateSortingLevel == 0) ? 0 : aiUpdateSortingLevel - 1;
		ResortAIListArray = true;
	}
#endif

	//
	// If this is an area object, initialize the area object list.  This will
	// be updated when CNWSArea::AddObjectToArea and
	// CNWSArea::RemoveObjectFromArea are called.  The list is deallocated when
	// BugFix::RemoveGameObject is called.
	//
	// The purpose of the list is to remove (extremely expensive) inner loop
	// pointer chasing in last update object computation for object ID to
	// object pointer resolution.
	//

	if ((Area = Object->AsArea( )) != NULL)
	{
		Area->BugFix_AreaObjectList.m_Array = NULL;
		Area->BugFix_AreaObjectList.m_nAllocatedSize = 0;
		Area->BugFix_AreaObjectList.m_nUsedSize = 0;
		Object->BugFix_UpdateGeneration = 0;
	}


	//
	// Clear update generation information that is used to speed up last update
	// object computation.  This number is incremented each time an area has
	// an object removed.  On last update computation for removing deleted or
	// gone from this area objects from the client's view, the update compare
	// can be skipped if the area is the same as the last check AND the area
	// has removed no objects since the last check.
	//

	if (((NWSObject = Object->AsNWSObject( )) != NULL) && (Object->GetObjectType( ) == NWN::OBJECT_TYPE_CREATURE))
	{
		NWSObject->GetBugFix_CNWSObject( )->LastUpdateAreaObjectId = NWN::INVALIDOBJID;
		Object->BugFix_UpdateGeneration = 0;
	}
}

int __cdecl BugFix::SortGameObjectsByAISortingLevel(__in void * Context, __in const void * Object0, __in const void * Object1)
{
	const NWN::CGameObject * GameObj0 = (const NWN::CGameObject *) (*(PVOID *) Object0);
	const NWN::CGameObject * GameObj1 = (const NWN::CGameObject *) (*(PVOID *) Object1);

	if (GameObj0->BugFix_AISortingLevel < GameObj1->BugFix_AISortingLevel)
	{
		return -1;
	}
	else if (GameObj0->BugFix_AISortingLevel == GameObj1->BugFix_AISortingLevel)
	{
		return 0;
	}
	else
	{
		return 1;
	}
} 

void BugFix::CheckAreaObjectLists(__in NWN::CNWSArea * Area)
{
	if (devMode == false)
	{
		return;
	}

	if (Area->BugFix_AreaObjectList.m_nUsedSize != Area->GetObjects( ).m_nUsedSize)
	{
		_logger->Info(  "CheckAreaObjectLists: Area object lists in area %p (%08X) are out of sync (wrong sizes)" , (void *) Area, Area->GetObjectId( ) );
		PrintObjectLists( Area );
		__debugbreak();
	}

	for (int i = 0; i < Area->BugFix_AreaObjectList.m_nUsedSize; i += 1)
	{
		if (Area->BugFix_AreaObjectList.m_Array[ i ].ObjectId != Area->BugFix_AreaObjectList.m_Array[ i ].Object->GetObjectId( ))
		{
			_logger->Info(  "CheckAreaObjectLists: Area object IDs (cache, id in object) in area %p (%08X) are out of sync (index %d)" , (void *) Area, Area->GetObjectId( ), i );
			PrintObjectLists( Area );
			__debugbreak();
		}

		if (Area->BugFix_AreaObjectList.m_Array[ i ].ObjectId != Area->GetObjects( ).m_Array[ i ])
		{
			_logger->Info(  "CheckAreaObjectLists: Area object lists in area %p (%08X) are out of sync (index %d)" , (void *) Area, Area->GetObjectId( ), i );
			PrintObjectLists( Area );
			__debugbreak();
		}
	}
}

void __fastcall BugFix::AddObjectToArea(__in NWN::CNWSArea * Area, __in NWN::CGameObject * Object)
{
	NWN::OBJECTID ObjectId;

	ObjectId = Object->GetObjectId( );

	CheckAreaObjectLists( Area );

	//
	// Duplicate objects may be added, these must be ignored.
	//

	for (int i = 0; i < Area->BugFix_AreaObjectList.m_nUsedSize; i += 1)
	{
		if (Area->BugFix_AreaObjectList.m_Array[ i ].ObjectId == ObjectId)
		{
			return;
		}
	}

	if (Area->BugFix_AreaObjectList.m_nUsedSize == Area->BugFix_AreaObjectList.m_nAllocatedSize)
	{
		NWN::BugFix_ObjectInfo * ObjectArray;
		
		ObjectArray = new NWN::BugFix_ObjectInfo[ (Area->BugFix_AreaObjectList.m_nAllocatedSize + 64) * 2 ];
		memcpy( ObjectArray,
		        Area->BugFix_AreaObjectList.m_Array,
		        Area->BugFix_AreaObjectList.m_nUsedSize * sizeof( NWN::BugFix_ObjectInfo ) );

		if (Area->BugFix_AreaObjectList.m_Array != NULL)
		{
			delete [] Area->BugFix_AreaObjectList.m_Array;
		}

		Area->BugFix_AreaObjectList.m_Array = ObjectArray;
		Area->BugFix_AreaObjectList.m_nAllocatedSize = (Area->BugFix_AreaObjectList.m_nAllocatedSize + 64) * 2;
	}

	Area->BugFix_AreaObjectList.m_Array[ Area->BugFix_AreaObjectList.m_nUsedSize ].Object = Object;
	Area->BugFix_AreaObjectList.m_Array[ Area->BugFix_AreaObjectList.m_nUsedSize ].ObjectId = ObjectId;
	Area->BugFix_AreaObjectList.m_nUsedSize += 1;

	if (Area->BugFix_AreaObjectList.m_nUsedSize - 1 != Area->GetObjects( ).m_nUsedSize)
	{
		_logger->Info(  "AddObjectToArea: Area object lists in area %p (%08X) are out of sync" , (void *) Area, Area->GetObjectId( ) );
		__debugbreak();
	}
}

void __fastcall BugFix::RemoveObjectFromArea(__in NWN::CNWSArea * Area, __in NWN::OBJECTID ObjectId)
{
	CheckAreaObjectLists( Area );

	for (int i = 0; i < Area->BugFix_AreaObjectList.m_nUsedSize; i += 1)
	{
		if (Area->BugFix_AreaObjectList.m_Array[ i ].ObjectId == ObjectId)
		{
			memmove( &Area->BugFix_AreaObjectList.m_Array[ i ],
			         &Area->BugFix_AreaObjectList.m_Array[ i + 1 ],
			         (Area->BugFix_AreaObjectList.m_nUsedSize - (i + 1)) * sizeof( NWN::BugFix_ObjectInfo ) );
			Area->BugFix_AreaObjectList.m_nUsedSize -= 1;
			Area->BugFix_AreaObjectList.m_Array[ Area->BugFix_AreaObjectList.m_nUsedSize ].Object = (NWN::CGameObject *) (INT_PTR) (0xFFFF0000);
			Area->BugFix_AreaObjectList.m_Array[ Area->BugFix_AreaObjectList.m_nUsedSize ].ObjectId = NWN::INVALIDOBJID;

			//
			// Bump the removal serial number on the area for last update
			// object computation to trigger object deletion notifications to
			// all connected players in the area.
			//

			Area->BugFix_UpdateGeneration += 1;

			return;
		}
	}

	_logger->Info(  "RemoveObjectFromArea: Removing unknown game object %08X from area %p (%08X)" , ObjectId, (void *) Area, Area->GetObjectId( ) );
	return;

	if (Area->BugFix_AreaObjectList.m_nUsedSize + 1 != Area->GetObjects( ).m_nUsedSize)
	{
		_logger->Info(  "RemoveObjectFromArea: Area object lists in area %p (%08X) are out of sync" , (void *) Area, Area->GetObjectId( ) );
		__debugbreak();
	}
}

/*
void __fastcall BugFix::UpdatePositionInObjectsArray(__in NWN::CNWSArea * Area, __in NWN::CGameObject * GameObject)
{
	NWN::BugFix_ObjectInfo * PtrArray;
	NWN::OBJECTID * ObjIdArray;
	int SwapIdx;
	int SwapCount;
	NWN::OBJECTID SwapObjId;
	int Count;
	bool Resort = false;

	Count = Area->GetObjects( ).m_nUsedSize;
	if (Area->BugFix_AreaObjectList.m_nUsedSize != Count)
	{
		logger->Info( wxT( "UpdatePositionInObjectsArray: Area object lists in area %p (%08X) are out of sync" ), (void *) Area, Area->GetObjectId( ) );
		__debugbreak();
	}

	PtrArray = Area->BugFix_AreaObjectList.m_Array;
	ObjIdArray = Area->GetObjects( ).m_Array;

	//
	// Sort objects in the accelerator by pointer array to match the ordering
	// in the by object ID array.
	//

	do {
		SwapCount = 0;
		SwapIdx = -1;
		SwapObjId = NWN::INVALIDOBJID;
		Resort = false;

		for (int i = 0; i < Count; i += 1)
		{
			if (SwapIdx == -1)
			{
				if (PtrArray[ i ].ObjectId == ObjIdArray[ i ])
				{
					continue;
				}

				SwapIdx = i;
				SwapObjId = ObjIdArray[ i ];
			}
			else
			{
				if (PtrArray[ i ].ObjectId != SwapObjId)
				{
					continue;
				}

				NWN::BugFix_ObjectInfo Swap = PtrArray[ SwapIdx ];
				PtrArray[ SwapIdx ] = PtrArray[ i ];
				PtrArray[ i ] = Swap;
				SwapIdx = -1;
				Resort = true;
				break;
			}
		}

		if (SwapIdx != -1)
		{
			logger->Info( wxT( "UpdatePositionInObjectsArray: Area object lists in area %p (%08X) are out of sync, SwapIdx %d has no partner" ), (void *) Area, Area->GetObjectId( ), SwapIdx );
			__debugbreak();
		}
	} while (Resort != false);

	if (SwapCount > 1)
	{
		logger->Info( wxT( "UpdatePositionInObjectsArray: Swap count %d" ), SwapCount );
	}

	CheckAreaObjectLists( Area );
}
*/

void __fastcall BugFix::UpdatePositionInObjectsArray_SwapUp(__in NWN::CNWSArea * Area, __in int Index)
{
	NWN::OBJECTID SwapObjectId;
	NWN::BugFix_ObjectInfo SwapObjectInfo;
	NWN::NWN2_DynamicArrayBlittable<NWN::OBJECTID> & Objects = Area->GetObjects( );
	NWN::NWN2_DynamicArrayBlittable<NWN::BugFix_ObjectInfo> & Pointers = Area->BugFix_AreaObjectList;

	if (Index >= Pointers.m_nUsedSize)
	{
		_logger->Info(  "UpdatePositionInObjectsArray_SwapUp: Index %d exceeds BugFix_AreaObjectList.m_nUsedSize %d (Objects.m_nUsedSize == %d)" , Index, Pointers.m_nUsedSize, Objects.m_nUsedSize );
		return;
	}

	if (Objects.m_nUsedSize != Pointers.m_nUsedSize)
	{
		_logger->Info(  "UpdatePositionInObjectsArray_SwapUp: BugFix_AreaObjectList.m_nUsedSize %d != Objects.m_nUsedSize %d" , Index, Pointers.m_nUsedSize, Objects.m_nUsedSize );
	}

	SwapObjectId = Objects.m_Array[ Index + 1 ];
	Objects.m_Array[ Index + 1 ] = Objects.m_Array[ Index ];
	Objects.m_Array[ Index ] = SwapObjectId;

	SwapObjectInfo = Pointers.m_Array[ Index + 1 ];
	Pointers.m_Array[ Index + 1 ] = Pointers.m_Array[ Index ];
	Pointers.m_Array[ Index ] = SwapObjectInfo;
}

void __fastcall BugFix::UpdatePositionInObjectsArray_SwapDown(__in NWN::CNWSArea * Area, __in int Index)
{
	NWN::OBJECTID SwapObjectId;
	NWN::BugFix_ObjectInfo SwapObjectInfo;
	NWN::NWN2_DynamicArrayBlittable<NWN::OBJECTID> & Objects = Area->GetObjects( );
	NWN::NWN2_DynamicArrayBlittable<NWN::BugFix_ObjectInfo> & Pointers = Area->BugFix_AreaObjectList;

	if (Index >= Pointers.m_nUsedSize)
	{
		_logger->Info(  "UpdatePositionInObjectsArray_SwapDown: Index %d exceeds BugFix_AreaObjectList.m_nUsedSize %d (Objects.m_nUsedSize == %d)" , Index, Pointers.m_nUsedSize, Objects.m_nUsedSize );
		return;
	}

	if (Objects.m_nUsedSize != Pointers.m_nUsedSize)
	{
		_logger->Info(  "UpdatePositionInObjectsArray_SwapUp: BugFix_AreaObjectList.m_nUsedSize %d != Objects.m_nUsedSize %d" , Index, Pointers.m_nUsedSize, Objects.m_nUsedSize );
	}

	SwapObjectId = Objects.m_Array[ Index - 1 ];
	Objects.m_Array[ Index - 1 ] = Objects.m_Array[ Index ];
	Objects.m_Array[ Index ] = SwapObjectId;

	SwapObjectInfo = Pointers.m_Array[ Index - 1 ];
	Pointers.m_Array[ Index - 1 ] = Pointers.m_Array[ Index ];
	Pointers.m_Array[ Index ] = SwapObjectInfo;
}

void BugFix::CheckLUOTable(__in NWN::CNWSPlayer * Player)
{
	NWN::NWN2_DynamicArrayBlittable<NWN::BugFix_LUOMap> * LUOMap;
	NWN::BugFix_LUOMap * LUOTable;
	NWN::OBJECTID ObjectId;

	if (devMode == false)
	{
		return;
	}

	LUOMap = Player->BugFix_LUOMap;
	LUOTable = LUOMap->m_Array;
	ObjectId = NWN::INVALIDOBJID;

	for (int i = 0; i < LUOMap->m_nUsedSize; i += 1)
	{
		if (LUOTable[ i ].LUO->GetPlayer( ) != Player)
		{
			_logger->Info(  "CheckLUOTable: LUO %p at index %d has wrong player %p (should be %p)" , LUOTable[ i ].LUO, i, (void *) LUOTable[ i ].LUO->GetPlayer( ), (void *) Player );
			__debugbreak();
		}

		if (i == 0)
		{
			ObjectId = LUOTable[ i ].ObjectId;
			continue;
		}

		if (LUOTable[ i ].ObjectId <= ObjectId)
		{
			_logger->Info(  "CheckLUOTable: Incorrect sorting in LUO table at index %d for player %p" , i, (void *) Player );
			__debugbreak();
		}

		ObjectId = LUOTable[ i ].ObjectId;
	}
}

void __fastcall BugFix::OnAddLUO(__in NWN::CNWSPlayer * Player, __in NWN::CLastUpdateObject * LUO)
{
	NWN::OBJECTID ObjectId;
	NWN::NWN2_DynamicArrayBlittable<NWN::BugFix_LUOMap> * LUOMap;
	NWN::BugFix_LUOMap * LUOEntry;
	NWN::BugFix_LUOMap * LUOTable;
	LONG High;
	LONG Low;
	LONG Middle;

	//
	// Establish backlink between LUO and player.
	//

	LUO->SetPlayer( Player );

	//
	// Check if we just added the first LUO up front, as in this case there is
	// no LUO table yet & it must be created.
	//

	if (Player->m_pActiveObjectsLastUpdate->m_nCount == 1)
	{
		LUOMap = new NWN::NWN2_DynamicArrayBlittable<NWN::BugFix_LUOMap>;
		LUOMap->m_Array = new NWN::BugFix_LUOMap[ 4096 ];
		LUOMap->m_nAllocatedSize = 4096;
		LUOMap->m_nUsedSize = 0;
		LUOTable = LUOMap->m_Array;
		Player->BugFix_LUOMap = LUOMap;
	}
	else
	{
		LUOMap = Player->BugFix_LUOMap;
		LUOTable = LUOMap->m_Array;
	}

	if (LUOMap->m_nUsedSize == LUOMap->m_nAllocatedSize)
	{
		NWN::BugFix_LUOMap * LUOTable;

		LUOTable = LUOMap->m_Array;
		LUOMap->m_nAllocatedSize *= 2;
		LUOMap->m_Array = new NWN::BugFix_LUOMap[ LUOMap->m_nAllocatedSize ];
		memcpy( LUOMap->m_Array,
		        LUOTable,
		        sizeof( NWN::BugFix_LUOMap ) * LUOMap->m_nUsedSize );
		delete [] LUOTable;
	}

	//
	// Perform a binary search to narrow down an insertion index.
	//

	ObjectId = LUO->m_nId;
	LUOEntry = NULL;

	Low = 0;
	High = LUOMap->m_nUsedSize - 1;

	while (High >= Low)
	{
		Middle = (Low + High) >> 1;
		LUOEntry = &LUOTable[ Middle ];

		if (ObjectId < LUOEntry->ObjectId)
		{
			if (Middle == 0)
			{
				LUOEntry = NULL;
				break;
			}

			High = Middle - 1;
		}
		else if (ObjectId > LUOEntry->ObjectId)
		{
			Low = Middle + 1;
		}
		else
		{
			break;
		}
	}

	if (High < Low)
	{
		Low = 0;
		High = LUOMap->m_nUsedSize - 1;

		if (LUOMap->m_nUsedSize == 0)
		{
			LUOTable[ 0 ].ObjectId = LUO->m_nId;
			LUOTable[ 0 ].LUO = LUO;
			LUOMap->m_nUsedSize += 1;
			return;
		}
	}
	else
	{
		if (LUOEntry != NULL)
		{
			_logger->Info(  "OnAddLUO: Created LUO %p for object %08x for player %p but that LUO %p already exists!" , (void *) LUO, ObjectId, (void *) Player, (void *) LUOEntry->LUO );
			__debugbreak();
			return;
		}
	}

	for (Middle = max( 0, Low ); Middle < High + 1; Middle += 1)
	{
		if (LUOTable[ Middle ].ObjectId > ObjectId)
		{
			break;
		}
	}

	memmove( &LUOTable[ Middle + 1 ],
	         &LUOTable[ Middle ],
	         (LUOMap->m_nUsedSize - Middle) * sizeof( NWN::BugFix_LUOMap ) );
	LUOMap->m_nUsedSize += 1;
	LUOTable[ Middle ].ObjectId = ObjectId;
	LUOTable[ Middle ].LUO = LUO;

	CheckLUOTable( Player );

	return;
}

void __fastcall BugFix::OnDeleteLUO(__in NWN::CLastUpdateObject * LUO)
{
	NWN::CNWSPlayer * Player;
	NWN::NWN2_DynamicArrayBlittable<NWN::BugFix_LUOMap> * LUOMap;
	NWN::BugFix_LUOMap * LUOEntry;
	NWN::BugFix_LUOMap * LUOTable;
	LONG High;
	LONG Low;
	LONG Middle;
	NWN::OBJECTID ObjectId;

	//
	// Recover the associated CNWSPlayer from the LUO backlink maintained by
	// xp_bugfix and remove the LUO from the look up table.
	//

	Player = LUO->GetPlayer( );
	ObjectId = LUO->m_nId;

	LUOMap = Player->BugFix_LUOMap;
	LUOTable = LUOMap->m_Array;
	LUOEntry = NULL;

	Low = 0;
	High = LUOMap->m_nUsedSize - 1;

	while (High >= Low)
	{
		Middle = (Low + High) >> 1;
		LUOEntry = &LUOTable[ Middle ];

		if (ObjectId < LUOEntry->ObjectId)
		{
			if (Middle == 0)
			{
				_logger->Info(  "OnDeleteLUO: LUO %p (%08x) for player %p doesn't exist in LUO table!" , (void *) LUO, ObjectId, (void *) Player );
				__debugbreak();
				return;
			}

			High = Middle - 1;
		}
		else if (ObjectId > LUOEntry->ObjectId)
		{
			Low = Middle + 1;
		}
		else
		{
			break;
		}
	}

	if (High < Low)
	{
		_logger->Info(  "OnDeleteLUO: LUO %p (%08x) for player %p doesn't exist in LUO table!" , (void *) LUO, ObjectId, (void *) Player );
		__debugbreak();
		return;
	}

	memmove( &LUOTable[ Middle ],
	         &LUOTable[ Middle + 1 ],
	         (LUOMap->m_nUsedSize - (Middle + 1) ) * sizeof( NWN::BugFix_LUOMap ) );
	LUOMap->m_nUsedSize -= 1;
	LUOTable[ LUOMap->m_nUsedSize ].ObjectId = NWN::INVALIDOBJID;
	LUOTable[ LUOMap->m_nUsedSize ].LUO = (NWN::CLastUpdateObject *) (INT_PTR) (0xFFFF0000);

	//
	// The entire LUO map has to be deleted when it has no more entries in it
	// as otherwise it would be leaked because :
	//
	// 1) Since we aren't hooking CNWSPlayer::CNWSPlayer, we don't otherwise
	//    have a constructor callout and thus don't know if the field is yet
	//    initialized in the CNWSPlayer unless by checking for a zero size.
	//
	// 2) Since we aren't hooking CNWSPlayer::~CNWSPlayer, we don't otherwise
	//    have a destructor callout (so otherwise the table would leak).
	//

	if (LUOMap->m_nUsedSize == 0)
	{
		delete [] LUOTable;
		delete LUOMap;
		Player->BugFix_LUOMap = (NWN::NWN2_DynamicArrayBlittable<NWN::BugFix_LUOMap> *) (INT_PTR) (0xFFFF0000);
	}

	return;
}

NWN::CLastUpdateObject * __fastcall BugFix::GetLastUpdateObject(__in NWN::CNWSPlayer * Player, __in NWN::OBJECTID ObjectId)
{
	NWN::NWN2_DynamicArrayBlittable<NWN::BugFix_LUOMap> * LUOMap;
	NWN::BugFix_LUOMap * LUOEntry;
	NWN::BugFix_LUOMap * LUOTable;
	LONG High;
	LONG Low;
	LONG Middle;

	//
	// Check for NULL first because otherwise the LUO map is not guaranteed to
	// be initialized.
	//

	if ((Player->m_pActiveObjectsLastUpdate == NULL) ||
	    (Player->m_pActiveObjectsLastUpdate->m_nCount == 0))
	{
		return NULL;
	}

	//
	// Perform a binary search against the look up table.
	//

	LUOMap = Player->BugFix_LUOMap;
	LUOTable = LUOMap->m_Array;
	LUOEntry = NULL;

	Low = 0;
	High = LUOMap->m_nUsedSize - 1;

	while (High >= Low)
	{
		Middle = (Low + High) >> 1;
		LUOEntry = &LUOTable[ Middle ];

		if (ObjectId < LUOEntry->ObjectId)
		{
			if (Middle == 0)
			{
				return NULL;
			}

			High = Middle - 1;
		}
		else if (ObjectId > LUOEntry->ObjectId)
		{
			Low = Middle + 1;
		}
		else
		{
			break;
		}
	}

	if (High < Low)
	{
		return NULL;
	}

	PreFetchCacheLine( PF_TEMPORAL_LEVEL_1, LUOEntry->LUO );

	return LUOEntry->LUO;
}

BOOL __stdcall BugFix::CheckAreaSurfaceMeshFaceIndex(__in ULONG Width, __in ULONG Height, __in ULONG Index)
{
	//
	// Continue if the index is in range, else log an error (at most once per
	// second).
	//

	if (Index < Width * Height)
		return TRUE;

	ULONG now = GetTickCount();

	if (now - plugin->lastlog > 1000)
	{
		plugin->lastlog = now;

		_logger->Info(  "CheckAreaSurfaceMeshFaceIndex: Avoided NWN2_AreaSurfaceMesh.m_TileGrid overrun in FindFace/IsValid."  );
	}

	return FALSE;
}

BOOL __fastcall BugFix::ShouldSkipOtherAreaLUODeleteCheck(__in NWN::CGameObject *PlayerCreature)
{
	NWN::CGameObject * AreaObject;
	NWN::OBJECTID AreaId;
	NWN::OBJECTID LastAreaId;
	NWN::BugFix_CNWSObject * BugFixNWSObject;
	NWN::CNWSObject * NWSObject;
	unsigned long UpdateGeneration;

	NWSObject = PlayerCreature->AsNWSObject( );
	BugFixNWSObject = NWSObject->GetBugFix_CNWSObject( );

	AreaId = NWSObject->GetArea( );
	LastAreaId = BugFixNWSObject->LastUpdateAreaObjectId;
	AreaObject = GetGameObject( AreaId );
	if (AreaObject == NULL)
	{
		return FALSE;
	}

	if (AreaObject->AsArea( ) == NULL)
	{
		_logger->Info("ShouldSkipOtherAreaLUODeleteCheck: %08x (%p) is not an area!", AreaId, (void *) AreaObject );
		__debugbreak();
		return FALSE;
	}

	//
	// If the player creature is in a different area than the last update, or
	// the area has removed objects since the last update, then do not skip the
	// deletion notification computation.
	//

	UpdateGeneration = AreaObject->BugFix_UpdateGeneration;
	if ((AreaId != LastAreaId) || (NWSObject->BugFix_UpdateGeneration != UpdateGeneration))
	{
		BugFixNWSObject->LastUpdateAreaObjectId = AreaId;
		NWSObject->BugFix_UpdateGeneration = UpdateGeneration;
		LUOAreaDeletionCheckRunCount += 1;
		return FALSE;
	}

	//
	// No objects could have possibly been deleted since the last check, so
	// there is no need to run the (very costly) area object deletion check.
	//

	LUOAreaDeletionCheckSkipCount += 1;
	return TRUE;
}

void BugFix::PrintObjectInfo(__in const char * Header, __in NWN::CGameObject * Object, __in NWN::OBJECTID ObjectId)
{
	const NWN::CExoString * FirstName;
	const NWN::CExoString * LastName;
	NWN::CNWSObject * NWSObject;
	if (Object == NULL)
	{
		_logger->Info(  "%s: ObjectId=%08X Object=%p" , Header, ObjectId, (void *) Object );
		return;
	}

	NWSObject = Object->AsNWSObject( );
	if (Object == NULL)
	{
		_logger->Info(  "%s: ObjectId=%08X Object=%p ObjectType=%02X" , Header, ObjectId, (void *) Object, Object->GetObjectType( ) );
		return;
	}

	FirstName = NWSObject->GetFirstName( );
	if ((FirstName->m_sString == NULL) || (FirstName->m_nBufferLength == 0))
	{
		FirstName = NULL;
	}

	LastName = NWSObject->GetLastName( );
	if ((LastName->m_sString == NULL) || (LastName->m_nBufferLength == 0))
	{
		LastName = NULL;
	}

	_logger->Info(  "%s: ObjectId=%08X Object=%p ObjectType=%02X FirstName=%s LastName=%s" , Header, ObjectId, (void *) Object, Object->GetObjectType( ), (FirstName != NULL ? FirstName->m_sString : ""), (LastName != NULL ? LastName->m_sString : "") );
}

void BugFix::PrintObjectLists(__in NWN::CNWSArea * Area)
{
	char Header[1024];

	_logger->Info(  "Printing area object list for Area %08x (%p)" , Area->GetObjectId( ), (void *) Area );

	for (int i = 0; i < Area->BugFix_AreaObjectList.m_nUsedSize; i += 1)
	{
		StringCbPrintfA( Header, sizeof( Header ), "PointerList[%d] : ", i );
		PrintObjectInfo( Header,
		                 Area->BugFix_AreaObjectList.m_Array[ i ].Object,
		                 Area->BugFix_AreaObjectList.m_Array[ i ].ObjectId );
	}

	for (int i = 0; i < Area->GetObjects( ).m_nUsedSize; i += 1)
	{
		StringCbPrintfA( Header, sizeof( Header ), "ObjectList[%d] : ", i );
		PrintObjectInfo( Header,
		                 GetGameObject( Area->GetObjects( ).m_Array[ i ] ),
						 Area->GetObjects( ).m_Array[ i ] );
	}

	LogServerDebugInfo();
}

void BugFix::LogServerDebugInfo()
{
	NWN::CExoString OutStr;
	typedef VOID (__stdcall * ConsoleCmdProc)(__in const NWN::CExoString & Str); // Really __thiscall but 'this' is unused
	ConsoleCmdProc Cmd;

	OutStr.m_nBufferLength = 0;
	OutStr.m_sString = NULL;

	_logger->Info(  "LogServerDebugInfo: Logging server debug info via console commands..."  );

	Cmd = (ConsoleCmdProc) (OFFS_ServerConsoleCommandMgr_Handle_LogAreaObjects);
	Cmd( OutStr );

	if (OutStr.m_sString != NULL)
	{
		FreeNwn2Heap( OutStr.m_sString );
		OutStr.m_sString = NULL;
	}

	Cmd = (ConsoleCmdProc) (OFFS_ServerConsoleCommandMgr_Handle_LogServerAI);
	Cmd( OutStr );

	if (OutStr.m_sString != NULL)
	{
		FreeNwn2Heap( OutStr.m_sString );
		OutStr.m_sString = NULL;
	}

	_logger->Info(  "LogServerDebugInfo: Done."  );
}

BOOL CALLBACK BugFix::FindServerGuiWindowEnumProc(__in HWND hwnd, __in LPARAM lParam)
{
	DWORD                         Pid;
	WCHAR                         ClassName[ 256 ];
	PFIND_SERVER_GUI_WINDOW_PARAM Param;

	Param = reinterpret_cast< PFIND_SERVER_GUI_WINDOW_PARAM >( lParam );

	//
	// Ignore windows that do not match the right nwn2server process id.
	//

	GetWindowThreadProcessId( hwnd, &Pid );

	if (Pid != Param->ProcessId)
		return TRUE;

	//
	// Ignore windows that are not of the class of the main server window GUI.
	//

	if (GetClassNameW( hwnd, ClassName, 256 ))
	{
		if (!wcscmp( ClassName, L"Exo - BioWare Corp., (c) 1999 - Generic Blank Application"))
		{
			Param->hwnd = hwnd;
			return FALSE;
		}
	}

	return TRUE;
}

HWND BugFix::FindServerGuiWindow()
{
	FIND_SERVER_GUI_WINDOW_PARAM Param;

	Param.hwnd      = 0;
	Param.ProcessId = GetCurrentProcessId( );

	EnumWindows( FindServerGuiWindowEnumProc, (LPARAM)&Param );

	return Param.hwnd;
}

__declspec(naked) bool __fastcall BugFix::CallCNetLayerInternal_MessageArrived(__in struct CNetLayerInternal * This, __in void * Unused, __in int ExoProtocol, __in SOCKET Socket, __in int Unk_always_zero, __in int Unk_always_1)
{
	__asm
	{
		mov     edx, OFFS_CNetLayerInternal_MessageArrived ; set jump target
		jmp     edx ; branch to target procedure
	}
}

__declspec(naked) NWN::CNWSPlayer * __fastcall BugFix::GetClientObjectByPlayerId2(__in struct NWN::CServerExoApp * This, __in void * Unused, __in NWN::PLAYERID PlayerId, __in UCHAR ExpansionMask)
{
	__asm
	{
		mov     edx, OFFS_CServerExoApp_GetClientObjectByPlayerId ; set jump target
		jmp     edx ; branch to target procedure
	}
}

NWN::CNWSPlayer * BugFix::GetClientObjectByPlayerId(__in NWN::PLAYERID PlayerId)
{
	return BugFix::GetClientObjectByPlayerId2(NWN::g_pAppManager->m_pServerExoApp, NULL, PlayerId, 0);
}

int __stdcall BugFix::recvfromHook(__in SOCKET s, __out char *buf, __in int len, __in int flags, __out struct sockaddr *from, __inout_opt int *fromlen)
{
	int callerlen;
	char *callerbuf;
	unsigned char data;
	int rlen;
	u_short port;
	sockaddr_in *sin;
	bool repeat;
	char localbuf[FRAME_DATA_SIZE];

#ifdef XP_BUGFIX_NETLAYER_INSTRUMENT

	LastRecvfromTick = GetTickCount( );

#endif

	callerlen = len;
	callerbuf = buf;

	//
	// If the reflector is enabled, a local receive buffer has to be used so as
	// to allow encapsulation of maximally sized game data frames (slightly
	// exceeding the normal internal receive buffer size).
	//

	if (reflectorEnabled)
	{
		len = sizeof(localbuf);
		buf = localbuf;
	}

	repeat = false;
	do {
		rlen = recvfrom(s, buf, len, flags, from, fromlen);

		if (rlen <= 0)
			return rlen;

		if (!from || !fromlen || *fromlen < sizeof(sockaddr_in))
			return rlen;

		data = (unsigned char)buf[0];

		//
		// If the reflector is enabled, handle encapsulated traffic.  ONLY
		// encapsulated traffic is allowed in reflector mode, aside from the
		// loopback address.
		//

		if (reflectorEnabled)
		{
			if (data == ENCAP_MAGIC)
			{
				rlen = ReflectorRecvfrom(s, buf, rlen, flags, from, fromlen);

				if (rlen <= 0)
					return rlen;

				data = (unsigned char)buf[0];
			}
			else if (((PSOCKADDR_IN)from)->sin_addr.s_addr != INADDR_LOOPBACK_NET)
			{
				//
				// Don't allow direct unencapsulated traffic not from localhost.
				//

				continue;
			}
		}

		//
		// Short circuit processing for obviously bogus packets so that these
		// have the minimal possible impact on server response time.
		//

		if (data != 'M' && data != 'B' && data != '\xfe') {
			repeat = true;
			badPacketCounter += 1;
			continue;
		}

		repeat = false;
	} while (repeat != false);
	
#ifdef XP_BUGFIX_NETLAYER_INSTRUMENT

	LastGoodRecvfromTick = GetTickCount( );

#endif

	if (RecvfromCallout != NULL)
	{
		rlen = RecvfromCallout(s, buf, rlen, flags, from, fromlen);

		if (rlen <= 0)
			return rlen;
	}

	RecvfromSuccess = true;

	//
	// Swap back to the caller's buffer if in reflector mode, as a shadow
	// buffer was necessary.  If a too large message was processed, drop it.
	//

	if (reflectorEnabled)
	{
		if (rlen > callerlen)
		{
			WSASetLastError( WSAEMSGSIZE );
			return -1;
		}

		memcpy(callerbuf, buf, rlen);
		buf = callerbuf;
		len = callerlen;
	}

	if ((rlen >= 4) && (!memcmp( buf, "BNLM", 4 )))
	{
		//
		// Giant hack for NWNX-4 which sends illegally short length BNLM
		// messages.  These trigger a (benign) read overrun in the game server.
		// The message validator accepts these messages, but ensure that no
		// uninitialized data is processed in such case.
		//

		if (rlen == 4)
		{
			if (callerlen < 11)
				return -1;

			//
			// Normalize the message to 11 bytes :
			//
			// (WORD) ClientPort
			// (BYTE) Cookie
			// (DWORD) PingNumber
			//
			// Plus the 4-byte BNLM header.
			//

			rlen = 11;
			ZeroMemory( buf + 4, 11 - 4 );
		}
	}
	else if ((rlen >= 4) && (!memcmp( buf, "BNDS", 4 )))
	{
		//
		// Giant hack for a third party server list tracker which sends
		// illegally short length BNDS messages.  These trigger a (benign) read
		// overrun in the game server.  The message validator rejects these
		// messages so patch them to be valid here.
		//

		if (rlen == 4)
		{
			if (callerlen < 6)
				return -1;

			//
			// Normalize the message to 11 bytes :
			//
			// (WORD) ClientPort
			//
			// Plus the 4-byte BNLM header.
			//

			rlen = 6;
			ZeroMemory( buf + 4, 6 - 4 );
		}
	}

	//
	// Perform validity checks on the datagram.
	//

	if (!CheckDatagram( buf, rlen ))
		return -1;

	if (len < 6)
		return rlen;

	sin = (sockaddr_in *) from;
	if (!memcmp(buf, "BNCS", 4 ))
	{
		if (NetLayerHandleBNCS( (char *) buf, len, sin, *fromlen ) == -1)
		{
			_logger->Info("* Dropped BNCS from %s:%lu due to already active connection.", inet_ntoa(sin->sin_addr), ntohs(sin->sin_port));
			return -1;
		}
	}
	else if (!memcmp(buf, "BNVS", 4 ))
	{
		if (NetLayerHandleBNVS( (char *) buf, len, sin, *fromlen ) == -1)
		{
			_logger->Info("* Dropped BNVS from %s:%lu due to already active connection.", inet_ntoa(sin->sin_addr), ntohs(sin->sin_port));
			return -1;
		}
	}

	if (rewriteClientUdpPort)
	{
		sin = (sockaddr_in *) from;
		port = ntohs(sin->sin_port);

		//
		// Patch up the client data port field in the message to match that of the
		// actual data port.  Some logic in nwn2server uses the client's claimed
		// data port and not the actual data port for player lookups, which causes
		// issues if two players are behind a NAT with a default configuration.
		//

		if (!memcmp(buf, "BNLM", 4))
			*(u_short *)(buf+4) = port;
		else if (!memcmp(buf, "BNXI", 4))
			*(u_short *)(buf+4) = port;
		else if (!memcmp(buf, "BNES", 4))
			*(u_short *)(buf+4) = port;
		else if (!memcmp(buf, "BNCS", 4))
			*(u_short *)(buf+4) = port;
		else if (!memcmp(buf, "BNLR", 4))
			*(u_short *)(buf+4) = port;
		else if (!memcmp(buf, "BNCR", 4))
			return -1;
		else if (!memcmp(buf, "BNER", 4))
			return -1;
		else if (!memcmp(buf, "BNDS", 4))
			*(u_short *)(buf+4) = port;
		else if (!memcmp(buf, "BNDR", 4))
			return -1;
		else if (!memcmp(buf, "BNVR", 4))
			return -1;
		else
			return rlen;

		if (plugin->verboseLogging)
			_logger->Info("* Patched client data port in %.4s packet.", buf);
	}

	if (!memcmp(buf, "BNCS", 4))
	{
		ULONG Remaining = (ULONG)(rlen - 4);
		PUCHAR Payload;
		Payload = (PUCHAR)(buf + 4);
		const ULONG OffsetToName = 2 + 1 + 4 + 2 + 1 + 4;

		if (Remaining < OffsetToName)
		{
			_logger->Info("* Malformed BNCS message dropped (too short)");

			return -1;
		}

		Payload += OffsetToName;
		Remaining -= OffsetToName;

		if (Remaining < 1)
		{
			_logger->Info("* Malformed BNCS message dropped (too short (1))");

			return -1;
		}

		UCHAR AccountNameLength = Payload[0];
		if ((AccountNameLength == 0) || (AccountNameLength > 40))
		{
			_logger->Info("* Malformed BNCS message dropped (zero account name length or account name too long)");

			return -1;
		}

		Payload += 1;
		Remaining -= 1;

		if (Remaining < (ULONG)(AccountNameLength) + 1)
		{
			_logger->Info("* Malformed BNCS message dropped (too short (1))");

			return -1;
		}

		PCHAR AccountName = (PCHAR)Payload;

		if (plugin->verboseLogging)
		{
			std::string AccountNamePrint(AccountName, AccountNameLength);
			_logger->Info("* Handling BNCS for player account name %s", AccountNamePrint.c_str());
		}

		bool TrailingSpaces = false;
		bool TrailingDots = false;
		for (ULONG i = 0; i < AccountNameLength; i += 1)
		{
			if ((AccountName[i] == '\0') ||
				 (AccountName[i] == '<') ||
				 (AccountName[i] == '>') ||
				 (AccountName[i] == '{') ||
				 (AccountName[i] == '}') ||
				 (AccountName[i] < ' ') ||
				 (AccountName[i] == '\\') ||
				 (AccountName[i] == '/') ||
				 (AccountName[i] == ':') ||
				 (AccountName[i] == '$') ||
				 (AccountName[i] == '~') ||
				 (AccountName[i] == '|'))
			{
				std::string AccountNamePrint(AccountName, AccountNameLength);

				_logger->Info("* Malformed BNCS message dropped (malformed account name %s", AccountNamePrint.c_str());

				return -1;
			}

			if (AccountName[i] == ' ')
			{
				TrailingSpaces = true;
			}
			else
			{
				TrailingSpaces = false;
			}

			if (AccountName[i] == '.')
			{
				TrailingDots = true;
			}
			else
			{
				TrailingDots = false;
			}
		}

		if (TrailingSpaces || AccountName[0] == ' ' || TrailingDots)
		{
			std::string AccountNamePrint(AccountName, AccountNameLength);

			_logger->Info("* Malformed BNCS message dropped (malformed account name with trailing or leading spaces %s", AccountNamePrint.c_str());

			return -1;
		}

		//
		// Logging in with a different case causes the player not to match an
		// existing CNWSPlayerTURD.  Fix the case to be the same as was used
		// for the first successful login since startup to avoid this issue.
		//

		try
		{
			CHAR AccountNameLookup[ 65 ];

			if ((CanonicalizeAccountNames != false) &&
				(AccountNameLength < ARRAYSIZE( AccountNameLookup ) - 1))
			{
				memcpy(
					AccountNameLookup,
					AccountName,
					AccountNameLength);

				AccountNameLookup[ AccountNameLength ] = '\0';

				_strlwr( AccountNameLookup );

				StringMap::iterator it;
				
				it = AccountNameMap.find( AccountNameLookup );
				
				if (it != AccountNameMap.end( ))
				{
					std::string AccountNameProvided(AccountName, AccountNameLength);

					if (it->second != AccountNameProvided)
					{
						_logger->Info("* Canonicalized case for BNCS for account name %s", AccountNameProvided.c_str());

						memcpy(
							AccountName,
							it->second.c_str( ),
							min( AccountNameLength, it->second.size( ) ) );
					}
				}
			}
		}
		catch (std::bad_alloc)
		{
		}

		// WORD Port
		// BYTE ConnectionType
		// DWORD ClientBuild
		// WORD ExpansionMask
		// BYTE Language
		// DWORD ApplicationId
		// SmallString AccountName
		// SmallString CDKeyHash
	}

	return rlen;
}

int __stdcall BugFix::sendtoMstHook(__in SOCKET s, __in const char *buf, __in int len, __in int flags, __in struct sockaddr_in *to, __in int tolen)
{
	static sockaddr_in sin = {0};

	if (to->sin_port != htons(6121))
		return sendtoOut(s, buf, len, flags, (struct sockaddr_in *) to, tolen);

	static ULONG LastMstResolveTick = 0;

	//
	// If it has been 30 minute since the last time the Mst hostname was
	// resolved, or this is the first send, resolve it now.
	//

	if ((LastMstResolveTick == 0) ||
		(GetTickCount() - LastMstResolveTick) >= 1800000)
	{
		hostent *he = gethostbyname(MASTER_SERVER_HOSTNAME);

		if (he == NULL)
		{
			_logger->Info("* BugFix::sendtoMstHook: Failed to resolve " MASTER_SERVER_HOSTNAME);
		}
		else
		{
			memcpy(&sin, to, tolen);
			LastMstResolveTick = GetTickCount();
			sin.sin_addr.s_addr = *(unsigned long *)he->h_addr;

			_logger->Info("* BugFix::sendtoMstHook: Master server hostname resolved.");
		}
	}

	//
	// Send a copy of the message to the emulation master server, then pass it
	// on to the BioWare master server.
	//

	sendtoOut(s, buf, len, flags, (struct sockaddr_in *) &sin, tolen);

	return sendtoOut(s, buf, len, flags, (struct sockaddr_in *) to, tolen);
}

int __stdcall BugFix::sendtoGlobalHook(__in SOCKET s, __in const char *buf, __in int len, __in int flags, __in struct sockaddr_in *to, __in int tolen)
{
	return sendtoOut(s, buf, len, flags, (sockaddr_in *) to, tolen);
}


int __stdcall BugFix::sendtoOut(__in SOCKET s, __in const char *buf, __in int len, __in int flags, __in struct sockaddr_in *to, __in int tolen)
{
	if ((len >= 4) &&
		(!memcmp(buf, "BNVR", 4 )))
	{
		NetLayerHandleBNVR( (char *) buf, len, to, tolen );
	}

	//
	// If the reflector is not enabled or the destination is loopback, send
	// the message directly.  Otherwise, route it through the reflector.
	//

	if (reflectorEnabled == false || (((PSOCKADDR_IN)to)->sin_addr.s_addr == INADDR_LOOPBACK_NET))
	{
		return sendto(s, buf, len, flags, (struct sockaddr *) to, tolen);
	}
	else
	{
		struct sockaddr_in sin_shadow;

		memcpy( &sin_shadow, to, tolen );
		return ReflectorSendto(s, buf, len, flags, &sin_shadow, tolen);
	}
}

bool BugFix::EnableRecvfromHook()
{
	const SIZE_T NumHooks = 2;
	HMODULE      Ws2_32;
	SIZE_T       i;
	ULONG        HookStatus[ NumHooks ];
	PVOID        OrigAddresses[ NumHooks ];
	PVOID        HookAddresses[ NumHooks ] =
	{
		recvfromHook,
		sendtoGlobalHook
	};
	CONST CHAR  *Symbols[ NumHooks ] =
	{
		"recvfrom",
		"sendto"
	};

	Ws2_32 = GetModuleHandleW( L"ws2_32.dll" );

	if (!Ws2_32)
		return false;

	for (i = 0; i < NumHooks; i += 1)
	{
		OrigAddresses[ i ] = GetProcAddress( Ws2_32, Symbols[ i ] );

		if (!OrigAddresses[ i ])
		{
			logger->Info(
				 "! Failed to resolve an import: %s" ,
				Symbols[ i ]
				);

			return false;
		}
	}

	if (!RedirectImageImports(
		GetModuleHandleW( 0 ),
		OrigAddresses,
		HookAddresses,
		HookStatus,
		NumHooks))
	{
		logger->Info(  "! Failed to hook imports."  );
		return false;
	}

	for (i = 0; i < NumHooks; i += 1)
	{
		if (!HookStatus[ i ])
		{
			logger->Info(
				 "! Failed to hook an import: %s" ,
				Symbols[ i ]
				);

			return false;
		}
	}

	logger->Info(  "* ws2_32!recvfrom hook installed."  );

	return true;
}

bool BugFix::rewriteClientUdpPort;
USHORT BugFix::aiUpdateSortingLevel;
ULONG BugFix::badPacketCounter;
ULONG BugFix::aiUpdateThrottle;
bool BugFix::overrideNetRecv;
bool BugFix::directPollNetRecv;
bool BugFix::aiUpdateSortPlayersFirst;
bool BugFix::devMode;
bool BugFix::reflectorEnabled;
std::string BugFix::reflectorSecret;

BugFix::GameObjCacheEntry BugFix::GameObjectCache[ 1 + BugFix::GAME_OBJ_CACHE_SIZE ];
ULONG BugFix::GameObjectCacheIndex;
ULONG BugFix::GameObjectCacheHits;
ULONG BugFix::GameObjectCacheMisses;
NWN::CGameObjectArrayNode * BugFix::GameObjectNodes[ BugFix::OBJARRAY_SIZE ];

bool BugFix::NextGameEffectIsSafe;
bool BugFix::RebaselineAISortingLevel;
bool BugFix::ResortAIListArray;
bool BugFix::RecvfromSuccess;

HWND BugFix::ServerGuiWindow;

ULONG BugFix::SafeGameEffectCount;
ULONG BugFix::UnsafeGameEffectCount;
ULONG BugFix::ItemDynamicAddedToAIListCount;
ULONG BugFix::AIUpdateIterationsThrottledCount;
ULONG BugFix::ThisLoopThrottledObjectCount;
ULONG BugFix::ThrottledObjectCount;
ULONG BugFix::ForcedNPCUpdateObjectCount;
ULONG BugFix::NonThrottledObjectCount;
ULONG BugFix::ThrottleImmuneObjectCount;
ULONG BugFix::AISortingLevelRebaselineCount;
ULONG BugFix::AIListSortCount;
ULONG BugFix::LUOAreaDeletionCheckSkipCount;
ULONG BugFix::LUOAreaDeletionCheckRunCount;

ULONG BugFix::AIUpdateStartTick;
ULONG BugFix::LastRecvfromTick;
ULONG BugFix::LastGoodRecvfromTick;
ULONG BugFix::LastPostFakeRecvIndicationTick;
ULONG BugFix::LastDirectNetRecvTick;

NWN::CGameObject * BugFix::AIUpdate_LastEventObject;
NWN::CGameObject * BugFix::AIUpdate_LastUpdateObject;
NWN::CServerAIMaster * BugFix::AIMaster;

RecvfromCalloutProc __stdcall SetRecvfromCallout(RecvfromCalloutProc Callout)
{
	RecvfromCalloutProc OldCallout = RecvfromCallout;
	RecvfromCallout = Callout;

	return OldCallout;
}

NWN::PLAYERID __stdcall GetCreatureControllingPlayerId(__in NWN::OBJECTID ObjectId)
{
	NWN::CNWSCreature * CreatureObject;
	NWN::CGameObject * GameObj;

	GameObj = BugFix::GetGameObject( ObjectId );
	if (GameObj == NULL)
	{
		return PLAYERID_INVALIDID;
	}

	if ((CreatureObject = GameObj->AsCreature( )) == NULL)
	{
		return PLAYERID_INVALIDID;
	}

	if (CreatureObject->GetIsPlayerCharacter( ) == FALSE)
	{
		return PLAYERID_INVALIDID;
	}

	return CreatureObject->GetControllingPlayerId( );
}

NWN::CLastUpdateObject *__stdcall GetPlayerLUOForObject(__in NWN::PLAYERID PlayerId, __in NWN::OBJECTID ObjectId)
{
	NWN::CNWSPlayer * PlayerObject = BugFix::GetClientObjectByPlayerId( PlayerId );
	if (PlayerObject == NULL)
	{
		return NULL;
	}

	return BugFix::GetLastUpdateObject(PlayerObject, ObjectId);
}

VOID TrackPlayerAccountName(__in const char * Name)
{
	CHAR LocalName[ 65 ];

	StringCbCopyA( LocalName, sizeof( LocalName ), Name );
	_strlwr( LocalName );

	try
	{
		AccountNameMap.insert( StringMap::value_type( LocalName, Name ) );
	}
	catch (std::bad_alloc)
	{
	}
}

__declspec(naked) void BugFixDisconnectPlayer()
{
	__asm
	{
		; //
		; // Ensure that a valid player ID is passed through so that a BootPC
		; // event on a disconnected creature does not crash the server due to
		; // a DisconnectPlayer call on PLAYERID_INVALIDID.
		; //

		cmp     dword ptr [esp+04h], MAX_PLAYERS
		jb      CallDisconnectPlayer

		xor     eax, eax
		ret     10h

CallDisconnectPlayer:
		push    esi
		mov     esi, ecx
		cmp     dword ptr [esi+7AB54h], 0
		mov     eax, OFFS_DisconnectPlayer + 10
		jmp     eax
	}
}
