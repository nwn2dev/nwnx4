/***************************************************************************
    NWNX Mysql - Database plugin for MySQL
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

#include "mysql_plugin.h"

#include "../../nwnx_version.h"
#include <cassert>
#include <mysql/errmsg.h>

#include "nwn2heap.h"

/***************************************************************************
    NWNX and DLL specific functions
***************************************************************************/

MySQL* plugin;

DLLEXPORT Plugin* GetPluginPointerV2() { return plugin; }

bool APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
		plugin = new MySQL();

		char szPath[MAX_PATH];
		GetModuleFileNameA(hModule, szPath, MAX_PATH);
		plugin->SetPluginFullPath(szPath);
	} else if (ul_reason_for_call == DLL_PROCESS_DETACH) {
		delete plugin;
	}
	return TRUE;
}

/***************************************************************************
    Implementation of MySQL Plugin
***************************************************************************/

MySQL::MySQL()
    : DBPlugin()
{
	header = "NWNX MySQL Plugin " NWNX_VERSION_INFO "\n"
	         "(c) 2007 by Ingmar Stieger (Papillon)\n"
	         "(c) 2008 by virusman\n"
	         "Source: https://github.com/nwn2dev/nwnx4 \n"
	         "MySQL Connector: " MARIADB_CLIENT_VERSION_STR;

	description = "This plugin provides database storage. It uses "
	              "MySQL 4 or 5 as database server.";

	subClass = "MySQL";
	version  = NWNX_VERSION_INFO;

	result = NULL;
	row    = NULL;
}

MySQL::~MySQL()
{
	Disconnect();

	// Clean prepared statement data if any
	for (size_t i = 0; i < m_prepStmts.size(); i++) {
		FreeResBuffer(i + 1);
	}
}

bool MySQL::Init(char* nwnxhome)
{
	SetupLogAndIniFile(nwnxhome);

	config->Read("server", &server, std::string("localhost"));
	config->Read("port", &port, 0);
	config->Read("user", &user, std::string(""));
	config->Read("password", &password, std::string(""));
	config->Read("schema", &schema, std::string("nwn2"));
	config->Read("charset", &charset, std::string(""));

	logger->Info("* Server: address='%s' port=%d", server.c_str(), port);
	logger->Info("* Authentication: user='%s' password='%s'", user.c_str(),
	             password.size() > 0 ? "REDACTED" : "None");
	logger->Info("* Database: schema='%s' charset='%s'", schema.c_str(), charset.c_str());

	if (!Connect()) {
		logger->Info("* Connection to MySQL server failed:\n  %s", mysql_error(&mysql));
	}

	logger->Info("* Plugin initialized.");

	return true;
}

bool MySQL::Connect()
{
	logger->Info("* Connecting to MySQL server...");
	// initialize the mysql structure
	if (!mysql_init(&mysql)) {
		logger->Err("!   Failed to init MySQL");
		return false;
	}

	// try to connect to the mysql server
	connection = mysql_real_connect(&mysql, server.c_str(), user.c_str(), password.c_str(),
	                                schema.c_str(), port, NULL, CLIENT_MULTI_STATEMENTS);
	if (connection == NULL) {
		mysql_close(&mysql);
		logger->Err("!   Failed to connect to server: %s", mysql_error(&mysql));
		return false;
	}

	// Set char set
	if (charset.size() > 0) {
		if (mysql_set_character_set(&mysql, charset.c_str()) != 0) {
			logger->Warn("* Cannot set character set '%s': %s", charset.c_str(),
			             mysql_error(&mysql));
		}
	}

	// Recover prepared statements
	for (size_t stmtID = 0; stmtID < m_prepStmts.size(); stmtID++) {
		auto& prepStmt = m_prepStmts[stmtID];

		auto stmt = mysql_stmt_init(&mysql);
		if (mysql_stmt_prepare(stmt, prepStmt->query.c_str(), -1) != 0) {
			logger->Err("[Stmt %d] Failed to prepare statement. Error %d: %s", stmtID,
			            mysql_stmt_errno(stmt), mysql_stmt_error(stmt));
			mysql_stmt_close(stmt); // ignore closing error, just deallocate
		} else {
			logger->Debug("[Stmt %d] re-prepared statement after disconnection", stmtID);
			prepStmt->stmt = stmt;
		}
	}

	return true;
}

