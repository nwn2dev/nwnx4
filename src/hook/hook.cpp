/***************************************************************************
   NWNX Hook - Responsible for the actual hooking
   Copyright (C) 2007 Ingmar Stieger (Papillon, papillon@nwnx.org)

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

***************************************************************************/

#include "hook.h"
#include "../misc/windows_utils.h"
#include "../nwnx_version.h"
#include "scorcohook.h"
#include <Shlobj.h>
#include <codecvt>
#include <filesystem>
#include <shellapi.h>
#include <unordered_set>

/*
 * Globals.
 */

SHARED_MEMORY* shmem;

CPluginHashMap cplugins;
PluginHashMap plugins;
LegacyPluginHashMap legacyplugins;

std::unique_ptr<LogNWNX> logger;
std::filesystem::path nwnxUserDir;
std::filesystem::path nwnxInstallDir;
SimpleIniConfig* config;

char hookReturnBuffer[MAX_BUFFER];

HMODULE g_Module;
volatile LONG g_InCrash;
PCRASH_DUMP_SECTION g_CrashDumpSectionView;

void (*SetLocalStringNextHook)() = nullptr;

/***************************************************************************
    Fake export function for detours
***************************************************************************/

extern "C" __declspec(dllexport) void dummy() { return; }

/***************************************************************************
    Hooking functions
***************************************************************************/

unsigned char* FindPattern(const unsigned char* pattern)
{
	int i;
	int patternLength  = (int)strlen((char*)pattern);
	unsigned char* ptr = (unsigned char*)0x400000;

	while (ptr < (unsigned char*)0x800000) {
		for (i = 0; i < patternLength && ptr[i] == pattern[i]; i++)
			;
		if (i == patternLength)
			return ptr;
		else
			ptr++;
	}

	return nullptr;
}

int NWNXGetInt(char* sPlugin, char* sFunction, char* sParam1, int nParam2)
{
	logger->Trace("call to NWNXGetInt(sPlugin=%s, sFunction=%s, sParam1=%s, nParam2=%d)", sPlugin,
	              sFunction, sParam1, nParam2);

	std::string_view pluginClass {sPlugin};

	// NWNX internal forwarding
	if (pluginClass == "NWNX") {
		std::string function(sFunction);
		if (function == "INSTALLED")
			return 1;
		else if (function == "GET PLUGIN COUNT")
			return (int)(plugins.size() + cplugins.size());
		else if (function == "FIND PLUGIN CLASS") {
			const std::string classname = sParam1;
			int i                       = 0;
			for (auto& p : cplugins) {
				if (p.first == classname) {
					return i;
				}
				i++;
			}
			for (auto& p : plugins) {
				if (p.first == classname) {
					return i;
				}
				i++;
			}
			return -1;
		} else {
			logger->Err("NWNXGetInt: Unsupported NWNX function '%s'.", sFunction);
			return 0;
		}
	}

	// CPlugin forwarding
	auto cpluginIt = cplugins.find(sPlugin);
	if (cpluginIt != cplugins.end()) {
		return cpluginIt->second->GetInt(sFunction, sParam1, nParam2);
	}

	// Plugin class forwarding
	auto pluginIt = plugins.find(sPlugin);
	if (pluginIt != plugins.end()) {
		return pluginIt->second->GetInt(sFunction, sParam1, nParam2);
	}

	logger->Warn("NWNXGetInt: Function class '%s' is not provided by any plugin. Check plugin_list "
	             "in nwnx.ini.",
	             sPlugin);
	return 0;
}

void NWNXSetInt(char* sPlugin, char* sFunction, char* sParam1, int nParam2, int nValue)
{
	logger->Trace("call to NWNXSetInt(sPlugin=%s, sFunction=%s, sParam1=%s, nParam2=%d, nValue=%d)",
	              sPlugin, sFunction, sParam1, nParam2, nValue);

	// CPlugin forwarding
	auto cpluginIt = cplugins.find(sPlugin);
	if (cpluginIt != cplugins.end()) {
		cpluginIt->second->SetInt(sFunction, sParam1, nParam2, nValue);
		return;
	}

	// Plugin class forwarding
	auto pluginIt = plugins.find(sPlugin);
	if (pluginIt != plugins.end()) {
		pluginIt->second->SetInt(sFunction, sParam1, nParam2, nValue);
		return;
	}

	logger->Warn("NWNXSetInt: Function class '%s' is not provided by any plugin. Check plugin_list "
	             "in nwnx.ini.",
	             sPlugin);
}

