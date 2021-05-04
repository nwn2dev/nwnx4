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

#include "stdwx.h"
#include "hook.h"

/*
 * Globals.
 */

SHARED_MEMORY *shmem;

PluginHashMap plugins;
LegacyPluginHashMap legacyplugins;

wxLogNWNX* logger;
wxString* nwnxhome;
wxFileConfig *config;

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

	return NULL;
}

int NWNXGetInt(char* sPlugin, char* sFunction, char* sParam1, int nParam2)
{
	wxLogDebug(wxT("call to NWNXGetInt(sPlugin=%s, sFunction=%s, sParam1=%s, nParam2=%d)"),
		sPlugin, sFunction, sParam1, nParam2);

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
		wxString plugin(sPlugin);
		wxString function(sFunction);
		if (plugin == wxT("NWNX"))
		{
			if (function == wxT("INSTALLED"))
				return 1;
			else if (function == wxT("GET PLUGIN COUNT"))
				return (int)plugins.size();
		}
		else
			wxLogMessage(wxT("* NWNXGetInt: Function class '%s' not provided by any plugin. Check your installation."), plugin);
	}
	return 0;
}


void NWNXSetInt(char* sPlugin, char* sFunction, char* sParam1, int nParam2, int nValue)
{
	wxLogDebug(wxT("call to NWNXSetInt(sPlugin=%s, sFunction=%s, sParam1=%s, nParam2=%d, nValue=%d)"),
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
		wxString plugin(sPlugin);
		wxString function(sFunction);
		wxLogMessage(wxT("* NWNXSetInt: Function class '%s' not provided by any plugin. Check your installation."), plugin);
	}
}

float NWNXGetFloat(char* sPlugin, char* sFunction, char* sParam1, int nParam2)
{
	wxLogDebug(wxT("call to NWNXGetFloat(sPlugin=%s, sFunction=%s, sParam1=%s, nParam2=%d)"),
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
		wxString plugin(sPlugin);
		wxString function(sFunction);
		wxLogMessage(wxT("* NWNXGetFloat: Function class '%s' not provided by any plugin. Check your installation."), plugin);
	}

	return 0.0;
}

void NWNXSetFloat(char* sPlugin, char* sFunction, char* sParam1, int nParam2, float fValue)
{
	wxLogDebug(wxT("call to NWNXSetFloat(sPlugin=%s, sFunction=%s, sParam1=%s, nParam2=%d, fValue=%f)"),
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
		wxString plugin(sPlugin);
		wxString function(sFunction);
		wxLogMessage(wxT("* NWNXSetFloat: Function class '%s' not provided by any plugin. Check your installation."), plugin);
	}
}

char* NWNXGetString(char* sPlugin, char* sFunction, char* sParam1, int nParam2)
{
    USES_CONVERSION;
	wxLogDebug(wxT("call to NWNXGetString(sPlugin=%s, sFunction=%s, sParam1=%s, nParam2=%d)"),
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
		wxString plugin(sPlugin);
		wxString function(sFunction);
		if (plugin == wxT("NWNX"))
		{
			if (function == wxT("GET PLUGIN CLASS"))
			{
				int i = 0;
				PluginHashMap::iterator it;
				for(it = plugins.begin(); it != plugins.end(); ++it)
				{
					i++;
					if (i == nParam2)
					{
						// sprintf_s(returnBuffer, MAX_BUFFER, wxT("%s"), it->first);
						return returnBuffer;
					}
				}
				return NULL;			
			}
		}
		else
			wxLogMessage(wxT("* NWNXGetString: Function class '%s' not provided by any plugin. Check your installation."), plugin);
	}
	return NULL;
}

void NWNXSetString(char* sPlugin, char* sFunction, char* sParam1, int nParam2, char* sValue)
{
	wxLogDebug(wxT("call to NWNXSetString(sPlugin=%s, sFunction=%s, sParam1=%s, nParam2=%d, sValue=%s)"),
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
		wxLogMessage(wxT("* NWNXSetString: Function class '%s' not provided by any plugin. Check your installation."),
			sPlugin);
	}
}

/***************************************************************************
    parseNWNCmdLine 
***************************************************************************/