void MySQL::Disconnect()
{
	// close the connection
	mysql_close(&mysql);
	mysql_close(connection);
	connection = NULL;
	logger->Info("* Disconnected MySQL server");
}

bool MySQL::Reconnect()
{
	logger->Info("* Reconnecting to MySQL server...");
	Disconnect();
	if (!Connect()) {
		return false;
	} else {
		logger->Info("* Connection to MySQL server succeeded.");
		return true;
	}
}

bool MySQL::Execute(char* query)
{
	MYSQL_RES* newResult;

	if (!connection) {
		if (!Reconnect()) {
			logger->Err("! Error: Not connected.");
			return FALSE;
		}
	}

	// eat any leftover resultsets so mysql does not get out of sync
	while (mysql_more_results(&mysql)) {
		mysql_next_result(&mysql);
		newResult = mysql_store_result(&mysql);
		mysql_free_result(newResult);
	}

	// execute the query
	logger->Info("* Executing: %s", query);
	if (mysql_query(connection, (const char*)query) != 0) {
		unsigned int error_no = mysql_errno(&mysql);
		logger->Err("! SQL Error: %s (%lu).", mysql_error(&mysql), error_no);

		// throw away last resultset if a SELECT statement failed
		if (_strnicmp(query, "SELECT", 6) == 0) {
			mysql_free_result(result);
			result     = NULL;
			row        = NULL;
			num_fields = 0;
		}

		if ((error_no == CR_SERVER_GONE_ERROR) && (Reconnect())) {
			// Retry query with new connection
			if (mysql_query(connection, (const char*)query) != 0)
				return FALSE;
		} else {
			return FALSE;
		}
	}

	// store the resultset in local memory
	newResult = mysql_store_result(&mysql);
	if (newResult == NULL) {
		if (mysql_field_count(&mysql) != 0) {
			// SELECT with an empty result set
			logger->Trace("* Retrieved an empty resultset (mysql_query)");
			mysql_free_result(result);
			result     = NULL;
			row        = NULL;
			num_fields = 0;
		} else {
			// NOT a SELECT like command
			logger->Trace("* Retrieved an invalid resultset (NO SELECT) (mysql_query)");

			// Try to advance to a non-empty resultset, if there is one.
			// This allows for calls to SQLFetch() to succeed automatically,
			// even if the first n resultsets are empty or not valid.
			newResult = AdvanceToNextValidResultset();
			if (newResult) {
				mysql_free_result(result);
				result     = newResult;
				row        = NULL;
				num_fields = mysql_num_fields(result);
			}
		}

		if (mysql_errno(&mysql) != 0) {
			logger->Err("! Error (mysql_store_result): %s", mysql_error(&mysql));
			return FALSE;
		}
	} else {
		// successfully retrieved the resultset
		mysql_free_result(result);
		result     = newResult;
		row        = NULL;
		num_fields = mysql_num_fields(result);
	}

	return TRUE;
}

int MySQL::Fetch(char* buffer)
{
	if (!connection) {
		logger->Err("! Error (Fetch): Not connected.");
		return -1;
	}

	// If the parameter is NEXT, try to load the next
	// resultset from the last multi-statement query.
	// If it is empty, try to fetch the next row
	// from the current resultset.
	if (strcmp(buffer, "NEXT") == 0) {
		logger->Trace("* Trying to fetch the next resultset");

		mysql_free_result(result);
		result     = NULL;
		num_fields = 0;

		result = AdvanceToNextValidResultset();
		if (result) {
			num_fields = mysql_num_fields(result);
		}
	}

	if (result) {
		row = mysql_fetch_row(result);
		if (row) {
			logger->Trace("* Fetch returns a row.");
			return 1;
		}
	}

	row = NULL;
	logger->Trace("* Fetch returns no row.");
	nwnxcpy(buffer, "");
	return 0;
}

int MySQL::GetData(int iCol, char* buffer)
{
	if (!row) {
		logger->Trace("! GetData: No valid row in resultset.");
		nwnxcpy(buffer, "");
		return -1;
	}

	logger->Trace("* GetData: Get column %d, buffer size %d bytes", iCol, MAX_BUFFER);

	if ((iCol < (int)num_fields) && row[iCol]) {
		nwnxcpy(buffer, row[iCol]);
		logger->Info("* Returning: %s (column %d)", buffer, iCol);
		return 0;
	} else {
		nwnxcpy(buffer, "");
		logger->Info("* Returning: (empty) (column %d)", iCol);
		return -1;
	}
}

