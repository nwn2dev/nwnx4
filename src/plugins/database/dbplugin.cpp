/***************************************************************************
    NWNX Database plugin - Generic base class for database plugins
    Copyright (C) 2007 Ingmar Stieger (Papillon, papillon@nwnx.org)

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

#include "dbplugin.h"
#include <cassert>

/***************************************************************************
    Implementation of DBPlugin
***************************************************************************/

DBPlugin::DBPlugin()
{
	header = "NWNX Base DB Plugin V.1.1";
	subClass = "DBPlugin";
	version = "1.1";
	description = "Overwrite this constructor in your derived plugin class.";
}

DBPlugin::~DBPlugin()
{
	delete config;
	logger->Info("* Plugin unloaded.");
}

bool DBPlugin::SetupLogAndIniFile(char* nwnxhome)
{
	assert(GetPluginFileName());

	/* Log file */
	std::string logfile(nwnxhome);
	logfile += "\\";
	logfile += GetPluginFileName();
	logfile += ".txt";
	logger = new LogNWNX(logfile);
	logger->Info(header.c_str());

	/* Ini file */
	std::string inifile(nwnxhome);
	inifile += "\\";
	inifile += GetPluginFileName();
	inifile += ".ini";
	logger->Trace("* reading inifile %s", inifile.c_str());

	config = new SimpleIniConfig(inifile);
	logger->Configure(config);
	return true;
}

void DBPlugin::GetFunctionClass(char* fClass)
{
	std::string myClass;
	if (config->Read("class", &myClass) )
	{
		logger->Info("* Registering under function class %s", myClass.c_str());
		strncpy_s(fClass, 128, myClass.c_str(), myClass.length());
	}
	else
	{
		strncpy(fClass, "SQL", 4);
	}
}

int DBPlugin::GetInt(char* sFunction, char* sParam1, int nParam2)
{
	logger->Trace("* Plugin GetInt(0x%x, %s, %d)", 0x0, sParam1, nParam2);

	std::string function(sFunction);

	if (function == "")
	{
		logger->Info("* Function not specified.");
		return -1;
	}

	if (function == "EXEC")
		return Execute(sParam1);
	else if (function == "FETCH")
		return Fetch(sParam1);
	else if (function == "GET AFFECTED ROWS")
		return GetAffectedRows();
	else if (function == "GET ERRNO")
		return GetErrno();
	else if (function == "GET INSID")
		return GetLastInsertID();

	return 0;

}

void DBPlugin::SetString(char* sFunction, char* sParam1, int nParam2, char* sValue)
{
	logger->Trace("* Plugin SetString(0x%x, %s, %d, %s)", 0x0, sParam1, nParam2, sValue);

	std::string function(sFunction);

	if (function == "")
	{
		logger->Info("* Function not specified.");
		return;
	}

	if (function == "EXEC")
		Execute(sParam1);
	else if (function == "SETSCORCOSQL")
		SetScorcoSQL(sParam1);
}

char* DBPlugin::GetString(char* sFunction, char* sParam1, int nParam2)
{
	logger->Trace("* Plugin GetString(0x%x, %s, %d)", 0x0, sParam1, nParam2);

	std::string function(sFunction);
	// std::string param1(sParam1);

	if (function == "")
	{
		logger->Info("* Function not specified.");
		return NULL;
	}

	if (function == "GETDATA")
	{
		GetData(nParam2, returnBuffer);
	}
	else if (function == "GET ESCAPE STRING")
		GetEscapeString(sParam1, returnBuffer);
	else if (function == "GET ERROR MESSAGE")
		return (char *) GetErrorMessage();
	else
	{
		// Process generic functions
		std::string query = ProcessQueryFunction(function.c_str());
		if (query != "")
		{
			sprintf_s(returnBuffer, MAX_BUFFER, "%s", query.c_str());
		}
		else
		{
			logger->Info("* Unknown function '%s' called.", function.c_str());
			return NULL;
		}
	}

	return returnBuffer;
}

bool DBPlugin::Execute(char* query)
{
	return FALSE;
}

int DBPlugin::GetAffectedRows()
{
	return -1;
}

int DBPlugin::GetLastInsertID()
{
	return 0;
}

int DBPlugin::Fetch(char* buffer)
{
	return -1;
}

int DBPlugin::GetData(int iCol, char* buffer)
{
	return -1;
}

int DBPlugin::GetErrno()
{
	return 0;
}

const char *DBPlugin::GetErrorMessage()
{
	return NULL;
}

void DBPlugin::GetEscapeString(char* str, char* buffer)
{
}

bool DBPlugin::WriteScorcoData(BYTE* pData, int Length)
{
	return 0;
}

BYTE* DBPlugin::ReadScorcoData(char *param, int *size)
{
	return NULL;
}

void DBPlugin::SetScorcoSQL(char *request)
{
	if(strlen(request) < MAXSQL)
		memcpy(scorcoSQL, request, strlen(request) + 1);
	else
		memcpy(scorcoSQL, request, MAXSQL);
}
