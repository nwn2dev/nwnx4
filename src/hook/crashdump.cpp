/***************************************************************************
    NWNX Hook - Responsible for the actual hooking
    Copyright (C) 2007 Ingmar Stieger (Papillon, papillon@nwnx.org)
	Copyright (C) 2008 Skywing (skywing@valhallalegends.com)

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

#include "crashdump.h"

/***************************************************************************
    Crash dump support.
***************************************************************************/

/*
 * Toplevel exception filter to report the crash to the master server process.
 */
LONG
WINAPI
CrashDumpExceptionFilter(
	__in PEXCEPTION_POINTERS ExceptionInfo
	)
{
//	DWORD OldProt;

	//
	// First, ensure that we're not doing something while another thread is
	// already reporting a crash, because the process is not frozen while we
	// handle this part of it in-process.  If we have multiple threads dying
	// at the same time, that's too bad, we'll only get the first reported
	// directly.  The others will at least still have their stack.
	//
	// N.B. We don't use a critical section for purposes of making the code
	// that runs in-process for the crash as minimal as possible, so as to
	// maximize our chances of successfully reporting an error without causing
	// a secondary failure that would obscure the actual, underlying issue.
	//

	if (InterlockedIncrement( &g_InCrash ) != 1)
		Sleep( INFINITE );

	//
	// Make the section view writable now.  We initially make it read only so
	// that we don't have it getting corrupted by bogus pointers, just as a
	// step to make sure we're going to be as reliable as possible in our
	// reporting process, as we really don't want to assume that the current
	// process is in a healthy state at this point.
	//

//	VirtualProtect(
//		g_CrashDumpSectionView,
//		sizeof( *g_CrashDumpSectionView ),
//		PAGE_READWRITE,
//		&OldProt
//		);

	//
	// Fill out the information.
	//

	memcpy(
		&g_CrashDumpSectionView->ExceptionPointers,
		ExceptionInfo,
		sizeof( EXCEPTION_POINTERS )
		);

	g_CrashDumpSectionView->ClientExceptionPointers = ExceptionInfo;
	g_CrashDumpSectionView->ThreadId                = GetCurrentThreadId();

	//
	// Set the event that tells the master that we've got a crash.
	//

	if (g_CrashDumpSectionView->CrashReportEvent)
	{
		SetEvent( g_CrashDumpSectionView->CrashReportEvent );

		//
		// We're done here, the process is toast, we are just going to wait for
		// the master to notice the crash and terminates us after writing out a
		// dump.  Note that if we don't have an ack event, then we'll just skip the
		// wait entirely.
		//

		if (g_CrashDumpSectionView->CrashAckEvent)
			WaitForSingleObject( g_CrashDumpSectionView->CrashAckEvent, 30000 );
	}

	//
	// Hrm, nobody seems to be home.  We'll just let it die.
	//

	TerminateProcess(
		GetCurrentProcess(),
		ExceptionInfo->ExceptionRecord->ExceptionCode
		);

	//
	// Shouldn't happen.
	//

	return EXCEPTION_CONTINUE_SEARCH;
}

/*
 * Register crash dump handler.
 */
bool
RegisterCrashDumpHandler(
	)
{
	HANDLE Section;
	WCHAR  SectionName[ 128 ];

	StringCbPrintfW(
		SectionName,
		sizeof( SectionName ),
		L"NWN2ServerCrashDumpSection-%lu",
		GetCurrentProcessId()
		);

	g_InCrash = 0;

	Section = CreateFileMappingW(
		INVALID_HANDLE_VALUE,
		0,
		PAGE_READWRITE,
		0,
		sizeof( CRASH_DUMP_SECTION ),
		SectionName
		);

	if (Section == INVALID_HANDLE_VALUE)
	{
//		DebugPrint( "Failed to create section, %lu\n", GetLastError() );

		return false;
	}

	g_CrashDumpSectionView = (PCRASH_DUMP_SECTION)MapViewOfFile(
		Section,
		FILE_MAP_WRITE,
		0,
		0,
		0
		);

	if (!g_CrashDumpSectionView)
	{
//		DebugPrint( "Failed to map section view, %lu\n", GetLastError() );

		CloseHandle( Section );

		return false;
	}

	//
	// Zero the section view.
	//

	ZeroMemory( g_CrashDumpSectionView, sizeof( CRASH_DUMP_SECTION ) );

	//
	// Set up our handler.
	//

	SetUnhandledExceptionFilter(
		CrashDumpExceptionFilter
		);

	//
	// We're all done.
	//

	return true;
}