MYSQL_RES* MySQL::AdvanceToNextValidResultset()
{
	MYSQL_RES* newResult;

	while (mysql_more_results(&mysql)) {
		logger->Trace("* Got a resultset");
		mysql_next_result(&mysql);
		newResult = mysql_store_result(&mysql);
		if (newResult == NULL) {
			logger->Trace("* Empty resultset");
			if (mysql_field_count(&mysql) != 0) {
				// SELECT with an empty result set
				logger->Trace("* Retrieved an empty resultset");
				return NULL;
			} else {
				// NOT a SELECT like command, advance to next resultset
				logger->Trace("* Retrieved an invalid resultset (NO SELECT)");
			}
		} else {
			// SELECT with a non-empty resultset
			logger->Trace("* Retrieved a non-empty resultset");
			return newResult;
		}
	}

	// no non-empty resultset found
	return NULL;
}

int MySQL::GetAffectedRows() { return mysql_affected_rows(&mysql); }

int MySQL::GetLastInsertID() { return mysql_insert_id(&mysql); }

void MySQL::GetEscapeString(char* str, char* buffer)
{
	if (*str == 0x0) {
		nwnxcpy(buffer, "");
		return;
	}

	size_t len, to_len;
	len = strlen(str);

	char* to = (char*)malloc(len * 2 + 1);
	to_len   = mysql_real_escape_string(&mysql, to, str, (unsigned long)len);
	nwnxcpy(buffer, to, to_len);
	free(to);
}

int MySQL::GetErrno() { return mysql_errno(&mysql); }

const char* MySQL::GetErrorMessage() { return mysql_error(&mysql); }

bool MySQL::WriteScorcoData(BYTE* pData, int Length)
{
	logger->Info("* SCO query: %s", scorcoSQL.c_str());

	int res;
	unsigned long len;
	char* Data = new char[Length * 2 + 1 + 2];
	char* pSQL = new char[MAXSQL + Length * 2 + 1];

	len     = mysql_real_escape_string(&mysql, Data + 1, (const char*)pData, Length);
	Data[0] = Data[len + 1] = 39; //'
	Data[len + 2]           = 0x0;
	sprintf(pSQL, scorcoSQL.c_str(), Data);

	MYSQL_RES* result = mysql_store_result(&mysql);
	res               = mysql_query(&mysql, (const char*)pSQL);

	mysql_free_result(result);
	delete[] pSQL;
	delete[] Data;

	if (res == 0)
		return true;
	else
		return false;
}

BYTE* MySQL::ReadScorcoData(char* param, int* size)
{
	logger->Info("* RCO query: %s", scorcoSQL.c_str());

	bool pSqlError;
	MYSQL_RES* rcoresult;
	if (strcmp(param, "FETCHMODE") != 0) {
		if (mysql_query(&mysql, scorcoSQL.c_str()) != 0) {
			pSqlError = true;
			return NULL;
		}

		/*if (result)
		{
		mysql_free_result(result);
		result = NULL;
		}*/
		rcoresult = mysql_store_result(&mysql);
		if (!rcoresult) {
			pSqlError = true;
			return NULL;
		}
	} else
		rcoresult = result;

	MYSQL_ROW row;
	pSqlError = false;
	row       = mysql_fetch_row(rcoresult);
	if (row) {
		unsigned long* length = mysql_fetch_lengths(rcoresult);
		// allocate buf for result!
		// char* buf = new char[*length];
		NWN2_HeapMgr* pHeapMgr = NWN2_HeapMgr::Instance();
		NWN2_Heap* pHeap       = pHeapMgr->GetDefaultHeap();
		char* buf              = (char*)pHeap->Allocate(*length);

		if (!buf)
			return NULL;

		memcpy(buf, row[0], length[0]);
		*size = length[0];
		mysql_free_result(rcoresult);
		return (BYTE*)buf;
	} else {
		logger->Info("* Empty RCO resultset");
		mysql_free_result(rcoresult);
		return NULL;
	}
}

