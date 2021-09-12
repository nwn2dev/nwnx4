/***************************************************************************
    Client.cpp - Implementation of the client class for Windows.
    Copyright (C) 2004 Jeroen Broekhuizen (jeroen@nwnx.org)
    
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
 ***************************************************************************
    Updates:
	04-07-2005: 
	  - Added extra logging information
    05-07-2005:
	  - Fixed return value of transmitFile function. Now realy returns
	    value of the socket transmision functions.
	  - Added extra check for dots in the character name in the 
	    createPattern function.
	  - Added extra logging.

 ***************************************************************************/

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <time.h>
#include <fstream>
using namespace std;

#ifndef STANDALONE
#include "NWNX4vaultster.h"
#else
#include <stdio.h>
#include <windows.h>
#endif

#include "client.h"

typedef unsigned char uchar;
typedef unsigned long ulong;

#ifndef STANDALONE
// use this variable for logging purposes
extern NWNX4Vaultster* plugin;
#define Log plugin->Log
#else
#define Log printf
#endif

CClient::CClient(void)
{
   status = STATUS_OK;
}

CClient::~CClient(void)
{
}

char CClient::serverVault[] = "";
char CClient::cryptoKey[] = "";
char CClient::password[] = "";
int CClient::port = 5100;

void CClient::setCommand (Functions cmd)
{
   switch (cmd) {
   case Get:
      command = CMD_GET;
      break;
   case Put:
      command = CMD_SEND;
      break;
   }
}

void CClient::setStatus (int stat)
{
   status = stat;
}
   
int CClient::getStatus ()
{
   return status;
}

void CClient::setPort (int p)
{
   // save the port number, which is used to connect
   port = p;
}

void CClient::setServervault (char* vault)
{
	// save a static string with the servervault directory
	strcpy_s(serverVault, 256, vault); 
}

void CClient::setCryptoKey (char* key)
{
	// save a copy of the blowfish key
	strcpy_s(cryptoKey, 128, key);
}

void CClient::setPassword (char* pwd)
{
	// save the password for identification
	strcpy_s (password, 64, pwd);
}

DWORD CClient::thread (LPVOID param)
{
   CClient* client = (CClient*)param;

   if (!client->run ()) {
      client->setStatus (STATUS_ERROR);
	  Log ("o There was an error during the transmission.\n");
   }
   else {
      client->setStatus (STATUS_OK);
	  Log ("o Portal successfully finished.\n");
   }

   int stat = client->getStatus ();
   return stat;
}

bool CClient::run ()
{
	char filename[256], pattern[32];
	fish.Initialize ((unsigned char*)cryptoKey, strlen (cryptoKey));

	memset (filename, 0, 256);
	memset (pattern, 0, 32);

	// make it linux compatible:
	_strlwr_s(character, 32);

	// try to find the latest file
	createPattern (pattern, 32);
	Log ("o Start searching for %s\n", pattern);
	if (!findLatestFile (pattern, 32, filename, 256)) {
		Log ("o Could not find latest file for %s\\%s!\n", gamespy, character);
		return false;
	}

	Log ("o Trying to connect to server %s on port %d...\n", server, port);
	// try to connect to the server
	socket.Create ();
	if (!socket.Connect (server, port)) {
		Log ("o Could not connect to %s on port %d.\n", server, port);
		return false;
	}
	Log ("o Connected: ");

	if (!handShake ()) {
		// wrong password, bail out!
		Log ("o Wrong password, can not finish job.\n");
		socket.Close ();
		return false;
	}
	Log ("ready for file transmission\n");

	if (!transmitFile (filename)) {
		// to bad! it failed
		Log ("o Failed to send over file '%s'.\n", filename);
		socket.Close ();
		return false;
	}
	Log ("o Job ready\n");

	socket.Close ();
	return true;
}

