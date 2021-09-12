/***************************************************************************
    IniFile.cpp - implementation of the CIniFile class.
    Copyright (C) 2005 Jeroen Broekhuizen (jeroen@nwnx.org) and
	Ingmar Stieger (papillon@nwnx.org)

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

//#include "stdafx.h"
#include <windows.h>
#include "IniFile.h"
#include <stdlib.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CIniFile::CIniFile(const wchar_t* lpszFileName)
{
	// profile functions require complete path
	_fullpath (m_szFileName, lpszFileName, MAX_PATH);
}

CIniFile::~CIniFile()
{
}

int CIniFile::ReadInteger (const wchar_t* lpszSection, const wchar_t* lpszKey, int iDefault)
{
	// read integer value from ini-file
	return GetPrivateProfileInt (lpszSection, lpszKey, iDefault, m_szFileName);
}

long CIniFile::ReadLong (const wchar_t* lpszSection, const wchar_t* lpszKey, long lDefault)
{
	// read long value from ini-file
	char buffer[256];
	
	ReadString(lpszSection, lpszKey, buffer, 256, "");
	if (_stricmp(buffer, "") != 0) 
		return atol(buffer);	
	else
		return lDefault;
}

void CIniFile::ReadString (const wchar_t* lpszSection, const wchar_t* lpszKey, LPSTR lpszBuffer, int Size, const wchar_t* lpszDefault)
{
	// read string value
	GetPrivateProfileString (lpszSection, lpszKey, lpszDefault, lpszBuffer, Size, m_szFileName);
}

bool CIniFile::ReadBool(const wchar_t* lpszSection, const wchar_t* lpszKey, bool iDefault)
{
	char buffer[256];
	ReadString(lpszSection, lpszKey, buffer, 256, "");
	if ((_stricmp(buffer, "true") == 0) || 
		(_stricmp(buffer, "yes") == 0) ||
		(_stricmp(buffer, "1") == 0))
	{
		return true;
	}

	return false;
}