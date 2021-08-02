#ifndef _NWNX4_NWN2LIB_NWN2_H
#define _NWNX4_NWN2LIB_NWN2_H

//
// General header file for reverse engineered NWN2Server information.
//

#define NWN2SERVER_VERSION 0x01231763
//#define NWN2SERVER_VERSION 0x01221588
//#define NWN2SERVER_VERSION 0x01211549
//#define NWN2SERVER_VERSION 0x00131409
//#define NWN2SERVER_VERSION 0x00121295

/*
 * Version specific offsets and check data.
 */

#if   NWN2SERVER_VERSION == 0x01231763

/*
 * 1.0.23.1763 & 1.0.23.1765
 */

#define OFFS_ProcessServerMessage      0x00635250 // CNWSMessage::HandlePlayerToServerMessage
#define OFFS_ProcessServerMessageHook  0x005895AD // CServerExoAppInternal::HandleMessage
#define OFFS_CalcPositionLoop0         0x004107ED // NWN2_JStar::SearchStep+0x5D [left]
#define OFFS_CalcPositionLoop0Ret      0x004107FE // NWN2_JStar::SearchStep+0x6E [left]
#define OFFS_CalcPositionLoop1         0x00410B9F // NWN2_JStar::SearchStep+0x40F [right]
#define OFFS_CalcPositionLoop1Ret      0x00410BB0 // NWN2_JStar::SearchStep+0x420 [right]

// NullDerefCrash0 is fixed in 1.0.21.1549
// #define OFFS_NullDerefCrash0           0x005DB05A // CNetLayerWindow::FrameTimeout
// #define OFFS_NullDerefCrash0RetNormal  0x005DB067 // CNetLayerWindow::FrameTimeout
// #define OFFS_NullDerefCrash0RetSkip    0x005DB1CB // CNetLayerWindow::FrameTimeout

// Still 1.0.13.1409 offsets.  Fixed in 1.0.13.1409.
// #define OFFS_NullDerefCrash1           0x0042F595
// #define OFFS_NullDerefCrash1RetNormal  0x0042F59B
// #define OFFS_NullDerefCrash1RetSkip    0x0042F5C1

#define OFFS_NullDerefCrash2           0x0074A03F // NWN2_Collider::UpdateCollider+0x4F
#define OFFS_NullDerefCrash2RetNormal  0x0074A045 // NWN2_Collider::UpdateCollider+0x55
#define OFFS_NullDerefCrash2RetSkip    0x0074A086 // NWN2_Collider::UpdateCollider+0x96

// Fixed in 1.0.21.1549.
// #define OFFS_NullDerefCrash3           0x004F45A4 // CNWSMessage::HandlePlayerToServerDungeonMasterMessage [0x24]
// #define OFFS_NullDerefCrash3RetNormal  0x004F45AA // CNWSMessage::HandlePlayerToServerDungeonMasterMessage [0x24]
// #define OFFS_NullDerefCrash3RetSkip    0x004F45C8 // CNWSMessage::HandlePlayerToServerDungeonMasterMessage [0x24]

// Fixed in 1.0.23.1763
// #define OFFS_NullDerefCrash4           0x0045C00F // CServerExoAppInternal::LoadCharacterStart+0x47F
// #define OFFS_NullDerefCrash4RetNormal  0x0045C016 // CServerExoAppInternal::LoadCharacterStart+0x486
// #define OFFS_NullDerefCrash4RetSkip    0x0045C262 // CServerExoAppInternal::LoadCharacterStart+0x6D2

//
// Actual crash occurs at 0x005D110C <1.0.13.1409> but that is not the right place to hook in
// to fix it.
//
// Doesn't appear to be needed anymore in 1.0.23.1763 (??)
// #define OFFS_Crash5                    0x005D6700 // CNetLayerWindow::UnpacketizeFullMessages+0x80
// #define OFFS_Crash5RetNormal           0x005D6706 // CNetLayerWindow::UnpacketizeFullMessages+0x86
// #define OFFS_Crash5RetSkip             0x005D6706 // CNetLayerWindow::UnpacketizeFullMessages+0x86

// BrianMeyer_Inventory_Msg_BadObjIdCrash
#define OFFS_NullDerefCrash6           0x00630F77 // CNWSMessage::HandlePlayerToServerInventoryMessage+0x887
#define OFFS_NullDerefCrash6RetNormal  0x00630F7E // CNWSMessage::HandlePlayerToServerInventoryMessage+0x88E
#define OFFS_NullDerefCrash6RetSkip    0x00630FE8 // CNWSMessage::HandlePlayerToServerInventoryMessage+0x8F8

//
// Actual crash occurs at 004e8c09 <1.0.13.1409>
//

#define OFFS_NullDerefCrash7           0x00696829 // CNWVirtualMachineCommands::ExecuteCommandActionExchangeItem+0xB9
#define OFFS_NullDerefCrash7RetNormal  0x0069682F // CNWVirtualMachineCommands::ExecuteCommandActionExchangeItem+0xBF
#define OFFS_NullDerefCrash7RetSkip    0x00696870 // CNWVirtualMachineCommands::ExecuteCommandActionExchangeItem+100

//
// Actual crash occurs at 004a2256 <1.0.13.1409>

#define OFFS_NullDerefCrash8           0x005D9B5C // CNWSItem::AcquireItem+0x8C
#define OFFS_NullDerefCrash8RetNormal  0x005D9B62 // CNWSItem::AcquireItem+0x92
#define OFFS_NullDerefCrash8RetSkip    0x005D9CD8 // CNWSItem::AcquireItem+0x208

#define OFFS_CheckUncompress0          0x00504476 // CNetLayerWindow::UnpacketizeFullMessages+0x126 [right]
#define OFFS_CheckUncompress0RetNormal 0x00504484 // CNetLayerWindow::UnpacketizeFullMessages+0x134 [right]
#define OFFS_CheckUncompress0RetSkip   0x00504415 // CNetLayerWindow::UnpacketizeFullMessages+0xC5  [left]

#define OFFS_CheckUncompress1          0x005046BE // CNetLayerWindow::UnpacketizeFullMessages+0x36E [bottom]
#define OFFS_CheckUncompress1RetNormal 0x005046CC // CNetLayerWindow::UnpacketizeFullMessages+0x37C [bottom]
#define OFFS_CheckUncompress1RetSkip   OFFS_CheckUncompress0RetSkip // Same, presently

#define OFFS_UncompressMessage         0x004FF470 // CNetLayerInternal::UncompressMessage+0

