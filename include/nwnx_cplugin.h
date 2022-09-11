#pragma once

#include <stdint.h>

extern "C" {

/// Information provided by nwnx4 during plugin initialization.
///
struct NWNXCPlugin_InitInfo {
	/// Path to this plugin DLL file
	const char* dll_path;
	/// Path to the NWNX4 user directory, where config files are stored and log files should be
	/// written.
	const char* nwnx_user_path;
	/// Path to the NWN2 installation directory, where nwn2server.exe is located.
	const char* nwn2_install_path;
	/// Path to the NWN2 home folder, usually 'Documents\Neverwinter Nights 2'
	const char* nwn2_home_path;
	/// Loaded module path. Either a .mod file or a directory.
	/// Note: this value depends on the parameters list in nwnx.ini. If the server has not been
	/// started with -module or -moduledir, this value is set to NULL.
	const char* nwn2_module_path;
	/// Path to the NWNX4 user directory, where nwnx4_controller.exe is located.
	const char* nwnx_install_path;
};

//
// REQUIRED VARIABLES TO INSTANTIATE:
//

/// Plugin ABI version for nwnx4 compatibility.
/// **Must be instantiated** and set to 1 in order to use this include file
__declspec(dllexport) extern const uint32_t nwnxcplugin_abi_version;

//
// REQUIRED FUNCTIONS TO IMPLEMENT:
//

/// Plugin initialization function. **Must** be defined ! Called by NWNX4 just
/// after loading the DLL.
/// @param info Useful information provided by nwnx4
/// @return A user data pointer (to a struct or object) containing the plugin runtime data
/// (prefer storing plugin data inside this struct rather than global variables). NULL if the
/// plugin failed to be initialized.
__declspec(dllexport) void* __cdecl NWNXCPlugin_New(NWNXCPlugin_InitInfo info);

//
// OPTIONAL FUNCTIONS TO IMPLEMENT:
//

/// Called when the plugin is unloaded, when the server process is terminated. Typically this
/// function is used to free the memory allocated for `cplugin` in `NWNXCPlugin_New`.
/// @param cplugin User data pointer, as returned by `NWNXCPlugin_New`
__declspec(dllexport) void __cdecl NWNXCPlugin_Delete(void* cplugin);

/// Returns the ID of the plugin. This ID is used in NWScript NWNXGetXXX / NWNXSetXXX functions
/// to send commands to the plugin.
/// @param cplugin User data pointer, as returned by `NWNXCPlugin_New`
/// @return The plugin ID as a null-terminated string
__declspec(dllexport) const char* __cdecl NWNXCPlugin_GetID(void* cplugin);

/// Returns the plugin user-friendly name and basic information, like "SuperPlugin by Foo Bar -
/// Provides example features"
/// @return null-terminated string
__declspec(dllexport) const char* __cdecl NWNXCPlugin_GetInfo();

/// Returns the plugin version information, like "v1.2.3"
/// @return null-terminated string
__declspec(dllexport) const char* __cdecl NWNXCPlugin_GetVersion();

/// Executed by NWScript function `NWNXGetInt`
/// @param cplugin User data pointer, as returned by `NWNXCPlugin_New`
/// @param sFunction NWScript function argument. Null-terminated string
/// @param sParam1 NWScript function argument. Null-terminated string
/// @param nParam2 NWScript function argument.
/// @return NWScript function result
__declspec(dllexport) int32_t __cdecl NWNXCPlugin_GetInt(void* cplugin,
                                                         const char* sFunction,
                                                         const char* sParam1,
                                                         int32_t nParam2);

/// Executed by NWScript function `NWNXSetInt`
/// @param cplugin User data pointer, as returned by `NWNXCPlugin_New`
/// @param sFunction NWScript function argument. Null-terminated string
/// @param sParam1 NWScript function argument. Null-terminated string
/// @param nParam2 NWScript function argument.
__declspec(dllexport) void __cdecl NWNXCPlugin_SetInt(
    void* cplugin, const char* sFunction, const char* sParam1, int32_t nParam2, int32_t nValue);

/// Executed by NWScript function `NWNXGetFloat`
/// @param cplugin User data pointer, as returned by `NWNXCPlugin_New`
/// @param sFunction NWScript function argument. Null-terminated string
/// @param sParam1 NWScript function argument. Null-terminated string
/// @param nParam2 NWScript function argument.
/// @return NWScript function result
__declspec(dllexport) float __cdecl NWNXCPlugin_GetFloat(void* cplugin,
                                                         const char* sFunction,
                                                         const char* sParam1,
                                                         int32_t nParam2);

/// Executed by NWScript function `NWNXSetFloat`
/// @param cplugin User data pointer, as returned by `NWNXCPlugin_New`
/// @param sFunction NWScript function argument. Null-terminated string
/// @param sParam1 NWScript function argument. Null-terminated string
/// @param nParam2 NWScript function argument.
/// @param fValue NWScript function argument. 32-bit floating point.
__declspec(dllexport) void __cdecl NWNXCPlugin_SetFloat(
    void* cplugin, const char* sFunction, const char* sParam1, int32_t nParam2, float fValue);

/// Executed by NWScript function `NWNXGetString`
/// @param cplugin User data pointer, as returned by `NWNXCPlugin_New`
/// @param sFunction NWScript function argument. Null-terminated string
/// @param sParam1 NWScript function argument. Null-terminated string
/// @param nParam2 NWScript function argument.
/// @param result Buffer for storing the returned string. **Must be null-terminated**.
/// @param resultSize Size of the result buffer
__declspec(dllexport) void __cdecl NWNXCPlugin_GetString(void* cplugin,
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
__declspec(dllexport) void __cdecl NWNXCPlugin_SetString(
    void* cplugin, const char* sFunction, const char* sParam1, int32_t nParam2, const char* sValue);

/// Executed by NWScript function `RetrieveCampaignObject`, before `NWNXCPlugin_GetGFF`. This
/// function checks if the GFF object to get does exist and returns the size of the buffer to
/// allocate in order to write the GFF data.
/// @return the size of the buffer NWN2 needs to allocate for receiving the GFF data. 0 if the
///     GFF data does not exist. Returning 0 will prevent `NWNXCPlugin_GetGFF` from being
///     executed
__declspec(dllexport) size_t __cdecl NWNXCPlugin_GetGFFSize(void* cplugin, const char* sVarName);

/// Executed by NWScript function `RetrieveCampaignObject`, after `NWNXCPlugin_GetGFFSize`. This
/// function copies the GFF data into the result buffer. Note that this function is only called
/// if `NWNXCPlugin_GetGFFSize` returned a size > 0, and is expected to always write valid GFF
/// data into the result buffer.
/// @param cplugin User data pointer, as returned by `NWNXCPlugin_New`
/// @param sVarName NWScript function argument 'sVarName'. Null-terminated string
/// @param result Buffer allocated by NWN2 for storing the resulting GFF
/// @param resultSize Size of result in bytes
__declspec(dllexport) void __cdecl NWNXCPlugin_GetGFF(void* cplugin,
                                                      const char* sVarName,
                                                      uint8_t* result,
                                                      size_t resultSize);

/// Executed by NWScript function `StoreCampaignObject`
/// @param cplugin User data pointer, as returned by `NWNXCPlugin_New`
/// @param sVarName NWScript function argument 'sVarName'. Null-terminated string
/// @param gffData Buffer containing the GFF object to set
/// @param gffDataSize Size of result in bytes
__declspec(dllexport) void __cdecl NWNXCPlugin_SetGFF(void* cplugin,
                                                      const char* sVarName,
                                                      const uint8_t* gffData,
                                                      size_t gffDataSize);

} // extern "C"