int MySQL::PrepPrepareStatement(const char* query)
{
	if (!connection) {
		if (!Reconnect()) {
			logger->Err("PrepPrepareStatement: Not connected.");
			return FALSE;
		}
	}

	auto stmtID = m_prepStmts.size() + 1;
	logger->Info("[Stmt %d] Prepare query: %s", stmtID, query);

	// init and prepare statement
	auto stmt = mysql_stmt_init(&mysql);
	if (mysql_stmt_prepare(stmt, query, -1) != 0) {
		logger->Err("[Stmt %d] Failed to prepare statement. Error %d: %s", stmtID,
		            mysql_stmt_errno(stmt), mysql_stmt_error(stmt));
		mysql_stmt_close(stmt); // ignore closing error, just deallocate
		return 0;
	}

	// Get number of params
	unsigned int paramsCount = mysql_stmt_param_count(stmt);

	// Store statement info
	auto& stmtInfo = m_prepStmts.emplace_back(new PreparedStmtInfo {
	    .stmt                  = stmt,
	    .query                 = std::string {query},
	    .params                = std::vector<MYSQL_BIND> {paramsCount},
	    .paramsBufferRealSizes = std::vector<size_t> {},
	    .results               = {},
	    .resBuffer             = {},
	});

	// Init params content
	auto& params = stmtInfo->params;
	memset(params.data(), 0, sizeof(MYSQL_BIND) * paramsCount);
	logger->Debug("[Stmt %d]     Allocated %d parameters", stmtID, paramsCount);

	stmtInfo->paramsBufferRealSizes.resize(paramsCount, 0);

	return stmtID;
}

inline bool MySQL::CheckAndAllocBind(int& stmtID, int& index, size_t requiredBufferSize)
{
	if (stmtID < 1 || stmtID > m_prepStmts.size()) {
		logger->Err("[Stmt %d] PrepBind: Invalid statement ID", stmtID);
		return false;
	}
	auto& stmtInfo = m_prepStmts[stmtID - 1];

	if (index < 1 || index > stmtInfo->params.size()) {
		logger->Err("[Stmt %d] PrepBind: invalid bind param index %d. Must be between 1 and %d",
		            stmtID, index, stmtInfo->params.size());
		return false;
	}

	assert(stmtInfo->params.size() == stmtInfo->paramsBufferRealSizes.size());
	auto& param         = stmtInfo->params[index - 1];
	auto& paramRealSize = stmtInfo->paramsBufferRealSizes[index - 1];

	if (requiredBufferSize > 0) {
		// Delete if needed
		if (param.buffer != nullptr && paramRealSize < requiredBufferSize) {
			delete param.buffer;
			param.buffer = nullptr;
		}

		// Allocate buffer for storing the value
		if (param.buffer == nullptr) {
			param.buffer  = new uint8_t[requiredBufferSize];
			paramRealSize = requiredBufferSize;
		}
	}

	return true;
}

bool MySQL::PrepBindString(int stmtID, int index, const char* value)
{
	auto len = strnlen(value, MAX_BUFFER) + 1;

	if (!CheckAndAllocBind(stmtID, index, len))
		return false;

	auto& param = m_prepStmts[stmtID - 1]->params[index - 1];

	param.buffer_type   = MYSQL_TYPE_STRING;
	param.buffer_length = len;
	strncpy((char*)param.buffer, value, len);

	logger->Info("[Stmt %d] PrepBindString: Set field %d to: %.*s", stmtID, index,
	             param.buffer_length, param.buffer);
	return true;
}

bool MySQL::PrepBindInt(int stmtID, int index, int value)
{
	if (!CheckAndAllocBind(stmtID, index, sizeof(value)))
		return false;

	auto& param = m_prepStmts[stmtID - 1]->params[index - 1];

	param.buffer_type               = MYSQL_TYPE_LONG;
	param.buffer_length             = 0;
	*(decltype(value)*)param.buffer = value;

	logger->Info("[Stmt %d] PrepBindInt: Set field %d to: %d", stmtID, index, param.buffer_length,
	             param.buffer);
	return true;
}

bool MySQL::PrepBindFloat(int stmtID, int index, float value)
{
	if (!CheckAndAllocBind(stmtID, index, sizeof(value)))
		return false;

	auto& param = m_prepStmts[stmtID - 1]->params[index - 1];

	param.buffer_type               = MYSQL_TYPE_FLOAT;
	param.buffer_length             = 0;
	*(decltype(value)*)param.buffer = value;

	logger->Info("[Stmt %d] PrepBindFloat: Set field %d to: %f", stmtID, index, param.buffer_length,
	             param.buffer);
	return true;
}

