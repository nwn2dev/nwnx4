
#include <cassert>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <queue>
#include <unordered_map>

#include <nwnx_cplugin.h>

// Set plugin ABI version. Current latest version is 1
const uint32_t nwnxcplugin_abi_version = 1;

class Plugin {
public:
	Plugin(const std::filesystem::path& logFile, const NWNXCPlugin_NWN2Hooks* hooks)
	{
		this->logFile.open(logFile);

		// Disable buffering
		// This is bad for performance but useful for testing
		this->logFile.rdbuf()->pubsetbuf(0, 0);

		this->hooks = *hooks;
	}

	std::ofstream logFile;

	int counter       = 0;
	float storedFloat = 0;
	std::string storedString;

	typedef std::vector<uint8_t> GffData;
	std::unordered_map<std::string, std::queue<GffData>> storedGFFs;

	NWNXCPlugin_NWN2Hooks hooks;
};

Plugin* g_plugin;

void* NWNXCPlugin_New(NWNXCPlugin_InitInfo info)
{
	auto logFilePath = std::filesystem::path(info.nwnx_user_path) / "xp_cplugin_example.txt";
	auto plugin      = new Plugin(logFilePath, info.nwn2_hooks);
	plugin->logFile << "Plugin initialized" << std::endl;
	plugin->logFile << "    dll_path: " << info.dll_path << std::endl;
	plugin->logFile << "    nwnx_user_path: " << info.nwnx_user_path << std::endl;
	plugin->logFile << "    nwnx_install_path: " << info.nwnx_install_path << std::endl;
	plugin->logFile << "    nwn2_home_path: " << info.nwn2_home_path << std::endl;
	plugin->logFile << "    nwn2_install_path: " << info.nwn2_install_path << std::endl;
	plugin->logFile << "    nwn2_module_path: "
	                << (info.nwn2_module_path == nullptr ? "unknown" : info.nwn2_module_path)
	                << std::endl;
	plugin->logFile << "    nwn2_hooks: " << std::endl;
	plugin->logFile << "        ExecuteScript: " << (void*)info.nwn2_hooks->ExecuteScript
	                << std::endl;
	plugin->logFile << "        ExecuteScriptEnhanced: "
	                << (void*)info.nwn2_hooks->ExecuteScriptEnhanced << std::endl;
	plugin->logFile << "        AddScriptParameterInt: "
	                << (void*)info.nwn2_hooks->AddScriptParameterInt << std::endl;
	plugin->logFile << "        AddScriptParameterString: "
	                << (void*)info.nwn2_hooks->AddScriptParameterString << std::endl;
	plugin->logFile << "        AddScriptParameterFloat: "
	                << (void*)info.nwn2_hooks->AddScriptParameterFloat << std::endl;
	plugin->logFile << "        AddScriptParameterObject: "
	                << (void*)info.nwn2_hooks->AddScriptParameterObject << std::endl;
	plugin->logFile << "        ClearScriptParams: " << info.nwn2_hooks->ClearScriptParams
	                << std::endl;
	g_plugin = plugin;
	return plugin;
}

void NWNXCPlugin_Delete(void* cplugin)
{
	auto plugin = static_cast<Plugin*>(cplugin);
	plugin->logFile << "Plugin cleaned" << std::endl;
	delete plugin;
}

const char* NWNXCPlugin_GetID(void* cplugin) { return "CPluginExample"; }

const char* NWNXCPlugin_GetVersion() { return "1.0.0 (" __DATE__ " " __TIME__ ")"; }

const char* NWNXCPlugin_GetInfo()
{
	return "NWNX4 CPlugin Example - A C++ plugin showcasing C plugin ABI";
}