void parseNWNCmdLine()
{
	// Crash when commandline empty ?? ->
	// CmdLineArgs args;

	/*
	for (unsigned int i = 0; i < args.size(); i++)
	{
		if (_stricmp(args[i], "-something") == 0)
		{
			// do something
			i++; 
		}
	}
	*/
}

void PayLoad(char *gameObject, char* nwnName, char* nwnValue)
{
    USES_CONVERSION;
	wxLogTrace(TRACE_VERBOSE, wxT("* Payload called"));

	if (!nwnName || !nwnValue)
		return;

	if (strncmp(nwnName, "NWNX!", 5) != 0) 	// not for us
		return;

	wxString fClass, function;
#ifdef UNICODE
	wxString name(nwnName, wxConvUTF8);
#else
	wxString name(nwnName);
#endif

	// maybe replace this code with old version..
	wxStringTokenizer tkz(name, wxT("!"));
	tkz.GetNextToken();

	if (tkz.HasMoreTokens())
	{
		fClass = tkz.GetNextToken();
		wxLogTrace(TRACE_VERBOSE, wxT("* fClass=%s"), fClass);
	}
	else
	{
		wxLogMessage(wxT("* Function class not specified."));
		return;
	}

	if (tkz.HasMoreTokens())
	{
		function = tkz.GetNextToken();
		wxLogTrace(TRACE_VERBOSE, wxT("* function=%s"), function);
		while (tkz.HasMoreTokens())
		{
			function.append(wxT("!") + tkz.GetNextToken());
		}		
	}
	else
	{
		wxLogMessage(wxT("* Function not specified."));
	}

	wxLogTrace(TRACE_VERBOSE, wxT("* Function class '%s', function '%s'."), fClass, function);

	// try to call the plugin
	LegacyPluginHashMap::iterator it = legacyplugins.find(fClass);
	if (it != legacyplugins.end())
	{
		// plugin found, handle the request
		LegacyPlugin* pPlugin = it->second;
		size_t valueLength = strlen(nwnValue);
		const char* pRes = pPlugin->DoRequest(gameObject, W2A(function.c_str()), nwnValue);
		if (pRes)
		{
			// copy result into nwn variable value while respecting the maximum size
			size_t resultLength = strlen(pRes);
			if (valueLength < resultLength)
			{
				strncpy(nwnValue, pRes, valueLength);
				*(nwnValue+valueLength) = 0x0;
			}
			else
			{
				strncpy(nwnValue, pRes, resultLength);
				*(nwnValue+resultLength) = 0x0;
			}
		}
	}
	else
	{
		//if (queryFunctions(fClass, function, nwnValue) == false)
		//{
			wxLogMessage(wxT("* Function class '%s' not provided by any plugin. Check your installation."),
				fClass);
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
    parseNWNCmdLine();
    unsigned char* hookAt;

	wxString logfile = *nwnxhome + wxT("\\nwnx.txt");
	logger = new wxLogNWNX(logfile, header);

	// signal controller that we are ready
	if (!SetEvent(shmem->ready_event))
	{
		wxLogMessage(wxT("* SetEvent failed (%d)"), GetLastError());
		return;
	}
	CloseHandle(shmem->ready_event);

	// open ini file
	wxString inifile = *nwnxhome + wxT("\\nwnx.ini");
	wxLogTrace(TRACE_VERBOSE, wxT("Reading inifile %s"), inifile);
	config = new wxFileConfig(wxEmptyString, wxEmptyString, 
		inifile, wxEmptyString, wxCONFIG_USE_NO_ESCAPE_CHARACTERS);
	
	bool missingFunction = false;
	hookAt = FindPattern(SET_NWNX_GETINT);
	if (hookAt)
	{
		wxLogDebug(wxT("Connecting NWNXGetInt (0x%x)..."), hookAt);
		int (*pt2NWNXSetFunctionPointer)(int (*pt2Function)(char*, char*, char*, int)) = (int (*)(int (*)(char*, char*, char*, int))) hookAt;
		pt2NWNXSetFunctionPointer(&NWNXGetInt);
	}
	else
	{
		wxLogDebug(wxT("NWNXGetInt NOT FOUND!"));
		missingFunction = true;
	}

	hookAt = FindPattern(SET_NWNX_GETFLOAT);
	if (hookAt)
	{
		wxLogDebug(wxT("Connecting NWNXGetFloat (0x%x)..."), hookAt);
		float (*pt2NWNXSetFunctionPointer)(float (*pt2Function)(char*, char*, char*, int)) = (float (*)(float (*)(char*, char*, char*, int))) hookAt;
		pt2NWNXSetFunctionPointer(&NWNXGetFloat);
	}
	else
	{
		wxLogDebug(wxT("NWNXGetFloat NOT FOUND!"));
		missingFunction = true;
	}

	hookAt = FindPattern(SET_NWNX_GETSTRING);
	if (hookAt)
	{
		wxLogDebug(wxT("Connecting NWNXGetString (0x%x)..."), hookAt);
		char* (*pt2NWNXSetFunctionPointer)(char* (*pt2Function)(char*, char*, char*, int)) = (char* (*)(char* (*)(char*, char*, char*, int))) hookAt;
		pt2NWNXSetFunctionPointer(&NWNXGetString);
	}
	else
	{
		wxLogDebug(wxT("NWNXGetString NOT FOUND!"));
		missingFunction = true;
	}

	hookAt = FindPattern(SET_NWNX_SETINT);
	if (hookAt)
	{
		wxLogDebug(wxT("Connecting NWNXSetInt (0x%x)..."), hookAt);
		void (*pt2NWNXSetFunctionPointer)(void (*pt2Function)(char*, char*, char*, int, int)) = (void (*)(void (*)(char*, char*, char*, int, int))) hookAt;
		pt2NWNXSetFunctionPointer(&NWNXSetInt);
	}
	else
	{
		wxLogDebug(wxT("NWNXSetInt NOT FOUND!"));
		missingFunction = true;
	}

	hookAt = FindPattern(SET_NWNX_SETFLOAT);
	if (hookAt)
	{
		wxLogDebug(wxT("Connecting NWNXSetFloat (0x%x)..."), hookAt);
		void (*pt2NWNXSetFunctionPointer)(void (*pt2Function)(char*, char*, char*, int, float)) = (void (*)(void (*)(char*, char*, char*, int, float))) hookAt;
		pt2NWNXSetFunctionPointer(&NWNXSetFloat);
	}
	else
	{
		wxLogDebug(wxT("NWNXSetFloat NOT FOUND!"));
		missingFunction = true;
	}

	hookAt = FindPattern(SET_NWNX_SETSTRING);
	if (hookAt)
	{
		wxLogDebug(wxT("Connecting NWNXSetString (0x%x)..."), hookAt);
		void (*pt2NWNXSetFunctionPointer)(void (*pt2Function)(char*, char*, char*, int, char*)) = (void (*)(void (*)(char*, char*, char*, int, char*))) hookAt;
		pt2NWNXSetFunctionPointer(&NWNXSetString);
	}
	else
	{
		wxLogDebug(wxT("NWNXSetString NOT FOUND!"));
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
		wxLogDebug(wxT("SetLocalString hooked at 0x%x"), oldHookAt);
	else
	{
		wxLogDebug(wxT("SetLocalString NOT FOUND!"));
		missingFunction = true;
	}

	if (missingFunction) 
		wxLogMessage(wxT(
			"!! One or more functions could not be hooked.\n" 
			"!! Please check the system requirements (NWN2 1.06 or later) for this \n"  
			"!! version of NWNX4, and come to our forums to get help or eventual updates.\n"));

	wxLogMessage(wxT("* Loading plugins..."));
	loadPlugins();

	// Suppress general protection fault error box
	bool noGPFaultErrorBox = true;
	config->Read(wxT("noGPFaultErrorBox"), &noGPFaultErrorBox);
	if (noGPFaultErrorBox)
	{
		DWORD dwMode = SetErrorMode(SEM_NOGPFAULTERRORBOX);
		SetErrorMode(dwMode | SEM_NOGPFAULTERRORBOX);
		wxLogMessage(wxT("* General protection fault error dialog disabled."));
	}

	wxLogMessage(wxT("* NWNX4 activated."));
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
    USES_CONVERSION;
	TCHAR fClass[128];
    wxString filename;
	wxString pattern(wxT("xp_*.dll"));
	wxDir dir(*nwnxhome);

    wxLogDebug(wxT("Enumerating plugins in current directory"));

    if (!dir.IsOpened())
    {
        // deal with the error here - wxDir would already log an error message
        // explaining the exact reason of the failure
        return;
    }

    bool cont = dir.GetFirst(&filename, pattern, wxDIR_FILES);
    while (cont)
    {
        wxLogDebug(wxT("Trying to load plugin %s"), filename);

		HINSTANCE hDLL = LoadLibrary(A2T((dir.GetName() + wxT("\\") + filename).c_str()));
		if (hDLL == NULL)
		{
			LPVOID lpMsgBuf;
			DWORD dw = GetLastError(); 
			FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM| FORMAT_MESSAGE_MAX_WIDTH_MASK ,
				NULL, dw, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				(LPTSTR) &lpMsgBuf,	0, NULL);

			wxLogMessage(wxT("* Loading plugin %s: Error %d. %s"), filename, dw, lpMsgBuf);
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
					if (!pPlugin->Init(A2T(nwnxhome->c_str())))
						wxLogMessage(wxT("* Loading plugin %s: Error during plugin initialization."), filename);
					else
					{
						pPlugin->GetFunctionClass(fClass);
						if (plugins.find(fClass) == plugins.end())
						{
							wxLogMessage(wxT("* Loading plugin %s: Successfully registered as class: %s"), 
								filename, fClass);
							plugins[fClass] = pPlugin;
						}
						else
						{
							wxLogMessage(wxT("* Skipping plugin %s: Class %s already registered by another plugin."),
								filename, fClass);
							FreeLibrary(hDLL);
						}
					}
				}
				else
					wxLogMessage(wxT("* Loading plugin %s: Error while instancing plugin."), filename);
			}
			else
				{
				GetLegacyPluginPointer pGetPluginPointer = (GetLegacyPluginPointer)GetProcAddress(hDLL, "GetPluginPointer");
				if (pGetPluginPointer)
				{
					LegacyPlugin* pPlugin = pGetPluginPointer();
					if (pPlugin)
					{
                        if (!pPlugin->Init(A2T(nwnxhome->c_str()))) {
                            wxLogMessage(wxT("* Loading plugin %s: Error during plugin initialization."),
                                         filename);
                        } else {
                            pPlugin->GetFunctionClass(fClass);
                            if (plugins.find(fClass) == plugins.end()) {
                                wxLogMessage(wxT("* Loading plugin %s: Successfully registered as class: %s"),
                                             filename, fClass);
                                legacyplugins[fClass] = pPlugin;
                            } else {
                                wxLogMessage(
                                            wxT("* Skipping plugin %s: Class %s already registered by another plugin."),
                                            filename, fClass);
                                FreeLibrary(hDLL);
                            }
                        }
					}
					else
						wxLogMessage(wxT("* Loading plugin %s: Error while instancing plugin."), filename);
				}
				else
					wxLogMessage(wxT("* Loading plugin %s: Error. Could not retrieve class object pointer."), filename);
			}
			//	wxLogMessage(wxT("* Loading plugin %s: Error. The plugin is not " 					"compatible with this version of NWNX."), filename);
		}
        cont = dir.GetNext(&filename);
    }
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

	shmem = NULL;

    for (HINSTANCE hinst = NULL; (hinst = DetourEnumerateModules(hinst)) != NULL;)
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

			nwnxhome = new wxString(shmem->nwnx_home);

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
		ExitProcess( ERROR_DEVICE_NOT_CONNECTED );
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
    int argc = 0;
    wxChar **argv = NULL;

    if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
        wxEntryStart(argc, argv);
		// We are doing a lazy initialization here to increase the robustness of the 
		// hooking DLL because it is not performed while the loader lock is held.
		// We hook the app entry point.
        TrueWinMain = (int (WINAPI *)(HINSTANCE, HINSTANCE, LPSTR, int)) DetourGetEntryPoint(NULL);
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&(PVOID&)TrueWinMain, NWNXWinMain);
        DetourTransactionCommit();
	}
	else if (ul_reason_for_call == DLL_PROCESS_DETACH)
	{
	    wxEntryCleanup();
		//
		// Doing complicated things from DLL_PROCESS_ATTACH is extremely bad.
		// Let's not.  Don't want to risk deadlocking the process during an
		// otherwise clean shutdown.
		//

//		wxLogMessage(wxT("* NWNX4 shutting down."));
	}
    return TRUE;
}