bool MySQL::PrepBindNull(int stmtID, int index)
{
	if (!CheckAndAllocBind(stmtID, index, 0))
		return false;

	auto& param = m_prepStmts[stmtID - 1]->params[index - 1];

	param.buffer_type   = MYSQL_TYPE_NULL;
	param.buffer_length = 0;
	param.buffer        = nullptr;

	logger->Info("[Stmt %d] PrepBindNull: Set field %d to: NULL", stmtID, index,
	             param.buffer_length);
	return true;
}

void MySQL::FreeResBuffer(int stmtID)
{
	logger->Debug("[Stmt %d] Freeing result buffer", stmtID);

	auto& stmtInfo = m_prepStmts[stmtID - 1];
	stmtInfo->results.clear();
	stmtInfo->results.shrink_to_fit();

	stmtInfo->resBuffer.reset();
}

bool MySQL::AllocateResBuffer(int stmtID)
{
	auto& stmtInfo = m_prepStmts[stmtID - 1];

	auto fieldsCount = mysql_stmt_field_count(stmtInfo->stmt);
	auto resMeta     = mysql_stmt_result_metadata(stmtInfo->stmt); // Allocates memory

	if (fieldsCount > 0) {
		auto fields = mysql_fetch_fields(resMeta);

		// Allocate the buffer for storing results
		// We only allocate one buffer for storing buffer lengths and data

		// Calculate total buffer size
		size_t resBufferSize = 0;
		for (size_t i = 0; i < fieldsCount; i++)
			resBufferSize += sizeof(unsigned long) + fields[i].length;

		// Allocate resBufferSize of memory for storing result data
		auto& resBuffer = stmtInfo->resBuffer.emplace();
		resBuffer.resize(resBufferSize, 0);

		// Initialize result structs to point to the allocated buffer
		stmtInfo->results.resize(fieldsCount);
		memset(stmtInfo->results.data(), 0, sizeof(MYSQL_BIND) * stmtInfo->results.size());

		logger->Debug("[Stmt %d] Allocating buffer of %llu bytes for %d result fields", stmtID,
		              resBuffer.size(), fieldsCount);

		auto resBufferOffset = 0;
		for (size_t i = 0; i < fieldsCount; i++) {
			logger->Debug("[Stmt %d]     field %d: type=%d length=%d", stmtID, i, fields[i].type,
			              fields[i].length);
			stmtInfo->results[i].buffer_type   = fields[i].type;
			stmtInfo->results[i].buffer_length = fields[i].length;

			stmtInfo->results[i].length = (unsigned long*)&resBuffer[resBufferOffset];
			resBufferOffset += sizeof(unsigned long);

			stmtInfo->results[i].buffer = (char*)&resBuffer[resBufferOffset];
			resBufferOffset += fields[i].length;
		}

		// Bind result structs
		if (mysql_stmt_bind_result(stmtInfo->stmt, stmtInfo->results.data()) != 0) {
			logger->Err("[Stmt %d] AllocateResBuffer: Failed to bind results. Error %d: %s", stmtID,
			            mysql_stmt_errno(stmtInfo->stmt), mysql_stmt_error(stmtInfo->stmt));
		}

		// Cache results
		if (mysql_stmt_store_result(stmtInfo->stmt)) {
			logger->Err("[Stmt %d] AllocateResBuffer: Failed to cache results. Error %d: %s",
			            stmtID, mysql_stmt_errno(stmtInfo->stmt), mysql_stmt_error(stmtInfo->stmt));
			if (mysql_stmt_errno(stmtInfo->stmt) == CR_COMMANDS_OUT_OF_SYNC) {
				logger->Err("[Stmt %d]     Did you execute the prepared statement?", stmtID);
			}
			return false;
		}

	} else {
		logger->Debug("[Stmt %d] Allocating nothing, as statement provides no results.", stmtID);
	}

	mysql_free_result(resMeta);

	return true;
}

