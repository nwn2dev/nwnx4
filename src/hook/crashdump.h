/***************************************************************************
NWNX Hook - Responsible for the actual hooking
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

#if !defined(CRASHDUMP_H_INCLUDED)
#define CRASHDUMP_H_INCLUDED

#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>
#include <windows.h>

/*
 * Types.
 */

typedef struct _CRASH_DUMP_SECTION {
	HANDLE CrashReportEvent;
	HANDLE CrashAckEvent;
	EXCEPTION_POINTERS ExceptionPointers;
	PEXCEPTION_POINTERS ClientExceptionPointers;
	ULONG ThreadId;
} CRASH_DUMP_SECTION, *PCRASH_DUMP_SECTION;

/*
 * Forwards.
 */

bool RegisterCrashDumpHandler();

/*
 * Globals.
 */

extern HMODULE g_Module;
extern volatile LONG g_InCrash;
extern PCRASH_DUMP_SECTION g_CrashDumpSectionView;

#endif
