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

#if !defined(HOOK_H_INCLUDED)
#define HOOK_H_INCLUDED

#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include "wx/dir.h"
#include "wx/hashset.h"
#include "wx/tokenzr.h"
#include "wx/fileconf.h"
#include "detours/detours.h"
#include "crashdump.h"
#include "../misc/log.h"
#include "../misc/cmdlineargs.h"
#include "../misc/shmem.h"
#include "../plugins/plugin.h"
#include "../plugins/legacy_plugin.h"
#include <atlbase.h>
#include <atlconv.h>
#include <chrono>
#include <thread>

#define MAX_BUFFER 64*1024

const wxString header = 
	wxT("NWN Extender 4 V.1.1.0\n") \
	wxT("(c) 2008 by Ingmar Stieger (Papillon)\n") \
	wxT("visit us at http://www.nwnx.org\n");

const unsigned char SET_NWNX_GETINT[] = {0xB4, 0x4E, 0xB4, 0x57, 0xB4, 0x4E, 0xB4, 0x58, 
                            0xB4, 0x2D, 0xB4, 0x47, 0xB4, 0x45, 0xB4, 0x54, 
                            0xB4, 0x49, 0xB4, 0x4E, 0xB4, 0x54, 0x00};

const unsigned char SET_NWNX_SETINT[] = {0xB4, 0x4E, 0xB4, 0x57, 0xB4, 0x4E, 0xB4, 0x58, 
                            0xB4, 0x2D, 0xB4, 0x53, 0xB4, 0x45, 0xB4, 0x54, 
                            0xB4, 0x49, 0xB4, 0x4E, 0xB4, 0x54, 0x00};

const unsigned char SET_NWNX_GETFLOAT[] = {0xB4, 0x4E, 0xB4, 0x57, 0xB4, 0x4E, 0xB4, 0x58, 
                              0xB4, 0x2D, 0xB4, 0x47, 0xB4, 0x45, 0xB4, 0x54, 
                              0xB4, 0x46, 0xB4, 0x4C, 0xB4, 0x4F, 0xB4, 0x41,
                              0xB4, 0x54, 0x00};

const unsigned char SET_NWNX_SETFLOAT[] = {0xB4, 0x4E, 0xB4, 0x57, 0xB4, 0x4E, 0xB4, 0x58, 
                              0xB4, 0x2D, 0xB4, 0x53, 0xB4, 0x45, 0xB4, 0x54, 
                              0xB4, 0x46, 0xB4, 0x4C, 0xB4, 0x4F, 0xB4, 0x41,
                              0xB4, 0x54, 0x00};

const unsigned char SET_NWNX_GETSTRING[] = {0xB4, 0x4E, 0xB4, 0x57, 0xB4, 0x4E, 0xB4, 0x58, 
                               0xB4, 0x2D, 0xB4, 0x47, 0xB4, 0x45, 0xB4, 0x54, 
                               0xB4, 0x53, 0xB4, 0x54, 0xB4, 0x52, 0xB4, 0x49,
                               0xB4, 0x4E, 0xB4, 0x47, 0x00};

const unsigned char SET_NWNX_SETSTRING[] = {0xB4, 0x4E, 0xB4, 0x57, 0xB4, 0x4E, 0xB4, 0x58, 
                               0xB4, 0x2D, 0xB4, 0x53, 0xB4, 0x45, 0xB4, 0x54, 
                               0xB4, 0x53, 0xB4, 0x54, 0xB4, 0x52, 0xB4, 0x49,
                               0xB4, 0x4E, 0xB4, 0x47, 0x00};


extern SHARED_MEMORY *shmem;

WX_DECLARE_STRING_HASH_MAP(Plugin*, PluginHashMap);
WX_DECLARE_STRING_HASH_MAP(LegacyPlugin*, LegacyPluginHashMap);
extern PluginHashMap plugins;
extern LegacyPluginHashMap legacyplugins;

extern wxLogNWNX* logger;
extern wxString* nwnxhome;
extern wxFileConfig *config;

extern char returnBuffer[MAX_BUFFER];

int NWNXGetInt(char* sPlugin, char* sFunction, char* sParam1, int nParam2);
void NWNXSetInt(char* sPlugin, char* sFunction, char* sParam1, int nParam2, int nValue);
float NWNXGetFloat(char* sPlugin, char* sFunction, char* sParam1, int nParam2);
void NWNXSetFloat(char* sPlugin, char* sFunction, char* sParam1, int nParam2, float fValue);
char* NWNXGetString(char* sPlugin, char* sFunction, char* sParam1, int nParam2);
void NWNXSetString(char* sPlugin, char* sFunction, char* sParam1, int nParam2, char* sValue);
void (*SetLocalStringNextHook)();
void PayLoad(char *gameObject, char* name, char* value);
void SetLocalStringHookProc();
DWORD FindHook();
unsigned char* FindPattern(const unsigned char* pattern);
void parseNWNCmdLine();
void loadPlugins();
void init();

static int (WINAPI * TrueWinMain)(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPSTR lpCmdLine, int nCmdShow) = NULL;
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved);

#endif
