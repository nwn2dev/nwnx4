/***************************************************************************
    NWNX Chat - NWNX4 port
    Copyright (C) 2006-2007 virusman (virusman@virusman.ru)
	Original Chat plugin was written by dumbo

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
#if !defined(CHAT_H_INCLUDED)
#define CHAT_H_INCLUDED

#define DLLEXPORT extern "C" __declspec(dllexport)

#include <wchar.h>
#include "windows.h"
#include "../plugin.h"
#include "../../misc/log.h"
#include "../../misc/ini.h"

class Chat : public Plugin
{
public:
	Chat();
	~Chat();

	bool Init(char* nwnxhome);
	void GetFunctionClass(char* fClass);
	int ChatProc(const int mode, const int id, const char **msg, const int to);

	virtual int GetInt(char* sFunction, char* sParam1, int nParam2);
	virtual char* GetString(char* sFunction, char* sParam1, int nParam2);
	// Unused, but required 
	void SetInt(char* sFunction, char* sParam1, int nParam2, int nValue);
	virtual void SetFloat(char* sFunction, char* sParam1, int nParam2, float fValue) { };
	virtual void SetString(char* sFunction, char* sParam1, int nParam2, char* sValue);


private:
	LogNWNX* logger;
	SimpleIniConfig *config;
	int supressMsg;
	int maxMsgLen;
	int processNPC;
	int ignore_silent;
	//char chatScript[17];
	//char servScript[17];
	std::string chatScript;
	std::string servScript;

};

#endif