bool MySQL::PrepExecute(int stmtID)
{
	if (stmtID < 1 || stmtID > m_prepStmts.size()) {
		logger->Err("[Stmt %d] PrepExecute: invalid statement ID", stmtID);
		return false;
	}

	if (!connection) {
		if (!Reconnect()) {
			logger->Err("[Stmt %d] PrepExecute: Not connected.", stmtID);
			return FALSE;
		}
	}

	logger->Info("[Stmt %d] PrepExecute: execute stmt", stmtID);

	auto& stmt = m_prepStmts[stmtID - 1]->stmt;

	// Free results if any
	if (mysql_stmt_free_result(stmt) != 0) {
		logger->Err("[Stmt %d] PrepExecute mysql_stmt_free_result error %d: %s", stmtID,
		            mysql_stmt_errno(stmt), mysql_stmt_error(stmt));
	}
	// Bind params
	if (mysql_stmt_bind_param(stmt, m_prepStmts[stmtID - 1]->params.data()) != 0) {
		logger->Err("[Stmt %d] PrepExecute mysql_stmt_bind_param error %d: %s", stmtID,
		            mysql_stmt_errno(stmt), mysql_stmt_error(stmt));
	}
	// Execute
	if (mysql_stmt_execute(stmt) != 0) {
		logger->Err("[Stmt %d] PrepExecute mysql_stmt_execute error %d: %s", stmtID,
		            mysql_stmt_errno(stmt), mysql_stmt_error(stmt));
		return false;
	}

	// Clean results if any. They will be filled & bound on first fetch
	FreeResBuffer(stmtID);

	return true;
}

bool MySQL::PrepFetch(int stmtID)
{
	if (stmtID < 1 || stmtID > m_prepStmts.size()) {
		logger->Err("[Stmt %d] PrepExecute: invalid statement ID", stmtID);
		return false;
	}
	auto& stmtInfo = m_prepStmts[stmtID - 1];

	if (!stmtInfo->resBuffer.has_value()) {
		if (!AllocateResBuffer(stmtID))
			return false;
	}

	// Fetch row
	auto res = mysql_stmt_fetch(stmtInfo->stmt);
	switch (res) {
		case 0:
			logger->Debug("[Stmt %d] PrepFetch returns a row.", stmtID);
			return true;
		case MYSQL_NO_DATA:
			logger->Debug("[Stmt %d] PrepFetch returns no row.", stmtID);
			FreeResBuffer(stmtID);
			return false;
		case MYSQL_DATA_TRUNCATED:
			logger->Err("[Stmt %d] PrepFetch truncated data", stmtID);

			for (size_t i = 0; i < stmtInfo->results.size(); i++) {
				auto& bind = stmtInfo->results[i];
				if (bind.length == nullptr)
					logger->Err("[Stmt %d]     Result %d is NULL bytes instead of %d", stmtID,
					            i + 1, bind.buffer_length);
				else if (*bind.length != bind.buffer_length)
					logger->Err("[Stmt %d]     Result %d is %d bytes instead of %d", stmtID, i + 1,
					            *bind.length, bind.buffer_length);
			}
			return false;
		default:
			logger->Err("[Stmt %d] PrepFetch mysql_stmt_fetch returned %d. Error %d: %s", stmtID,
			            res, mysql_stmt_errno(stmtInfo->stmt), mysql_stmt_error(stmtInfo->stmt));
			if (mysql_stmt_errno(stmtInfo->stmt) == CR_COMMANDS_OUT_OF_SYNC) {
				logger->Err("[Stmt %d]     Did you execute the prepared statement?", stmtID);
			}
			return false;
	}
	return true;
}

