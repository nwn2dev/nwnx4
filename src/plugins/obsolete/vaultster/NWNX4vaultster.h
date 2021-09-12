/***************************************************************************
    NWNX4 Vaultster - Vaultster functions plugin
    Copyright (C) 2004 Jeroen Broekhuizen (nwnx@jengine.nl)
	Modified by Patrice Torguet (torguet@gmail.com) for NWNX4

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
#if !defined(NWNX4_VAULSTER_H_INCLUDED)
#define NWNX4_VAULSTER_H_INCLUDED

#define DLLEXPORT extern "C" __declspec(dllexport)

#include "windows.h"
#include "../plugin.h"
#include "../../misc/log.h"
#include "../../misc/ini.h"

#include "Client.h"
#include "Server.h"

class NWNX4Vaultster : public Plugin
{
public:
	NWNX4Vaultster();
	~NWNX4Vaultster();

	bool Init(char* nwnxhome);
	
	bool SetupLogAndIniFile(char* nwnxhome);

	int GetInt(char* sFunction, char* sParam1, int nParam2);
	void SetInt(char* sFunction, char* sParam1, int nParam2, int nValue) {};
	float GetFloat(char* sFunction, char* sParam1, int nParam2) { return 0.0; }
	void SetFloat(char* sFunction, char* sParam1, int nParam2, float fValue) {};
	void SetString(char* sFunction, char* sParam1, int nParam2, char* sValue) {};
	char* GetString(char* sFunction, char* sParam1, int nParam2) { return NULL; }
	void GetFunctionClass(char* fClass);

	void Log(const char* Msg, ...);

private:
	LogNWNX* logger;
	SimpleIniConfig* config;
	int logLevel;
	int maxClients;
	CClient* clients;
	CServer server;
	HANDLE hServer;
};

#endif