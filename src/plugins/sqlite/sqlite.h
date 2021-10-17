/***************************************************************************
    NWNX SQLite - Database plugin for SQLite
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
#if !defined(SQLITE_H_INCLUDED)
#define SQLITE_H_INCLUDED

#define DLLEXPORT extern "C" __declspec(dllexport)

#include "../dbplugin.h"
#include "lib/sqlite3.h"

class SQLite : public DBPlugin
{
public:
	SQLite();
	~SQLite() override;

	bool Init(char* nwnxhome);  

private:
	std::string dbfile;
	bool firstfetch;
	bool wrapTransaction;

	sqlite3 *sdb;
	sqlite3_stmt* pStmt;

	bool Connect();
	void Disconnect();
	bool Execute(char* query) override;
	int Fetch(char* buffer) override;
	int GetData(int iCol, char* buffer) override;
	int GetAffectedRows() override;
	void GetEscapeString(char* str, char* buffer) override;
	int GetErrno() override;
	const char *GetErrorMessage() override;
	int GetLastInsertID() override;
	bool SQLite::WriteScorcoData(BYTE* pData, int Length) override;
	BYTE* SQLite::ReadScorcoData(char *param, int *size) override;

	/// Free *pStmt if possible
	static void SafeFinalize(sqlite3_stmt** pStmt);
};

#endif