char* MySQL::PrepGetDataString(int stmtID, int index, char* buffer)
{
	if (stmtID < 1 || stmtID > m_prepStmts.size()) {
		logger->Err("[Stmt %d] PrepGetDataString: invalid statement ID", stmtID);
		return nullptr;
	}
	auto& stmtInfo = m_prepStmts[stmtID - 1];

	if (index < 1 || index > stmtInfo->results.size()) {
		logger->Err(
		    "[Stmt %d] PrepGetDataString: invalid bind result index %d. Must be between 1 and %d",
		    stmtID, index, stmtInfo->results.size());
		return nullptr;
	}
	auto& result = stmtInfo->results[index - 1];

	switch (result.buffer_type) {
		case MYSQL_TYPE_TINY:
			snprintf(buffer, MAX_BUFFER, "%hhd", *(signed char*)result.buffer);
			break;
		case MYSQL_TYPE_SHORT:
			snprintf(buffer, MAX_BUFFER, "%hd", *(short*)result.buffer);
			break;
		case MYSQL_TYPE_LONG:
		case MYSQL_TYPE_INT24:
			snprintf(buffer, MAX_BUFFER, "%d", *(int*)result.buffer);
			break;
		case MYSQL_TYPE_LONGLONG:
			snprintf(buffer, MAX_BUFFER, "%lld", *(long long int*)result.buffer);
			break;

		case MYSQL_TYPE_FLOAT:
			snprintf(buffer, MAX_BUFFER, "%f", *(float*)result.buffer);
			break;
		case MYSQL_TYPE_DOUBLE:
			snprintf(buffer, MAX_BUFFER, "%f", *(double*)result.buffer);
			break;

		case MYSQL_TYPE_DATETIME:
		case MYSQL_TYPE_TIMESTAMP: {
			const auto dt = (MYSQL_TIME*)result.buffer;
			snprintf(buffer, MAX_BUFFER, "%04d-%02d-%02dT%02d:%02d:%02d.%03d", dt->year, dt->month,
			         dt->day, dt->hour, dt->minute, dt->second, dt->second_part);
		} break;
		case MYSQL_TYPE_TIME: {
			const auto dt = (MYSQL_TIME*)result.buffer;
			snprintf(buffer, MAX_BUFFER, "%02d:%02d:%02d.%03d", dt->hour, dt->minute, dt->second,
			         dt->second_part);
		} break;
		case MYSQL_TYPE_DATE: {
			const auto dt = (MYSQL_TIME*)result.buffer;
			snprintf(buffer, MAX_BUFFER, "%04d-%02d-%02d", dt->year, dt->month, dt->day);
		} break;

		case MYSQL_TYPE_STRING:
		case MYSQL_TYPE_VARCHAR:
		case MYSQL_TYPE_VAR_STRING:

		case MYSQL_TYPE_BIT:
		case MYSQL_TYPE_BLOB:
		case MYSQL_TYPE_TINY_BLOB:
		case MYSQL_TYPE_MEDIUM_BLOB:
		case MYSQL_TYPE_LONG_BLOB:
		case MYSQL_TYPE_ENUM:
			buffer = (char*)result.buffer;
			break;

		case MYSQL_TYPE_NULL:
		case MYSQL_TYPE_JSON:
		case MYSQL_TYPE_GEOMETRY:
			logger->Err("[Stmt %d] PrepGetDataString(%d): Unsupported SQL type %d", stmtID, index,
			            result.buffer_type);
			return nullptr;

		default:
			logger->Err("[Stmt %d] PrepGetDataString(%d): Unknown SQL type %d", stmtID, index,
			            result.buffer_type);
			return nullptr;
	}

	logger->Debug("[Stmt %d] PrepGetDataStr(%d): Returned field value: %s", stmtID, index, buffer);
	return buffer;
}

int MySQL::PrepGetDataInt(int stmtID, int index)
{
	if (stmtID < 1 || stmtID > m_prepStmts.size()) {
		logger->Err("[Stmt %d] PrepGetDataInt: invalid statement ID", stmtID);
		return false;
	}
	auto& stmtInfo = m_prepStmts[stmtID - 1];

	if (index < 1 || index > stmtInfo->results.size()) {
		logger->Err(
		    "[Stmt %d] PrepGetDataInt: invalid bind result index %d. Must be between 1 and %d",
		    stmtID, index, stmtInfo->results.size());
		return false;
	}
	auto& result = stmtInfo->results[index - 1];

	int res = 0;

	switch (result.buffer_type) {
		case MYSQL_TYPE_TINY:
			res = *(signed char*)result.buffer;
			break;
		case MYSQL_TYPE_SHORT:
			res = *(short*)result.buffer;
			break;
		case MYSQL_TYPE_LONG:
		case MYSQL_TYPE_INT24:
			res = *(int*)result.buffer;
			break;
		case MYSQL_TYPE_LONGLONG:
			logger->Warn(
			    "[Stmt %d] PrepGetDataInt(%d): NWN2 cannot handle integer values outside of the "
			    "signed 32bit range. Consider retrieving this value as a string to avoid integer "
			    "overflows, or reduce the size of the column type.",
			    stmtID, index);
			res = *(long long int*)result.buffer;
			break;

		case MYSQL_TYPE_FLOAT:
		case MYSQL_TYPE_DOUBLE:
			logger->Err(
			    "[Stmt %d] PrepGetDataInt(%d): Cannot convert float to int (loss of precision)",
			    stmtID, index);
			return 0;

		case MYSQL_TYPE_DATETIME:
		case MYSQL_TYPE_TIMESTAMP:
		case MYSQL_TYPE_TIME:
		case MYSQL_TYPE_DATE:
			logger->Err("[Stmt %d] PrepGetDataInt(%d): Cannot convert a date / time / datetime / "
			            "timestamp to int",
			            stmtID, index);
			return 0;

		case MYSQL_TYPE_STRING:
		case MYSQL_TYPE_VARCHAR:
		case MYSQL_TYPE_VAR_STRING:
		case MYSQL_TYPE_ENUM:

		case MYSQL_TYPE_BIT:
		case MYSQL_TYPE_BLOB:
		case MYSQL_TYPE_TINY_BLOB:
		case MYSQL_TYPE_MEDIUM_BLOB:
		case MYSQL_TYPE_LONG_BLOB:
			logger->Err("[Stmt %d] PrepGetDataInt(%d): Cannot convert text to int", stmtID, index);
			return 0;

		case MYSQL_TYPE_NULL:
		case MYSQL_TYPE_JSON:
		case MYSQL_TYPE_GEOMETRY:
			logger->Err("[Stmt %d] PrepGetDataInt(%d): Unsupported SQL type %d", stmtID, index,
			            result.buffer_type);
			return 0;

		default:
			logger->Err("[Stmt %d] PrepGetDataInt(%d): Unknown SQL type %d", stmtID, index,
			            result.buffer_type);
			return 0;
	}

	logger->Debug("[Stmt %d] PrepGetDataInt(%d): Returned field value: %d", stmtID, index, res);
	return res;
}

