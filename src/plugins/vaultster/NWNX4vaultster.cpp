/***************************************************************************
    NWNX4 Vaultster - Vaultster functions plugin
    Copyright (C) 2004 Jeroen Broekhuizen (nwnx@jengine.nl)
	2008 Modified by Patrice Torguet (torguet@gmail.com) for NWNX4

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

#include "NWNX4vaultster.h"
#include <cassert>

/***************************************************************************
    NWNX and DLL specific functions
***************************************************************************/

NWNX4Vaultster* plugin;

DLLEXPORT Plugin* GetPluginPointerV2()
{
	return plugin;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		plugin = new NWNX4Vaultster();

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
    Implementation of Timer Plugin
***************************************************************************/

NWNX4Vaultster::NWNX4Vaultster()
{
	header =
		"NWNX4 Vaultster - Vaultster functions plugin V 0.0.1\n" \
		"(c) 2004 Jeroen Broekhuizen (nwnx@jengine.nl)\n" \
		"2008 Modified by Patrice Torguet (torguet@gmail.com) for NWNX4\n" \
		"visit us at http://www.nwnx.org\n";

	description = "This plugin provides portalling functionnalities.";

	subClass = "VAULTSTER";
	version = "0.0.1";

	//QueryPerformanceFrequency(&liFrequency);
}

NWNX4Vaultster::~NWNX4Vaultster()
{
	logger->Info("* Plugin unloaded.");
}

bool NWNX4Vaultster::SetupLogAndIniFile(char* nwnxhome)
{
	assert(GetPluginFileName());

	// Log file
	std::string logfile(nwnxhome);
	logfile.append("\\");
	logfile.append(GetPluginFileName());
	logfile.append(".txt");
	logger = new LogNWNX(logfile);
	logger->Info(header.c_str());

	// Ini file
	std::string inifile(nwnxhome);
	inifile.append("\\");
	inifile.append(GetPluginFileName());
	inifile.append(".ini");
	logger->Trace("* reading inifile %s", inifile.c_str());

	config = new SimpleIniConfig(inifile);
	logger->Configure(config);

	return true;
}

bool NWNX4Vaultster::Init(char* nwnxhome)
{
	std::string buf;
	std::string key;
	bool startServer;
	bool validateclient;
	int port;
	DWORD serverId;
	int lowprior;
	int serverCount;

	SetupLogAndIniFile(nwnxhome);

	// start up WSA (winsock library)
	if (!CSock::StartWSA ()) {
		return false;
	}

	// load in the settings
	if (config->Read("startserver", &startServer) )
	{
		logger->Info("* read startserver: %d", startServer);
	}
	else
	{
		logger->Info("* startserver not found in ini file");
		startServer = 1;
		logger->Info("* Defaulting to 1");
	}
	if (config->Read("validateclient", &validateclient) )
	{
		logger->Info("* read validateclient: %d", validateclient);
	}
	else
	{
		logger->Info("* validateclient not found in ini file");
		validateclient = 1;
		logger->Info("* Defaulting to 1");
	}
	server.setValidate (validateclient);
	if (config->Read("MaxClients", &maxClients) )
	{
		logger->Info("* read MaxClients: %d", maxClients);
	}
	else
	{
		logger->Info("* MaxClients not found in ini file");
		maxClients = 10;
		logger->Info("* Defaulting to 10");
	}
	server.setMaxClients (maxClients);
	if (config->Read("Path", &buf) )
	{
		logger->Info("* read Path: %s", buf.c_str());
	}
	else
	{
		logger->Info("* Path not found in ini file");
		buf = ".";
		logger->Info("* Defaulting to .");
	}
	// remove trailing backspace
	if (buf.size() > 0 && buf[buf.size() - 1] == '\\')
		buf.resize(buf.size() - 1);
	logger->Info("* Path is now: %s", buf.c_str());

	CClient::setServervault((char*)buf.c_str());
	server.setServervault((char*)buf.c_str());

	if (config->Read("Key", &key) )
	{
		logger->Info("* read Key: %s", key.c_str());
	}
	else
	{
		logger->Info("* Key not found in ini file");
		key = "*invalid";
		logger->Info("* Defaulting to *invalid");
	}
	CClient::setCryptoKey ((char*)key.c_str());
	server.setCryptoKey ((char*)key.c_str());

	if (config->Read("Password", &buf) )
	{
		logger->Info("* read Password");
	}
	else
	{
		logger->Info("* Password not found in ini file");
		buf = "PWD!@";
		logger->Info("* Defaulting to PWD!@");
	}
	CClient::setPassword ((char*)buf.c_str());
	server.setPassword ((char*)buf.c_str());

	if (config->Read("Port", &port) )
	{
		logger->Info("* read Port %d", port);
	}
	else
	{
		logger->Info("* Port not found in ini file");
		port = 5100;
		logger->Info("* Defaulting to 5100");
	}
	CClient::setPort (port);
	server.setPort (port);

	if (config->Read("Low prior", &lowprior) )
	{
		logger->Info("* read Low prior %d", lowprior);
	}
	else
	{
		logger->Info("* Low prior not found in ini file");
		lowprior = 0;
		logger->Info("* Defaulting to 0");
	}

	// load in the known servers
	if (validateclient) {
		if (!config->Read("Count", &serverCount) )
		{
			serverCount = 0;
		}
		logger->Info("o Loading in %d known servers.\n", serverCount);
		for (int i = 0; i < serverCount; i++) {
			key = "Server" + std::to_string(i + 1);
			if (config->Read(key, &buf) )
			{
				if (!buf.empty()) server.addKnownServer((char*)buf.c_str());
				logger->Info("o Loaded: %s\n", buf.c_str());
			}
		}
	}

	// initialize the clients
	clients = new CClient[maxClients];

	// start up the server
	if (startServer) {
		hServer = CreateThread (NULL, 0, CServer::thread, &server, 0, &serverId);
		if (hServer == NULL) {
			// Failing starting up the server should not end
			// VaultSTER, the client part can still run.
			logger->Info("o Failed to start up the server.\n");
			return TRUE;
		}
		else {
			logger->Info("o Server started on port %d.\n", port);
			// set the server thread priority
			if (lowprior == 1)
				SetThreadPriority (hServer, THREAD_PRIORITY_BELOW_NORMAL);
			else if (lowprior == 2)
				SetThreadPriority (hServer, THREAD_PRIORITY_LOWEST);
		}
	}
	logger->Info("* Plugin initialized.");

	return true;
}

void NWNX4Vaultster::GetFunctionClass(char* fClass)
{
	strncpy_s(fClass, 128, "VAULTSTER", 9);
}


int NWNX4Vaultster::GetInt(char* sFunction, char* sParam1, int nParam2)
{
	logger->Trace("* Plugin GetInt(0x%x, %s, %d)", 0x0, sParam1, nParam2);
	int returnInt = 0;

	// std::string wxRequest(sFunction);
	std::string function(sFunction);
	std::string parameters(sParam1);

	if (function == "")
	{
		logger->Info("* Function not specified.");
		return NULL;
	}
	else if ((function == "GET") || (function == "SEND"))
	{
		char* pos[2];
		DWORD id;
		int i;

		// find a not busy client spot in the array
		for (i = 0; i < maxClients; i++) {
			if (clients[i].getStatus() == STATUS_OK)
				break;
		}

		if (i == maxClients) {
			logger->Info("o Too many clients already.\n");
			// can not help this client yet
			return -2;
		}
		else
			clients[i].setStatus (STATUS_BUSY);

		pos[0] = (char*)strchr (parameters.c_str(), '|');
		pos[1] = strchr (&pos[0][1], '|');
		if (!pos[0] || !pos[1]) {
			logger->Info("o Invalid parameter (%s)!\n", parameters.c_str());
			return -4;
		}

		// set up the client for running
		memset (clients[i].server, 0, 128);
		memset (clients[i].gamespy, 0, 128);
		memset (clients[i].character, 0, 32);
		strncpy_s (clients[i].server, 128, parameters.c_str(), pos[0] - &parameters[0]);
		strncpy_s (clients[i].gamespy, 128, &pos[0][1], pos[1] - pos[0] - 1);
		strcpy_s  (clients[i].character, 32, &pos[1][1]);

		Functions cmd = (_stricmp((char*)function.c_str(),"GET") == 0 ? Get : (_stricmp((char*)function.c_str(),"SEND") == 0 ? Put : Status));
		clients[i].setCommand (cmd);

		// start up the client thread
		clients[i].hThread = CreateThread (NULL, 0, CClient::thread, &clients[i], 0, &id);
		if (clients[i].hThread == NULL) {
			logger->Info("o Failed to start client thread!\n");
			returnInt = -1;
		}
		else {
			returnInt = i;
		}
		return returnInt;
	} else {
		// return status about given player
		int job = nParam2;
		int status = clients[job].getStatus ();
		logger->Info("o Retreived status for %s\\%s = %d\n", clients[job].gamespy, clients[job].character, status);
		if (status == STATUS_ERROR)
			clients[job].setStatus (STATUS_OK);
		return status;
	}
	return 0;
}

void NWNX4Vaultster::Log(const char *pcMsg, ...)
{
	va_list argList;
	char acBuffer[1024];

	// build up the string
	va_start(argList, pcMsg);
	vsprintf_s(acBuffer, 1024, pcMsg, argList);
	va_end(argList);

	// log string in file
	logger->Info(acBuffer);
}