#define OFFS_NullDerefCrash9           0x005D405D // CItemRepository::GetItemPtrInRepository+0x5D
#define OFFS_NullDerefCrash9RetNormal  0x005D4062 // CItemRepository::GetItemPtrInRepository+0x62
#define OFFS_NullDerefCrash9RetSkip    0x005D40BB // CItemRepository::GetItemPtrInRepository+0xBB

#define OFFS_NullDerefCrash10          0x005B6AED // CNWSCreatureStats::ValidateLevelUp+0x3FD
#define OFFS_NullDerefCrash10RetNormal 0x005B6AF3 // CNWSCreatureStats::ValidateLevelUp+0x403
#define OFFS_NullDerefCrash10RetSkip   0x005B6730 // CNWSCreatureStats::ValidateLevelUp+0x40

// Not needed for 1.0.22.1588
// #define OFFS_CGameEffectDtor           0x00521B30 // CGameEffect::~CGameEffect
// #define OFFS_ms_iGameEffectCount       0x0085D5A8 // CGameEffect::ms_iGameEffectCount
// #define OFFS_CGameEffectDtorRet        0x00521B38 // CGameEffect::~CGameEffect+8

#define CHECK_ProcessServerMessageHook 0x000ABC9F

#define OFFS_NullDerefCrash11          0x005724fa // CNWSPlayer::DropTURD+0x2da
#define OFFS_NullDerefCrash11RetNormal 0x00572501 // CNWSPlayer::DropTURD+0x2e1
#define OFFS_NullDerefCrash11RetSkip   0x0057250a // CNWSPlayer::DropTURD+0x2ea

#define OFFS_SendCompressionHook       0x00508DE2 // CNetLayerInternal::SendMessageToPlayer+0x52
#define OFFS_SendCompressionHookDoZlib 0x00508de8 // CNetLayerInternal::SendMessageToPlayer+0x58
#define OFFS_SendCompressionHookNoZlib 0x0050902f // CNetLayerInternal::SendMessageToPlayer+0x29f

#define OFFS_SendMessageToPlayer       0x00508D90 // CNetLayerInternal::SendMessageToPlayer+0x0
#define OFFS_FrameReceive              0x00507740 // CNetLayerWindow::FrameRecieve+0x0
#define OFFS_SetInFrameTimer           0x004FEE80 // CNetLayerWindow::SetInFrameTimer+0x0
#define OFFS_CallFrameTimeout          0x0050918A // CNetLayerInternal::UpdateStatusLoop+140 (+139 for raw instr)
#define OFFS_FrameTimeout              0x00508B80 // CNetLayerWindow::FrameTimeout+0x0
#define OFFS_FrameSend                 0x00504110 // CNetLayerWindow::FrameSend+0x0
#define OFFS_HandleMessage             0x0054a390 // CServerExoApp::HandleMessage+0x0
#define OFFS_DisconnectPlayer          0x00506280 // CNetLayerInternal::DisconnectPlayer+0x0

#define OFFS_NullDerefCrash12          0x00632963 // CNWSMessage::HandlePlayerToServerLevelUpMessage+0x443
#define OFFS_NullDerefCrash12Ret       0x00632972 // CNWSMessage::HandlePlayerToServerLevelUpMessage+0x452

#define OFFS_GameObjectUpdateTime1     0x00589E26 // CServerExoAppInternal::UpdateGameObjectsForPlayer+0xc6
#define OFFS_GameObjUpdateSizeLimit1   0x0056EEFC // CNWSMessage::SendServerToPlayerGameObjUpdate+0x7c
#define OFFS_GetNextIslandExit         0x0040F8B1 // NWN2_AreaSurfaceMesh::GetNextIslandExit+0x48

#define OFFS_GetHighResolutionTimer    0x0043E240 // CExoTimersInternal::GetHighResolutionTimer

#define OFFS_g_pAppManager             0x0086442C // g_pAppManager
#define OFFS_NWServerGetGameObject     0x005BA910 // NWServer::GetGameObject

#define CrControllingPlayerId          0x00404    // offsetof(CNWSCreature, m_dwControllingPlayerId)
#define CrPlayerCharacter              0x00eec    // offsetof(CNWSCreature, m_bPlayerCharacter)
#define CrAppearance                   0x0180c    // offsetof(CNWSCreature, m_cApperance)
#define CrStats                        0x01fc4    // offsetof(CNWSCreature, m_pStats)

#define CaHeadVariation                0x00704    // offsetof(CNWSCreatureAppearanceInfo, m_nHeadVariation)
#define CaTailVariation                0x00705    // offsetof(CNWSCreatureAppearanceInfo, m_nTailVariation)
#define CaWingVariation                0x00706    // offsetof(CNWSCreatureAppearanceInfo, m_nWingVariation)
#define CaHairVariation                0x00707    // offsetof(CNWSCreatureAppearanceInfo, m_nHairVariation)
#define CaFacialHairVariation          0x00708    // offsetof(CNWSCreatureAppearanceInfo, m_nFacialHairVariation)
#define CaTint                         0x0070c    // offsetof(CNWSCreatureAppearanceInfo, m_cTint)
#define CaHeadTint                     0x0073c    // offsetof(CNWSCreatureAppearanceInfo, m_cHeadTint)
#define CaHairTint                     0x0076c    // offsetof(CNWSCreatureAppearanceInfo, m_cHairTint)

#define CsRace                         0x0000a    // offsetof(CNWSCreatureStatsCore, m_uRace)
#define CsTint                         0x00650    // offsetof(CNWSCreatureStatsCore, m_cTint)
#define CsHeadTint                     0x00680    // offsetof(CNWSCreatureStatsCore, m_cHeadTint)
#define CsHairTint                     0x006b0    // offsetof(CNWSCreatureStatsCore, m_cHairTint)
#define CsHairVariation                0x006ec    // offsetof(CNWSCreatureStatsCore, m_nHairVariation)
#define CsFacialHairVariation          0x006ed    // offsetof(CNWSCreatureStatsCore, m_nFacialHairVariation)
#define CsHeadVariation                0x006f2    // offsetof(CNWSCreatureStatsCore, m_nHeadVariation)
#define CsTailVariation                0x00dc8    // offsetof(CNWSCreatureStatsCore, m_nTailVariation)
#define CsWingVariation                0x00dc9    // offsetof(CNWSCreatureStatsCore, m_nTailVariation)

#define SoAILevel                      0x00130    // offsetof(CNWSObject, m_nAILevel)
#define SoDialogOwner                  0x00128    // offsetof(CNWSObject, m_oidDialogOwner)
#define SoArea                         0x00144    // offsetof(CNWSObject, m_oidArea)
#define SoBugFixNWSObject              0x00304    // offsetof(CNWSObject, BugFix_NWSObjectData) *** Added by xp_bugfix ***
#define SoFirstName                    0x002d4    // offsetof(CNWSObject, m_sFirstName)
#define SoLastName                     0x002e4    // offsetof(CNWSObject, m_sLastName)