int32_t
NWNXCPlugin_GetInt(void* cplugin, const char* sFunction, const char* sParam1, int32_t nParam2)
{
	auto plugin = static_cast<Plugin*>(cplugin);
	std::string function {sFunction};

	plugin->logFile << "NWNXCPlugin_GetInt(\"" << function << "\", \"" << sParam1 << "\", "
	                << nParam2 << ")" << std::endl;

	if (function == "GET_COUNTER") {
		plugin->logFile << "  Return counter value: " << plugin->counter << std::endl;
		return plugin->counter;
	} else if (function == "INC_COUNTER") {
		plugin->counter++;
		plugin->logFile << "  Increment and return counter value: " << plugin->counter << std::endl;
		return plugin->counter;
	} else if (function == "TEST_EXECUTESCRIPT") {
		constexpr uint32_t OBJID_MODULE = 0;
		bool executed                   = false;
		plugin->hooks.ExecuteScript("gui_test_executescript", OBJID_MODULE, &executed, false);
		return executed == true ? 12 : 0;
	} else if (function == "TEST_EXECUTESCRIPTBAD") {
		constexpr uint32_t OBJID_MODULE = 0;
		bool executed                   = true;
		plugin->hooks.ExecuteScript("euqsgdihohcqsc", OBJID_MODULE, &executed, false);
		return executed == false ? 13 : 0;
	} else if (function == "TEST_EXECUTESCRIPTENH") {
		constexpr uint32_t OBJID_MODULE = 0;
		plugin->hooks.ClearScriptParams();
		plugin->hooks.AddScriptParameterString(sParam1);
		plugin->hooks.AddScriptParameterFloat(13.37f);
		plugin->hooks.AddScriptParameterInt(-1234);
		plugin->hooks.AddScriptParameterObject(0x01020304);
		bool executed = false;
		auto res = plugin->hooks.ExecuteScriptEnhanced("gui_test_executescriptenh", OBJID_MODULE,
		                                               true, &executed, false);
		if (!executed)
			res -= 1000;
		return res;
	} else if (function == "TEST_EXECUTESCRIPTENHBAD") {
		constexpr uint32_t OBJID_MODULE = 0;
		bool executed                   = true;
		auto res = plugin->hooks.ExecuteScriptEnhanced("euqsgdihohcqsc", OBJID_MODULE, true,
		                                               &executed, false);
		if (executed)
			res -= 1000;
		return res;
	} else {
		plugin->logFile << "  ERROR: NWNXCPlugin_GetInt: unknown function \"" << function << "\""
		                << std::endl;
		return 0;
	}
}

void NWNXCPlugin_SetInt(
    void* cplugin, const char* sFunction, const char* sParam1, int32_t nParam2, int32_t nValue)
{
	auto plugin = static_cast<Plugin*>(cplugin);
	std::string function {sFunction};

	plugin->logFile << "NWNXCPlugin_SetInt(\"" << sFunction << "\", \"" << sParam1 << "\", "
	                << nParam2 << ", " << nValue << ")" << std::endl;

	if (strcmp(sFunction, "SET_COUNTER") == 0) {
		plugin->logFile << "  Set counter to " << plugin->counter << std::endl;
		plugin->counter = nValue;
	} else {
		plugin->logFile << "  ERROR: NWNXCPlugin_SetInt: unknown function \"" << sFunction << "\""
		                << std::endl;
	}
}

float NWNXCPlugin_GetFloat(void* cplugin,
                           const char* sFunction,
                           const char* sParam1,
                           int32_t nParam2)
{
	auto plugin = static_cast<Plugin*>(cplugin);

	plugin->logFile << "NWNXCPlugin_GetFloat(\"" << sFunction << "\", \"" << sParam1 << "\", "
	                << nParam2 << ")" << std::endl;

	plugin->logFile << "  Return " << plugin->storedFloat << std::endl;
	return plugin->storedFloat;
}

void NWNXCPlugin_SetFloat(
    void* cplugin, const char* sFunction, const char* sParam1, int32_t nParam2, float fValue)
{
	auto plugin = static_cast<Plugin*>(cplugin);

	plugin->logFile << "NWNXCPlugin_SetFloat(\"" << sFunction << "\", \"" << sParam1 << "\", "
	                << nParam2 << ", " << fValue << ")" << std::endl;

	plugin->logFile << "  Set to " << plugin->storedFloat << std::endl;
	plugin->storedFloat = fValue;
}