float NWNXGetFloat(char* sPlugin, char* sFunction, char* sParam1, int nParam2)
{
	logger->Trace("call to NWNXGetFloat(sPlugin=%s, sFunction=%s, sParam1=%s, nParam2=%d)", sPlugin,
	              sFunction, sParam1, nParam2);

	// CPlugin forwarding
	auto cpluginIt = cplugins.find(sPlugin);
	if (cpluginIt != cplugins.end()) {
		return cpluginIt->second->GetFloat(sFunction, sParam1, nParam2);
	}

	// Plugin class forwarding
	auto pluginIt = plugins.find(sPlugin);
	if (pluginIt != plugins.end()) {
		return pluginIt->second->GetFloat(sFunction, sParam1, nParam2);
	}

	logger->Warn("NWNXGetFloat: Function class '%s' is not provided by any plugin. Check "
	             "plugin_list in nwnx.ini.",
	             sPlugin);
	return 0.0;
}

void NWNXSetFloat(char* sPlugin, char* sFunction, char* sParam1, int nParam2, float fValue)
{
	logger->Trace(
	    "call to NWNXSetFloat(sPlugin=%s, sFunction=%s, sParam1=%s, nParam2=%d, fValue=%f)",
	    sPlugin, sFunction, sParam1, nParam2, fValue);

	// CPlugin forwarding
	auto cpluginIt = cplugins.find(sPlugin);
	if (cpluginIt != cplugins.end()) {
		cpluginIt->second->SetFloat(sFunction, sParam1, nParam2, fValue);
		return;
	}

	// Plugin class forwarding
	auto pluginIt = plugins.find(sPlugin);
	if (pluginIt != plugins.end()) {
		pluginIt->second->SetFloat(sFunction, sParam1, nParam2, fValue);
		return;
	}

	logger->Warn("NWNXSetFloat: Function class '%s' is not provided by any plugin. Check "
	             "plugin_list in nwnx.ini.",
	             sPlugin);
}

char* NWNXGetString(char* sPlugin, char* sFunction, char* sParam1, int nParam2)
{
	logger->Trace("call to NWNXGetString(sPlugin=%s, sFunction=%s, sParam1=%s, nParam2=%d)",
	              sPlugin, sFunction, sParam1, nParam2);

	std::string_view pluginClass {sPlugin};

	// NWNX internal forwarding
	if (pluginClass == "NWNX") {
		std::string function(sFunction);
		if (function == "GET PLUGIN CLASS") {
			int index = nParam2;
			if (index < 0)
				return nullptr;

			if (index < cplugins.size()) {
				auto p = cplugins.begin();
				std::advance(p, index);
				strncpy_s(hookReturnBuffer, MAX_BUFFER, p->first.c_str(), p->first.size());
				return hookReturnBuffer;
			}

			index -= cplugins.size();
			if (index < plugins.size()) {
				auto p = plugins.begin();
				std::advance(p, index);
				strncpy_s(hookReturnBuffer, MAX_BUFFER, p->first.c_str(), p->first.size());
				return hookReturnBuffer;
			}
			return nullptr;
		} else if (function == "GET PLUGIN INFO") {
			int index = nParam2;
			if (index < 0)
				return nullptr;

			if (index < cplugins.size()) {
				auto p = cplugins.begin();
				std::advance(p, index);
				auto info = p->second->GetInfo();
				strncpy_s(hookReturnBuffer, MAX_BUFFER, info.c_str(), info.size());
				return hookReturnBuffer;
			}

			index -= cplugins.size();
			if (index < plugins.size()) {
				auto p = plugins.begin();
				std::advance(p, index);
				return p->second->GetString((char*)"GET DESCRIPTION", sParam1, nParam2);
			}
			return nullptr;
		} else if (function == "GET PLUGIN VERSION") {
			int index = nParam2;
			if (index < 0)
				return nullptr;

			if (index < cplugins.size()) {
				auto p = cplugins.begin();
				std::advance(p, index);
				auto version = p->second->GetVersion();
				strncpy_s(hookReturnBuffer, MAX_BUFFER, version.c_str(), version.size());
				return hookReturnBuffer;
			}

			index -= cplugins.size();
			if (index < plugins.size()) {
				auto p = plugins.begin();
				std::advance(p, index);
				return p->second->GetString((char*)"GET VERSION", sParam1, nParam2);
			}
			return nullptr;
		} else {
			logger->Err("NWNXGetString: Unsupported NWNX function '%s'.", sFunction);
			return nullptr;
		}
	}

	// CPlugin forwarding
	auto cpluginIt = cplugins.find(sPlugin);
	if (cpluginIt != cplugins.end()) {
		hookReturnBuffer[0] = '\0';
		cpluginIt->second->GetString(sFunction, sParam1, nParam2, hookReturnBuffer, MAX_BUFFER);
		return hookReturnBuffer;
	}

	// Plugin class forwarding
	auto pluginIt = plugins.find(sPlugin);
	if (pluginIt != plugins.end()) {
		return pluginIt->second->GetString(sFunction, sParam1, nParam2);
	}

	logger->Warn("NWNXGetString: Function class '%s' is not provided by any plugin. Check "
	             "plugin_list in nwnx.ini.",
	             sPlugin);
	return nullptr;
}

