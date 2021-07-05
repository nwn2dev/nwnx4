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
#include <filesystem>

/*
 * Globals.
 */

SHARED_MEMORY *shmem;

PluginHashMap plugins;
LegacyPluginHashMap legacyplugins;

LogNWNX* logger;
std::string* nwnxHome;
SimpleIniConfig* config;

char returnBuffer[MAX_BUFFER];


HMODULE             g_Module;
volatile LONG       g_InCrash;
PCRASH_DUMP_SECTION g_CrashDumpSectionView;

/***************************************************************************
    Fake export function for detours
***************************************************************************/

extern "C" __declspec(dllexport)
void dummy()
{
	return ;
}

/***************************************************************************
    Hooking functions
***************************************************************************/

unsigned char* FindPattern(const unsigned char* pattern)
{
	int i;
	int patternLength = (int)strlen((char*)pattern);
	unsigned char* ptr = (unsigned char*) 0x400000;

	while (ptr < (unsigned char*) 0x800000)
	{
		for (i = 0; i < patternLength && ptr[i] == pattern[i]; i++);
        if (i == patternLength)
			return ptr;
		else
			ptr++;
	}

	return nullptr;
}

int NWNXGetInt(char* sPlugin, char* sFunction, char* sParam1, int nParam2)
{
	logger->Debug("call to NWNXGetInt(sPlugin=%s, sFunction=%s, sParam1=%s, nParam2=%d)", sPlugin, sFunction, sParam1, nParam2);

	// try to call the plugin
	PluginHashMap::iterator it = plugins.find(sPlugin);
	if (it != plugins.end())
	{
		// plugin found, handle the request
		Plugin* pPlugin = it->second;
		return pPlugin->GetInt(sFunction, sParam1, nParam2);
	}
	else
	{
		std::string plugin(sPlugin);
		std::string function(sFunction);
		if (plugin == "NWNX")
		{
			if (function == "INSTALLED")
				return 1;
			else if (function == "GET PLUGIN COUNT")
				return (int)plugins.size();
		}
		else
			logger->Info("* NWNXGetInt: Function class '%s' not provided by any plugin. Check your installation.", plugin.c_str());
	}
	return 0;
}


void NWNXSetInt(char* sPlugin, char* sFunction, char* sParam1, int nParam2, int nValue)
{
	logger->Debug("call to NWNXSetInt(sPlugin=%s, sFunction=%s, sParam1=%s, nParam2=%d, nValue=%d)",
		sPlugin, sFunction, sParam1, nParam2, nValue);

	// try to call the plugin
	PluginHashMap::iterator it = plugins.find(sPlugin);
	if (it != plugins.end())
	{
		// plugin found, handle the request
		Plugin* pPlugin = it->second;
		pPlugin->SetInt(sFunction, sParam1, nParam2, nValue);
	}
	else
	{
		std::string plugin(sPlugin);
		std::string function(sFunction);
		logger->Info("* NWNXSetInt: Function class '%s' not provided by any plugin. Check your installation.", plugin.c_str());
	}
}

float NWNXGetFloat(char* sPlugin, char* sFunction, char* sParam1, int nParam2)
{
	logger->Debug("call to NWNXGetFloat(sPlugin=%s, sFunction=%s, sParam1=%s, nParam2=%d)",
		sPlugin, sFunction, sParam1, nParam2);

	// try to call the plugin
	PluginHashMap::iterator it = plugins.find(sPlugin);
	if (it != plugins.end())
	{
		// plugin found, handle the request
		Plugin* pPlugin = it->second;
		return pPlugin->GetFloat(sFunction, sParam1, nParam2);
	}
	else
	{
		std::string plugin(sPlugin);
		std::string function(sFunction);
		logger->Info("* NWNXGetFloat: Function class '%s' not provided by any plugin. Check your installation.", plugin.c_str());
	}

	return 0.0;
}

void NWNXSetFloat(char* sPlugin, char* sFunction, char* sParam1, int nParam2, float fValue)
{
	logger->Debug("call to NWNXSetFloat(sPlugin=%s, sFunction=%s, sParam1=%s, nParam2=%d, fValue=%f)",
		sPlugin, sFunction, sParam1, nParam2, fValue);

	// try to call the plugin
	PluginHashMap::iterator it = plugins.find(sPlugin);
	if (it != plugins.end())
	{
		// plugin found, handle the request
		Plugin* pPlugin = it->second;
		pPlugin->SetFloat(sFunction, sParam1, nParam2, fValue);
	}
	else
	{
		std::string plugin(sPlugin);
		std::string function(sFunction);
		logger->Info("* NWNXSetFloat: Function class '%s' not provided by any plugin. Check your installation.", plugin.c_str());
	}
}

