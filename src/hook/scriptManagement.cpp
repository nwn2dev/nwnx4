
#include "scriptManagement.h"
#include <bit>
#include <cstdint>
#include <cstring>
#include <string>

#include "../misc/log.h"
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

struct CVirtualMachine { };
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
                           // const NWN::CExoString& scriptName,
                           NWN::OBJECTID objectID,
                           void* ParamList,
                           uint32_t unknow1,
                           uint32_t unknow2);
static CVirtualMachine_ExecuteScriptEnhanced_t CVirtualMachine_ExecuteScriptEnhanced
    = std::bit_cast<CVirtualMachine_ExecuteScriptEnhanced_t>(NWN2_OFFSET_EXECUTESCRIPT_ENH);

//
using CVirtualMachine_InitParam_t = void(__thiscall*)(void* paramLst, uint32_t iNb);
static CVirtualMachine_InitParam_t CVirtualMachine_InitParam
    = std::bit_cast<CVirtualMachine_InitParam_t>(NWN2_OFFSET_InitParam);

//
using CVirtualMachine_CleanParam_t = void(__thiscall*)(void* paramLst);
static CVirtualMachine_CleanParam_t CVirtualMachine_CleanParam
    = std::bit_cast<CVirtualMachine_CleanParam_t>(NWN2_OFFSET_CleanParam);

namespace NWScript {

void ExecuteScript(const char* sScript, NWN::OBJECTID oTarget)
{
	logger->Trace("ExecuteScript %s, %lu", sScript, oTarget);
	CVirtualMachine_ExecuteScript(
	    *nwn2_vm,
	    NWN::CExoString {.m_sString = (char*)sScript, // un-const cast, safe as param is read only
	                     .m_nBufferLength = strlen(sScript)},
	    oTarget, 1, 1);
}

int32_t ExecuteScriptEnhanced(const char* sScriptName, NWN::OBJECTID oTarget, bool bClearParams)
{
	logger->Trace("ExecuteScriptEnhanced %s, %lu", sScriptName, oTarget);

	const NWN::CExoString script
	    = {.m_sString = (char*)sScriptName, .m_nBufferLength = strlen(sScriptName)};

	NWN2ParamsList save = *nwn2_scriptparams;

	nwn2_scriptparams->list = scriptparams.data();
	nwn2_scriptparams->size = scriptparams.size();

	// call the script
	int retValue
	    = CVirtualMachine_ExecuteScriptEnhanced(*nwn2_vm, script, oTarget, nwn2_scriptparams, 1, 1);

	// Is the script ok?
	if (retValue != 0)
		retValue = ((uint32_t*)*nwn2_vm)[1];
	else
		retValue = -1;

	*nwn2_scriptparams = save;

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
