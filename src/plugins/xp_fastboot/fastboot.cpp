/***************************************************************************
    NWNX FastBoot - NWN2Server Zero Module Copy (Startup Time Improvements)
    Copyright (C) 2008 Skywing (skywing@valhallalegends.com)

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

#include "fastboot.h"
#include <cassert>

#define PLUGIN_VERSION "0.0.1"

FastBoot* plugin;
LogNWNX* _logger;

/***************************************************************************
    NWNX and DLL specific functions
***************************************************************************/

DLLEXPORT Plugin* GetPluginPointerV2() { return plugin; }

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
		plugin = new FastBoot();

		char szPath[MAX_PATH];
		GetModuleFileNameA(hModule, szPath, MAX_PATH);
		plugin->SetPluginFullPath(szPath);
	} else if (ul_reason_for_call == DLL_PROCESS_DETACH) {
		delete plugin;
	}
	return TRUE;
}

/***************************************************************************
    Implementation of FastBoot Plugin
***************************************************************************/

FastBoot::FastBoot()
    : logged(false)
{
	header = "NWNX FastBoot Plugin " PLUGIN_VERSION "\n"
	         "(c) 2008 by Skywing \n"
	         "Visit NWNX at: http://www.nwnx.org\n";

	description
	    = "This reduces NWN2Server start times by using links instead of copying module data.";

	subClass = "FASTBOOT";
	version  = PLUGIN_VERSION;
}

FastBoot::~FastBoot() { logger->Info("* Plugin unloaded."); }

bool FastBoot::Init(char* nwnxhome)
{
	assert(GetPluginFileName());

	/* Log file */
	std::string logfile(nwnxhome);
	logfile.append("\\");
	logfile.append(GetPluginFileName());
	logfile.append(".txt");
	logger = new LogNWNX(logfile);
	logger->Info(header.c_str());

	_logger = logger;

	if (!disableModuleCopy()) {
		logger->Info("* Failed to initialize module copy hook.");
		return true;
	}

	logger->Info("* Plugin initialized.");
	return true;
}

void FastBoot::GetFunctionClass(char* fClass) { strncpy_s(fClass, 128, "FASTBOOT", 8); }

void FastBoot::SetString(char* sFunction, char* sParam1, int nParam2, char* sValue)
{
	logger->Trace("* Plugin SetString(0x%x, %s, %d, %s)", 0x0, sParam1, nParam2, sValue);
}

char* FastBoot::GetString(char* sFunction, char* sParam1, int nParam2)
{
	logger->Trace("* Plugin GetString(0x%x, %s, %d)", 0x0, sParam1, nParam2);
	return NULL;
}

bool FastBoot::disableModuleCopy()
{
	const SIZE_T NumHooks = 1;
	HMODULE Kernel32;
	SIZE_T i;
	ULONG HookStatus[NumHooks];
	PVOID OrigAddresses[NumHooks];
	PVOID HookAddresses[NumHooks] = {ModuleCopyFileA};
	CONST CHAR* Symbols[NumHooks] = {"CopyFileA"};

	Kernel32 = GetModuleHandleW(L"kernel32.dll");

	if (!Kernel32)
		return false;

	for (i = 0; i < NumHooks; i += 1) {
		OrigAddresses[i] = GetProcAddress(Kernel32, Symbols[i]);

		if (!OrigAddresses[i]) {
			logger->Info("! Failed to resolve an import: %s", Symbols[i]);

			return false;
		}
	}

	if (!RedirectImageImports(GetModuleHandleW(0), OrigAddresses, HookAddresses, HookStatus,
	                          NumHooks)) {
		logger->Info("! Failed to hook imports.");
		return false;
	}

	for (i = 0; i < NumHooks; i += 1) {
		if (!HookStatus[i]) {
			logger->Info("! Failed to hook an import: %s", Symbols[i]);

			return false;
		}
	}

	logger->Info("* Module hard link hook installed.");

	return true;
}

BOOL WINAPI FastBoot::ModuleCopyFileA(__in LPCSTR lpExistingFileName,
                                      __in LPCSTR lpNewFileName,
                                      __in BOOL bFailIfExists)
{
#if 0
	const char *Dot;

	//
	// Check if we're copying a ".mod" file.  If so then we shall simply make a
	// hard link instead.
	//

	Dot = strrchr( lpExistingFileName, '.' );

	if (!Dot)
		return CopyFileA( lpExistingFileName, lpNewFileName, bFailIfExists );

	if (_stricmp( Dot + 1, "mod" ))
		return CopyFileA( lpExistingFileName, lpNewFileName, bFailIfExists );

	Dot = strrchr( lpNewFileName, '.' );

	if (!Dot)
		return CopyFileA( lpExistingFileName, lpNewFileName, bFailIfExists );

	if (_stricmp( Dot + 1, "mod" ))
		return CopyFileA( lpExistingFileName, lpNewFileName, bFailIfExists );
#else
	//
	// Determine the full path of the destination file.
	//

	char NameBuf[MAX_PATH + 1];
	char* NamePart;

	if (!GetFullPathNameA(lpNewFileName, MAX_PATH, NameBuf, &NamePart))
		return CopyFileA(lpExistingFileName, lpNewFileName, bFailIfExists);

	//
	// Check if we are copying to a currentgame directory.  This is a bit ugly
	// using a substring match, but it'll do.  The reason for not using the
	// match on .mod anymore is so that we create hardlinks for directory mode
	// modules.
	//

	_strlwr(NameBuf);

	if (!strstr(NameBuf, "\\nwn2\\currentgame"))
		return CopyFileA(lpExistingFileName, lpNewFileName, bFailIfExists);

#endif

	//
	// Remove the existing file if we are requested to.
	//

	if (!bFailIfExists)
		DeleteFileA(lpNewFileName);

	if (!plugin->logged) {
		plugin->logged = true;

		_logger->Info("* Creating link at '%s' to '%s'.", lpNewFileName, lpExistingFileName);
	}

	if (!CreateHardLinkA(lpNewFileName, lpExistingFileName, 0)) {
		_logger->Info("* Failed to create hard link - %lu.", GetLastError());

		return FALSE;
	}

	return TRUE;
}