void NWNXSetString(char* sPlugin, char* sFunction, char* sParam1, int nParam2, char* sValue)
{
	logger->Trace(
	    "call to NWNXSetString(sPlugin=%s, sFunction=%s, sParam1=%s, nParam2=%d, sValue=%s)",
	    sPlugin, sFunction, sParam1, nParam2, sValue);

	// CPlugin forwarding
	auto cpluginIt = cplugins.find(sPlugin);
	if (cpluginIt != cplugins.end()) {
		cpluginIt->second->SetString(sFunction, sParam1, nParam2, sValue);
		return;
	}

	// Plugin class forwarding
	auto pluginIt = plugins.find(sPlugin);
	if (pluginIt != plugins.end()) {
		pluginIt->second->SetString(sFunction, sParam1, nParam2, sValue);
		return;
	}

	logger->Warn("NWNXSetString: Function class '%s' is not provided by any plugin. Check "
	             "plugin_list in nwnx.ini.",
	             sPlugin);
}

void LegacyPluginPayLoad(char* gameObject, char* nwnName, char* nwnValue)
{
	// NWNX!TIME!START!PARAM"
	logger->Trace("Payload called");

	if (!nwnName || !nwnValue)
		return;

	std::string_view name {nwnName};
	if (!name.starts_with("NWNX!")) // not for us
		return;

	std::string fClass, function;

	auto sep = name.find('!', 5);
	if (sep != std::string::npos) {
		fClass = name.substr(5, sep - 5);
		logger->Trace("fClass=%s", fClass.c_str());
	} else {
		logger->Info("Function class not specified.");
		return;
	}

	if (sep + 1 < name.size()) {
		function = name.substr(sep + 1);
		logger->Trace("function=%s", function.c_str());
	} else
		logger->Info("Function not specified.");

	logger->Trace("Function class '%s', function '%s'.", fClass.c_str(), function.c_str());

	// try to call the plugin
	auto it = legacyplugins.find(fClass);
	if (it != legacyplugins.end()) {
		// plugin found, handle the request
		LegacyPlugin* pPlugin = it->second;
		size_t valueLength    = strlen(nwnValue);
		const char* pRes      = pPlugin->DoRequest(gameObject, (char*)function.c_str(), nwnValue);
		if (pRes) {
			// copy result into nwn variable value while respecting the maximum size
			size_t resultLength = strlen(pRes);
			if (valueLength < resultLength) {
				strncpy_s(nwnValue, valueLength, pRes, valueLength);
				*(nwnValue + valueLength) = 0x0;
			} else {
				strncpy_s(nwnValue, resultLength, pRes, resultLength);
				*(nwnValue + resultLength) = 0x0;
			}
		}
	} else {
		// if (queryFunctions(fClass, function, nwnValue) == false)
		//{
		logger->Info(
		    "Function class '%s' is not provided by any plugin. Check plugin_list in nwnx.ini.",
		    fClass.c_str());
		*nwnValue = 0x0; //??
		//}
	}
}