void CClient::createPattern (char* pattern, int patternLen)
{
	// make sure there is no dot in the filename
	char* dot = strchr (character, '.');
	if (dot != NULL)
		dot[0] = 0;

	// determine the length of the current pattern
	// Edit by Mithreas - cap the pattern length to 12 (not 14).  This is because
	// if a player makes several chars with the same name, and their name is 14
	// chars long, their filenames are (e.g.)
	// daedinangthalion.bic
	// daedinangthalio1.bic
	// i.e. the last character(s) may be replaced by numbers.
	int characterLen = strlen (character);
	int patternLenMax = (characterLen >= 12) ? 12 : characterLen;
	
	// create the pattern string
	strncpy_s(pattern, patternLen, character, patternLenMax);
	strcat_s (pattern, patternLen, "*.bic");
}

bool CClient::findLatestFile (char* pattern, int patternLen, char* filename, int filenameLen)
{
	WIN32_FIND_DATA Search;
	WIN32_FILE_ATTRIBUTE_DATA fad;
	char path[MAX_PATH];
	bool found = false;

	// try to find the first file in the dir
	sprintf_s(path, MAX_PATH, "%s\\%s\\%s", serverVault, gamespy, pattern);
	Log ("o Searching for %s\n", path);
	HANDLE hSearch = FindFirstFile (path, &Search);
	if (hSearch != INVALID_HANDLE_VALUE) {
		FILETIME latestFT;
		char buffer[256];

		// get date from this first file
		sprintf_s(buffer, 256, "%s\\%s\\%s", serverVault, gamespy, Search.cFileName);
		GetFileAttributesEx (buffer, GetFileExInfoStandard, &fad);
		latestFT = fad.ftLastAccessTime;
		strcpy_s (filename, filenameLen, buffer);

		Log ("o Found at least one file : %s.\n", filename);
				
		// check the rest of the files (if any)
		while (FindNextFile (hSearch, &Search)) {
			sprintf_s(buffer, 256, "%s%s\\%s", serverVault, gamespy, Search.cFileName);
			GetFileAttributesEx (buffer, GetFileExInfoStandard, &fad);
			if (CompareFileTime (&fad.ftLastAccessTime, &latestFT) > 0) {
				latestFT = fad.ftLastAccessTime;
				strcpy_s(filename, filenameLen, buffer);
			}
		}

		// we at least found one, otherwise we wouldn't be here
		found = true;
	}

	Log ("o Will use file : %s.\n", filename);

	// finalize
	FindClose (hSearch);
	return found;
}

void CClient::generatePassword (char pwd[128])
{
	memset (pwd, 0, 128);
	strcpy_s (pwd, 128, password);
	
	// build the dummy part of the password
	srand ((unsigned)time(NULL));
	for (int i = strlen(password); i < 128; i++)
		pwd[i] = 33 + (rand() % 90);
}

bool CClient::handShake ()
{
	int ret = 0;
	char pwd[128];
	uchar encrypted[128];
	
	// send the command to the server
	socket.Send (command);
	socket.Receive (ret);
	if (ret == INFO_NACK)
		return false;
	
	// now encrypt the password and send it over
	generatePassword (pwd);
	fish.Encode ((uchar*)pwd, encrypted, 128);
	socket.Send (encrypted, 128);
	
	// hopefully the password is accepted
	socket.Receive (ret);
	if (ret == INFO_NACK)
		return false;
	return true;
}

bool CClient::transmitFile (char* localFile)
{
	int ret = 0;
	sendEncrypted (gamespy);
	sendEncrypted (character);
	Log ("o Character is : %s.\n", character);
	socket.Receive (ret);
	if (ret == INFO_NACK)
		return false;
		
	// now perform the requested job
	switch (command) {
	case CMD_SEND:
		Log ("o Sending file\n");
		return socket.SendFile (localFile);
	case CMD_GET:
		Log ("o Receiving file\n");
		return socket.ReceiveFile (localFile);
	}

	// invalid command (should not be able to get here).
	return false;
}

void CClient::sendEncrypted (char* str)
{
	uchar encrypted[256];
	
	// encode the string
	ulong patternLen = strlen (str);
	ulong len = fish.GetOutputLength (patternLen);
	fish.Encode ((uchar*)str, encrypted, patternLen);

   // send the data to the server
	socket.Send (len);
	socket.Send (encrypted, len);
}