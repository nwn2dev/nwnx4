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

#pragma once

#include <windows.h>
#include <string>

#define MAX_BUFFER 64*1024

using namespace std;

class Plugin
{
public:
	Plugin();
	virtual ~Plugin();

	// Called when a plugin DLL gets loaded.
	virtual bool Init(char*);

	// Functions for getting data in and out of the plugin
	virtual int GetInt(char* sFunction, char* sParam1, int nParam2) { return 0; }
	virtual void SetInt(char* sFunction, char* sParam1, int nParam2, int nValue) { return; }
	virtual float GetFloat(char* sFunction, char* sParam1, int nParam2) { return 0.0; }
	virtual void SetFloat(char* sFunction, char* sParam1, int nParam2, float fValue) { return; }
	virtual char* GetString(char* sFunction, char* sParam1, int nParam2) { return NULL; }
	virtual void SetString(char* sFunction, char* sParam1, int nParam2, char* sValue) { return; }

	// Process query functions like GET_VERSION, ...
	string ProcessQueryFunction(string function);

	// Return the function class of the plugin in fClass
	virtual void GetFunctionClass(char* fClass);

	// Plugin file name functions
	char* GetPluginFileName();
	char* GetPluginFullPath();
	void SetPluginFullPath(char* fileName);

	// Copy a plugin response into the buffer provided by NWN
	void nwnxcpy(char* buffer, const char* response);
	void nwnxcpy(char* buffer, const char* response, size_t len);

protected:
	string header;
	string subClass;
	string version;
	string description;
	char returnBuffer[MAX_BUFFER];

private:
	char *pluginFileName;
	char *pluginFullPath;
};