#define GoArObjList                    0x00034    // offsetof(CGameObject, BugFix_AreaObjectList) *** Added by xp_bugfix ***
#define GoIdSelf                       0x000a0    // offsetof(CGameObject, m_idSelf)

#define ItShouldUpdateEffects          0x00b1c    // offsetof(CNWSItem, m_bShouldUpdateEffects)
#define ItGameObjectBaseClassOffset    0x00730    // offsetof(CNWSItem, CGameObject)

#define ArGameObjectBaseClassOffset    0x00ad0    // offsetof(CNWSArea, CGameObject)
#define ArObjects                      0x00a90    // offsetof(CNWSArea, m_cObjects)


#define code4optRestart_SetBufferCount 0x007ca649 // code4optRestart+0x169
#define code4optRestart_SetBufferCount2 0x007ca6c6 // code4optRestart+0x1e6
#define code4optRestart_SetBufferCount3 0x007CA627 // code4optRestart+0x170
//#define code4initLow_BufferCountOffs   0x007a2703 // code4initLow+0x253

#define OFFS_ObjArrayAddInternalObject 0x0040D3C0 // CGameObjectArray::AddInternalObject+0x0
#define OFFS_ObjArrayAddObjectAtPos    0x0040D060 // CGameObjectArray::AddObjectAtPos+0x0
#define OFFS_ObjArrayDeleteAll         0x0040E920 // CGameObjectArray::DeleteAll+0x0
#define OFFS_ObjArrayGetGameObject     0x0054D190 // CGameObjectArray::GetGameObject+0x0
#define OFFS_ObjArrayDelete            0x0040D470 // CGameObjectArray::Delete+0x0
#define OFFS_AIMasterUpdateStateGetObj 0x0066F316 // CServerAIMaster::UpdateState+0x306
#define OFFS_AIMasterUpdateStateNoObj  0x0066F41F // CServerAIMaster::UpdateState+0x40f
#define OFFS_AIMasterUpdateStateGotObj 0x0066F36C // CServerAIMaster::UpdateState+0x35c

//
// The code from UpdateState+35c to UpdateState+365 seems to be 100% redundant,
// and shows up in the hot list in the profiler.  Remove it.
//

#define OFFS_AIMasterUpdateStateGotOba 0x0066F375 // CServerAIMaster::UpdateState+0x365

#define OFFS_AIMasterUpdateStateGetOb2 0x0066F13C // CServerAIMaster::UpdateState+0x12c
#define OFFS_AIMasterUpdateStateNoObj2 0x0066F2AF // CServerAIMaster::UpdateState+0x29f
#define OFFS_AIMasterUpdateStateGotOb2 0x0066F192 // CServerAIMaster::UpdateState+0x182

#define OFFS_TransitionBMPFixPatch     0x00560D2E // CNWSMessage::SendServerToPlayerArea_ClientArea+0x14e
#define OFFS_TransitionBMPFixResume    0x00560D74 // CNWSMessage::SendServerToPlayerArea_ClientArea+0x194
#define OFFS_SetAreaTransitionBMP      0x0056F2E0 // CNWSPlayer::SetAreaTransitionBMP
#define OFFS_CNWSMessage_WriteWORD     0x0074DEF0 // CNWSMessage::WriteWORD
#define OFFS_CNWSMessage_WriteCExoString 0x0074F180 // CNWSMessage::WriteCExoString

#define OFFS_CExoNetInternal_SendMessageToMst 0x006E212E // CExoNetInternal::SendMessageA+0xEE (sendto IAT ref)

#define OFFS_NWN2_Pathfinder_Dtor_PatchJmp 0x005F9884 // NWN2_Pathfinder::~NWN2_Pathfinder+0x34
#define OFFS_NWN2_Pathfinder_ClearPath_PatchJmp 0x00410DE3 // NWN2_Pathfinder::ClearPath

//
// The following are used to make the CServerAIMaster list store pointers and
// not object IDs.
//

#define OFFS_CServerAIMaster_AddObjectPtrPatch 0x0066EC0E // CServerAIMaster::AddObject+0xE
#define OFFS_CServerAIMaster_RemoveObjectPtrPatch 0x0066D3E5 // CServerAIMaster::RemoveObject+0x5
#define OFFS_CServerAIMaster_UpdateStatePostProcessGetObj 0x0066F4D5 // CServerAIMaster::UpdateStatePostProcess+0x65
#define OFFS_CServerAIMaster_UpdateStatePostProcessGotObj 0x0066F524 // CServerAIMaster::UpdateStatePostProcess+0xb4

//
// The following is a performance optimization to avoid querying the game
// object table unnecessarily in a performance path.
//
// Note, superseded by below rewrite of GetClientObjectByClientId.
//

#define OFFS_CServerExoAppInternal_GetClientObjectByClientId_RedundantGetObj 0x00588753 // CServerExoAppInternal::GetClientObjectByObjectId+0x33

#define OFFS_CServerExoAppInternal_GetClientObjectByClientId_Patch 0x00588748 // CServerExoAppInternal::GetClientObjectByObjectId+0x28
#define OFFS_CServerExoAppInternal_GetClientObjectByClientId_CheckNext 0x00588767 // CServerExoAppInternal::GetClientObjectByObjectId+0x47

//
// The following avoids a game object table lookup to check for expansion pack
// on every GetFeat call.  All players have all expansion packs in practice and
// this is a nonuseful check anyway, so ignore it.
//

#define OFFS_CNWSCreatureStats_HasFeat_PatchJmp 0x0059EF35 // CNWSCreatureStats::HasFeat+0x5
#define OFFS_CNWSCreatureStats_HasFeat_RealCheck 0x0059EF77 // CNWSCreatureStats::HasFeat+0x47

//
// Used to dynamically register items with the CServerAIMaster if and only if
// they have an effect applied.
//

#define OFFS_CServerAIMaster_AddObject 0x0066EC00 // CServerAIMaster::AddObject
#define OFFS_CServerAIMaster_RemoveObject 0x0066D3E0 // CServerAIMaster::RemoveObject
#define OFFS_CNWSItem_CNWSItem_NopAddObject 0x005d52a1  // CNWSItem::CNWSItem+0xb1
#define OFFS_CNWSObject_ApplyEffect_PatchAddItemToAIMaster 0x00584134 // CNWSObject::ApplyEffect+0x4a4
#define OFFS_CNWSObject_UpdateEffectListEx_PatchAddItemToAIMaster 0x0057fc13 // CNWSObject::UpdateEffectListEx+0x113
#define OFFS_CNWSObject_ApplyEffect    0x00583C90 // CNWSObject::ApplyEffect
#define OFFS_CNWSObject_ApplyEffect_Resume 0x00583C98 // CNWSObject::ApplyEffect+0x8