void NWNXCPlugin_GetString(void* cplugin,
                           const char* sFunction,
                           const char* sParam1,
                           int32_t nParam2,
                           char* result,
                           size_t resultSize)
{
	auto plugin = static_cast<Plugin*>(cplugin);

	plugin->logFile << "NWNXCPlugin_GetString(\"" << sFunction << "\", \"" << sParam1 << "\", "
	                << nParam2 << ")" << std::endl;

	// Copy stored string into result buffer
	plugin->logFile << "  Return " << plugin->storedString << std::endl;
	strncpy_s(result, resultSize, plugin->storedString.c_str(), plugin->storedString.size());
}

void NWNXCPlugin_SetString(
    void* cplugin, const char* sFunction, const char* sParam1, int32_t nParam2, const char* sValue)
{
	auto plugin = static_cast<Plugin*>(cplugin);

	plugin->logFile << "NWNXCPlugin_SetString(\"" << sFunction << "\", \"" << sParam1 << "\", "
	                << nParam2 << ", \"" << sValue << "\")" << std::endl;

	// Copy sValue into stored string
	plugin->storedString = std::string(sValue);
	plugin->logFile << "  Set to \"" << plugin->storedString << "\"" << std::endl;
}

size_t NWNXCPlugin_GetGFFSize(void* cplugin, const char* sVarName)
{
	auto plugin = static_cast<Plugin*>(cplugin);

	plugin->logFile << "NWNXCPlugin_GetGFFSize(\"" << sVarName << "\")" << std::endl;

	auto queueIt = plugin->storedGFFs.find(sVarName);
	if (queueIt != plugin->storedGFFs.end()) {
		if (queueIt->second.size() > 0)
			return queueIt->second.front().size();
	}
	return 0;
}

void NWNXCPlugin_GetGFF(void* cplugin, const char* sVarName, uint8_t* result, size_t resultSize)
{
	assert(result != nullptr);
	assert(resultSize > 0);

	auto plugin = static_cast<Plugin*>(cplugin);

	plugin->logFile << "NWNXCPlugin_GetGFF(\"" << sVarName << "\")" << std::endl;

	auto queueIt = plugin->storedGFFs.find(sVarName);

	// NWNXCPlugin_GetGFFSize has the responsibility of checking if the GFF
	// data to return does exist.
	// If NWNXCPlugin_GetGFF is called, it means that the GFF data must exist
	// and that it fits into the result buffer.
	assert(queueIt != plugin->storedGFFs.end());
	assert(queueIt->second.size() > 0);
	assert(queueIt->second.front().size() == resultSize);

	auto& gff = queueIt->second.front();
	memcpy(result, gff.data(), gff.size());

	queueIt->second.pop();

	plugin->logFile << "  Returned GFF and popped queue with key \"" << sVarName << "\" => "
	                << queueIt->second.size() << " GFFs with this key" << std::endl;
}

void NWNXCPlugin_SetGFF(void* cplugin,
                        const char* sVarName,
                        const uint8_t* gffData,
                        size_t gffDataSize)
{
	auto plugin = static_cast<Plugin*>(cplugin);

	char gffHeaderText[9] = {0};
	strncpy_s(gffHeaderText, 9, (const char*)gffData, 8);
	plugin->logFile << "NWNXCPlugin_SetGFF(\"" << sVarName << "\", \"" << gffHeaderText << "...\", "
	                << gffDataSize << ")" << std::endl;

	auto queueIt = plugin->storedGFFs.find(sVarName);
	if (queueIt == plugin->storedGFFs.end()) {
		plugin->storedGFFs.insert({sVarName, {}});
		queueIt = plugin->storedGFFs.find(sVarName);
	}

	queueIt->second.push({gffData, gffData + gffDataSize});
	plugin->logFile << "  Append GFF data for key \"" << sVarName << "\" => "
	                << queueIt->second.size() << " GFFs with this key" << std::endl;
}
