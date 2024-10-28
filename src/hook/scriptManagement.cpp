
#include "scriptManagement.h"
#include <bit>
#include <cstdint>
#include <cstring>
#include <string>

#include "../misc/log.h"
#include "nwn2heap.h"
extern std::unique_ptr<LogNWNX> logger;

constexpr uint32_t NWN_DEFAULT_EXECUTESCRIPT_ENH_PARAMS_LEN = 32;

struct NWN2Param {
	uint32_t _;
	uint32_t value;
	uint32_t ptr;
	uint32_t size;
	uint32_t type;
};
static_assert(sizeof(NWN2Param) == 5 * 4);

static std::vector<NWN2Param> scriptparams;

// 1.23
// g_pVirtualMachine
constexpr uint32_t NWN2_OFFSET_CVIRTUALMACHINE = 0x00864424;
// CVirtualMachine::ExecuteScript
constexpr uint32_t NWN2_OFFSET_EXECUTESCRIPT     = 0x0072B380;
constexpr uint32_t NWN2_OFFSET_EXECUTESCRIPT_ENH = 0x0072B050;

constexpr uint32_t NWN2_OFFSET_InitParam  = 0x0055EA40;
constexpr uint32_t NWN2_OFFSET_CleanParam = 0x006b5cd0;

struct NWN2ParamsList {
	struct NWN2Param* list;
	size_t size;
};
static_assert(sizeof(NWN2ParamsList) == 8);

static struct NWN2ParamsList* nwn2_scriptparams = (struct NWN2ParamsList*)(0x0086F15C);
// static size_t scriptparams_count = 0;

struct CVirtualMachine {
	uint32_t _;
	uint32_t execscript_ret_value;
};
static CVirtualMachine** nwn2_vm
    = std::bit_cast<struct CVirtualMachine**>(NWN2_OFFSET_CVIRTUALMACHINE);

// Function hooks
using CVirtualMachine_ExecuteScript_t = BOOL(__thiscall*)(CVirtualMachine* thisVM,
                                                          const NWN::CExoString& scriptName,
                                                          NWN::OBJECTID objectId,
                                                          uint32_t unknown1,
                                                          uint32_t unknown2);
static CVirtualMachine_ExecuteScript_t CVirtualMachine_ExecuteScript
    = std::bit_cast<CVirtualMachine_ExecuteScript_t>(NWN2_OFFSET_EXECUTESCRIPT);

//
using CVirtualMachine_ExecuteScriptEnhanced_t
    = int32_t(__thiscall*)(CVirtualMachine* thisVM,
                           const NWN::CExoString& scriptName,
                           NWN::OBJECTID objectID,
                           void* ParamList,
                           uint32_t unknow1,
                           uint32_t unknow2);
static CVirtualMachine_ExecuteScriptEnhanced_t CVirtualMachine_ExecuteScriptEnhanced
    = std::bit_cast<CVirtualMachine_ExecuteScriptEnhanced_t>(NWN2_OFFSET_EXECUTESCRIPT_ENH);

// //
// using CVirtualMachine_InitParam_t = void(__thiscall*)(void* paramLst, uint32_t iNb);
// static CVirtualMachine_InitParam_t CVirtualMachine_InitParam
//     = std::bit_cast<CVirtualMachine_InitParam_t>(NWN2_OFFSET_InitParam);

// //
// using CVirtualMachine_CleanParam_t = void(__thiscall*)(void* paramLst);
// static CVirtualMachine_CleanParam_t CVirtualMachine_CleanParam
//     = std::bit_cast<CVirtualMachine_CleanParam_t>(NWN2_OFFSET_CleanParam);

