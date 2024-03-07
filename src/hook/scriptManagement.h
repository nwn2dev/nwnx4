#if !defined(SCRIPTMANAGEMENT_H_INCLUDED)
#define SCRIPTMANAGEMENT_H_INCLUDED

#define DLLEXPORT extern "C" __declspec(dllexport)

#include "../NWN2Lib/NWN2.h"
#include "../NWN2Lib/NWN2Common.h"
#include <cstdint>

namespace NWScript {

extern "C" {
void ExecuteScript(const char* sScript, NWN::OBJECTID oTarget);
int32_t ExecuteScriptEnhanced(const char* sScriptName, NWN::OBJECTID oTarget, bool bClearParams);
void AddScriptParameterInt(int32_t nParam);
void AddScriptParameterString(const char* sParam);
void AddScriptParameterFloat(float fParam);
void AddScriptParameterObject(NWN::OBJECTID oParam);
void ClearScriptParams();
}

} // namespace NWScript

#endif