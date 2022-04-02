#pragma once

#include <stdint.h>

extern "C" {

#define NWNX_MAX_BUFFER 64*1024

/// Information provided by nwnx4 during plugin initialization.
///
struct NWNXCPlugin_InitInfo {
    /// Path to this plugin DLL file
    const char* dll_path;
    /// Path to the NWNX4 base directory, where nwnx4_controller.exe is located.
    const char* nwnx_path;
    /// Path to the NWN2 installation directory, where nwn2server.exe is located.
    const char* nwn2server_path;
};


//
// REQUIRED VARIABLES TO INSTANTIATE:
//

/// Plugin ABI version for nwnx4 compatibility.
/// **Must be instantiated** and set to 1 in order to use this include file
__declspec(dllexport)
extern uint32_t nwnxcplugin_abi_version;

//
// REQUIRED FUNCTIONS TO IMPLEMENT:
//

/// Plugin initialization function. **Must** be defined ! Called by NWNX4 just
/// after loading the DLL.
/// @param info Useful information provided by nwnx4
/// @return A user data pointer (to a struct or object) containing the plugin runtime data (prefer
///     storing plugin data inside this struct rather than global variables). NULL if the plugin
///     failed to be initialized.
__declspec(dllexport)
void* NWNXCPlugin_New(NWNXCPlugin_InitInfo info);

//
// OPTIONAL FUNCTIONS TO IMPLEMENT:
//

/// Called when the plugin is unloaded, when the server process is terminated. Typically this
/// function is used to free the memory allocated for `cplugin` in `NWNXCPlugin_New`.
/// @param cplugin User data pointer, as returned by `NWNXCPlugin_New`
__declspec(dllexport)
void NWNXCPlugin_Delete(void* cplugin);

/// Returns the ID of the plugin. This ID is used in NWScript NWNXGetXXX / NWNXSetXXX functions to
/// send commands to the plugin.
/// @param cplugin User data pointer, as returned by `NWNXCPlugin_New`
/// @return The plugin ID as a null-terminated string
__declspec(dllexport)
const char* NWNXCPlugin_GetID(void* cplugin);

/// Returns the plugin user-friendly name and information, like "SuperPlugin by Foo Bar"
/// @return null-terminated string
__declspec(dllexport)
const char* NWNXCPlugin_GetName();

/// Returns the plugin version information, like "v1.2.3"
/// @return null-terminated string
__declspec(dllexport)
const char* NWNXCPlugin_GetVersion();

/// Executed by NWScript function `NWNXGetInt`
/// @param cplugin User data pointer, as returned by `NWNXCPlugin_New`
/// @param sFunction NWScript function argument. Null-terminated string
/// @param sParam1 NWScript function argument. Null-terminated string
/// @param nParam2 NWScript function argument.
/// @return NWScript function result
__declspec(dllexport)
int32_t NWNXCPlugin_GetInt(void* cplugin,
                           const char* sFunction,
                           const char* sParam1,
                           int32_t nParam2);

/// Executed by NWScript function `NWNXSetInt`
/// @param cplugin User data pointer, as returned by `NWNXCPlugin_New`
/// @param sFunction NWScript function argument. Null-terminated string
/// @param sParam1 NWScript function argument. Null-terminated string
/// @param nParam2 NWScript function argument.
__declspec(dllexport)
void NWNXCPlugin_SetInt(void* cplugin,
                        const char* sFunction,
                        const char* sParam1,
                        int32_t nParam2,
                        int32_t nValue);

/// Executed by NWScript function `NWNXGetFloat`
/// @param cplugin User data pointer, as returned by `NWNXCPlugin_New`
/// @param sFunction NWScript function argument. Null-terminated string
/// @param sParam1 NWScript function argument. Null-terminated string
/// @param nParam2 NWScript function argument.
/// @return NWScript function result
__declspec(dllexport)
float NWNXCPlugin_GetFloat(void* cplugin,
                           const char* sFunction,
                           const char* sParam1,
                           int32_t nParam2);

/// Executed by NWScript function `NWNXSetFloat`
/// @param cplugin User data pointer, as returned by `NWNXCPlugin_New`
/// @param sFunction NWScript function argument. Null-terminated string
/// @param sParam1 NWScript function argument. Null-terminated string
/// @param nParam2 NWScript function argument.
/// @param fValue NWScript function argument. 32-bit floating point.
__declspec(dllexport)
void NWNXCPlugin_SetFloat(void* cplugin,
                          const char* sFunction,
                          const char* sParam1,
                          int32_t nParam2,
                          float fValue);

/// Executed by NWScript function `NWNXGetString`
/// @param cplugin User data pointer, as returned by `NWNXCPlugin_New`
/// @param sFunction NWScript function argument. Null-terminated string
/// @param sParam1 NWScript function argument. Null-terminated string
/// @param nParam2 NWScript function argument.
/// @param result Buffer for storing the returned string. **Must be null-terminated**.
/// @param resultSize Size of the result buffer
__declspec(dllexport)
void NWNXCPlugin_GetString(void* cplugin,
                           const char* sFunction,
                           const char* sParam1,
                           int32_t nParam2,
                           char* result,
                           size_t resultSize);

/// Executed by NWScript function `NWNXSetString`
/// @param cplugin User data pointer, as returned by `NWNXCPlugin_New`
/// @param sFunction NWScript function argument. Null-terminated string
/// @param sParam1 NWScript function argument. Null-terminated string
/// @param nParam2 NWScript function argument.
/// @param sValue null-terminated string
__declspec(dllexport)
void NWNXCPlugin_SetString(void* cplugin,
                           const char* sFunction,
                           const char* sParam1,
                           int32_t nParam2,
                           const char* sValue);

} // extern "C"