namespace NWScript {

static int OriginalCNWSMsgAddr = 0;

int* GetPtrToCNWSMessage()
{
	int ptr = *(int*)OFFS_g_pAppManager;
	ptr     = *(int*)(ptr + 4);
	ptr     = *(int*)(ptr + 4);
	return (int*)(ptr + 0x10020);
}

void ApplyScriptCNWSMessage()
{
	int* ptrToCNWSMessage = GetPtrToCNWSMessage();

	static unsigned char* scriptCNWSMsg;
	static bool isInit = false;

	if (!isInit) {
		OriginalCNWSMsgAddr    = *ptrToCNWSMessage;
		NWN2_HeapMgr* pHeapMgr = NWN2_HeapMgr::Instance();
		NWN2_Heap* pHeap       = pHeapMgr->GetDefaultHeap();
		scriptCNWSMsg          = (unsigned char*)pHeap->Allocate(0x58);
		unsigned char* Msg1    = (unsigned char*)pHeap->Allocate(0x80);
		unsigned char* Msg2    = (unsigned char*)pHeap->Allocate(0x80);

		scriptCNWSMsg[0] = 0xC0;
		scriptCNWSMsg[1] = 0x42;
		scriptCNWSMsg[2] = 0x80;
		scriptCNWSMsg[3] = 0x00;

		((uint32_t*)scriptCNWSMsg)[0x14] = 0xFFFFFFFF;
		((uint32_t*)scriptCNWSMsg)[0x15] = 0x7F000000;

		((uint32_t*)scriptCNWSMsg)[1] = (int)Msg1;
		((uint32_t*)scriptCNWSMsg)[2] = 0x80;
		((uint32_t*)scriptCNWSMsg)[3] = 0x0;

		((uint32_t*)scriptCNWSMsg)[4] = (int)Msg2;
		((uint32_t*)scriptCNWSMsg)[5] = 0x80;
		((uint32_t*)scriptCNWSMsg)[6] = 0;
		((uint32_t*)scriptCNWSMsg)[7] = 0;

		scriptCNWSMsg[0x20] = 0;

		((uint32_t*)scriptCNWSMsg)[9]   = 0x0;
		((uint32_t*)scriptCNWSMsg)[0xa] = 0x0;
		((uint32_t*)scriptCNWSMsg)[0xb] = 0x0;
		((uint32_t*)scriptCNWSMsg)[0xc] = 0x0;

		((uint32_t*)scriptCNWSMsg)[0xd]  = 0x0;
		((uint32_t*)scriptCNWSMsg)[0xe]  = 0x0;
		((uint32_t*)scriptCNWSMsg)[0xf]  = 0x0;
		((uint32_t*)scriptCNWSMsg)[0x10] = 0x0;
		scriptCNWSMsg[0x44]              = 0x0;
		scriptCNWSMsg[0x45]              = 0x0;
		isInit                           = true;
	}

	if (scriptCNWSMsg != NULL)
		*ptrToCNWSMessage = (int)(scriptCNWSMsg);
}

void RestoreOriginalCNWSMessage()
{
	// Be sure to not restore before saving the Original CNWSMsg
	if (OriginalCNWSMsgAddr != 0) {
		int* ptrToCNWSMessage = GetPtrToCNWSMessage();
		*ptrToCNWSMessage     = OriginalCNWSMsgAddr;
	}
}

void ExecuteScript(const char* sScript,
                   NWN::OBJECTID oTarget,
                   bool* outExecuted,
                   bool bReplaceCNWSMsg)
{
	logger->Trace("ExecuteScript %s, %lu", sScript, oTarget);

	if (bReplaceCNWSMsg)
		ApplyScriptCNWSMessage();

	auto executed = CVirtualMachine_ExecuteScript(
	    *nwn2_vm,
	    NWN::CExoString {.m_sString = (char*)sScript, // un-const cast, safe as param is read only
	                     .m_nBufferLength = strlen(sScript)},
	    oTarget, 1, 1);
	if (outExecuted != nullptr)
		*outExecuted = executed;

	if (bReplaceCNWSMsg)
		RestoreOriginalCNWSMessage();
}

int32_t ExecuteScriptEnhanced(const char* sScriptName,
                              NWN::OBJECTID oTarget,
                              bool bClearParams,
                              bool* outExecuted,
                              bool bReplaceCNWSMsg)
{
	logger->Trace("ExecuteScriptEnhanced %s, %lu", sScriptName, oTarget);

	if (bReplaceCNWSMsg)
		ApplyScriptCNWSMessage();

	const NWN::CExoString script
	    = {.m_sString = (char*)sScriptName, .m_nBufferLength = strlen(sScriptName)};

	NWN2ParamsList save = *nwn2_scriptparams;

	nwn2_scriptparams->list = scriptparams.data();
	nwn2_scriptparams->size = scriptparams.size();

	// call the script
	int retValue
	    = CVirtualMachine_ExecuteScriptEnhanced(*nwn2_vm, script, oTarget, nwn2_scriptparams, 1, 1);

	if (outExecuted != nullptr)
		*outExecuted = retValue != 0;

	// Is the script ok?
	if (retValue != 0)
		retValue = (*nwn2_vm)->execscript_ret_value;
	else
		retValue = -1;

	*nwn2_scriptparams = save;

	if (bReplaceCNWSMsg)
		RestoreOriginalCNWSMessage();

	return retValue;
}
void AddScriptParameterInt(int32_t nParam)
{
	logger->Trace("AddScriptParameterInt %d", nParam);

	scriptparams.push_back(NWN2Param {
	    ._     = 0,
	    .value = std::bit_cast<uint32_t>(nParam),
	    .ptr   = 0,
	    .size  = 0,
	    .type  = 0,
	});
}
void AddScriptParameterString(const char* sParam)
{
	logger->Trace("AddScriptParameterString %s", sParam);

	scriptparams.push_back(NWN2Param {
	    ._     = 0,
	    .value = 0,
	    .ptr   = std::bit_cast<uint32_t>(sParam),
	    .size  = strlen(sParam) + 1,
	    .type  = 2,
	});
}
void AddScriptParameterFloat(float fParam)
{
	logger->Trace("AddScriptParameterFloat %f", fParam);

	scriptparams.push_back(NWN2Param {
	    ._     = 0,
	    .value = std::bit_cast<uint32_t>(fParam),
	    .ptr   = 0,
	    .size  = 0,
	    .type  = 1,
	});
}
void AddScriptParameterObject(NWN::OBJECTID oParam)
{
	logger->Trace("AddScriptParameterObject %lu", oParam);

	scriptparams.push_back(NWN2Param {
	    ._     = 0,
	    .value = oParam,
	    .ptr   = 0,
	    .size  = 0,
	    .type  = 4,
	});
}

void ClearScriptParams()
{
	logger->Trace("ClearScriptParams");
	scriptparams.clear();
}
} // namespace NWScript