//
// Used for AIUpdate throttling.
//

#define OFFS_CServerAIMaster_UpdateState_NextUpdateObject 0x0066F41F // CServerAIMaster::UpdateState+0x40f
#define OFFS_CServerAIMaster_UpdateState 0x0066F010 // CServerAIMaster::UpdateState
#define OFFS_CServerExoApp_Internal_MainLook_CallAIUpdate 0x0059A54B // CServerExoAppInternal::MainLoop+0xfb

//
// Used by bypass WSAAsyncSelect and directly poll for network messages from
// the socket.
//

#define OFFS_CNetLayerInternal_MessageArrived 0x004FF450 // CNetLayerInternal::MessageArrived

//
// Used to track a list of object pointers in each area, for fast lookup.
//

// #define OFFS_CNWSArea_AddObjectToArea  0x00610860 // CNWSArea::AddObjectToArea
// #define OFFS_CNWSArea_AddObjectToArea_Resume 0x00610866 // CNWSArea::AddObjectToArea+0x6
#define OFFS_CNWSArea_AddObjectToArea  0x006108DA // CNWSArea::AddObjectToArea+0x7a
#define OFFS_CNWSArea_AddObjectToArea_Resume 0x006108E6 // CNWSArea::AddObjectToArea+0x86
#define OFFS_CNWSArea_RemoveObjectFromArea 0x006077A0  // CNWSArea::RemoveObjectFromArea
#define OFFS_CNWSArea_RemoveObjectFromArea_Resume 0x006077AA // CNWSArea::RemoveObjectFromArea+0xa


//
// Used to ensure proper sorting of the area object list.
//

// #define OFFS_CNWSArea_UpdatePositionInObjectsArray 0x0060FF90 // CNWSArea::UpdatePositionInObjectsArea
// #define OFFS_CNWSArea_UpdatePositionInObjectsArray_Resume 0x0060FF95 // CNWSArea::UpdatePositionInObjectsArea+0x5

#define OFFS_CNWSArea_UpdatePositionInObjectsArray_SwapUp 0x00610542 // CNWSArea::UpdatePositionInObjectsArray+0x5b2
#define OFFS_CNWSArea_UpdatePositionInObjectsArray_SwapUp_Resume 0x0061055D // CNWSArea::UpdatePositionInObjectsArray+0x5cd
#define OFFS_CNWSArea_UpdatePositionInObjectsArray_SwapDown 0x00610611  // CNWSArea::UpdatePositionInObjectsArray+0x681
#define OFFS_CNWSArea_UpdatePositionInObjectsArray_SwapDown_Resume 0x0061062A // CNWSArea::UpdatePositionInObjectsArray+0x69a

//
// Used to use the optimized object in area array for SortObjectsForGameObjectUpdate.
//

#define OFFS_CNWSMessage_SortObjectsForGameObjectUpdate_GetObj 0x005567D0 // CNWSMessage::SortObjectsForGameObjectUpdate+0x110
#define OFFS_CNWSMessage_SortObjectsForGameObjectUpdate_GotObj 0x0055682B // CNWSMessage::SortObjectsForGameObjectUpdate+0x16b

//
// Used to use the optimized object in area array for GetFirstObjectIndiceByX.
//

#define OFSS_CNWSArea_GetFirstObjectIndiceByX_SetArrayOffset 0x00606E72 // CNWSArea::GetFirstObjectIndiceByX+0x12
#define OFFS_CNWSArea_GetFirstObjectIndiceByX_GetObj 0x00606E80 // CNWSArea::GetFirstObjectIndiceByX+0x20
#define OFFS_CNWSArea_GetFirstObjectIndiceByX_GotObj 0x00606EBD // CNWSArea::GetFirstObjectIndiceByX+0x5d

//
// Used to elide all CNWSMessage::DeleteLastUpdateObjectsInOtherAreas
// processing if the object is in the same area as the last time processing
// occurred and the area has removed no objects since.
//

#define OFFS_CNWSMessage_DeleteLastUpdateObjectsInOtherAreas_Hook 0x00552CB4 // CNWSMessage::DeleteLastUpdateObjectsInOtherAreas+0x14
#define OFFS_CNWSMessage_DeleteLastUpdateObjectsInOtherAreas_Resume 0x00552CBB // CNWSMessage::DeleteLastUpdateObjectsInOtherAreas+0x1b

#define PlrActiveObjectsLastUpdate      000ch   /*    +0x00c m_pActiveObjectsLastUpdate : Ptr32 CExoLinkedList<CLastUpdateObject> */
#define AsmTileGridWidth                00e0h   /*    +0x0e0 m_TileGridWidth  : Int4B */

//
// Used to maintain a fast lookup table of NWN::OBJECTID to CLastUpdateObject.
//

#define OFFS_CLastUpdateObject_Destructor 0x005B9660 // CLastUpdateObject::~CLastUpdateObject
#define OFFS_CLastUpdateObject_Destructor_Resume 0x005B9665 // CLastUpdateObject::~CLastUpdateObject+0x5
#define OFFS_CNWSMessage_CreateNewLastUpdateObject 0x0056DF10 // CNWSMessage::CreateNewLastUpdateObject
#define OFFS_CNWSMessage_CreateNewLastUpdateObject_Resume 0x0056DF15 // CNWSMessage::CreateNewLastUpdateObject+0x5

//
// Used to accelerate lookups of CLastUpdateObject by NWN::OBJECTID.
//

#define OFFS_CNWSPlayer_GetLastUpdateObject 0x005709C0 // CNWSPlayer::GetLastUpdateObject
#define OFFS_CNWSMessage_TestObjectUpdateDifferences_GetLUO 0x0056E92E // CNWSMessage::TestObjectUpdateDifferences+0x1e
#define OFFS_CNWSMessage_TestObjectUpdateDifferences_GotLUO 0x0056E94F // CNWSMessage::TestObjectUpdateDifferences+0x3f
#define OFFS_CNWSMessage_TestObjectUpdateDifferences_NewLUO 0x0056E9A0 // CNWSMessage::TestObjectUpdateDifferences+0x90

//
// Used to print server area object state to a log file.
//

