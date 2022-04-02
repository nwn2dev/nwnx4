
#include <filesystem>
#include <iostream>
#include <fstream>

#include <nwnx_cplugin.h>

// Set plugin ABI version. Current latest version is 1
uint32_t nwnxcplugin_abi_version = 1;


class Plugin {
public:
	Plugin(const std::filesystem::path& logFile){
		this->logFile.open(logFile);
		this->logFile.rdbuf()->pubsetbuf(0, 0); // Disable buffering
	}

	std::ofstream logFile;

	int counter = 0;
	float storedFloat = 0;
	std::string storedString;
};

void* NWNXCPlugin_New(NWNXCPlugin_InitInfo info){
	auto logFilePath = std::filesystem::path(info.nwnx_path) / "xp_cplugin_example.txt";
	auto plugin = new Plugin(logFilePath);
	plugin->logFile << "Plugin initialized" << std::endl;
	return plugin;
}

void NWNXCPlugin_Delete(void* cplugin){
	auto plugin = static_cast<Plugin*>(cplugin);
	plugin->logFile << "Plugin cleaned" << std::endl;
	delete plugin;
}

const char* NWNXCPlugin_GetID(void* cplugin){
	return "CPluginExample";
}

const char* NWNXCPlugin_GetVersion(){
	return "1.0.0 (" __DATE__ " " __TIME__ ")";
}

const char* NWNXCPlugin_GetName(){
	return "NWNX4 C Plugin Example";
}

int32_t NWNXCPlugin_GetInt(void* cplugin,
                           const char* sFunction,
                           const char* sParam1,
                           int32_t nParam2){
	auto plugin = static_cast<Plugin*>(cplugin);
	std::string function{sFunction};

	plugin->logFile << "NWNXCPlugin_GetInt(\"" << function << "\", \"" << sParam1 <<  "\", " << nParam2 <<  ")" << std::endl;

	if(function == "GET_COUNTER"){
		plugin->logFile << "  Return counter value: " << plugin->counter << std::endl;
		return plugin->counter;
	}
	else if(function == "INC_COUNTER"){
		plugin->counter++;
		plugin->logFile << "  Increment and return counter value: " << plugin->counter << std::endl;
		return plugin->counter;
	}
	else{
		plugin->logFile << "  ERROR: NWNXCPlugin_GetInt: unknown function \"" << function << "\"" << std::endl;
		return 0;
	}
}

void NWNXCPlugin_SetInt(void* cplugin,
                        const char* sFunction,
                        const char* sParam1,
                        int32_t nParam2,
                        int32_t nValue){
	auto plugin = static_cast<Plugin*>(cplugin);
	std::string function{sFunction};

	plugin->logFile << "NWNXCPlugin_SetInt(\"" << sFunction << "\", \"" << sParam1 <<  "\", " << nParam2 << ", " << nValue <<  ")" << std::endl;

	if(strcmp(sFunction, "SET_COUNTER") == 0){
		plugin->logFile << "  Set counter to " << plugin->counter << std::endl;
		plugin->counter = nValue;
	}
	else{
		plugin->logFile << "  ERROR: NWNXCPlugin_SetInt: unknown function \"" << sFunction << "\"" << std::endl;
	}
}

float NWNXCPlugin_GetFloat(void* cplugin,
                           const char* sFunction,
                           const char* sParam1,
                           int32_t nParam2){
	auto plugin = static_cast<Plugin*>(cplugin);

	plugin->logFile << "NWNXCPlugin_GetFloat(\"" << sFunction << "\", \"" << sParam1 <<  "\", " << nParam2 <<  ")" << std::endl;

	plugin->logFile << "  Return " << plugin->storedFloat << std::endl;
	return plugin->storedFloat;
}

void NWNXCPlugin_SetFloat(void* cplugin,
                          const char* sFunction,
                          const char* sParam1,
                          int32_t nParam2,
                          float fValue){
	auto plugin = static_cast<Plugin*>(cplugin);

	plugin->logFile << "NWNXCPlugin_SetFloat(\"" << sFunction << "\", \"" << sParam1 <<  "\", " << nParam2 << ", " << fValue <<  ")" << std::endl;

	plugin->logFile << "  Set to " << plugin->storedFloat << std::endl;
	plugin->storedFloat = fValue;
}

void NWNXCPlugin_GetString(void* cplugin,
                           const char* sFunction,
                           const char* sParam1,
                           int32_t nParam2,
                           char* result,
                           size_t resultSize){
	auto plugin = static_cast<Plugin*>(cplugin);

	plugin->logFile << "NWNXCPlugin_GetString(\"" << sFunction << "\", \"" << sParam1 <<  "\", " << nParam2 << ")" << std::endl;

	// Copy stored string into result buffer
	plugin->logFile << "  Return " << plugin->storedString << std::endl;
	strncpy_s(result, resultSize, plugin->storedString.c_str(), plugin->storedString.size());
}

void NWNXCPlugin_SetString(void* cplugin,
                           const char* sFunction,
                           const char* sParam1,
                           int32_t nParam2,
                           const char* sValue){
	auto plugin = static_cast<Plugin*>(cplugin);

	plugin->logFile << "NWNXCPlugin_SetString(\"" << sFunction << "\", \"" << sParam1 <<  "\", " << nParam2 << ", \"" << sValue << "\")" << std::endl;

	// Copy sValue into stored string
	plugin->storedString = std::string(sValue);
	plugin->logFile << "  Set to \"" << plugin->storedString << "\"" << std::endl;
}