char* NWNXGetString(char* sPlugin, char* sFunction, char* sParam1, int nParam2)
{
	logger->Debug("call to NWNXGetString(sPlugin=%s, sFunction=%s, sParam1=%s, nParam2=%d)",
		sPlugin, sFunction, sParam1, nParam2);

	// try to call the plugin
	PluginHashMap::iterator it = plugins.find(sPlugin);
	if (it != plugins.end())
	{
		// plugin found, handle the request
		Plugin* pPlugin = it->second;
		return pPlugin->GetString(sFunction, sParam1, nParam2);
	}
	else
	{
		std::string plugin(sPlugin);
		std::string function(sFunction);
		if (plugin == "NWNX")
		{
			if (function == "GET PLUGIN CLASS")
			{
				int i = 0;
				PluginHashMap::iterator it;
				for(it = plugins.begin(); it != plugins.end(); ++it)
				{
					i++;
					if (i == nParam2)
					{
						// sprintf_s(returnBuffer, MAX_BUFFER, "%s", it->first);
						return returnBuffer;
					}
				}
				return nullptr;
			}
		}
		else
			logger->Info("* NWNXGetString: Function class '%s' not provided by any plugin. Check your installation.", plugin.c_str());
	}
	return nullptr;
}

void NWNXSetString(char* sPlugin, char* sFunction, char* sParam1, int nParam2, char* sValue)
{
	logger->Debug("call to NWNXSetString(sPlugin=%s, sFunction=%s, sParam1=%s, nParam2=%d, sValue=%s)",
		sPlugin, sFunction, sParam1, nParam2, sValue);

	// try to call the plugin
	PluginHashMap::iterator it = plugins.find(sPlugin);
	if (it != plugins.end())
	{
		// plugin found, handle the request
		Plugin* pPlugin = it->second;
		pPlugin->SetString(sFunction, sParam1, nParam2, sValue);
	}
	else
	{
		logger->Info("* NWNXSetString: Function class '%s' not provided by any plugin. Check your installation.",
			sPlugin);
	}
}


void PayLoad(char *gameObject, char* nwnName, char* nwnValue)
{
	// NWNX!TIME!START!PARAM"
	logger->Trace("* Payload called");

	if (!nwnName || !nwnValue)
		return;

	if (strncmp(nwnName, "NWNX!", 5) != 0) 	// not for us
		return;

	std::string fClass, function;
	std::string name(nwnName);


	// Reimplementation without wxwidget
	auto sep = name.find('!', 5);
	if(sep != std::string::npos)
	{
		fClass = name.substr(5, sep - 5);
		logger->Trace("* fClass=%s", fClass.c_str());
	}
	else
	{
		logger->Info("* Function class not specified.");
		return;
	}

	if(sep + 1 < name.size()){
		function = name.substr(sep + 1);
		logger->Trace("* function=%s", function.c_str());
	}
	else
		logger->Info("* Function not specified.");


	logger->Trace("* Function class '%s', function '%s'.", fClass.c_str(), function.c_str());



	// // maybe replace this code with old version..
	// wxStringTokenizer tkz(name, wxT("!"));
	// tkz.GetNextToken();

	// if (tkz.HasMoreTokens())
	// {
	// 	fClass = tkz.GetNextToken();
	// 	logger->Trace("* fClass=%s", fClass);
	// }
	// else
	// {
	// 	logger->Info("* Function class not specified.");
	// 	return;
	// }

	// if (tkz.HasMoreTokens())
	// {
	// 	function = tkz.GetNextToken();
	// 	logger->Trace("* function=%s", function);
	// 	while (tkz.HasMoreTokens())
	// 	{
	// 		function.append(wxT("!") + tkz.GetNextToken());
	// 	}
	// }
	// else
	// {
	// 	logger->Info("* Function not specified.");
	// }

	// logger->Trace("* Function class '%s', function '%s'.", fClass, function);

	// try to call the plugin
	auto it = legacyplugins.find(fClass);
	if (it != legacyplugins.end())
	{
		// plugin found, handle the request
		LegacyPlugin* pPlugin = it->second;
		size_t valueLength = strlen(nwnValue);
		const char* pRes = pPlugin->DoRequest(gameObject, (char*) function.c_str(), nwnValue);
		if (pRes)
		{
			// copy result into nwn variable value while respecting the maximum size
			size_t resultLength = strlen(pRes);
			if (valueLength < resultLength)
			{
				strncpy_s(nwnValue, valueLength, pRes, valueLength);
				*(nwnValue+valueLength) = 0x0;
			}
			else
			{
				strncpy_s(nwnValue, resultLength, pRes, resultLength);
				*(nwnValue+resultLength) = 0x0;
			}
		}
	}
	else
	{
		//if (queryFunctions(fClass, function, nwnValue) == false)
		//{
			logger->Info("* Function class '%s' not provided by any plugin. Check your installation.",
				fClass.c_str());
			*nwnValue = 0x0; //??
		//}
	}
}



