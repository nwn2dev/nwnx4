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

#include "nwn2heap.h"

#include <filesystem>

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
		"(built using SQLite " SQLITE_VERSION ")\n";

	description =
		"This plugin provides database storage. It uses " \
	    "SQLite " SQLITE_VERSION " as database server and therefore is " \
		"very ease to configure and maintain.";

	subClass = "SQLite";
	version = "1.1.0";

	firstfetch = false;
	pStmt = nullptr;
}

SQLite::~SQLite()
{
	Disconnect();
}

bool SQLite::Init(char* nwnxhome)
{
	// Read config
	SetupLogAndIniFile(nwnxhome);
	config->Read("file", &dbfile, std::string("sqlite.db"));
	config->Read("wrap_transaction", &wrapTransaction, false);

	// Make sqlite database path relative to nwnx directory
	auto path = std::filesystem::path(dbfile);
	if(path.is_relative())
		path = std::filesystem::path(nwnxhome) / dbfile;
	dbfile = path.string();

	// Open database
	logger->Info("* SQLite database file is %s", dbfile.c_str());
	if (!Connect())
	{
		return false;
	}

	logger->Info("* Plugin initialized.");
	return true;
}

bool SQLite::Connect()
{
	// Open database
	int rc = sqlite3_open(dbfile.c_str(), &sdb);
	if (rc != SQLITE_OK)
	{
		logger->Err("* Could not open database: %s", sqlite3_errmsg(sdb));
	    sqlite3_close(sdb);
		sdb = nullptr;
		return false;
	}

	sqlite3_extended_result_codes(sdb, true);

	if(wrapTransaction){
		// begin implicit transaction
		rc = sqlite3_prepare_v2(sdb, "BEGIN", -1, &pStmt, nullptr);
		if (rc != SQLITE_OK)
			logger->Info("* %s", sqlite3_errmsg(sdb));
		else
		{
			rc = sqlite3_step(pStmt);
			if ((rc & 0xff) != SQLITE_DONE)
				logger->Info("* %s", sqlite3_errmsg(sdb));
		}
		SafeFinalize(&pStmt);
	}

	return true;
}

void SQLite::Disconnect()
{
	// End statement if any
	SafeFinalize(&pStmt);

	if(wrapTransaction){
		// Commit on exit
		auto rc = sqlite3_prepare_v2(sdb, "COMMIT", -1, &pStmt, nullptr);
		if (rc != SQLITE_OK)
			logger->Info("* %s", sqlite3_errmsg(sdb));
		else
		{
			rc = sqlite3_step(pStmt);
			if ((rc & 0xff) != SQLITE_DONE)
				logger->Info("* %s", sqlite3_errmsg(sdb));
		}
		SafeFinalize(&pStmt);
	}

	// Close sqlite connection
	if (sdb != nullptr)
		sqlite3_close(sdb);
}

bool SQLite::Execute(char* query)
{
	int rc;
	sqlite3_stmt* pNewStmt;

	// prepare query
	logger->Info("* Executing: %s", query);
	rc = sqlite3_prepare_v2(sdb, query, -1, &pNewStmt, nullptr);
	if (rc != SQLITE_OK)
	{
		logger->Err("! SQL Error: %s", sqlite3_errmsg(sdb));
		SafeFinalize(&pNewStmt);

		// throw away last resultset if a SELECT statement failed
		if (_strnicmp(query, "SELECT", 6) == 0)
			SafeFinalize(&pStmt);

		return FALSE;
	}

	// execute query
	rc = sqlite3_step(pNewStmt);
	switch(rc & 0xff)
	{
		case SQLITE_DONE:
			logger->Trace("* Step: SQLITE_DONE");
			if (sqlite3_column_name(pNewStmt,0) != nullptr)
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
				rc = sqlite3_prepare(sdb, (const char*) query, -1, &pNewStmt, nullptr);
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
		// sqlite3_step has already been called in SQLite::Execute
		firstfetch = false;
		rc = SQLITE_ROW;	
	}
	else
	{
		// Execute step again to go to next result
		logger->Trace("* Fetch: fetching next result row");
		rc = sqlite3_step(pStmt);
		if ((rc & 0xff) == SQLITE_ERROR)
		{
			logger->Err("! SQL Error (fetch): %s", sqlite3_errmsg(sdb));
		}
	}

	if ((rc & 0xff) == SQLITE_ROW)
	{
		// There is a row available
		return true;
	}
	else
	{
		// No more rows / there was an error
		SafeFinalize(&pStmt);
		return false;
	}
}