#define OFFS_ServerConsoleCommandMgr_Handle_LogAreaObjects 0x006BB170 // ServerConsoleCommandMgr::Handle_LogAreaObjects
#define OFFS_ServerConsoleCommandMgr_Handle_LogServerAI 0x006BB560 // ServerConsoleCommandMgr::Handle_LogServerAI

//
// Used to accelerate time surface mesh intersection.
//

#define OFFS_NWN2_TileSurfaceMesh_Calc_Contact_0 0x004182D0 // NWN2_TileSurfaceMesh::Calc_Contact<0> 

//
// Used to filter illegal messages.
//

#define OFFS_CServerExoApp_GetClientObjectByPlayerId 0x0054A6A0 // CServerExoApp::GetClientObjectByPlayerId

//
// Used to disable character creation.
//

#define OFFS_CNWSMessage_HandlePlayerToServerLoginMessage_CmpLoadCharacterStart4 0x0062656D // CNWSMessage::HandlePlayerToServerLoginMessage+0x1cd

//
// Used to fix hash table crash for >=2GB pointers.
//

#define OFFS_NWN2_HashTable_Hash1_Divide 0x004593c6 // nwn2server!NWN2_HashTable<unsigned long,NWN2_DynamicArrayBlittable<NWN2_RenderableMgr::DeleteRenderableInstanceCallback> *>::Hash+0x6:
#define OFFS_NWN2_HashTable_Hash2_Divide 0x006719b6 // nwn2server!NWN2_HashTable<int,bool>::Hash+0x6:
#define OFFS_NWN2_HashTable_Hash3_Divide 0x00671a96 // nwn2server!NWN2_HashTable<unsigned long,FixupTuple *>::Hash+0x6:

//
// Used to fix NWN2_AreaSurfaceMesh::FindFace walking off the end of the tile
// grid array in boundary cases.
//

#define OFFS_NWN2_AreaSurfaceMesh_FindFace_CallFindFace 0x00416087 // nwn2server!NWN2_AreaSurfaceMesh::FindFace+0x107
#define OFFS_NWN2_SurfaceMesh_FindFace    0x0041FCE0 // nwn2server!NWN2_SurfaceMesh::FindFace

//
// NWN2_AreaSurfaceMesh fields.
//

#define AsmTileGridHeight               00dch   /*    +0x0dc m_TileGridHeight : Int4B */
#define AsmTileGridWidth                00e0h   /*    +0x0e0 m_TileGridWidth  : Int4B */

//
// Used to fix NWN2_AreaSurfaceMesh::IsValid walking off the end of the tile
// grid array in boundary cases.
//

#define OFFS_NWN2_AreaSurfaceMesh_IsValid_CallFindFace 0x004161D7 // nwn2server!NWN2_AreaSurfaceMesh::IsValid+0x107

//
// Used for TLS integration.
//

#define OFFS_CNetLayerInternal_HandlesBNCSMessage_CallInitializePlayerInfo 0x00506F40 // nwn2server!CNetLayerInternal::HandleBNCSMessage+0x5e0
#define OFFS_CNetLayerPlayerInfo_Initialize                                0x00504770 // nwn2server!CNetLayerPlayerInfo::Initialize

//
// Used for fine grained facing updates.
//

#define OFFS_CLastUpdateObjectFacingFactor_Float 0x007FED24 // nwn2server!_real

//
// Used to fix NWN2_Pathfinder::CalcPath2 indexing an invalid tile grid.
//

#define OFFS_NWN2_Pathfinder_CalcPath2_FindFaceHook1 0x004116DA // 1.0.23.1765: nwn2server!NWN2_Pathfinder::CalcPath2+0x8aa

//
// Used to fix a crash when running out of ammo.
//

#define OFFS_NWN2_CNWSCreature_ResolveSafeProjectileItemPatch 0x006362A7 // 1.0.23.1765: nwn2server!CNWSCreature::ResolveSafeProjectile+0x97

//
// Used to fix a crash when copying automaps between objects when unpossessing
// a creature, if one object has a different automap size than another.  This
// is suspected to happen if one onject was created before instanced areas were
// created.  The fix disables automap copy back entirely.
//

#define OFFS_CNWSCreature_UnpossessFamiliar_DisableAutoMapCopy 0x005CEA2D // 1.0.23.1765: nwn2server!nwn2server!CNWSCreature::UnpossessFamiliar+0x1ed

//
// Used to reduce the default (2000) AABB count for area objects.
//

#define OFFS_CNWSArea_CNWSArea_InitialPatchAABBCount 0x00772ADE // 1.0.23.1765: nwn2server!CNWArea::CNWArea+0x7e

//
// Used to reduce the amount of attachment points allocated by default.
//

#define OFFS_NWN2_DynamicArray_NWN2_Object_NWN2_AttachmentPoint_add_DefaultCountPatch 0x0040DC04 // 1.0.23.1765: nwn2server!NWN2_DynamicArray<NWN2_Object::NWN2_AttachmentPoint>::add+0x14
#define OFFS_NWN2_Object_AddBaseAttachmentPoint_DefaultCountPatch 0x0040DCED // 1.0.23.1765: nwn2server!NWN2_Object::AddBaseAttachmentPoint+0x4d
#define OFFS_NWN2_Object_DefineAttachmentPoint_DefaultCountPatch 0x0077DF18 // 1.0.23.1765: nwn2server!NWN2_Object::DefineAttachmentPoint+0x88

//
// Used a fix a crash when resting while in transit.
//

#define OFFS_CNWSMessage_HandlePlayerToServerInputMessage_RestPatch1 0x00633F22 // 1.0.23.1765: nwn2server!CNWSMessage::HandlePlayerToServerInputMessage+0xf62

#elif NWN2SERVER_VERSION == 0x01221588

/*
 * 1.0.22.1588
 */

#define OFFS_ProcessServerMessage      0x004FFA70 // CNWSMessage::HandlePlayerToServerMessage
#define OFFS_ProcessServerMessageHook  0x0045418D // CServerExoAppInternal::HandleMessage
#define OFFS_CalcPositionLoop0         0x007096DD // NWN2_JStar::SearchStep+0x5D [left]
#define OFFS_CalcPositionLoop0Ret      0x007096EE // NWN2_JStar::SearchStep+0x6E [left]
#define OFFS_CalcPositionLoop1         0x00709A8F // NWN2_JStar::SearchStep+0x40F [right]
#define OFFS_CalcPositionLoop1Ret      0x00709AA0 // NWN2_JStar::SearchStep+0x420 [right]

// NullDerefCrash0 is fixed in 1.0.21.1549
// #define OFFS_NullDerefCrash0           0x005DB05A // CNetLayerWindow::FrameTimeout
// #define OFFS_NullDerefCrash0RetNormal  0x005DB067 // CNetLayerWindow::FrameTimeout
// #define OFFS_NullDerefCrash0RetSkip    0x005DB1CB // CNetLayerWindow::FrameTimeout