void __declspec(naked) SetLocalStringHookProc()
{
	__asm {

		push ecx	  // save register contents
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

		call PayLoad
		add esp, 0xC

		pop ebp		// restore register contents
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
	char* ptr = (char*) 0x400000;
	while (ptr < (char*) 0x700000)
	{
		if ((ptr[0] == (char) 0x8B) &&
			(ptr[1] == (char) 0x44) &&
			(ptr[2] == (char) 0x24) &&
			(ptr[4] == (char) 0x6A) &&
			(ptr[5] == (char) 0x03) &&
			(ptr[6] == (char) 0x50) &&
			(ptr[7] == (char) 0xE8) &&
			(ptr[12] == (char) 0x8B)
			)
			return (DWORD) ptr;
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
    unsigned char* hookAt;

	auto logfile = std::string(*nwnxHome) + "\\nwnx.txt";
	logger = new LogNWNX(logfile);
	logger->Info("NWN Extender 4 V.1.1.0");
	logger->Info("(c) 2008 by Ingmar Stieger (Papillon)");
	logger->Info("visit us at http://www.nwnx.org");

    logger->Info("NWNX Home: %s", nwnxHome);

    // signal controller that we are ready
	if (!SetEvent(shmem->ready_event))
	{
		logger->Info("* SetEvent failed (%d)", GetLastError());
		return;
	}
	CloseHandle(shmem->ready_event);

	// open ini file
	auto inifile = *nwnxHome + "\\nwnx.ini";
	logger->Trace("Reading inifile %s", inifile.c_str());
	config = new SimpleIniConfig(inifile);
	logger->Configure(config);

	bool missingFunction = false;
	hookAt = FindPattern(SET_NWNX_GETINT);
	if (hookAt)
	{
		logger->Debug("Connecting NWNXGetInt (0x%x)...", hookAt);
		int (*pt2NWNXSetFunctionPointer)(int (*pt2Function)(char*, char*, char*, int)) = (int (*)(int (*)(char*, char*, char*, int))) hookAt;
		pt2NWNXSetFunctionPointer(&NWNXGetInt);
	}
	else
	{
		logger->Debug("NWNXGetInt NOT FOUND!");
		missingFunction = true;
	}

	hookAt = FindPattern(SET_NWNX_GETFLOAT);
	if (hookAt)
	{
		logger->Debug("Connecting NWNXGetFloat (0x%x)...", hookAt);
		float (*pt2NWNXSetFunctionPointer)(float (*pt2Function)(char*, char*, char*, int)) = (float (*)(float (*)(char*, char*, char*, int))) hookAt;
		pt2NWNXSetFunctionPointer(&NWNXGetFloat);
	}
	else
	{
		logger->Debug("NWNXGetFloat NOT FOUND!");
		missingFunction = true;
	}

	hookAt = FindPattern(SET_NWNX_GETSTRING);
	if (hookAt)
	{
		logger->Debug("Connecting NWNXGetString (0x%x)...", hookAt);
		char* (*pt2NWNXSetFunctionPointer)(char* (*pt2Function)(char*, char*, char*, int)) = (char* (*)(char* (*)(char*, char*, char*, int))) hookAt;
		pt2NWNXSetFunctionPointer(&NWNXGetString);
	}
	else
	{
		logger->Debug("NWNXGetString NOT FOUND!");
		missingFunction = true;
	}

	hookAt = FindPattern(SET_NWNX_SETINT);
	if (hookAt)
	{
		logger->Debug("Connecting NWNXSetInt (0x%x)...", hookAt);
		void (*pt2NWNXSetFunctionPointer)(void (*pt2Function)(char*, char*, char*, int, int)) = (void (*)(void (*)(char*, char*, char*, int, int))) hookAt;
		pt2NWNXSetFunctionPointer(&NWNXSetInt);
	}
	else
	{
		logger->Debug("NWNXSetInt NOT FOUND!");
		missingFunction = true;
	}

	hookAt = FindPattern(SET_NWNX_SETFLOAT);
	if (hookAt)
	{
		logger->Debug("Connecting NWNXSetFloat (0x%x)...", hookAt);
		void (*pt2NWNXSetFunctionPointer)(void (*pt2Function)(char*, char*, char*, int, float)) = (void (*)(void (*)(char*, char*, char*, int, float))) hookAt;
		pt2NWNXSetFunctionPointer(&NWNXSetFloat);
	}
	else
	{
		logger->Debug("NWNXSetFloat NOT FOUND!");
		missingFunction = true;
	}

	hookAt = FindPattern(SET_NWNX_SETSTRING);
	if (hookAt)
	{
		logger->Debug("Connecting NWNXSetString (0x%x)...", hookAt);
		void (*pt2NWNXSetFunctionPointer)(void (*pt2Function)(char*, char*, char*, int, char*)) = (void (*)(void (*)(char*, char*, char*, int, char*))) hookAt;
		pt2NWNXSetFunctionPointer(&NWNXSetString);
	}
	else
	{
		logger->Debug("NWNXSetString NOT FOUND!");
		missingFunction = true;
	}

	//Detours hook
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DWORD oldHookAt = FindHook();
	*(DWORD*)&SetLocalStringNextHook = oldHookAt;
	DetourAttach(&(PVOID&) SetLocalStringNextHook, SetLocalStringHookProc);
	DetourTransactionCommit();

	if (oldHookAt)
		logger->Debug("SetLocalString hooked at 0x%x", oldHookAt);
	else
	{
		logger->Debug("SetLocalString NOT FOUND!");
		missingFunction = true;
	}

	if (missingFunction)
		logger->Info(
			"!! One or more functions could not be hooked.\n"
			"!! Please check the system requirements (NWN2 1.06 or later) for this \n"
			"!! version of NWNX4, and come to our forums to get help or eventual updates.\n");

	logger->Info("* Loading plugins...");
	loadPlugins();

	// Suppress general protection fault error box
	bool noGPFaultErrorBox;
	config->Read("noGPFaultErrorBox", &noGPFaultErrorBox, true);
	if (noGPFaultErrorBox)
	{
		DWORD dwMode = SetErrorMode(SEM_NOGPFAULTERRORBOX);
		SetErrorMode(dwMode | SEM_NOGPFAULTERRORBOX);
		logger->Info("* General protection fault error dialog disabled.");
	}

	logger->Info("* NWNX4 activated.");
}

/***************************************************************************
    Plugin handling
***************************************************************************/

typedef Plugin* (WINAPI* GetPluginPointer)();
typedef LegacyPlugin* (WINAPI* GetLegacyPluginPointer)();

// Called upon initialization (see above).
// Loads all plugins based on a filename pattern
//
void loadPlugins()
{
	char fClass[128];
    std::string filename;
	std::string pattern("xp_*.dll");
	auto dir = std::filesystem::directory_entry(*nwnxHome);

    logger->Debug("Enumerating plugins in current directory");

    for(auto& file : std::filesystem::directory_iterator(dir.path())){
    	auto filename = file.path().filename().string();
    	// TODO: handle case sensitivity
    	if(filename.substr(0, 3) == "xp_" && filename.substr(filename.size() - 4) == ".dll"){
        	logger->Debug("Trying to load plugin %s", filename.c_str());

			HINSTANCE hDLL = LoadLibrary((wchar_t*)file.path().string().c_str()); // Warning: unconst cast
			if (hDLL == nullptr)
			{
				LPVOID lpMsgBuf;
				DWORD dw = GetLastError();
				FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM| FORMAT_MESSAGE_MAX_WIDTH_MASK ,
					nullptr, dw, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
					(LPTSTR) &lpMsgBuf,	0, nullptr);

				logger->Info("* Loading plugin %s: Error %d. %s", filename.c_str(), dw, lpMsgBuf);
			}
			else
			{
				// create an instance of plugin
				GetPluginPointer pGetPluginPointer = (GetPluginPointer)GetProcAddress(hDLL, "GetPluginPointerV2");
				if (pGetPluginPointer)
				{
					Plugin* pPlugin = pGetPluginPointer();
					if (pPlugin)
					{
						if (!pPlugin->Init(nwnxHome->data()))
							logger->Info("* Loading plugin %s: Error during plugin initialization.", filename.c_str());
						else
						{
							pPlugin->GetFunctionClass(fClass);
							if (plugins.find(fClass) == plugins.end())
							{
								logger->Info("* Loading plugin %s: Successfully registered as class: %s",
									filename.c_str(), fClass);
								plugins[fClass] = pPlugin;
							}
							else
							{
								logger->Info("* Skipping plugin %s: Class %s already registered by another plugin.",
									filename.c_str(), fClass);
								FreeLibrary(hDLL);
							}
						}
					}
					else
						logger->Info("* Loading plugin %s: Error while instancing plugin.", filename.c_str());
				}
				else
					{
					GetLegacyPluginPointer pGetPluginPointer = (GetLegacyPluginPointer)GetProcAddress(hDLL, "GetPluginPointer");
					if (pGetPluginPointer)
					{
						LegacyPlugin* pPlugin = pGetPluginPointer();
						if (pPlugin)
						{
							if (!pPlugin->Init(nwnxHome->data()))
								logger->Info("* Loading plugin %s: Error during plugin initialization.", filename.c_str());
							else
							{
								pPlugin->GetFunctionClass(fClass);
								if (plugins.find(fClass) == plugins.end())
								{
									logger->Info("* Loading plugin %s: Successfully registered as class: %s",
										filename.c_str(), fClass);
									legacyplugins[fClass] = pPlugin;
								}
								else
								{
									logger->Info("* Skipping plugin %s: Class %s already registered by another plugin.",
										filename.c_str(), fClass);
									FreeLibrary(hDLL);
								}
							}
						}
						else
							logger->Info("* Loading plugin %s: Error while instancing plugin.", filename.c_str());
					}
					else
						logger->Info("* Loading plugin %s: Error. Could not retrieve class object pointer.", filename.c_str());
				}
				//	logger->Info("* Loading plugin %s: Error. The plugin is not " 					"compatible with this version of NWNX."), filename);
			}
    	}
    }

  //   if (!dir.IsOpened())
  //   {
  //       // deal with the error here - wxDir would already log an error message
  //       // explaining the exact reason of the failure
  //       return;
  //   }

  //   bool cont = dir.GetFirst(&filename, pattern, wxDIR_FILES);
  //   while (cont)
  //   {
  //       logger->Debug("Trying to load plugin %s", filename);

		// HINSTANCE hDLL = LoadLibrary(dir.GetName() + wxT("\\") + filename);
		// if (hDLL == nullptr)
		// {
		// 	LPVOID lpMsgBuf;
		// 	DWORD dw = GetLastError();
		// 	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM| FORMAT_MESSAGE_MAX_WIDTH_MASK ,
		// 		nullptr, dw, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		// 		(LPTSTR) &lpMsgBuf,	0, nullptr);

		// 	logger->Info("* Loading plugin %s: Error %d. %s", filename, dw, lpMsgBuf);
		// }
		// else
		// {
		// 	// create an instance of plugin
		// 	GetPluginPointer pGetPluginPointer = (GetPluginPointer)GetProcAddress(hDLL, "GetPluginPointerV2");
		// 	if (pGetPluginPointer)
		// 	{
		// 		Plugin* pPlugin = pGetPluginPointer();
		// 		if (pPlugin)
		// 		{
		// 			if (!pPlugin->Init((char*)nwnxHome->c_str()))
		// 				logger->Info("* Loading plugin %s: Error during plugin initialization.", filename);
		// 			else
		// 			{
		// 				pPlugin->GetFunctionClass(fClass);
		// 				if (plugins.find(fClass) == plugins.end())
		// 				{
		// 					logger->Info("* Loading plugin %s: Successfully registered as class: %s",
		// 						filename, fClass);
		// 					plugins[fClass] = pPlugin;
		// 				}
		// 				else
		// 				{
		// 					logger->Info("* Skipping plugin %s: Class %s already registered by another plugin.",
		// 						filename, fClass);
		// 					FreeLibrary(hDLL);
		// 				}
		// 			}
		// 		}
		// 		else
		// 			logger->Info("* Loading plugin %s: Error while instancing plugin.", filename);
		// 	}
		// 	else
		// 		{
		// 		GetLegacyPluginPointer pGetPluginPointer = (GetLegacyPluginPointer)GetProcAddress(hDLL, "GetPluginPointer");
		// 		if (pGetPluginPointer)
		// 		{
		// 			LegacyPlugin* pPlugin = pGetPluginPointer();
		// 			if (pPlugin)
		// 			{
		// 				if (!pPlugin->Init((char*)nwnxHome->c_str()))
		// 					logger->Info("* Loading plugin %s: Error during plugin initialization.", filename);
		// 				else
		// 				{
		// 					pPlugin->GetFunctionClass(fClass);
		// 					if (plugins.find(fClass) == plugins.end())
		// 					{
		// 						logger->Info("* Loading plugin %s: Successfully registered as class: %s",
		// 							filename, fClass);
		// 						legacyplugins[fClass] = pPlugin;
		// 					}
		// 					else
		// 					{
		// 						logger->Info("* Skipping plugin %s: Class %s already registered by another plugin.",
		// 							filename, fClass);
		// 						FreeLibrary(hDLL);
		// 					}
		// 				}
		// 			}
		// 			else
		// 				logger->Info("* Loading plugin %s: Error while instancing plugin.", filename);
		// 		}
		// 		else
		// 			logger->Info("* Loading plugin %s: Error. Could not retrieve class object pointer.", filename);
		// 	}
		// 	//	logger->Info("* Loading plugin %s: Error. The plugin is not " 					"compatible with this version of NWNX."), filename);
		// }
  //       cont = dir.GetNext(&filename);
  //   }
}

/***************************************************************************
    Redirected EXE Entry point
***************************************************************************/

int WINAPI NWNXWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
					   LPSTR lpCmdLine, int nCmdShow)
{
    ULONG cbData = 0;

	GUID my_guid =
	{ /* d9ab8a40-f4cc-11d1-b6d7-006097b010e3 */
		0xd9ab8a40,
		0xf4cc,
		0x11d1,
		{0xb6, 0xd7, 0x00, 0x60, 0x97, 0xb0, 0x10, 0xe3}
	};

	shmem = nullptr;

	for (HINSTANCE hinst = nullptr; (hinst = DetourEnumerateModules(hinst)) != nullptr;)
	{
	    shmem = (SHARED_MEMORY*) DetourFindPayload(hinst, my_guid, &cbData);

	    if (shmem)
		{
			//
			// Start the crash dump client first off, as the controller will try and
			// connect to it after we acknowledge booting.
			//

			RegisterCrashDumpHandler();

			//
			// Initialize plugins and load configuration data.
			//

            nwnxHome = new std::string(shmem->nwnx_home);

			// Initialize hook.
			init();
			break;
		}
    }

	/*
	 * If we didn't connect to the controller then bail out here.
	 */
	if (!shmem)
	{
		//DebugPrint( "NWNXWinMain(): Failed to connect to controller!\n" );
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
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		// We are doing a lazy initialization here to increase the robustness of the
		// hooking DLL because it is not performed while the loader lock is held.
		// We hook the app entry point.
        TrueWinMain = (int (WINAPI *)(HINSTANCE, HINSTANCE, LPSTR, int)) DetourGetEntryPoint(nullptr);
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&(PVOID&)TrueWinMain, NWNXWinMain);
        DetourTransactionCommit();
	}
	else if (ul_reason_for_call == DLL_PROCESS_DETACH)
	{
		//
		// Doing complicated things from DLL_PROCESS_ATTACH is extremely bad.
		// Let's not.  Don't want to risk deadlocking the process during an
		// otherwise clean shutdown.
		//

//		wxLogMessage(wxT("* NWNX4 shutting down."));
	}
    return TRUE;
}
