/***************************************************************************
    NWNX Plugin - Plugins are derived from this class
    Copyright (C) 2007 Ingmar Stieger (Papillon, papillon@blackdagger.com) 

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
#include "../plugins/legacy_plugin.h"

LegacyPlugin::LegacyPlugin()
{
	pluginFileName = NULL;
	pluginFullPath = NULL;
}

LegacyPlugin::~LegacyPlugin()
{
}

bool LegacyPlugin::Init(TCHAR* parameter)
{
	return true;
}

void LegacyPlugin::ProcessQueryFunction(string function, char* buffer)
{
	if (function == "GET_SUBCLASS")
		nwnxcpy(buffer, subClass.c_str()); 
	else if (function == "GET_VERSION")
		nwnxcpy(buffer, version.c_str()); 
	else if (function == "GET_DESCRIPTION")
		nwnxcpy(buffer, description.c_str()); 
}

void LegacyPlugin::GetFunctionClass(TCHAR* fClass)
{
	fClass = NULL;
}

TCHAR* LegacyPlugin::GetPluginFileName()
{
	return pluginFileName;
}

TCHAR* LegacyPlugin::GetPluginFullPath()
{
	return pluginFullPath;
}

void LegacyPlugin::SetPluginFullPath(TCHAR* fileName)
{
    TCHAR drive[5], dir[255], fname[50], ext[3];
    _tsplitpath(fileName, drive, dir, fname, ext);

    pluginFullPath = new TCHAR[MAX_BUFFER];
    pluginFileName = new TCHAR[MAX_BUFFER];
    wxStrcpy(pluginFullPath, fileName);
	wxStrcat(pluginFileName, dir);
    wxStrcat(pluginFileName, "\\");
    wxStrcat(pluginFileName, fname);
}

void LegacyPlugin::nwnxcpy(char* buffer, const char* response)
{
	nwnxcpy(buffer, response, strlen(response));
}

void LegacyPlugin::nwnxcpy(char* buffer, const char* response, size_t len)
{
	if (len > MAX_BUFFER)
		len = MAX_BUFFER - 1;

	memcpy(buffer, response, len);
	buffer[len] = 0x0;
}