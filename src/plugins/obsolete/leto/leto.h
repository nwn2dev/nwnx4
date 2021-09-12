/***************************************************************************
    NWNX Leto - Leto bridge
    Copyright (C) 2007 virusman (virusman@virusman.ru)
	Copyright (C) 2004 David Frauzel (dragonsong), dragon@weathersong.net

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
#if !defined(LETO_H_INCLUDED)
#define LETO_H_INCLUDED

#define DLLEXPORT extern "C" __declspec(dllexport)

#include "windows.h"
#include "../legacy_plugin.h"
#include "../../misc/log.h"

typedef char* (CALLBACK* LPFNONCREATE)(const char*);
typedef char* (CALLBACK* LPFNONRELEASE)();
typedef char* (CALLBACK* LPFNONREQUEST)(char*,char*,char*);

class Leto : public LegacyPlugin
{
public:
	Leto();
	~Leto();

	bool Init(char* nwnxhome);  
	const char* DoRequest(char *gameObject, char* request, char* parameters);
	void GetFunctionClass(char* fClass);

	LPCSTR sTimeReport(LARGE_INTEGER c1, LARGE_INTEGER c2);
	char sTime[45];

private:
	LogNWNX* logger;
	HINSTANCE hDLL;

	LPFNONCREATE lpfnOnCreate;
	LPFNONRELEASE lpfnOnRelease;
	LPFNONREQUEST lpfnOnRequest;

	int iDebug;

	LARGE_INTEGER clkFreq, clkStart, clkEnd;
};

#endif