void __declspec(naked) SetLocalStringHookProc()
{
	__asm {

		push ecx // save register contents
		push edx
		push ebx
		push esi
		push edi
		push ebp

		mov eax, dword ptr ss:[esp+0x20] // variable value (param 3)
		mov eax, [eax]
		push eax
		mov eax, dword ptr ss:[esp+0x20] // variable name (param 2)
		mov eax, [eax]
		push eax
		lea eax, dword ptr ss:[ecx-0x190] // game object (param 1) ??
		push eax

		call LegacyPluginPayLoad
		add esp, 0xC

		pop ebp // restore register contents
		pop edi
		pop esi
		pop ebx
		pop edx
		pop ecx

		mov eax, dword ptr ss:[esp+0x14] // arg 5
		push eax
		mov eax, dword ptr ss:[esp+0x14] // arg 4
		push eax
		mov eax, dword ptr ss:[esp+0x14] // arg 3
		push eax
		mov eax, dword ptr ss:[esp+0x14] // arg 2
		push eax
		mov eax, dword ptr ss:[esp+0x14] // arg 1
		push eax

		call SetLocalStringNextHook // call original function

		add esp, 0xC
        retn 8
	}
}

DWORD FindHook()
{
	char* ptr = (char*)0x400000;
	while (ptr < (char*)0x700000) {
		if ((ptr[0] == (char)0x8B) && (ptr[1] == (char)0x44) && (ptr[2] == (char)0x24)
		    && (ptr[4] == (char)0x6A) && (ptr[5] == (char)0x03) && (ptr[6] == (char)0x50)
		    && (ptr[7] == (char)0xE8) && (ptr[12] == (char)0x8B))
			return (DWORD)ptr;
		else
			ptr++;
	}
	return NULL;
}

/***************************************************************************
    Initialization
***************************************************************************/

// init() is called by NWNXWinMain, which is the first that
// gets executed in the server proces.