// Still 1.0.13.1409 offsets.  Fixed in 1.0.13.1409.
// #define OFFS_NullDerefCrash1           0x0042F595
// #define OFFS_NullDerefCrash1RetNormal  0x0042F59B
// #define OFFS_NullDerefCrash1RetSkip    0x0042F5C1

#define OFFS_NullDerefCrash2           0x007324FF // NWN2_Collider::UpdateCollider+0x4F
#define OFFS_NullDerefCrash2RetNormal  0x00732505 // NWN2_Collider::UpdateCollider+0x55
#define OFFS_NullDerefCrash2RetSkip    0x00732546 // NWN2_Collider::UpdateCollider+0x96

// Fixed in 1.0.21.1549.
// #define OFFS_NullDerefCrash3           0x004F45A4 // CNWSMessage::HandlePlayerToServerDungeonMasterMessage [0x24]
// #define OFFS_NullDerefCrash3RetNormal  0x004F45AA // CNWSMessage::HandlePlayerToServerDungeonMasterMessage [0x24]
// #define OFFS_NullDerefCrash3RetSkip    0x004F45C8 // CNWSMessage::HandlePlayerToServerDungeonMasterMessage [0x24]

#define OFFS_NullDerefCrash4           0x0045C00F // CServerExoAppInternal::LoadCharacterStart+0x47F
#define OFFS_NullDerefCrash4RetNormal  0x0045C016 // CServerExoAppInternal::LoadCharacterStart+0x486
#define OFFS_NullDerefCrash4RetSkip    0x0045C262 // CServerExoAppInternal::LoadCharacterStart+0x6D2

//
// Actual crash occurs at 0x005D110C <1.0.13.1409> but that is not the right place to hook in
// to fix it.
//
#define OFFS_Crash5                    0x005D6700 // CNetLayerWindow::UnpacketizeFullMessages+0x80
#define OFFS_Crash5RetNormal           0x005D6706 // CNetLayerWindow::UnpacketizeFullMessages+0x86
#define OFFS_Crash5RetSkip             0x005D6706 // CNetLayerWindow::UnpacketizeFullMessages+0x86

// BrianMeyer_Inventory_Msg_BadObjIdCrash
#define OFFS_NullDerefCrash6           0x004FB197 // CNWSMessage::HandlePlayerToServerInventoryMessage+0x887
#define OFFS_NullDerefCrash6RetNormal  0x004FB19E // CNWSMessage::HandlePlayerToServerInventoryMessage+0x88E
#define OFFS_NullDerefCrash6RetSkip    0x004FB208 // CNWSMessage::HandlePlayerToServerInventoryMessage+0x8F8

//
// Actual crash occurs at 004e8c09 <1.0.13.1409>
//

#define OFFS_NullDerefCrash7           0x00560ED9 // CNWVirtualMachineCommands::ExecuteCommandActionExchangeItem+0xB9
#define OFFS_NullDerefCrash7RetNormal  0x00560EDF // CNWVirtualMachineCommands::ExecuteCommandActionExchangeItem+0xBF
#define OFFS_NullDerefCrash7RetSkip    0x00560F20 // CNWVirtualMachineCommands::ExecuteCommandActionExchangeItem+100

//
// Actual crash occurs at 004a2256 <1.0.13.1409>

#define OFFS_NullDerefCrash8           0x004A3FEA // CNWSItem::AcquireItem+0x8A
#define OFFS_NullDerefCrash8RetNormal  0x004A3FF0 // CNWSItem::AcquireItem+0x90
#define OFFS_NullDerefCrash8RetSkip    0x004A4168 // CNWSItem::AcquireItem+0x208

#define OFFS_CheckUncompress0          0x005D67A6 // CNetLayerWindow::UnpacketizeFullMessages+0x126 [right]
#define OFFS_CheckUncompress0RetNormal 0x005D67B4 // CNetLayerWindow::UnpacketizeFullMessages+0x134 [right]
#define OFFS_CheckUncompress0RetSkip   0x005D6745 // CNetLayerWindow::UnpacketizeFullMessages+0xC5  [left]

#define OFFS_CheckUncompress1          0x005D69EE // CNetLayerWindow::UnpacketizeFullMessages+0x36E [bottom]
#define OFFS_CheckUncompress1RetNormal 0x005D69FC // CNetLayerWindow::UnpacketizeFullMessages+0x37C [bottom]
#define OFFS_CheckUncompress1RetSkip   OFFS_CheckUncompress0RetSkip // Same, presently

#define OFFS_UncompressMessage         0x005D17F0 // CNetLayerInternal::UncompressMessage+0

#define OFFS_NullDerefCrash9           0x0049E50D // CItemRepository::GetItemPtrInRepository+0x5D
#define OFFS_NullDerefCrash9RetNormal  0x0049E512 // CItemRepository::GetItemPtrInRepository+0x62
#define OFFS_NullDerefCrash9RetSkip    0x0049E56B // CItemRepository::GetItemPtrInRepository+0xBB

#define OFFS_NullDerefCrash10          0x00480C5D // CNWSCreatureStats::ValidateLevelUp+0x3FD
#define OFFS_NullDerefCrash10RetNormal 0x00480C63 // CNWSCreatureStats::ValidateLevelUp+0x403
#define OFFS_NullDerefCrash10RetSkip   0x004808A0 // CNWSCreatureStats::ValidateLevelUp+0x40

// Not needed for 1.0.22.1588
// #define OFFS_CGameEffectDtor           0x00521B30 // CGameEffect::~CGameEffect
// #define OFFS_ms_iGameEffectCount       0x0085D5A8 // CGameEffect::ms_iGameEffectCount
// #define OFFS_CGameEffectDtorRet        0x00521B38 // CGameEffect::~CGameEffect+8

#define CHECK_ProcessServerMessageHook 0x000AB8DF

#elif NWN2SERVER_VERSION == 0x01211549

/*
 * 1.0.21.1549
 */

#define OFFS_ProcessServerMessage      0x004FFB70 // CNWSMessage::HandlePlayerToServerMessage
#define OFFS_ProcessServerMessageHook  0x0045417F // CServerExoAppInternal::HandleMessage
#define OFFS_CalcPositionLoop0         0x007098DD // NWN2_JStar::SearchStep [left]
#define OFFS_CalcPositionLoop0Ret      0x007098EE // NWN2_JStar::SearchStep [left]
#define OFFS_CalcPositionLoop1         0x00709C8F // NWN2_JStar::SearchStep [right]
#define OFFS_CalcPositionLoop1Ret      0x00709CA0 // NWN2_JStar::SearchStep [right]

