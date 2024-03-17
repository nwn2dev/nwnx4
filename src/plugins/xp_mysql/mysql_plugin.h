/***************************************************************************
    NWNX Mysql - Database plugin for MySQL
    Copyright (C) 2007 Ingmar Stieger (Papillon, papillon@blackdagger.com)

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
#if !defined(MYSQL_H_INCLUDED)
#define MYSQL_H_INCLUDED

#define DLLEXPORT            extern "C" __declspec(dllexport)
#define CR_SERVER_GONE_ERROR 2006

typedef unsigned long ulong;

#include "../database/dbplugin.h"
#include <memory>
#include <mysql/mysql.h>
#include <optional>

class MySQL : public DBPlugin {
public:
	MySQL();
	~MySQL() override;

	bool Init(char* nwnxhome);

	bool WriteScorcoData(BYTE* pData, int Length) override;
	BYTE* ReadScorcoData(char* param, int* size) override;

private:
	bool Connect() override;
	void Disconnect() override;
	bool Reconnect();
	bool Execute(char* query) override;
	int Fetch(char* buffer) override;
	int GetData(int iCol, char* buffer) override;
	int GetAffectedRows() override;
	void GetEscapeString(char* str, char* buffer) override;
	MYSQL_RES* AdvanceToNextValidResultset();
	int GetErrno() override;
	const char* GetErrorMessage() override;
	int GetLastInsertID() override;

	int PrepPrepareStatement(const char* query) override;
	bool PrepBindString(int stmtID, int index, const char* value) override;
	bool PrepBindInt(int stmtID, int index, int value) override;
	bool PrepBindFloat(int stmtID, int index, float value) override;
	bool PrepBindNull(int stmtID, int index) override;
	bool PrepExecute(int stmtID) override;
	bool PrepFetch(int stmtID) override;
	char* PrepGetDataString(int stmtID, int index, char* buffer) override;
	int PrepGetDataInt(int stmtID, int index) override;
	float PrepGetDataFloat(int stmtID, int index) override;

	MYSQL mysql;
	MYSQL* connection;
	MYSQL_RES* result;
	MYSQL_ROW row;

	unsigned int num_fields;

	std::string server;
	std::string user;
	std::string password;
	std::string schema;
	std::string charset;
	int port;

	typedef struct {
		MYSQL_STMT* stmt;
		std::string query;
		std::vector<MYSQL_BIND> params;
		std::vector<size_t> paramsBufferRealSizes;
		std::vector<MYSQL_BIND> results;
		std::optional<std::vector<uint8_t>> resBuffer;
	} PreparedStmtInfo;

	// Note: mysql will keep pointers to params and results array
	std::vector<std::unique_ptr<PreparedStmtInfo>> m_prepStmts;

	bool CheckAndAllocBind(int& stmtID, int& index, size_t requiredBufferSize);

	bool AllocateResBuffer(int stmtIndex);
	void FreeResBuffer(int stmtIndex);
};

#endif