void init()
{
	// Add nwn2server-dll to DLL search path
	if (!SetDefaultDllDirectories(LOAD_LIBRARY_SEARCH_DEFAULT_DIRS)) {
		auto errInfo = GetLastErrorInfo();
		logger->Err("SetDefaultDllDirectories failed. Error %d: %s", errInfo.first, errInfo.second);
	}
	auto dllDirPath = nwnxUserDir / "nwn2server-dll";
	if (std::filesystem::exists(dllDirPath)) {
		if (!AddDllDirectory(dllDirPath.c_str())) {
			auto errInfo = GetLastErrorInfo();
			logger->Err("AddDllDirectoryA failed. Error %d: %s", errInfo.first, errInfo.second);
		}
	}
	dllDirPath = nwnxInstallDir / "nwn2server-dll";
	if (std::filesystem::exists(dllDirPath)) {
		if (!AddDllDirectory(dllDirPath.c_str())) {
			auto errInfo = GetLastErrorInfo();
			logger->Err("AddDllDirectoryA failed. Error %d: %s", errInfo.first, errInfo.second);
		}
	}

	const auto logfile = nwnxUserDir / "nwnx.txt";
	logger             = std::make_unique<LogNWNX>(logfile.string());
	logger->Info("NWNX4 " NWNX_VERSION_INFO);
	logger->Info("(c) 2008 by Ingmar Stieger (Papillon)");
	logger->Info("Contribute to the project at https://github.com/nwn2dev/nwnx4 !");

	logger->Info("NWNX4 user directory: %ls", nwnxUserDir.c_str());
	logger->Info("NWNX4 install directory: %ls", nwnxInstallDir.c_str());

	// signal controller that we are ready
	if (!SetEvent(shmem->ready_event)) {
		auto errInfo = GetLastErrorInfo();
		logger->Err("SetEvent failed. Error %d: %s", errInfo.first, errInfo.second);
		return;
	}
	CloseHandle(shmem->ready_event);

	// open ini file
	auto inifile = nwnxUserDir / "nwnx.ini";
	logger->Info("NWNX4 Config: %ls", inifile.c_str());
	config = new SimpleIniConfig(inifile.string());
	logger->Configure(config);

	bool missingFunction = false;
	unsigned char* hookAt;
	hookAt = FindPattern(SET_NWNX_GETINT);
	if (hookAt) {
		logger->Debug("Connecting NWNXGetInt (0x%x)...", hookAt);
		int (*pt2NWNXSetFunctionPointer)(int (*pt2Function)(char*, char*, char*, int))
		    = (int (*)(int (*)(char*, char*, char*, int)))hookAt;
		pt2NWNXSetFunctionPointer(&NWNXGetInt);
	} else {
		logger->Err("NWNXGetInt NOT FOUND!");
		missingFunction = true;
	}

	hookAt = FindPattern(SET_NWNX_GETFLOAT);
	if (hookAt) {
		logger->Debug("Connecting NWNXGetFloat (0x%x)...", hookAt);
		float (*pt2NWNXSetFunctionPointer)(float (*pt2Function)(char*, char*, char*, int))
		    = (float (*)(float (*)(char*, char*, char*, int)))hookAt;
		pt2NWNXSetFunctionPointer(&NWNXGetFloat);
	} else {
		logger->Err("NWNXGetFloat NOT FOUND!");
		missingFunction = true;
	}

	hookAt = FindPattern(SET_NWNX_GETSTRING);
	if (hookAt) {
		logger->Debug("Connecting NWNXGetString (0x%x)...", hookAt);
		char* (*pt2NWNXSetFunctionPointer)(char* (*pt2Function)(char*, char*, char*, int))
		    = (char* (*)(char* (*)(char*, char*, char*, int)))hookAt;
		pt2NWNXSetFunctionPointer(&NWNXGetString);
	} else {
		logger->Err("NWNXGetString NOT FOUND!");
		missingFunction = true;
	}

	hookAt = FindPattern(SET_NWNX_SETINT);
	if (hookAt) {
		logger->Debug("Connecting NWNXSetInt (0x%x)...", hookAt);
		void (*pt2NWNXSetFunctionPointer)(void (*pt2Function)(char*, char*, char*, int, int))
		    = (void (*)(void (*)(char*, char*, char*, int, int)))hookAt;
		pt2NWNXSetFunctionPointer(&NWNXSetInt);
	} else {
		logger->Err("NWNXSetInt NOT FOUND!");
		missingFunction = true;
	}

	hookAt = FindPattern(SET_NWNX_SETFLOAT);
	if (hookAt) {
		logger->Debug("Connecting NWNXSetFloat (0x%x)...", hookAt);
		void (*pt2NWNXSetFunctionPointer)(void (*pt2Function)(char*, char*, char*, int, float))
		    = (void (*)(void (*)(char*, char*, char*, int, float)))hookAt;
		pt2NWNXSetFunctionPointer(&NWNXSetFloat);
	} else {
		logger->Err("NWNXSetFloat NOT FOUND!");
		missingFunction = true;
	}

	hookAt = FindPattern(SET_NWNX_SETSTRING);
	if (hookAt) {
		logger->Debug("Connecting NWNXSetString (0x%x)...", hookAt);
		void (*pt2NWNXSetFunctionPointer)(void (*pt2Function)(char*, char*, char*, int, char*))
		    = (void (*)(void (*)(char*, char*, char*, int, char*)))hookAt;
		pt2NWNXSetFunctionPointer(&NWNXSetString);
	} else {
		logger->Err("NWNXSetString NOT FOUND!");
		missingFunction = true;
	}

	// Detours hook
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DWORD oldHookAt                  = FindHook();
	*(DWORD*)&SetLocalStringNextHook = oldHookAt;
	DetourAttach(&(PVOID&)SetLocalStringNextHook, SetLocalStringHookProc);
	DetourTransactionCommit();

	if (oldHookAt)
		logger->Debug("SetLocalString hooked at 0x%x", oldHookAt);
	else {
		logger->Err("SetLocalString NOT FOUND!");
		missingFunction = true;
	}

	if (!SCORCOHook(logger.get()))
		missingFunction = true;

	if (missingFunction)
		logger->Err(
		    "!! One or more functions could not be hooked.\n"
		    "!! Please check the system requirements (NWN2 1.06 or later) for this \n"
		    "!! version of NWNX4, and come to our forums to get help or eventual updates.\n");

	loadPlugins();

	// Set SQL plugin for forwarding SCORCO functions
	// This is necessary for SQL plugins that are still using the old Plugin class ABI
	auto dbPluginIt = plugins.find("SQL");
	if (dbPluginIt != plugins.cend()) {
		SCORCOSetLegacyDBPlugin(dynamic_cast<DBPlugin*>(dbPluginIt->second));
	}

	// Suppress general protection fault error box
	bool noGPFaultErrorBox;
	config->Read("noGPFaultErrorBox", &noGPFaultErrorBox, true);
	if (noGPFaultErrorBox) {
		DWORD dwMode = SetErrorMode(SEM_NOGPFAULTERRORBOX);
		SetErrorMode(dwMode | SEM_NOGPFAULTERRORBOX);
		logger->Info("General protection fault error dialog disabled.");
	}

	logger->Info("NWNX4 activated.");
}

