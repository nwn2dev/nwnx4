#include "Server.h"
#include "Client.h"
#include <windows.h>
#include <stdio.h>
#include "IniFile.h"

#define Log printf

void main(int argc, char **argv) {
	char buf[284], key[15];
	bool startServer, validateClient;
	DWORD serverId;
	CServer server;

	// write copy information to the log file
	Log ("NWNX2 Vaultster version 1.4.6 for Windows.\n");
	Log ("Copyright 2004-2005 (C) Jeroen Broekhuizen\n\n");

	// start up WSA (winsock library)
	if (!CSock::StartWSA ()) {
		Log ("Could not initialize the winsock library\n");
		exit(1);
	}

	// load in the settings
	CIniFile iniFile("vaultster.ini");
	startServer = iniFile.ReadInteger ("VAULTSTER", "startserver", 1);
	validateClient = iniFile.ReadInteger ("VAULTSTER", "validateclient", 1);
	server.setValidate (validateClient);
	int maxClients = iniFile.ReadInteger ("VAULTSTER", "MaxClients", 10);
	server.setMaxClients (maxClients);
	iniFile.ReadString ("VAULTSTER", "Path", buf, MAX_PATH, ".");
	// remove trailing backspace
	if (buf[strlen (buf)-1] == '\\')
		buf[strlen (buf)-1] = 0;
	CClient::setServervault (buf);
	server.setServervault (buf);
	iniFile.ReadString ("VAULTSTER", "Key", buf, 128, "*invalid");
	CClient::setCryptoKey (buf);
	server.setCryptoKey (buf);
	iniFile.ReadString ("VAULTSTER", "Password", buf, 128, "PWD!@");
	CClient::setPassword (buf);
	server.setPassword (buf);
	int port = iniFile.ReadInteger ("VAULTSTER", "Port", 5100);
	CClient::setPort (port);
	server.setPort (port);
	int lowprior = iniFile.ReadInteger ("VAULTSTER", "Low prior", 0);

	// load in the know servers
	if (validateClient) {
		int serverCount = iniFile.ReadInteger ("VAULTSTER", "Count", 0);
		Log ("o Loading in %d known servers.\n", serverCount);
		for (int i = 0; i < serverCount; i++) {
			sprintf (key, "Server%d", i+1);
			iniFile.ReadString ("VAULTSTER", key, buf, 284, "");
			if (buf[0] != 0) server.addKnownServer (buf);
			Log ("o Loaded: %s\n", buf);
		}
	}

	// initialize the clients
	CClient *clients = new CClient[maxClients];

	// start up the server
	if (startServer) {
		HANDLE hServer = CreateThread (NULL, 0, CServer::thread, &server, 0, &serverId);
		if (hServer == NULL) {
			// Failing starting up the server should not end
			// VaultSTER, the client part can still run.
			Log ("o Failed to start up the server.\n");
			exit(1);
		}
		else {
			Log ("o Server started on port %d.\n", port);
			// set the server thread priority
			if (lowprior == 1)
				SetThreadPriority (hServer, THREAD_PRIORITY_BELOW_NORMAL);
			else if (lowprior == 2)
				SetThreadPriority (hServer, THREAD_PRIORITY_LOWEST);
		}
	}

	bool cont = true;
	while(cont) {
		printf("Type:\n");
		printf("1- to upload a file.\n");
		printf("2- to download a file.\n");
		printf("3- to quit.\n");
		int val;
		scanf("%d", &val);
		while(val != 1 && val != 2 && val != 3) {
			printf("Please type 1, 2 or 3\n");
			scanf("%d", &val);
		}
		fflush(stdin);
		if (val == 3) break;
		
		/*
		printf("type in the server name:");
		char server[1024];
		gets(server);
		printf("type in the GSID:");
		char gsid[1024];
		gets(gsid);
		printf("type in the character name:");
		char character[1024];
		gets(character);
		*/
		char *server="vault.alandfaraway.org";
		char *gsid="Patrice Torguet";
		char *character="hialmar";
		
		char* pos[2];
		DWORD id;
		int i;

		// find a not busy client spot in the array
		for (i = 0; i < maxClients; i++) {
			if (clients[i].getStatus() == STATUS_OK)
				break;
		}

		if (i == maxClients) {
			Log ("o Too many clients already.\n");
			// can not help this client yet
			continue;
		}
		else
			clients[i].setStatus(STATUS_BUSY);

		// set up the client for running
		memset (clients[i].server, 0, 128);
		memset (clients[i].gamespy, 0, 128);
		memset (clients[i].character, 0, 32);
		strcpy (clients[i].server, server);
		strcpy (clients[i].gamespy, gsid);
		strcpy (clients[i].character, character);
		// for testing only
		//clients[i].setPort(5100);
		Functions cmd = (val == 2 ? Get : (val == 1 ? Put : Status));
		clients[i].setCommand (cmd);

		// start up the client
		if (CClient::thread(&clients[i]) == STATUS_OK)
			printf("Transfer completed\n");
		else printf("There was an error\n");
	}
}