// NullDerefCrash0 is fixed in 1.0.21.1549
// #define OFFS_NullDerefCrash0           0x005DB05A // CNetLayerWindow::FrameTimeout
// #define OFFS_NullDerefCrash0RetNormal  0x005DB067 // CNetLayerWindow::FrameTimeout
// #define OFFS_NullDerefCrash0RetSkip    0x005DB1CB // CNetLayerWindow::FrameTimeout

// Still 1.0.13.1409 offsets.  Fixed in 1.0.13.1409.
// #define OFFS_NullDerefCrash1           0x0042F595
// #define OFFS_NullDerefCrash1RetNormal  0x0042F59B
// #define OFFS_NullDerefCrash1RetSkip    0x0042F5C1

#define OFFS_NullDerefCrash2           0x0073271F // NWN2_Collider::UpdateCollider
#define OFFS_NullDerefCrash2RetNormal  0x00732725 // NWN2_Collider::UpdateCollider
#define OFFS_NullDerefCrash2RetSkip    0x00732766 // NWN2_Collider::UpdateCollider

// Fixed in 1.0.21.1549.
// #define OFFS_NullDerefCrash3           0x004F45A4 // CNWSMessage::HandlePlayerToServerDungeonMasterMessage [0x24]
// #define OFFS_NullDerefCrash3RetNormal  0x004F45AA // CNWSMessage::HandlePlayerToServerDungeonMasterMessage [0x24]
// #define OFFS_NullDerefCrash3RetSkip    0x004F45C8 // CNWSMessage::HandlePlayerToServerDungeonMasterMessage [0x24]

#define OFFS_NullDerefCrash4           0x0045C00F // CServerExoAppInternal::LoadCharacterStart
#define OFFS_NullDerefCrash4RetNormal  0x0045C016 // CServerExoAppInternal::LoadCharacterStart 
#define OFFS_NullDerefCrash4RetSkip    0x0045C262 // CServerExoAppInternal::LoadCharacterStart

//
// Actual crash occurs at 0x005D110C <1.0.13.1409> but that is not the right place to hook in
// to fix it.
//
#define OFFS_Crash5                    0x005D6820 // CNetLayerWindow::UnpacketizeFullMessages
#define OFFS_Crash5RetNormal           0x005D6826 // CNetLayerWindow::UnpacketizeFullMessages
#define OFFS_Crash5RetSkip             0x005D6826 // CNetLayerWindow::UnpacketizeFullMessages

// BrianMeyer_Inventory_Msg_BadObjIdCrash
#define OFFS_NullDerefCrash6           0x004FB297 // CNWSMessage::HandlePlayerToServerInventoryMessage+887
#define OFFS_NullDerefCrash6RetNormal  0x004FB29E // CNWSMessage::HandlePlayerToServerInventoryMessage+88E
#define OFFS_NullDerefCrash6RetSkip    0x004FB308 // CNWSMessage::HandlePlayerToServerInventoryMessage+8F6

//
// Actual crash occurs at 004e8c09 <1.0.13.1409>
//

#define OFFS_NullDerefCrash7           0x00561029 // CNWVirtualMachineCommands::ExecuteCommandActionExchangeItem+B9
#define OFFS_NullDerefCrash7RetNormal  0x0056102F // CNWVirtualMachineCommands::ExecuteCommandActionExchangeItem+BF
#define OFFS_NullDerefCrash7RetSkip    0x00561070 // CNWVirtualMachineCommands::ExecuteCommandActionExchangeItem=100

//
// Actual crash occurs at 004a2256 <1.0.13.1409>

#define OFFS_NullDerefCrash8           0x004A405A // CNWSItem::AcquireItem+8A
#define OFFS_NullDerefCrash8RetNormal  0x004A4060 // CNWSItem::AcquireItem+90
#define OFFS_NullDerefCrash8RetSkip    0x004A41D8 // CNWSItem::AcquireItem+208

#define OFFS_CheckUncompress0          0x005D68C6 // CNetLayerWindow::UnpacketizeFullMessages+126 [right]
#define OFFS_CheckUncompress0RetNormal 0x005D68D4 // CNetLayerWindow::UnpacketizeFullMessages+134 [right]
#define OFFS_CheckUncompress0RetSkip   0x005D6865 // CNetLayerWindow::UnpacketizeFullMessages+C5  [left]

#define OFFS_CheckUncompress1          0x005D6B0E // CNetLayerWindow::UnpacketizeFullMessages+370 [bottom]
#define OFFS_CheckUncompress1RetNormal 0x005D6B1C // CNetLayerWindow::UnpacketizeFullMessages+37C [bottom]
#define OFFS_CheckUncompress1RetSkip   OFFS_CheckUncompress0RetSkip // Same, presently

#define OFFS_UncompressMessage         0x005D1910 // CNetLayerInternal::UncompressMessage+0

#define OFFS_NullDerefCrash9           0x0049E56D // CItemRepository::GetItemPtrInRepository+5D
#define OFFS_NullDerefCrash9RetNormal  0x0049E572 // CItemRepository::GetItemPtrInRepository+62
#define OFFS_NullDerefCrash9RetSkip    0x0049E5CB // CItemRepository::GetItemPtrInRepository+BB

#define OFFS_NullDerefCrash10          0x00480C7D // CNWSCreatureStats::ValidateLevelUp
#define OFFS_NullDerefCrash10RetNormal 0x00480C83 // CNWSCreatureStats::ValidateLevelUp
#define OFFS_NullDerefCrash10RetSkip   0x004808C0 // CNWSCreatureStats::ValidateLevelUp

#define OFFS_CGameEffectDtor           0x00521B30 // CGameEffect::~CGameEffect
#define OFFS_ms_iGameEffectCount       0x0085D5A8 // CGameEffect::ms_iGameEffectCount
#define OFFS_CGameEffectDtorRet        0x00521B38 // CGameEffect::~CGameEffect+8

#define CHECK_ProcessServerMessageHook 0x000AB9ED

#elif NWN2SERVER_VERSION == 0x00131409

/*
 * 1.0.13.1409
 */

