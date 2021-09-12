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

#include "chat.h"
#include "hook.h"
#include <cassert>

/***************************************************************************
    NWNX and DLL specific functions
***************************************************************************/

Chat* plugin;

DLLEXPORT Plugin* GetPluginPointerV2()
{
	return plugin;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		plugin = new Chat();

		char szPath[MAX_PATH];
		GetModuleFileName(hModule, szPath, MAX_PATH);
		plugin->SetPluginFullPath(szPath);
	}
	else if (ul_reason_for_call == DLL_PROCESS_DETACH)
	{
		delete plugin;
	}
    return TRUE;
}


/***************************************************************************
    Implementation of Chat Plugin
***************************************************************************/

Chat::Chat()
{
	header =
		"NWNX4 Chat Plugin V.0.3.6\n" \
		"(c) 2005-2006 by dumbo (dumbo@nm.ru)\n" \
		"(c) 2006-2007 by virusman (virusman@virusman.ru)\n" \
		"visit us at http://www.nwnx.org\n";

	description = "This plugin provides chat event hook.";

	subClass = "CHAT";
	version = "0.3.6";
}

Chat::~Chat()
{
	logger->Info("* Plugin unloaded.");
}

bool Chat::Init(char* nwnxhome)
{
	assert(GetPluginFileName());

	/* Log file */
	std::string logfile(nwnxhome);
	logfile.append("\\");
	logfile.append(GetPluginFileName());
	logfile.append(".txt");
	logger = new LogNWNX(logfile);
	logger->Info(header.c_str());

	//strcpy(chatScript, "chat_script");
	//strcpy(servScript, "chat_script");
	//maxMsgLen = 1024;
	//processNPC = 1;

	std::string inifile(nwnxhome);
	inifile.append("\\");
	inifile.append(GetPluginFileName());
	inifile.append(".ini");
	logger->Trace("* reading inifile %s", inifile.c_str());

	config = new SimpleIniConfig(inifile);
	logger->Configure(config);

	if (!config->Read("chat_script", &chatScript) )
	{
		logger->Info("* Chat script name not found in ini file");
		chatScript = "chat_script";
		logger->Info("* Using default chat script '%s'", chatScript.c_str());
	}

	if (!config->Read("server_script", &servScript) )
	{
		logger->Info("* Server script name not found in ini file");
		servScript = "chat_script";
		logger->Info("* Using default server script '%s'", servScript.c_str());
	}

	maxMsgLen = 1024;
	config->Read("max_msg_len", &maxMsgLen);
	processNPC = 0;
	config->Read("processnpc", &processNPC);
	ignore_silent = 0;
	config->Read("ignore_silent", &ignore_silent);

	lastMsg = new char[maxMsgLen+13];

	if(!HookFunctions())
	{
		logger->Info("* Hooking error.");
		return false;
	}

	logger->Info("* Plugin initialized.");
	return true;
}

void Chat::GetFunctionClass(char* fClass)
{
	strncpy_s(fClass, 128, "CHAT", 4);
}



void Chat::SetInt(char * sFunction, char * sParam1, int nParam2, int nValue)
{
	std::string function(sFunction);
	if (function == "LOGNPC")
	{
		processNPC = nValue;
	}
	else if (function == "IGNORESILENT")
	{
		ignore_silent = nValue; 
	}
	else if (function == "SUPRESS")
	{
		if (!scriptRun) return; // only in chat script
		if (nValue == 1) 
			supressMsg = 1;
	}

}

void Chat::SetString(char * sFunction, char * sParam1, int nParam2, char * value)
{
	std::string function(sFunction);
	if (function == "LOG") {
		if (!scriptRun ) return; // only in chat script
		logger->Info("%s", sParam1);
	
	}

}

int Chat::GetInt(char* sFunction, char* sParam1, int nParam2)
{
	std::string function = sFunction;
	if (function == "GETID") {
		logger->Debug("%s - getting ID for %d", sFunction, nParam2);
		if (nParam2) {  
			return GetID(nParam2);
		}
		logger->Debug("%s - returns -1", sFunction);
	} else  if (function == "SPEAK") { //makes someone say something
		logger->Info("o SPEAK: %s", sParam1);
		int oSender, oRecipient, nChannel;
		int nParamLen = strlen(sParam1);
		char *nLastDelimiter = strrchr(sParam1, '?');
		if (!nLastDelimiter || (nLastDelimiter-sParam1)<0)
		{
			logger->Err("%s - nLastDelimiter error", sFunction);
			return FALSE;
		}
		int nMessageLen = nParamLen-(nLastDelimiter-sParam1)+1;
		char *sMessage = new char[nMessageLen];
		if(sscanf(sParam1, "%x¬%x¬%d¬", &oSender, &oRecipient, &nChannel)<3)
		{
			logger->Err("o sscanf error");
			delete[] sMessage;
			return FALSE;
		}
		strncpy(sMessage, nLastDelimiter+1, nMessageLen-1);
		int nRecipientID = GetID(oRecipient);
		/*if((nChannel==4 || nChannel==20) && oRecipient <= 0x7F000000)
		{
			logger->Info("o oRecipient is not a PC");
			delete[] sMessage;
			return FALSE;
		}*/
		if(nChannel!=4 && nChannel!=20) nRecipientID=-1;
		logger->Info("o SendMsg(%d, %08lX, '%s', %d)", nChannel, oSender, sMessage, nRecipientID);
		int nResult = SendMsg(nChannel, oSender, sMessage, nRecipientID);
		logger->Err("o Return value: %d", nResult); //return value for full message delivery acknowledgement
		delete[] sMessage;
		if (nResult) 
			return TRUE;
		else
			return FALSE;
	}
	return FALSE;

}
char* Chat::GetString(char* sFunction, char* sParam1, int nParam2)
{
	std::string function = sFunction;
	returnBuffer[0] = 0;
	if (function == "TEXT" && scriptRun) {
		strncpy(returnBuffer, lastMsg, MAX_BUFFER);
		logger->Info("TEXT: '%s'", lastMsg);
	}
	return returnBuffer;

}


int Chat::ChatProc(const int mode, const int id, const char **msg, const int to)
{
	if ( !msg || !*msg ) return 0; // don't process null-string
	int cmode = mode & 0xFF;
	logger->Info("o CHAT: mode=%lX, from_oID=%08lX, msg='%s', to_ID=%08lX", cmode, id, *(char **)msg, to);
	//Log("o CHAT: mode=%lX, from_oID=%08lX, msg='%s', to_ID=%08lX\n", cmode, id, (char *)msg, to);
	sprintf(lastMsg, "%02d%10d", cmode, to);
	strncat(lastMsg, (char*)*msg, maxMsgLen);
	logger->Debug("lastMsg: %s", lastMsg);
	supressMsg = 0;
	if(ignore_silent && (cmode==0xD || cmode==0xE)) return 0;
	if ( (processNPC && id != 0x7F000000) || (!processNPC && (unsigned long)id >> 16 == 0x7FFF) )
	{
		RunScript((char *)chatScript.c_str(), id);
	}
	else if (cmode==5 && id==0x7F000000) {
		RunScript((char *)servScript.c_str(), to);
	}
	return supressMsg;
}

