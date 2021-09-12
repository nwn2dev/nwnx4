/***************************************************************************
    NWNX Ruby
    Copyright (C) 2010 virusman (virusman@virusman.ru)

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
#if !defined(RUBY_H_INCLUDED)
#define RUBY_H_INCLUDED

#define DLLEXPORT extern "C" __declspec(dllexport)

#include <wchar.h>
#include "windows.h"
#include "../legacy_plugin.h"
#include "../../misc/log.h"
#include "../../misc/ini.h"
#include "NWNStructures.h"
#include "ruby.h"

class Ruby : public LegacyPlugin
{
public:
	Ruby();
	~Ruby();

	bool Init(char* nwnxhome);
	void GetFunctionClass(char* fClass);
	void Log(int priority, const char *pcMsg, ...);
	const char* DoRequest(char *gameObject, char* request, char* parameters);
	char *Eval(char *value);

	char *pGameObject;
	DWORD nGameObjectID;

	VALUE cNWScript;
	int nError;

private:
	LogNWNX* logger;
	SimpleIniConfig *config;

};

#endif