#define OFFS_ProcessServerMessage      0x004FD860
#define OFFS_ProcessServerMessageHook  0x0045336F
#define OFFS_CalcPositionLoop0         0x007042AD
#define OFFS_CalcPositionLoop0Ret      0x007042BE
#define OFFS_CalcPositionLoop1         0x0070465F
#define OFFS_CalcPositionLoop1Ret      0x00704670
#define OFFS_NullDerefCrash0           0x005D58DA
#define OFFS_NullDerefCrash0RetNormal  0x005D58E3
#define OFFS_NullDerefCrash0RetSkip    0x005D5A3B
#define OFFS_NullDerefCrash1           0x0042F595
#define OFFS_NullDerefCrash1RetNormal  0x0042F59B
#define OFFS_NullDerefCrash1RetSkip    0x0042F5C1
#define OFFS_NullDerefCrash2           0x0072C41F
#define OFFS_NullDerefCrash2RetNormal  0x0072C425
#define OFFS_NullDerefCrash2RetSkip    0x0072C466
#define OFFS_NullDerefCrash3           0x004F2327
#define OFFS_NullDerefCrash3RetNormal  0x004F232D
#define OFFS_NullDerefCrash3RetSkip    0x004F234B
#define OFFS_NullDerefCrash4           0x0045B1FF
#define OFFS_NullDerefCrash4RetNormal  0x0045B206
#define OFFS_NullDerefCrash4RetSkip    0x0045B452
//
// Actual crash occurs at 0x005D110C but that is not the right place to hook in
// to fix it.
//
#define OFFS_Crash5                    0x005D10A0
#define OFFS_Crash5RetNormal           0x005D10A6
#define OFFS_Crash5RetSkip             0x005D10A6

// BrianMeyer_Inventory_Msg_BadObjIdCrash
#define OFFS_NullDerefCrash6           0x004F9027
#define OFFS_NullDerefCrash6RetNormal  0x004F902E
#define OFFS_NullDerefCrash6RetSkip    0x004F9098

//
// Actual crash occurs at 004e8c09 
//

#define OFFS_NullDerefCrash7           0x0055D3A9
#define OFFS_NullDerefCrash7RetNormal  0x0055D3AF
#define OFFS_NullDerefCrash7RetSkip    0x0055D3F0

//
// Actual crash occurs at 004a2256

#define OFFS_NullDerefCrash8           0x004A287A
#define OFFS_NullDerefCrash8RetNormal  0x004A2880
#define OFFS_NullDerefCrash8RetSkip    0x004A29F8

#define OFFS_CheckUncompress0          0x005D1146
#define OFFS_CheckUncompress0RetNormal 0x005D1154
#define OFFS_CheckUncompress0RetSkip   0x005D10E5

#define OFFS_CheckUncompress1          0x005D138E
#define OFFS_CheckUncompress1RetNormal 0x005D139C
#define OFFS_CheckUncompress1RetSkip   0x005D10E5

#define OFFS_UncompressMessage         0x005CC190

#define OFFS_NullDerefCrash9           0x0049CDDD
#define OFFS_NullDerefCrash9RetNormal  0x0049CDE2
#define OFFS_NullDerefCrash9RetSkip    0x0049CE3B

#define CHECK_ProcessServerMessageHook 0x000AA4ED



#elif NWN2SERVER_VERSION == 0x00121295

/*
 * 1.0.12.1295
 */

#define OFFS_ProcessServerMessage      0x004FC320
#define OFFS_ProcessServerMessageHook  0x0045295F
#define OFFS_CalcPositionLoop0         0x006FB23D
#define OFFS_CalcPositionLoop0Ret      0x006FB24E
#define OFFS_CalcPositionLoop1         0x006FB5EF
#define OFFS_CalcPositionLoop1Ret      0x006FB600
#define OFFS_NullDerefCrash0           0x005D1F9A
#define OFFS_NullDerefCrash0RetNormal  0x005D1FA3
#define OFFS_NullDerefCrash0RetSkip    0x005D20FB
#define OFFS_NullDerefCrash1           0x0042EF85
#define OFFS_NullDerefCrash1RetNormal  0x0042EF8B
#define OFFS_NullDerefCrash1RetSkip    0x0042EFB1
#define OFFS_NullDerefCrash2           0x0072331F
#define OFFS_NullDerefCrash2RetNormal  0x00723325
#define OFFS_NullDerefCrash2RetSkip    0x00723366
#define OFFS_NullDerefCrash3           0x004F1127
#define OFFS_NullDerefCrash3RetNormal  0x004F112D
#define OFFS_NullDerefCrash3RetSkip    0x004F114B
#define OFFS_NullDerefCrash4           0x0045A81F
#define OFFS_NullDerefCrash4RetNormal  0x0045A826
#define OFFS_NullDerefCrash4RetSkip    0x0045AA72
//
// Actual crash occurs at 0x005CD7CC but that is not the right place to hook in
// to fix it.
//
#define OFFS_Crash5                    0x005CD760
#define OFFS_Crash5RetNormal           0x005CD766
#define OFFS_Crash5RetSkip             0x005CD766

// BrianMeyer_Inventory_Msg_BadObjIdCrash
#define OFFS_NullDerefCrash6           0x004F7E27
#define OFFS_NullDerefCrash6RetNormal  0x004F7E2E
#define OFFS_NullDerefCrash6RetSkip    0x004F7E98

//
// Actual crash occurs at 004e7a49 
//

#define OFFS_NullDerefCrash7           0x00558949
#define OFFS_NullDerefCrash7RetNormal  0x0055894F
#define OFFS_NullDerefCrash7RetSkip    0x00558990

//
// Actual crash occurs at 004a1586

#define OFFS_NullDerefCrash8           0x004A18AA
#define OFFS_NullDerefCrash8RetNormal  0x004A18B0
#define OFFS_NullDerefCrash8RetSkip    0x004A1A28

#define OFFS_CheckUncompress0          0x005CD806
#define OFFS_CheckUncompress0RetNormal 0x005CD814
#define OFFS_CheckUncompress0RetSkip   0x005CD7A5

#define OFFS_CheckUncompress1          0x005CDA4E
#define OFFS_CheckUncompress1RetNormal 0x005CDA5C
#define OFFS_CheckUncompress1RetSkip   0x005CD7A5

#define OFFS_UncompressMessage         0x005C8850

#define OFFS_NullDerefCrash9           0x0049BE0D
#define OFFS_NullDerefCrash9RetNormal  0x0049BE12
#define OFFS_NullDerefCrash9RetSkip    0x0049BE6B


#define CHECK_ProcessServerMessageHook 0x000A99BD

#elif (NWN2SERVER_VERSION == 0x0111153)

/*
 * 1.0.11.1153
 */

#define OFFS_ProcessServerMessage      0x004FC070
#define OFFS_ProcessServerMessageHook  0x0045276F

#define CHECK_ProcessServerMessageHook 0x000A98FD

#else

#error Unsupported nwn2server version!

#endif


#include "NWN2Common.h"



#endif
