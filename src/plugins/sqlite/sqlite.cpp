/***************************************************************************
    NWNX SQLite - Database plugin for SQLite
    Copyright (C) 2006 Ingmar Stieger (Papillon, papillon@nwnx.org)

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

#include "sqlite.h"

/***************************************************************************
    NWNX and DLL specific functions
***************************************************************************/

SQLite* plugin;

DLLEXPORT Plugin* GetPluginPointerV2()
{
	return plugin;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		plugin = new SQLite();

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
    Implementation of SQLite Plugin
***************************************************************************/

SQLite::SQLite()
{
	header =
		"NWNX SQLite Plugin V.1.1.0\n" \
		"(c) 2007 by Ingmar Stieger (Papillon)\n" \
		"visit us at http://www.nwnx.org\n" \
		"(built using SQLite 3.3.17)\n";

	description =
		"This plugin provides database storage. It uses " \
	    "SQLite 3.3.17 as databaserver server and therefore is " \
		"very ease to configure and maintain.";

	subClass = "SQLite";
	version = "1.1.0";

	firstfetch = false;
	pStmt = NULL;
}

SQLite::~SQLite()
{
	Disconnect();
}

bool SQLite::Init(char* nwnxhome)
{
	SetupLogAndIniFile(nwnxhome);
	if (config->Read("file", &dbfile) )
	{
		logger->Info("* SQLite database file is %s", dbfile.c_str());
	}
	else
	{
		logger->Info("* SQLite database 'file=' setting not found in ini file");
		dbfile = nwnxhome;
		dbfile.append("\\sqlite.db");
		logger->Info("* Using default file %s", dbfile.c_str());
	}

	logger->Trace("* Opening database file %s", dbfile.c_str());
	if (!Connect())
	{
		return false;
	}

	logger->Info("* Plugin initialized.");
	return true;
}

bool SQLite::Connect()
{
	int rc;
	sqlite3_stmt* pStmt;

	rc = sqlite3_open((const char*)dbfile.c_str(), &sdb);
	if (rc)
	{
		logger->Info("* Could not open database: %s", sqlite3_errmsg(sdb));
	    sqlite3_close(sdb);
		sdb = NULL;
		return FALSE;
	}

	sqlite3_extended_result_codes(sdb, true);

	// begin implicit transaction
	rc = sqlite3_prepare(sdb, "BEGIN", -1, &pStmt, NULL);
	if (rc != SQLITE_OK)
		logger->Info("* %s", sqlite3_errmsg(sdb));
	else
	{
		rc = sqlite3_step(pStmt);
		if ((rc & 0xff) != SQLITE_DONE)
			logger->Info("* %s", sqlite3_errmsg(sdb));
	}
	SafeFinalize(&pStmt);
	return TRUE;
}

void SQLite::Disconnect()
{
	int rc;

	if (!sdb)
		return;

	// end implicit transaction
	SafeFinalize(&pStmt);
	rc = sqlite3_prepare(sdb, "COMMIT", -1, &pStmt, NULL);
	if (rc != SQLITE_OK)
		logger->Info("* %s", sqlite3_errmsg(sdb));
	else
	{
		rc = sqlite3_step(pStmt);
		if ((rc & 0xff) != SQLITE_DONE)
			logger->Info("* %s", sqlite3_errmsg(sdb));
	}
	SafeFinalize(&pStmt);
	sqlite3_close(sdb);
}

bool SQLite::Execute(char* query)
{
	int rc;
	sqlite3_stmt* pNewStmt;

	// prepare query
	logger->Info("* Executing: %s", query);
	rc = sqlite3_prepare(sdb, (const char*) query, -1, &pNewStmt, NULL);
	if (rc != SQLITE_OK)
	{
		logger->Err("! SQL Error: %s", sqlite3_errmsg(sdb));
		SafeFinalize(&pNewStmt);

		// throw away last resultset if a SELECT statement failed
		if (_strnicmp(query, "SELECT", 6) == 0)
			SafeFinalize(&pStmt);

		return FALSE;
	}
	// execute step
	rc = sqlite3_step(pNewStmt);
	switch(rc & 0xff)
	{
		case SQLITE_DONE:
			logger->Trace("* Step: SQLITE_DONE");
			if (sqlite3_column_name(pNewStmt,0) != NULL)
			{
				// pNewStmt returned an empty resultset (as opposed
				// to a query that returns no result set at all, like
				// UPDATE). Empty pStmt so SQLFetch knows there is no data 
				SafeFinalize(&pStmt);
			}
			SafeFinalize(&pNewStmt);
		break;
		case SQLITE_ROW:
			logger->Trace("* Step: SQLITE_ROW");
			SafeFinalize(&pStmt);
			pStmt = pNewStmt;
			firstfetch = true;
		break;
		case SQLITE_ERROR:
			SafeFinalize(&pNewStmt);

			// For COMMIT: try to repeat the command with all resultsets closed
			int errorno = sqlite3_errcode(sdb);
			if (errorno == SQLITE_ERROR_OPENSTMT)
			{
				logger->Trace("* Closing open resultset.");
				SafeFinalize(&pStmt);
				rc = sqlite3_prepare(sdb, (const char*) query, -1, &pNewStmt, NULL);
				rc = sqlite3_step(pNewStmt) & 0xff;
				SafeFinalize(&pNewStmt);
			}

			if (rc == SQLITE_ERROR)
			{
				logger->Err("! SQL Error: %s (%d)", sqlite3_errmsg(sdb), errorno);
			}
			return FALSE;
		break;
	}

	return TRUE;
}

int SQLite::GetAffectedRows()
{
	return sqlite3_changes(sdb);
}

int SQLite::Fetch(char* buffer)
{
	int rc;
	if (firstfetch)
	{
		firstfetch = false;
		rc = SQLITE_ROW;	
	}
	else
	{
		logger->Trace("* Fetch: fetching next result row");
		rc = sqlite3_step(pStmt);
		if ((rc & 0xff) == SQLITE_ERROR)
		{
			logger->Err("! SQL Error (fetch): %s", sqlite3_errmsg(sdb));
		}
	}

	if ((rc & 0xff) == SQLITE_ROW)
	{
		return 1;
	}
	else
	{
		SafeFinalize(&pStmt);
		return 0;
	}
}

int SQLite::GetData(int iCol, char* buffer)
{
	const char* pCol;

	if (!pStmt)
	{
		logger->Trace("* GetData: No valid statement prepared.");
		nwnxcpy(buffer, "");
		return -1;
	}

	logger->Trace("* GetData: Get column %d, buffer size %d bytes", iCol, MAX_BUFFER);

	pCol = (const char*) sqlite3_column_text(pStmt, iCol);
	if (pCol)
	{
		nwnxcpy(buffer, pCol);
		logger->Info("* Returning: %s", buffer);
		return 0;
	}
	else
	{
		nwnxcpy(buffer, "");
		logger->Info("* Returning: (empty)");
		return -1;
	}
}

void SQLite::SafeFinalize(sqlite3_stmt** pStmt)
{
	if (*pStmt)
	{
		sqlite3_finalize(*pStmt);
		*pStmt = NULL;
	}
}

void SQLite::GetEscapeString(char* str, char* buffer)
{
	if (*str == NULL)
	{
		nwnxcpy(buffer, "");
		return;
	}

	size_t len = strlen(str);
	char* to = sqlite3_mprintf("%q", str);
	nwnxcpy(buffer, to);
	sqlite3_free(to);
}

int SQLite::GetErrno()
{
	return sqlite3_errcode(sdb);
}

const char *SQLite::GetErrorMessage()
{
	return sqlite3_errmsg(sdb);
}
