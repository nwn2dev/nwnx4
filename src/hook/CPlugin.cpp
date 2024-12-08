#include "CPlugin.h"

#include "../misc/log.h"
#include <cassert>
#include <filesystem>

extern std::unique_ptr<LogNWNX> logger;

namespace CompatV1 {

void __cdecl ExecuteScript(const char* sScript, NWN::OBJECTID oTarget, bool* outExecuted)
{
	return NWScript::ExecuteScript(sScript, oTarget, outExecuted, true);
}
int __cdecl ExecuteScriptEnhanced(const char* sScript,
                                  NWN::OBJECTID oTarget,
                                  bool bClearParams,
                                  bool* outExecuted)
{
	return NWScript::ExecuteScriptEnhanced(sScript, oTarget, bClearParams, outExecuted, true);
}

struct NWN2Hooks {
	using ExecuteScriptFn            = decltype(ExecuteScript);
	using ExecuteScriptEnhancedFn    = decltype(ExecuteScriptEnhanced);
	using AddScriptParameterIntFn    = CPlugin::NWN2Hooks::AddScriptParameterIntFn;
	using AddScriptParameterStringFn = CPlugin::NWN2Hooks::AddScriptParameterStringFn;
	using AddScriptParameterFloatFn  = CPlugin::NWN2Hooks::AddScriptParameterFloatFn;
	using AddScriptParameterObjectFn = CPlugin::NWN2Hooks::AddScriptParameterObjectFn;
	using ClearScriptParamsFn        = CPlugin::NWN2Hooks::ClearScriptParamsFn;

	ExecuteScriptFn* ExecuteScript;
	ExecuteScriptEnhancedFn* ExecuteScriptEnhanced;
	AddScriptParameterIntFn* AddScriptParameterInt;
	AddScriptParameterStringFn* AddScriptParameterString;
	AddScriptParameterFloatFn* AddScriptParameterFloat;
	AddScriptParameterObjectFn* AddScriptParameterObject;
	ClearScriptParamsFn* ClearScriptParams;
};

struct InitInfo {
	const char* dll_path;
	const char* nwnx_user_path;
	const char* nwn2_install_path;
	const char* nwn2_home_path;
	const char* nwn2_module_path;
	const char* nwnx_install_path;
	const struct NWN2Hooks* nwn2_hooks;
};

typedef void*(__cdecl NewPluginFn)(InitInfo info);
typedef void(__cdecl GetStringFn)(void* cplugin,
                                  const char* sFunction,
                                  const char* sParam1,
                                  int32_t nParam2,
                                  char* result,
                                  size_t resultSize);

} // namespace CompatV1