/***************************************************************************
    Plugin handling
***************************************************************************/

// Used to parse comma-separated plugin list in nwnx.ini
static std::vector<std::filesystem::path> ParsePluginsList(const std::string& list)
{
	std::vector<std::filesystem::path> ret;

	bool inValue = false; // true when the state machine is parsing a value
	size_t start = 0;
	size_t end   = 0; // points to the last non-whitespace character of a value
	for (size_t i = 0; i < list.size(); i++) {
		// Skip all whitespaces
		if (list[i] == ' ' || list[i] == '\t') {
			continue;
		}

		if (!inValue) {
			// Start parsing a value
			start   = i;
			end     = start;
			inValue = true;

			continue;
		}

		if (list[i] == ',') {
			// Insert new element in array, stop parsing the value
			ret.push_back(list.substr(start, end - start));
			inValue = false;
			start   = i + 1;
			end     = start;

			continue;
		}

		// Advance end marker (non whitespace character)
		end = i + 1;
	}

	if (inValue) {
		ret.push_back(list.substr(start, end - start));
	}

	return ret;
}
std::unordered_map<std::string, std::string> ParseServerCommandLine()
{

	std::unordered_map<std::string, std::string> ret;
	const wchar_t* argName = nullptr;
	std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;

	int argc;
	auto argv = CommandLineToArgvW(GetCommandLineW(), &argc);
	for (size_t i = 1; i < argc; i++) {
		if (i % 2 == 1) {
			if (argv[i][0] == L'-')
				argName = argv[i] + 1; // skip dash
			else
				argName = nullptr;
		} else {
			if (argName != nullptr)
				ret[conv.to_bytes(argName)] = conv.to_bytes(argv[i]);
		}
	}

	return ret;
}

