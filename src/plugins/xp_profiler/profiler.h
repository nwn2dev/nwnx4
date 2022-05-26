/***************************************************************************
	NWNX Profiler - Profiler plugin
	Copyright (C) 2003 Ingmar Stieger (Papillon, papillon@blackdagger.com)
	Copyright (C) 2007 virusman (virusman@virusman.ru)

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
#if !defined(PROFILER_H_INCLUDED)
#define PROFILER_H_INCLUDED

#define DLLEXPORT extern "C" __declspec(dllexport)

#include "windows.h"
#include "../plugin.h"
#include "../../misc/log.h"
#include "../../misc/ini.h"

class Profiler : public Plugin
{
public:
	Profiler();
	~Profiler();

	bool Init(char* nwnxhome);
	const char* DoRequest(char *gameObject, char* request, char* parameters);
	void GetFunctionClass(char* fClass);
	void SetInt(char* sFunction, char* sParam1, int nParam2, int nValue){}
	void SetFloat(char* sFunction, char* sParam1, int nParam2, float fValue){}
	void SetString(char* sFunction, char* sParam1, int nParam2, char* sValue){}

	void LoadConfiguration(char* nwnxhome);

	enum ELogLevel {logNothing, logStats, logCallstack};
	int	m_LogLevel;
	int log_scriptparts;

	LogNWNX* logger;
private:
	SimpleIniConfig* config;
};

#endif