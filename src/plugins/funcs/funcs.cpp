/***************************************************************************
    NWNX Funcs - Various functions plugin
    Copyright (C) 2010 Andrew Brockert (Zebranky, andrew@mercuric.net) 

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

#include "funcs.h"
#include <cassert>

/***************************************************************************
    NWNX and DLL specific functions
***************************************************************************/

Funcs* plugin;

typedef unsigned long objid_t;
typedef unsigned long dword;
void ** pCAppManager = (void **)0x0086442C;
void * (__fastcall *CServerExoApp__GetCreatureByGameObjectID)(void *, void *, objid_t) = (void * (__fastcall *)(void *, void *, objid_t))0x0054A1B0;
unsigned short (__fastcall *CNWSCreature__GetSoundSet)(void *) = (unsigned short (__fastcall *)(void *))0x005504A0;

DLLEXPORT Plugin* GetPluginPointerV2()
{
	return plugin;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		plugin = new Funcs();

		char szPath[MAX_PATH];
		GetModuleFileNameA(hModule, szPath, MAX_PATH);
		plugin->SetPluginFullPath(szPath);
	}
	else if (ul_reason_for_call == DLL_PROCESS_DETACH)
	{
		delete plugin;
	}
    return TRUE;
}


/***************************************************************************
    Implementation of Funcs Plugin
***************************************************************************/

Funcs::Funcs()
{
	header =
		"NWNX Funcs Plugin V.0.0.1\n" \
		"(c) 2010 Andrew Brockert (Zebranky)\n" \
		"visit us at http://www.nwnx.org\n";

	description =
		"This plugin provides functions to poke at NWN2 internals.";

	subClass = "FUNCS";
	version = "0.0.1";
}

Funcs::~Funcs()
{
	logger->Info("* Plugin unloaded.");
}

bool Funcs::Init(char* nwnxhome)
{
	assert(GetPluginFileName());

	/* Log file */
	std::string logfile(nwnxhome);
	logfile.append("\\");
	logfile.append(GetPluginFileName());
	logfile.append(".txt");
	logger = new LogNWNX(logfile);
	logger->Info(header.c_str());

	logger->Info("* Plugin initialized.");
	return true;
}

void Funcs::GetFunctionClass(char* fClass)
{
	strncpy_s(fClass, 128, "FUNCS", 5);
}

int Funcs::GetInt(char* sFunction, char* sParam1, int nParam2)
{
	logger->Trace("* Plugin GetInt(0x%x, %s, %d)", 0x0, sParam1, nParam2);

	std::string wxRequest(sFunction);
	std::string function(sFunction);

	if (function == "")
	{
		logger->Info("* Function not specified.");
		return NULL;
	}
	else if(function == "GETSOUNDSET")
	{
		objid_t creature_oid = nParam2;
		void *cre = CServerExoApp__GetCreatureByGameObjectID(*pCAppManager, NULL, creature_oid);
		if(cre != NULL)
		{
			return CNWSCreature__GetSoundSet(cre);
		}
		else
		{
			return 0;
		}
	}
	else // unknown function
	{
		return 0;
	}
}