// Called upon initialization (see above).
//
void loadPlugins()
{
	logger->Info("Loading plugins...");

	std::vector<std::filesystem::path> pluginList;
	std::string pluginListStr;
	if (!config->Read("plugin_list", &pluginListStr)) {
		// plugin_list not found in config
		logger->Warn("No 'plugin_list' found in nwnx.ini. All plugins in the nwnx4 directory will "
		             "be loaded. This behavior is not recommended !");

		std::unordered_set<std::string> addedPlugins;

		// Add all plugins in nwnx4 dir to pluginList
		for (const auto& dir : {nwnxUserDir, nwnxInstallDir}) {
			for (auto& file : std::filesystem::directory_iterator(dir)) {
				auto filename = file.path().filename().string();
				if (filename.length() > 4 && filename.substr(0, 3) == "xp_"
				    && filename.substr(filename.size() - 4) == ".dll") {
					if (!addedPlugins.contains(filename)) {
						pluginList.push_back(file);
						addedPlugins.insert(filename);
					}
				}
			}
		}
	} else
		pluginList = ParsePluginsList(pluginListStr);

	std::string nwn2InstallDir {std::filesystem::current_path().string()};
	std::string nwnxUserDirStr {nwnxUserDir.string()};
	std::string nwnxInstallDirStr {nwnxInstallDir.string()};

	// Get nwn2 home directory (for loading CPlugins)
	auto serverArgs = ParseServerCommandLine();
	std::string nwn2HomeDir;
	if (serverArgs.contains("home")) {
		auto& dir   = serverArgs["home"];
		nwn2HomeDir = std::string(dir.begin(), dir.end());
	} else {
		wchar_t path[MAX_PATH];
		SHGetFolderPathW(nullptr, CSIDL_PERSONAL, nullptr, SHGFP_TYPE_CURRENT, path);

		auto dir    = std::filesystem::path(path) / "Neverwinter Nights 2";
		nwn2HomeDir = dir.string();
	}

	// Get loaded module path
	std::string nwn2ModulePath;
	if (serverArgs.contains("module"))
		nwn2ModulePath
		    = (std::filesystem::path(nwn2HomeDir) / "modules" / (serverArgs["module"] + ".mod"))
		          .string();
	else if (serverArgs.contains("moduledir"))
		nwn2ModulePath
		    = (std::filesystem::path(nwn2HomeDir) / "modules" / serverArgs["moduledir"]).string();
	const char* nwn2ModulePathCStr = nwn2ModulePath.size() > 0 ? nwn2ModulePath.c_str() : nullptr;

	// Start loading plugins
	for (auto& pluginPath : pluginList) {
		if (++pluginPath.begin() == pluginPath.end()) {
			// The plugin is provided as name only (no slashes), and must be
			// stored in the ./plugins/ folder
			pluginPath += ".dll";
			pluginPath = "plugins" / pluginPath;
		}
		// If relative path, prepend the nwnx4 dir
		if (pluginPath.is_relative()) {

			auto pluginFullPath = nwnxUserDir / pluginPath;
			if (std::filesystem::exists(pluginFullPath))
				pluginPath = pluginFullPath;
			else
				pluginPath = nwnxInstallDir / pluginPath;
		}

		auto pluginPathStr = pluginPath.string();
		auto pluginName    = pluginPath.stem().string();
		logger->Debug("Loading plugin %s (%s)", pluginName.c_str(), pluginPathStr.c_str());

		// Load DLL file
		HINSTANCE hDLL = LoadLibraryA(pluginPathStr.c_str());
		if (hDLL == nullptr) {
			// Failed to load the plugin
			auto errInfo = GetLastErrorInfo();
			logger->Err("Cannot load plugin dll %s: Error %d: %s", pluginName.c_str(),
			            errInfo.first, errInfo.second);
			continue;
		}

		// Search for CPlugins, a C ABI compatible plugin
		auto cpluginABIVersion
		    = reinterpret_cast<uint32_t*>(GetProcAddress(hDLL, "nwnxcplugin_abi_version"));
		if (cpluginABIVersion != nullptr) {
			try {
				CPlugin::InitInfo initInfo {
				    .dll_path          = pluginPathStr.c_str(),
				    .nwnx_user_path    = nwnxUserDirStr.c_str(),
				    .nwn2_install_path = nwn2InstallDir.c_str(),
				    .nwn2_home_path    = nwn2HomeDir.c_str(),
				    .nwn2_module_path  = nwn2ModulePathCStr,
				    .nwnx_install_path = nwnxInstallDirStr.c_str(),
				};

				// Instantiate & initialize CPlugin
				auto cplugin = std::make_unique<CPlugin>(hDLL, initInfo);

				if (cplugin->GetIsLoaded()) {
					auto id = cplugin->GetID();
					if (cplugins.contains(id)) {
						logger->Warn("Skipping C plugin v%d '%s': Class %s already registered by "
						             "another plugin.",
						             *cpluginABIVersion, pluginName.c_str(), id.c_str());
					} else {
						logger->Info("Loading C plugin v%d '%s': Successfully registered as ID: %s",
						             *cpluginABIVersion, pluginName.c_str(), id.c_str());
						logger->Info("    Version: %s", cplugin->GetVersion().c_str());
						logger->Info("    Info: %s", cplugin->GetInfo().c_str());
						cplugins[id] = std::move(cplugin);
					}
				} else {
					logger->Err("Failed to initialize C plugin v%d '%s'", *cpluginABIVersion,
					            pluginName.c_str());
				}
			} catch (const std::exception& exception) {
				logger->Err("Exception thrown while loading C plugin v%d '%s': %s",
				            *cpluginABIVersion, pluginName.c_str(), exception.what());
			}

			continue;
		}

		// Search for V2 plugin function
		void* pGetPluginPointer = GetProcAddress(hDLL, "GetPluginPointerV2");
		if (pGetPluginPointer != nullptr) {
			// Load
			typedef Plugin*(WINAPI * GetPluginPointer)();

			Plugin* pPlugin = ((GetPluginPointer)pGetPluginPointer)();
			if (pPlugin != nullptr) {
				auto nwnxUserDirStr = nwnxUserDir.string();
				if (!pPlugin->Init(nwnxUserDirStr.data()))
					logger->Err("Loading plugin %s: Error during plugin initialization (init() "
					            "returned false).",
					            pluginName.c_str());
				else {
					char fClass[128];
					pPlugin->GetFunctionClass(fClass);
					if (plugins.find(fClass) == plugins.end()) {
						logger->Info("Loading plugin %s: Successfully registered as class: %s",
						             pluginName.c_str(), fClass);
						plugins[fClass] = pPlugin;
					} else {
						logger->Warn(
						    "Skipping plugin %s: Class %s already registered by another plugin.",
						    pluginName.c_str(), fClass);
						FreeLibrary(hDLL);
					}
				}
			} else
				logger->Err("Loading plugin %s: Error while instancing plugin "
				            "(GetPluginPointerV2() returned null).",
				            pluginName.c_str());

			continue;
		}

		// Search for legacy plugin function
		pGetPluginPointer = GetProcAddress(hDLL, "GetPluginPointer");
		if (pGetPluginPointer != nullptr) {
			typedef LegacyPlugin*(WINAPI * GetLegacyPluginPointer)();

			LegacyPlugin* pPlugin = ((GetLegacyPluginPointer)pGetPluginPointer)();
			if (pPlugin != nullptr) {
				auto nwnxUserDirStr = nwnxUserDir.string();
				if (!pPlugin->Init(nwnxUserDirStr.data()))
					logger->Err("Loading plugin %s: [LEGACY] Error during plugin initialization "
					            "(init() returned false).",
					            pluginName.c_str());
				else {
					char fClass[128];
					pPlugin->GetFunctionClass(fClass);
					if (plugins.find(fClass) == plugins.end()) {
						logger->Info(
						    "Loading plugin %s: [LEGACY] Successfully registered as class: %s",
						    pluginName.c_str(), fClass);
						legacyplugins[fClass] = pPlugin;
					} else {
						logger->Warn("Skipping plugin %s: [LEGACY] Class %s already registered by "
						             "another plugin.",
						             pluginName.c_str(), fClass);
						FreeLibrary(hDLL);
					}
				}
			} else
				logger->Err("Loading plugin %s: [LEGACY] Error while instancing plugin "
				            "(GetPluginPointer() returned null).",
				            pluginName.c_str());

			continue;
		}

		// No compatible ABI found
		logger->Err("Loading plugin %s: Error. Could not find a compatible ABI.",
		            pluginName.c_str());
	}
}