int SQLite::GetData(int iCol, char* buffer)
{
	const char* pCol;

	// Check that a statement has been executed previously
	if (!pStmt)
	{
		logger->Trace("* GetData: No valid statement prepared.");
		nwnxcpy(buffer, "");
		return -1;
	}

	logger->Trace("* GetData: Get column %d, buffer size %d bytes", iCol, MAX_BUFFER);

	pCol = (const char*) sqlite3_column_text(pStmt, iCol);
	if (pCol != nullptr)
	{
		// There is some data
		nwnxcpy(buffer, pCol);
		logger->Info("* Returning: %s", buffer);
		return 0;
	}
	else
	{
		// There is no data / value is null
		nwnxcpy(buffer, "");
		logger->Info("* Returning: (empty)");
		return -1;
	}
}

void SQLite::SafeFinalize(sqlite3_stmt** pStmt)
{
	if (*pStmt != nullptr)
	{
		sqlite3_finalize(*pStmt);
		*pStmt = nullptr;
	}
}

void SQLite::GetEscapeString(char* str, char* buffer)
{
	if (*str == '\0')
	{
		nwnxcpy(buffer, "");
		return;
	}

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

int SQLite::GetLastInsertID()
{
	return sqlite3_last_insert_rowid(sdb);
}

bool SQLite::WriteScorcoData(BYTE* pData, int Length)
{
	logger->Info("* SCO query: %s", scorcoSQL);

	// End any previous SQL statement
	SafeFinalize(&pStmt);

	// Prepare new SQL statement
	auto rc = sqlite3_prepare_v2(sdb, scorcoSQL, -1, &pStmt, nullptr);
	if (rc != SQLITE_OK)
	{
		logger->Err("! SQL Error: Cannot prepare query %s: %s", scorcoSQL, sqlite3_errmsg(sdb));
		SafeFinalize(&pStmt);
		return false;
	}

	// Bind blob parameter
	rc = sqlite3_bind_blob(pStmt, 1, pData, Length, SQLITE_STATIC);
	if (rc != SQLITE_OK)
	{
		logger->Err("! SQL Error: Cannot bind blob: %s", sqlite3_errmsg(sdb));
		SafeFinalize(&pStmt);
		return false;
	}

	// execute step
	rc = sqlite3_step(pStmt);
	switch(rc & 0xff)
	{
		case SQLITE_DONE:
			// successfully inserted
			logger->Trace("* Step: SQLITE_DONE");
			SafeFinalize(&pStmt);
			break;

		case SQLITE_ERROR:
			logger->Err("! SQL Error: %s (%d)", sqlite3_errmsg(sdb), sqlite3_errcode(sdb));
			SafeFinalize(&pStmt);
			return false;
	}

	return true;
}

BYTE* SQLite::ReadScorcoData(char *param, int *size)
{
	logger->Info("* RCO query: %s", scorcoSQL);

	// Prepare SQL statement
	if (strcmp(param, "FETCHMODE") != 0){
		SafeFinalize(&pStmt);

		auto rc = sqlite3_prepare_v2(sdb, scorcoSQL, -1, &pStmt, nullptr);
		if (rc != SQLITE_OK)
		{
			logger->Err("! SQL Error: Cannot prepare query %s: %s", scorcoSQL, sqlite3_errmsg(sdb));
			SafeFinalize(&pStmt);
			return nullptr;
		}
		firstfetch = false;
	}

	// Execute step
	Fetch(nullptr);

	// Extract raw object data
	auto value = sqlite3_column_blob(pStmt, 1);
	if (value != nullptr)
	{
		auto valueLen = sqlite3_column_bytes(pStmt, 1);

		NWN2_HeapMgr *pHeapMgr = NWN2_HeapMgr::Instance();
		NWN2_Heap *pHeap = pHeapMgr->GetDefaultHeap();
		void* buf = pHeap->Allocate(valueLen);

		// Could not allocate memory
		if (buf == nullptr)
			return nullptr;

		// Copy raw object data to nwn2
		memcpy(buf, value, valueLen);
		*size = valueLen;

		return (BYTE*)buf;
	}
	else
	{
		logger->Info("* Empty RCO resultset");
		return nullptr;
	}
}
