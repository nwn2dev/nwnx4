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
#if !defined(FUNCS_H_INCLUDED)
#define FUNCS_H_INCLUDED

#define DLLEXPORT extern "C" __declspec(dllexport)

#include "../../misc/log.h"
#include "../plugin.h"
#include "windows.h"

class Funcs : public Plugin {
public:
	Funcs();
	~Funcs();

	bool Init(char* nwnxhome);

	int GetInt(char* sFunction, char* sParam1, int nParam2);
	void SetInt(char* sFunction, char* sParam1, int nParam2, int nValue) {};
	float GetFloat(char* sFunction, char* sParam1, int nParam2) { return 0.0; }
	void SetFloat(char* sFunction, char* sParam1, int nParam2, float fValue) {};
	void SetString(char* sFunction, char* sParam1, int nParam2, char* sValue) { return; }
	char* GetString(char* sFunction, char* sParam1, int nParam2) { return NULL; }
	void GetFunctionClass(char* fClass);

private:
	LogNWNX* logger;
};

#endif