/***************************************************************************
    Redirected EXE Entry point
***************************************************************************/

int WINAPI NWNXWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	ULONG cbData = 0;

	GUID my_guid = {/* d9ab8a40-f4cc-11d1-b6d7-006097b010e3 */
	                0xd9ab8a40,
	                0xf4cc,
	                0x11d1,
	                {0xb6, 0xd7, 0x00, 0x60, 0x97, 0xb0, 0x10, 0xe3}};

	shmem = nullptr;

	for (HINSTANCE hinst = nullptr; (hinst = DetourEnumerateModules(hinst)) != nullptr;) {
		shmem = (SHARED_MEMORY*)DetourFindPayload(hinst, my_guid, &cbData);

		if (shmem) {
			//
			// Start the crash dump client first off, as the controller will try and
			// connect to it after we acknowledge booting.
			//

			RegisterCrashDumpHandler();

			//
			// Initialize plugins and load configuration data.
			//

			nwnxUserDir    = std::filesystem::path {shmem->nwnx_user_dir};
			nwnxInstallDir = std::filesystem::path {shmem->nwnx_install_dir};

			// Initialize hook.
			init();
			break;
		}
	}

	/*
	 * If we didn't connect to the controller then bail out here.
	 */
	if (!shmem) {
		// DebugPrint( "NWNXWinMain(): Failed to connect to controller!\n" );
		ExitProcess(ERROR_DEVICE_NOT_CONNECTED);
	}

	/*
	 * Call the original entrypoint of the process now that we have done our
	 * preprocessing.
	 */

	return TrueWinMain(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
}

// Called by Windows when the DLL gets loaded or unloaded
// It just starts the IPC server.
//
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
		// We are doing a lazy initialization here to increase the robustness of the
		// hooking DLL because it is not performed while the loader lock is held.
		// We hook the app entry point.
		TrueWinMain = (int(WINAPI*)(HINSTANCE, HINSTANCE, LPSTR, int))DetourGetEntryPoint(nullptr);
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID&)TrueWinMain, NWNXWinMain);
		DetourTransactionCommit();
	} else if (ul_reason_for_call == DLL_PROCESS_DETACH) {
		//
		// Doing complicated things from DLL_PROCESS_ATTACH is extremely bad.
		// Let's not.  Don't want to risk deadlocking the process during an
		// otherwise clean shutdown.
		//

		//		wxLogMessage(wxT("NWNX4 shutting down."));
	}
	return TRUE;
}
