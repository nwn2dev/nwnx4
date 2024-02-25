#if !defined(SCRIPTMANAGEMENT_H_INCLUDED)
#define SCRIPTMANAGEMENT_H_INCLUDED

#define DLLEXPORT extern "C" __declspec(dllexport)

#include "../NWN2Lib/NWN2.h"
#include "../NWN2Lib/NWN2Common.h"
#include <iostream>

namespace ScriptManagement {
// 1.23
// g_pVirtualMachine
constexpr uintptr_t NWN2_OFFSET_CVIRTUALMACHINE = 0x00864424;
// CVirtualMachine::ExecuteScript
constexpr uintptr_t NWN2_OFFSET_EXECUTESCRIPT     = 0x0072B380;
constexpr uintptr_t NWN2_OFFSET_EXECUTESCRIPT_ENH = 0x0072B050;

constexpr uintptr_t NWN2_OFFSET_InitParam  = 0x0055EA40;
constexpr uintptr_t NWN2_OFFSET_CleanParam = 0x006b5cd0;

struct CVirtualMachine { };

using CVirtualMachine_ExecuteScript_t = BOOL(__thiscall*)(CVirtualMachine* thisVM,
                                                          const NWN::CExoString& scriptName,
                                                          NWN::OBJECTID objectId,
                                                          uint32_t unknown1,
                                                          uint32_t unknown2);
CVirtualMachine_ExecuteScript_t CVirtualMachine_ExecuteScript
    = std::bit_cast<CVirtualMachine_ExecuteScript_t>(NWN2_OFFSET_EXECUTESCRIPT);

using CVirtualMachine_ExecuteScriptEnhanced_t = int(__thiscall*)(CVirtualMachine* thisVM,
                                                                 const NWN::CExoString& scriptName,
                                                                 NWN::OBJECTID objectID,
                                                                 void* ParamList,
                                                                 uint32_t unknow1,
                                                                 uint32_t unknow2);
CVirtualMachine_ExecuteScriptEnhanced_t CVirtualMachine_ExecuteScriptEnhanced
    = std::bit_cast<CVirtualMachine_ExecuteScriptEnhanced_t>(NWN2_OFFSET_EXECUTESCRIPT_ENH);

using CVirtualMachine_InitParam_t  = void(__thiscall*)(void* paramLst, uint32_t iNb);
using CVirtualMachine_CleanParam_t = void(__thiscall*)(void* paramLst);

CVirtualMachine_InitParam_t CVirtualMachine_InitParam
    = std::_Bit_cast<CVirtualMachine_InitParam_t>(NWN2_OFFSET_InitParam);
CVirtualMachine_CleanParam_t CVirtualMachine_CleanParam
    = std::_Bit_cast<CVirtualMachine_CleanParam_t>(NWN2_OFFSET_CleanParam);

CVirtualMachine* GetNwn2VirtualMachine()
{
	const auto vm = std::bit_cast<CVirtualMachine**>(NWN2_OFFSET_CVIRTUALMACHINE);
	return *vm;
}

// Add this redirection just so, in case of literal value, don't need to type 3.5f instead of 3.5
void AddFParam(uint32_t* ptrParams, float fParam)
{
	// float Param
	ptrParams[1] = *(uint32_t*)(&fParam); // Set Value
	ptrParams[4] = 1; // Set Type
}

void AddParam(uint32_t* ptrParams, float fParam) { AddFParam(ptrParams, fParam); }

void AddParam(uint32_t* ptrParams, double dParam) { AddFParam(ptrParams, dParam); }

void AddParam(uint32_t* ptrParams, NWN::OBJECTID oParam)
{
	// NwnObject Param
	ptrParams[1] = (uint32_t)oParam; // Set Value
	ptrParams[4] = 4; // Set Type
}

void AddParam(uint32_t* ptrParams, const char* sParam)
{
	// For string it's a little different
	ptrParams[1] = 0; // No "data"
	ptrParams[2] = (uint32_t)(sParam); // Set dataptr
	ptrParams[3] = strlen(sParam) + 1; // set datasize (including endofchar)
	ptrParams[4] = 2; // Set type
}

void AddParam(uint32_t* ptrParams, int iParam)
{
	// Int Param
	ptrParams[1] = *(uint32_t*)(&iParam); // Set Value
	ptrParams[4] = 0; // Set Type
}

void AddParameters(uint32_t* ptrParams) { }

template <typename T, typename... Targs>
void AddParameters(uint32_t* ptrParams, T value, Targs... Fargs)
{
	// call the build for the "first param"
	AddParam(ptrParams, value);

	// Call for the nexts parameters
	AddParameters(ptrParams + 5, Fargs...);
}

// Because we don't really create new NWString, just "cheat" a little
// and remove them from the parameters list (change them in int)
void PrepareCleanParameter(uint32_t* ptrParams, int iSize)
{
	for (int i = 0; i < iSize; i++) {
		// We have a string ?
		if (ptrParams[4] == 2) {
			// change it to a "0" int.
			ptrParams[1] = 0;
			ptrParams[2] = 0;
			ptrParams[3] = 0;
			ptrParams[4] = 0;
		}
		ptrParams += 5;
	}
}
} // namespace ScriptManagement

template <typename... Targs>
int ExecuteEnhancedScript_sp(std::string sScriptName, NWN::OBJECTID oTarget, Targs... Parameters)
{
	int retValue = -1;
	const NWN::CExoString script
	    = {.m_sString = (sScriptName).data(), .m_nBufferLength = (sScriptName.size() + 1)};

	int iSize = sizeof...(Parameters);

	void* param1 = (void*)0x0086F15C;

	uint32_t* save = new uint32_t[iSize];

	for (int i = 0; i < iSize; i++) {
		save[i] = ((uint32_t*)param1)[i];
	}

	ScriptManagement::CVirtualMachine_InitParam(param1, iSize);

	uint32_t* ptrParams    = *(uint32_t**)param1;
	((uint32_t*)param1)[1] = iSize;

	// Prepare Parameters
	ScriptManagement::AddParameters(ptrParams, Parameters...);

	// call the script
	const auto vm = ScriptManagement::GetNwn2VirtualMachine();
	retValue      = ScriptManagement::CVirtualMachine_ExecuteScriptEnhanced(vm, script, oTarget,
	                                                                        (void*)0x0086F15C, 1, 1);

	// Is the script ok?
	if (retValue != 0)
		retValue = ((uint32_t*)vm)[1];
	else
		retValue = -1;

	ScriptManagement::PrepareCleanParameter(ptrParams, iSize);
	param1 = (void*)0x0086F15C;
	ScriptManagement::CVirtualMachine_CleanParam(param1); // Problème si string ?

	// Pop the saved values
	for (int i = 0; i < iSize; i++) {
		((uint32_t*)param1)[i] = save[i];
	}

	delete[] save;

	return retValue;
};

#endif