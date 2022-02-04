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
	header      = "NWNX Base DB Plugin V.1.1";
	subClass    = "DBPlugin";
	version     = "1.1";
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
	if (config->Read("class", &myClass)) {
		logger->Info("* Registering under function class %s", myClass.c_str());
		strncpy_s(fClass, 128, myClass.c_str(), myClass.length());
	} else {
		strncpy(fClass, "SQL", 4);
	}
}

void DBPlugin::SetInt(char* sFunction, char* sParam1, int nParam2, int nValue)
{
	logger->Trace("* Plugin SetInt(%s, %s, %d, %d)", sFunction, sParam1, nParam2, nValue);

	std::string_view function {sFunction};
	if (function == "PSBindI") {
		int col    = (nParam2 >> 24) & 0xFF;
		int stmtID = nParam2 & 0x00FFFFFF;
		PrepBindInt(stmtID, col, nValue);
	} else if (function == "PSBindN") {
		int col    = (nParam2 >> 24) & 0xFF;
		int stmtID = nParam2 & 0x00FFFFFF;
		PrepBindNull(stmtID, col);
	} else {
		logger->Err("! Unknown SetInt function '%s'", sFunction);
	}
}

int DBPlugin::GetInt(char* sFunction, char* sParam1, int nParam2)
{
	logger->Trace("* Plugin GetInt(%s, %s, %d)", sFunction, sParam1, nParam2);

	std::string_view function {sFunction};

	if (function == "") {
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
	else if (function == "PSPrep")
		return PrepPrepareStatement(sParam1);
	else if (function == "PSFetch")
		return PrepFetch(nParam2);
	else if (function == "PSExec")
		return PrepExecute(nParam2);
	else if (function == "PSGetI") {
		int col    = (nParam2 >> 24) & 0xFF;
		int stmtID = nParam2 & 0x00FFFFFF;
		return PrepGetDataInt(stmtID, col);
	} else {
		logger->Err("! Unknown GetInt function '%s'", sFunction);
		return -1;
	}

	return 0;
}

void DBPlugin::SetFloat(char* sFunction, char* sParam1, int nParam2, float fValue)
{
	logger->Trace("* Plugin SetFloat(%s, %s, %d, %f)", sFunction, sParam1, nParam2, fValue);

	std::string_view function {sFunction};
	if (function == "PSBindF") {
		int col    = (nParam2 >> 24) & 0xFF;
		int stmtID = nParam2 & 0x00FFFFFF;
		PrepBindFloat(stmtID, col, fValue);
	} else {
		logger->Err("! Unknown SetFloat function '%s'", sFunction);
	}
}
float DBPlugin::GetFloat(char* sFunction, char* sParam1, int nParam2)
{
	logger->Trace("* Plugin GetFloat(%s, %s, %d)", sFunction, sParam1, nParam2);

	std::string_view function {sFunction};

	if (function == "") {
		logger->Info("* Function not specified.");
		return -1;
	}
	if (function == "PSGetF") {
		int col    = (nParam2 >> 24) & 0xFF;
		int stmtID = nParam2 & 0x00FFFFFF;
		return PrepGetDataFloat(stmtID, col);
	} else {
		logger->Err("! Unknown GetFloat function '%s'", sFunction);
		return -1;
	}

	return 0;
}

void DBPlugin::SetString(char* sFunction, char* sParam1, int nParam2, char* sValue)
{
	logger->Trace("* Plugin SetString(%s, %s, %d, %s)", sFunction, sParam1, nParam2, sValue);

	std::string_view function {sFunction};

	if (function == "") {
		logger->Info("* Function not specified.");
		return;
	}

	if (function == "EXEC")
		Execute(sParam1);
	else if (function == "SETSCORCOSQL")
		SetScorcoSQL(sParam1);
	else if (function == "PSBindS") {
		int col    = (nParam2 >> 24) & 0xFF;
		int stmtID = nParam2 & 0x00FFFFFF;
		PrepBindString(stmtID, col, sValue);
	} else
		logger->Err("! Unknown SetString function '%s'", sFunction);
}

char* DBPlugin::GetString(char* sFunction, char* sParam1, int nParam2)
{
	logger->Trace("* Plugin GetString(%s, %s, %d)", sFunction, sParam1, nParam2);

	std::string_view function {sFunction};

	if (function == "") {
		logger->Info("* Function not specified.");
		return NULL;
	}

	if (function == "GETDATA") {
		GetData(nParam2, this->returnBuffer);
	} else if (function == "GET ESCAPE STRING")
		GetEscapeString(sParam1, this->returnBuffer);
	else if (function == "GET ERROR MESSAGE")
		return (char*)GetErrorMessage();
	else if (function == "PSGetS") {
		int col    = (nParam2 >> 24) & 0xFF;
		int stmtID = nParam2 & 0x00FFFFFF;
		return PrepGetDataString(stmtID, col, this->returnBuffer);
	} else {
		// Process generic functions
		std::string query = ProcessQueryFunction(sFunction);
		if (query != "") {
			sprintf_s(this->returnBuffer, MAX_BUFFER, "%s", query.c_str());
		} else {
			logger->Info("* Unknown function '%s' called.", sFunction);
			return NULL;
		}
	}

	return this->returnBuffer;
}

bool DBPlugin::Execute(char* query)
{
	logger->Err("* %s not implemented for this plugin", __FUNCTION__);
	return FALSE;
}

int DBPlugin::GetAffectedRows()
{
	logger->Err("* %s not implemented for this plugin", __FUNCTION__);
	return -1;
}

int DBPlugin::GetLastInsertID()
{
	logger->Err("* %s not implemented for this plugin", __FUNCTION__);
	return 0;
}

int DBPlugin::Fetch(char* buffer)
{
	logger->Err("* %s not implemented for this plugin", __FUNCTION__);
	return -1;
}

int DBPlugin::GetData(int iCol, char* buffer)
{
	logger->Err("* %s not implemented for this plugin", __FUNCTION__);
	return -1;
}

int DBPlugin::GetErrno()
{
	logger->Err("* %s not implemented for this plugin", __FUNCTION__);
	return 0;
}

const char* DBPlugin::GetErrorMessage()
{
	logger->Err("* %s not implemented for this plugin", __FUNCTION__);
	return NULL;
}

void DBPlugin::GetEscapeString(char* str, char* buffer)
{
	logger->Err("* %s not implemented for this plugin", __FUNCTION__);
}

bool DBPlugin::WriteScorcoData(BYTE* pData, int Length)
{
	logger->Err("* %s not implemented for this plugin", __FUNCTION__);
	return 0;
}

BYTE* DBPlugin::ReadScorcoData(char* param, int* size)
{
	logger->Err("* %s not implemented for this plugin", __FUNCTION__);
	return NULL;
}

void DBPlugin::SetScorcoSQL(const char* request) { scorcoSQL = request; }

int DBPlugin::PrepPrepareStatement(const char* query)
{
	logger->Err("* %s not implemented for this plugin", __FUNCTION__);
	return -1;
}
bool DBPlugin::PrepBindString(int stmtID, int index, const char* value)
{
	logger->Err("* %s not implemented for this plugin", __FUNCTION__);
	return false;
}
bool DBPlugin::PrepBindInt(int stmtID, int index, int value)
{
	logger->Err("* %s not implemented for this plugin", __FUNCTION__);
	return false;
}
bool DBPlugin::PrepBindFloat(int stmtID, int index, float value)
{
	logger->Err("* %s not implemented for this plugin", __FUNCTION__);
	return false;
}
bool DBPlugin::PrepBindNull(int stmtID, int index)
{
	logger->Err("* %s not implemented for this plugin", __FUNCTION__);
	return false;
}
bool DBPlugin::PrepExecute(int stmtID)
{
	logger->Err("* %s not implemented for this plugin", __FUNCTION__);
	return false;
}
bool DBPlugin::PrepFetch(int stmtID)
{
	logger->Err("* %s not implemented for this plugin", __FUNCTION__);
	return false;
}
char* DBPlugin::PrepGetDataString(int stmtID, int iCol, char* buffer)
{
	logger->Err("* %s not implemented for this plugin", __FUNCTION__);
	return nullptr;
}
int DBPlugin::PrepGetDataInt(int stmtID, int iCol)
{
	logger->Err("* %s not implemented for this plugin", __FUNCTION__);
	return -1;
}
float DBPlugin::PrepGetDataFloat(int stmtID, int iCol)
{
	logger->Err("* %s not implemented for this plugin", __FUNCTION__);
	return -1;
}
