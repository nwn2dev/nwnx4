#include "CPlugin.h"

#include "../misc/log.h"
#include <cassert>
#include <filesystem>

extern LogNWNX* logger;

CPlugin::CPlugin(HINSTANCE hDLL, const CPlugin::InitInfo& initInfo)
{

	auto abiVersion = reinterpret_cast<uint32_t*>(GetProcAddress(hDLL, "nwnxcplugin_abi_version"));
	assert(abiVersion != nullptr);
	m_dll.abiVersion = *abiVersion;

	// Map functions
	if (m_dll.abiVersion == 1) {
		// Latest ABI version: just map functions as is
		// Note: older ABI version will need to provide custom function pointers to ensure
		// compatibility
		if(logger->Level() >= LogLevel::trace){
			// clang-format off
			logger->Trace("NWNXCPlugin_New: %lu", GetProcAddress(hDLL, "NWNXCPlugin_New"));
			logger->Trace("NWNXCPlugin_Delete: %lu", GetProcAddress(hDLL, "NWNXCPlugin_Delete"));
			logger->Trace("NWNXCPlugin_GetInfo: %lu", GetProcAddress(hDLL, "NWNXCPlugin_GetInfo"));
			logger->Trace("NWNXCPlugin_GetVersion: %lu", GetProcAddress(hDLL, "NWNXCPlugin_GetVersion"));
			logger->Trace("NWNXCPlugin_GetID: %lu", GetProcAddress(hDLL, "NWNXCPlugin_GetID"));
			logger->Trace("NWNXCPlugin_GetInt: %lu", GetProcAddress(hDLL, "NWNXCPlugin_GetInt"));
			logger->Trace("NWNXCPlugin_SetInt: %lu", GetProcAddress(hDLL, "NWNXCPlugin_SetInt"));
			logger->Trace("NWNXCPlugin_GetFloat: %lu", GetProcAddress(hDLL, "NWNXCPlugin_GetFloat"));
			logger->Trace("NWNXCPlugin_SetFloat: %lu", GetProcAddress(hDLL, "NWNXCPlugin_SetFloat"));
			logger->Trace("NWNXCPlugin_GetString: %lu", GetProcAddress(hDLL, "NWNXCPlugin_GetString"));
			logger->Trace("NWNXCPlugin_SetString: %lu", GetProcAddress(hDLL, "NWNXCPlugin_SetString"));
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
		auto dllName       = std::filesystem::path(initInfo.dll_path).stem().string();
		m_dll.getID = [dllName](void*) { return dllName.c_str(); };
	}

	// Initialize instance
	m_instancePtr = m_dll.newPlugin(initInfo);
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
std::string
CPlugin::GetInfo() const
{
	if (m_dll.getInfo)
		return std::string(m_dll.getInfo());
	return GetID();
}
std::string
CPlugin::GetVersion() const
{
	if (m_dll.getVersion)
		return std::string(m_dll.getVersion());
	return "unknown";
}
std::string
CPlugin::GetID() const
{
	return std::string(m_dll.getID(m_instancePtr));
}
int
CPlugin::GetInt(const char* sFunction, const char* sParam1, int nParam2)
{
	if (m_dll.getInt) {
		return m_dll.getInt(m_instancePtr, sFunction, sParam1, nParam2);
	}
	return 0;
}

void
CPlugin::SetInt(const char* sFunction, const char* sParam1, int nParam2, int nValue)
{
	if (m_dll.setInt) {
		m_dll.setInt(m_instancePtr, sFunction, sParam1, nParam2, nValue);
	}
}

float
CPlugin::GetFloat(const char* sFunction, const char* sParam1, int nParam2)
{
	if (m_dll.getFloat) {
		return m_dll.getFloat(m_instancePtr, sFunction, sParam1, nParam2);
	}
	return 0.0;
}

void
CPlugin::SetFloat(const char* sFunction, const char* sParam1, int nParam2, float fValue)
{
	if (m_dll.setFloat) {
		m_dll.setFloat(m_instancePtr, sFunction, sParam1, nParam2, fValue);
	}
}

void
CPlugin::GetString(const char* sFunction,
                   const char* sParam1,
                   int nParam2,
                   char* result,
                   size_t resultSize)
{
	if (m_dll.getString) {
		return m_dll.getString(m_instancePtr, sFunction, sParam1, nParam2, result, resultSize);
	}
}

void
CPlugin::SetString(const char* sFunction, const char* sParam1, int nParam2, char* sValue)
{
	if (m_dll.setString) {
		m_dll.setString(m_instancePtr, sFunction, sParam1, nParam2, sValue);
	}
}