float MySQL::PrepGetDataFloat(int stmtID, int index)
{
	if (stmtID < 1 || stmtID > m_prepStmts.size()) {
		logger->Err("[Stmt %d] PrepGetDataFloat: invalid statement ID", stmtID);
		return false;
	}
	auto& stmtInfo = m_prepStmts[stmtID - 1];

	if (index < 1 || index > stmtInfo->results.size()) {
		logger->Err(
		    "[Stmt %d] PrepGetDataFloat: invalid bind result index %d. Must be between 1 and %d",
		    stmtID, index, stmtInfo->results.size());
		return false;
	}
	auto& result = stmtInfo->results[index - 1];

	float res = 0;

	switch (result.buffer_type) {
		case MYSQL_TYPE_TINY:
		case MYSQL_TYPE_SHORT:
		case MYSQL_TYPE_LONG:
		case MYSQL_TYPE_INT24:
		case MYSQL_TYPE_LONGLONG:
			logger->Err(
			    "[Stmt %d] PrepGetDataFloat(%d): Cannot convert int to float (loss of precision)",
			    stmtID, index);
			return 0;

		case MYSQL_TYPE_FLOAT:
			res = *(float*)result.buffer;
			break;
		case MYSQL_TYPE_DOUBLE:
			res = *(double*)result.buffer;
			break;

		case MYSQL_TYPE_DATETIME:
		case MYSQL_TYPE_TIMESTAMP:
		case MYSQL_TYPE_TIME:
		case MYSQL_TYPE_DATE:
			logger->Err("[Stmt %d] PrepGetDataFloat(%d): Cannot convert a date / time / datetime / "
			            "timestamp to float",
			            stmtID, index);
			return 0;

		case MYSQL_TYPE_STRING:
		case MYSQL_TYPE_VARCHAR:
		case MYSQL_TYPE_VAR_STRING:
		case MYSQL_TYPE_ENUM:

		case MYSQL_TYPE_BIT:
		case MYSQL_TYPE_BLOB:
		case MYSQL_TYPE_TINY_BLOB:
		case MYSQL_TYPE_MEDIUM_BLOB:
		case MYSQL_TYPE_LONG_BLOB:
			logger->Err("[Stmt %d] PrepGetDataFloat(%d): Cannot convert text to float", stmtID,
			            index);
			return 0;

		case MYSQL_TYPE_NULL:
		case MYSQL_TYPE_JSON:
		case MYSQL_TYPE_GEOMETRY:
			logger->Err("[Stmt %d] PrepGetDataFloat(%d): Unsupported SQL type %d", stmtID, index,
			            result.buffer_type);
			return 0;

		default:
			logger->Err("[Stmt %d] PrepGetDataFloat(%d): Unknown SQL type %d", stmtID, index,
			            result.buffer_type);
			return 0;
	}

	logger->Debug("[Stmt %d] PrepGetDataFloat(%d): Returned field value: %d", stmtID, index, res);
	return res;
}