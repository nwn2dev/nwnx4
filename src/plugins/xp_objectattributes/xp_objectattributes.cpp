/***************************************************************************
    NWNX ObjectAttributes - NWN2Server Game Object Attributes Editor
    Copyright (C) 2008-2011 Skywing (skywing@valhallalegends.com).  This
    instance of the core XPObjectAttributes functionality is licensed under the
    GPLv2 for the usage of the NWNX4 project, nonwithstanding other licenses
    granted by the copyright holder.

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

#include "xp_objectattributes.h"
#include <cassert>

#define PLUGIN_VERSION "0.0.1"

ObjectAttributesPlugin* plugin;

/***************************************************************************
    NWNX and DLL specific functions
***************************************************************************/

DLLEXPORT Plugin* GetPluginPointerV2() { return plugin; }

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
		plugin = new ObjectAttributesPlugin();

		char szPath[MAX_PATH];
		GetModuleFileNameA(hModule, szPath, MAX_PATH);
		plugin->SetPluginFullPath(szPath);
	} else if (ul_reason_for_call == DLL_PROCESS_DETACH) {
		delete plugin;
	}
	return TRUE;
}

/***************************************************************************
    Implementation of ObjectAttributes Plugin
***************************************************************************/

ObjectAttributesPlugin::ObjectAttributesPlugin()
{
	header = "NWNX ObjectAttributes Plugin " PLUGIN_VERSION "\n"
	         "(c) 2008-2011 by Skywing \n"
	         "Visit NWNX at: http://www.nwnx.org\n";

	description
	    = "This plugin provides scripting support functionality to edit game object attributes.";

	subClass = "NWNX4-ObjectAttributes";
	version  = PLUGIN_VERSION;
}

ObjectAttributesPlugin::~ObjectAttributesPlugin() { logger->Info("* Plugin unloaded."); }

bool ObjectAttributesPlugin::Init(char* nwnxhome)
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

void ObjectAttributesPlugin::GetFunctionClass(char* fClass)
{
	strncpy_s(fClass, 128, "OBJECTATTRIBUTES", 16);
}

void ObjectAttributesPlugin::SetString(char* sFunction, char* sParam1, int nParam2, char* sValue)
{
	logger->Trace("* Plugin SetString(0x%x, %s, %d, %s)", 0x0, sParam1, nParam2, sValue);

	//
	// Invoke the main dispatcher.
	//

	OnXPObjectAttributesSetString("OBJECTATTRIBUTES", sFunction, sParam1, nParam2, sValue);
}

char* ObjectAttributesPlugin::GetString(char* sFunction, char* sParam1, int nParam2)
{
	logger->Trace("* Plugin GetString(0x%x, %s, %d)", 0x0, sParam1, nParam2);
	return NULL;
}
