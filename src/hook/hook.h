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

#include "../misc/ini.h"
#include "../misc/log.h"
#include "../misc/shmem.h"
#include "../plugins/legacy_plugin.h"
#include "../plugins/plugin.h"
#include "CPlugin.h"
#include "crashdump.h"
#include <detours/detours.h>
#include <filesystem>
#include <memory>
#include <unordered_map>
#include <windows.h>

#define MAX_BUFFER 64 * 1024

const unsigned char SET_NWNX_GETINT[]
    = {0xB4, 0x4E, 0xB4, 0x57, 0xB4, 0x4E, 0xB4, 0x58, 0xB4, 0x2D, 0xB4, 0x47,
       0xB4, 0x45, 0xB4, 0x54, 0xB4, 0x49, 0xB4, 0x4E, 0xB4, 0x54, 0x00};

const unsigned char SET_NWNX_SETINT[]
    = {0xB4, 0x4E, 0xB4, 0x57, 0xB4, 0x4E, 0xB4, 0x58, 0xB4, 0x2D, 0xB4, 0x53,
       0xB4, 0x45, 0xB4, 0x54, 0xB4, 0x49, 0xB4, 0x4E, 0xB4, 0x54, 0x00};

const unsigned char SET_NWNX_GETFLOAT[]
    = {0xB4, 0x4E, 0xB4, 0x57, 0xB4, 0x4E, 0xB4, 0x58, 0xB4, 0x2D, 0xB4, 0x47, 0xB4, 0x45,
       0xB4, 0x54, 0xB4, 0x46, 0xB4, 0x4C, 0xB4, 0x4F, 0xB4, 0x41, 0xB4, 0x54, 0x00};

const unsigned char SET_NWNX_SETFLOAT[]
    = {0xB4, 0x4E, 0xB4, 0x57, 0xB4, 0x4E, 0xB4, 0x58, 0xB4, 0x2D, 0xB4, 0x53, 0xB4, 0x45,
       0xB4, 0x54, 0xB4, 0x46, 0xB4, 0x4C, 0xB4, 0x4F, 0xB4, 0x41, 0xB4, 0x54, 0x00};

const unsigned char SET_NWNX_GETSTRING[]
    = {0xB4, 0x4E, 0xB4, 0x57, 0xB4, 0x4E, 0xB4, 0x58, 0xB4, 0x2D, 0xB4, 0x47, 0xB4, 0x45, 0xB4,
       0x54, 0xB4, 0x53, 0xB4, 0x54, 0xB4, 0x52, 0xB4, 0x49, 0xB4, 0x4E, 0xB4, 0x47, 0x00};

const unsigned char SET_NWNX_SETSTRING[]
    = {0xB4, 0x4E, 0xB4, 0x57, 0xB4, 0x4E, 0xB4, 0x58, 0xB4, 0x2D, 0xB4, 0x53, 0xB4, 0x45, 0xB4,
       0x54, 0xB4, 0x53, 0xB4, 0x54, 0xB4, 0x52, 0xB4, 0x49, 0xB4, 0x4E, 0xB4, 0x47, 0x00};

extern SHARED_MEMORY* shmem;

typedef std::unordered_map<std::string, std::unique_ptr<CPlugin>> CPluginHashMap;
typedef std::unordered_map<std::string, Plugin*> PluginHashMap;
typedef std::unordered_map<std::string, LegacyPlugin*> LegacyPluginHashMap;
extern CPluginHashMap cplugins;
extern PluginHashMap plugins;
extern LegacyPluginHashMap legacyplugins;

extern std::unique_ptr<LogNWNX> logger;
extern std::filesystem::path nwnxUserDir;
extern std::filesystem::path nwnxInstallDir;
extern SimpleIniConfig* config;

int NWNXGetInt(char* sPlugin, char* sFunction, char* sParam1, int nParam2);
void NWNXSetInt(char* sPlugin, char* sFunction, char* sParam1, int nParam2, int nValue);
float NWNXGetFloat(char* sPlugin, char* sFunction, char* sParam1, int nParam2);
void NWNXSetFloat(char* sPlugin, char* sFunction, char* sParam1, int nParam2, float fValue);
char* NWNXGetString(char* sPlugin, char* sFunction, char* sParam1, int nParam2);
void NWNXSetString(char* sPlugin, char* sFunction, char* sParam1, int nParam2, char* sValue);
extern void (*SetLocalStringNextHook)();
void LegacyPluginPayLoad(char* gameObject, char* name, char* value);
void SetLocalStringHookProc();
DWORD FindHook();
unsigned char* FindPattern(const unsigned char* pattern);
void parseNWNCmdLine();
void loadPlugins();
void init();

static int(WINAPI* TrueWinMain)(HINSTANCE hInstance,
                                HINSTANCE hPrevInstance,
                                LPSTR lpCmdLine,
                                int nCmdShow)
    = nullptr;
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved);

#endif