CPlugin::CPlugin(HINSTANCE hDLL, const CPlugin::InitInfo& initInfo)
{

	auto abiVersion = reinterpret_cast<uint32_t*>(GetProcAddress(hDLL, "nwnxcplugin_abi_version"));
	assert(abiVersion != nullptr);
	m_dll.abiVersion = *abiVersion;

	// Map functions
	if (m_dll.abiVersion == 1) {

		auto newPlugin
		    = reinterpret_cast<CompatV1::NewPluginFn*>(GetProcAddress(hDLL, "NWNXCPlugin_New"));
		m_dll.newPlugin = [newPlugin](const InitInfo* info) {
			auto hooks = CompatV1::NWN2Hooks {
			    // v2 added 1 additional parameter for ExecuteScript & ExecuteScriptEnhanced (see
			    // commit edc229f)
			    .ExecuteScript            = CompatV1::ExecuteScript,
			    .ExecuteScriptEnhanced    = CompatV1::ExecuteScriptEnhanced,
			    .AddScriptParameterInt    = info->nwn2_hooks->AddScriptParameterInt,
			    .AddScriptParameterString = info->nwn2_hooks->AddScriptParameterString,
			    .AddScriptParameterFloat  = info->nwn2_hooks->AddScriptParameterFloat,
			    .AddScriptParameterObject = info->nwn2_hooks->AddScriptParameterObject,
			    .ClearScriptParams        = info->nwn2_hooks->ClearScriptParams,
			};
			// v2 reordered InitInfo members
			auto initInfo = CompatV1::InitInfo {
			    .dll_path          = info->dll_path,
			    .nwnx_user_path    = info->nwnx_user_path,
			    .nwn2_install_path = info->nwn2_install_path,
			    .nwn2_home_path    = info->nwn2_home_path,
			    .nwn2_module_path  = info->nwn2_module_path,
			    .nwnx_install_path = info->nwnx_install_path,
			    .nwn2_hooks        = &hooks,
			};
			// v2 is passing initInfo as a const pointer
			return newPlugin(initInfo);
		};

		// v2 changed NWNXCPlugin_GetString behaviour: now NWNXCPlugin_GetString must return the
		// result string pointer, to avoid unnecessary strcpy from a stored value into `result`
		auto getString = reinterpret_cast<CompatV1::GetStringFn*>(
		    GetProcAddress(hDLL, "NWNXCPlugin_GetString"));
		m_dll.getString = [getString](void* cplugin, const char* sFunction, const char* sParam1,
		                              int32_t nParam2, char* result, size_t resultSize) {
			getString(cplugin, sFunction, sParam1, nParam2, result, resultSize);
			return result;
		};

		// clang-format off
		m_dll.deletePlugin = reinterpret_cast<DeletePluginFn*>(GetProcAddress(hDLL, "NWNXCPlugin_Delete"));
		m_dll.getInfo      = reinterpret_cast<GetInfoFn*>(     GetProcAddress(hDLL, "NWNXCPlugin_GetInfo"));
		m_dll.getVersion   = reinterpret_cast<GetVersionFn*>(  GetProcAddress(hDLL, "NWNXCPlugin_GetVersion"));
		m_dll.getID        = reinterpret_cast<GetIDFn*>(       GetProcAddress(hDLL, "NWNXCPlugin_GetID"));
		m_dll.getInt       = reinterpret_cast<GetIntFn*>(      GetProcAddress(hDLL, "NWNXCPlugin_GetInt"));
		m_dll.setInt       = reinterpret_cast<SetIntFn*>(      GetProcAddress(hDLL, "NWNXCPlugin_SetInt"));
		m_dll.getFloat     = reinterpret_cast<GetFloatFn*>(    GetProcAddress(hDLL, "NWNXCPlugin_GetFloat"));
		m_dll.setFloat     = reinterpret_cast<SetFloatFn*>(    GetProcAddress(hDLL, "NWNXCPlugin_SetFloat"));
		m_dll.setString    = reinterpret_cast<SetStringFn*>(   GetProcAddress(hDLL, "NWNXCPlugin_SetString"));
		m_dll.getGFFSize   = reinterpret_cast<GetGFFSizeFn*>(  GetProcAddress(hDLL, "NWNXCPlugin_GetGFFSize"));
		m_dll.getGFF       = reinterpret_cast<GetGFFFn*>(      GetProcAddress(hDLL, "NWNXCPlugin_GetGFF"));
		m_dll.setGFF       = reinterpret_cast<SetGFFFn*>(      GetProcAddress(hDLL, "NWNXCPlugin_SetGFF"));
		// clang-format on
	} else if (m_dll.abiVersion == 2) {
		// Latest ABI version: just map functions as is
		// Note: older ABI version will need to provide custom function pointers to ensure
		// compatibility
		if (logger->Level() >= LogLevel::trace) {
			// clang-format off
			logger->Trace("NWNXCPlugin_New: %lu",        GetProcAddress(hDLL, "NWNXCPlugin_New"));
			logger->Trace("NWNXCPlugin_Delete: %lu",     GetProcAddress(hDLL, "NWNXCPlugin_Delete"));
			logger->Trace("NWNXCPlugin_GetInfo: %lu",    GetProcAddress(hDLL, "NWNXCPlugin_GetInfo"));
			logger->Trace("NWNXCPlugin_GetVersion: %lu", GetProcAddress(hDLL, "NWNXCPlugin_GetVersion"));
			logger->Trace("NWNXCPlugin_GetID: %lu",      GetProcAddress(hDLL, "NWNXCPlugin_GetID"));
			logger->Trace("NWNXCPlugin_GetInt: %lu",     GetProcAddress(hDLL, "NWNXCPlugin_GetInt"));
			logger->Trace("NWNXCPlugin_SetInt: %lu",     GetProcAddress(hDLL, "NWNXCPlugin_SetInt"));
			logger->Trace("NWNXCPlugin_GetFloat: %lu",   GetProcAddress(hDLL, "NWNXCPlugin_GetFloat"));
			logger->Trace("NWNXCPlugin_SetFloat: %lu",   GetProcAddress(hDLL, "NWNXCPlugin_SetFloat"));
			logger->Trace("NWNXCPlugin_GetString: %lu",  GetProcAddress(hDLL, "NWNXCPlugin_GetString"));
			logger->Trace("NWNXCPlugin_SetString: %lu",  GetProcAddress(hDLL, "NWNXCPlugin_SetString"));
			logger->Trace("NWNXCPlugin_GetGFFSize: %lu", GetProcAddress(hDLL, "NWNXCPlugin_GetGFFSize"));
			logger->Trace("NWNXCPlugin_GetGFF: %lu",     GetProcAddress(hDLL, "NWNXCPlugin_GetGFF"));
			logger->Trace("NWNXCPlugin_SetGFF: %lu",     GetProcAddress(hDLL, "NWNXCPlugin_SetGFF"));
			// clang-format on
		}
		// clang-format off
		m_dll.newPlugin    = reinterpret_cast<NewPluginFn*>(   GetProcAddress(hDLL, "NWNXCPlugin_New"));
		m_dll.deletePlugin = reinterpret_cast<DeletePluginFn*>(GetProcAddress(hDLL, "NWNXCPlugin_Delete"));
		m_dll.getInfo      = reinterpret_cast<GetInfoFn*>(     GetProcAddress(hDLL, "NWNXCPlugin_GetInfo"));
		m_dll.getVersion   = reinterpret_cast<GetVersionFn*>(  GetProcAddress(hDLL, "NWNXCPlugin_GetVersion"));
		m_dll.getID        = reinterpret_cast<GetIDFn*>(       GetProcAddress(hDLL, "NWNXCPlugin_GetID"));
		m_dll.getInt       = reinterpret_cast<GetIntFn*>(      GetProcAddress(hDLL, "NWNXCPlugin_GetInt"));
		m_dll.setInt       = reinterpret_cast<SetIntFn*>(      GetProcAddress(hDLL, "NWNXCPlugin_SetInt"));
		m_dll.getFloat     = reinterpret_cast<GetFloatFn*>(    GetProcAddress(hDLL, "NWNXCPlugin_GetFloat"));
		m_dll.setFloat     = reinterpret_cast<SetFloatFn*>(    GetProcAddress(hDLL, "NWNXCPlugin_SetFloat"));
		m_dll.getString    = reinterpret_cast<GetStringFn*>(   GetProcAddress(hDLL, "NWNXCPlugin_GetString"));
		m_dll.setString    = reinterpret_cast<SetStringFn*>(   GetProcAddress(hDLL, "NWNXCPlugin_SetString"));
		m_dll.getGFFSize   = reinterpret_cast<GetGFFSizeFn*>(  GetProcAddress(hDLL, "NWNXCPlugin_GetGFFSize"));
		m_dll.getGFF       = reinterpret_cast<GetGFFFn*>(      GetProcAddress(hDLL, "NWNXCPlugin_GetGFF"));
		m_dll.setGFF       = reinterpret_cast<SetGFFFn*>(      GetProcAddress(hDLL, "NWNXCPlugin_SetGFF"));
		// clang-format on
	} else
		throw std::exception(
		    ("Unsupported CPlugin ABI version " + std::to_string(m_dll.abiVersion)).c_str());

	// Check mandatory definitions
	if (!m_dll.newPlugin)
		throw std::exception("Missing required function: NWNXCPlugin_New");

	// Provide fallbacks if needed
	if (!m_dll.getID) {
		// Fallback to plugin file name
		auto dllName = std::filesystem::path(initInfo.dll_path).stem().string();
		m_dll.getID  = [dllName](void*) { return dllName.c_str(); };
	}

	// Sanity checks
	if ((bool)m_dll.getGFFSize ^ (bool)m_dll.getGFF) {
		throw std::exception("Both NWNXCPlugin_GetGFFSize and NWNXCPlugin_GetGFF must be "
		                     "implemented by the plugin in order to use SCORCO functions");
	}

	// Initialize instance
	m_instancePtr = m_dll.newPlugin(&initInfo);
	if (m_instancePtr == nullptr) {
		logger->Err("NWNXCPlugin_New returned null");
		return;
	}
}

CPlugin::~CPlugin()
{
	if (m_dll.deletePlugin)
		m_dll.deletePlugin(m_instancePtr);
}
