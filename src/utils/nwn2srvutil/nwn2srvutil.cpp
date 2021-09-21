#include <string.h>
#include <stdlib.h>
#define WIN32_NO_STATUS
#include <windows.h>
#undef WIN32_NO_STATUS
#include <ntnative.h>
#include <strsafe.h>
#include <io.h>
#include <tlhelp32.h>

#define CROSS_SESSION 1

BOOLEAN
FindProcessPid(
	__in const wchar_t *ProcessName,
	__out PULONG Pid,
	__out PULONG NumProcesses
	)
{
	HANDLE         Snapshot;
	PROCESSENTRY32 ProcInfo;
	BOOL           Status;
	ULONG          Count;

	Count         = 0;
	*Pid          = 0;
	*NumProcesses = 0;

	Snapshot = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );

	if (Snapshot == INVALID_HANDLE_VALUE)
		return FALSE;

	ProcInfo.dwSize = sizeof( PROCESSENTRY32 );

	for (Status = Process32First( Snapshot, &ProcInfo );
	     Status != FALSE;
	     Status = Process32Next( Snapshot, &ProcInfo ))
	{
		wchar_t *Exe;

		Exe = wcsrchr( ProcInfo.szExeFile, L'\\' );

		if (Exe)
			*Exe++ = L'\0';
		else
			Exe = ProcInfo.szExeFile;

		if (!_wcsicmp(Exe, ProcessName))
		{
			*Pid = ProcInfo.th32ProcessID;

			Count++;
		}
	}

	CloseHandle( Snapshot );

	*NumProcesses = Count;

	return (*Pid != 0 && Count == 1);
}

BOOLEAN
InjectProcessDll(
	__in HANDLE Process,
	__in CONST CHAR *DllName,
	__in int Argc,
	__in char **Argv
	)
{
	char                     Code[ 512 ];
	SIZE_T                   Written;
	PVOID                    Buf;
	HANDLE                   Thread;
	char                    *CmdLineBuf;
	size_t                   Len = 1;
	size_t                   Pos = 0;
	PVOID                    CmdBuf;
	THREAD_BASIC_INFORMATION ThreadInfo;
	ULONG                    ReturnLength;

	/*
	 * Based off of Andreas Hansson's remotehdl - for sake of small code.
	 */

	memcpy(
		Code + 256,
		"\xe8\x00\x00\x00\x00\x58\x2d\x05\x01\x00\x00\x50\xb8\x00\x00\x00\x00\xff\xd0\x6a\x00\xb8\x00\x00\x00\x00\xff\xd0",
		28
		);
	*(DWORD_PTR*)(Code + 256 + 13) = (DWORD_PTR)LoadLibraryA;
	*(DWORD_PTR*)(Code + 256 + 22) = (DWORD_PTR)ExitThread;

	StringCbCopyA(
		Code,
		256,
		DllName
		);

	Buf = VirtualAllocEx(
		Process,
		0,
		4096,
		MEM_COMMIT,
		PAGE_EXECUTE_READWRITE
		);

	if (!Buf)
	{
		wprintf(L"Couldn't allocate memory (%lu).\n",
			GetLastError());
		return FALSE;
	}

	if (!WriteProcessMemory(
		Process,
		Buf,
		Code,
		sizeof( Code ),
		&Written))
	{
		wprintf(L"Couldn't write process memory (%lu).\n",
			GetLastError());

		return FALSE;
	}

#if CROSS_SESSION
	if (!NT_SUCCESS(RtlCreateUserThread(
		Process,
		0,
		TRUE,
		0,
		0,
		0,
		(PUCHAR)Buf + 256,
		0,
		&Thread,
		0)))
		Thread = 0;
#else
	Thread = CreateRemoteThread(
		Process,
		0,
		0,
		(LPTHREAD_START_ROUTINE)((PUCHAR)Buf + 256),
		0,
		CREATE_SUSPENDED,
		0
		);
#endif

	if (!Thread)
	{
		wprintf(L"Couldn't create thread (%lu).\n",
			GetLastError());

		VirtualFreeEx(
			Process,
			Buf,
			0,
			MEM_RELEASE
			);

		return FALSE;
	}

	for (int i = 0;
	     i < Argc;
	     i += 1)
	{
		Len += strlen( Argv[ i ] ) + 1;
	}

	CmdLineBuf = (char *) malloc( Len );

	if (!CmdLineBuf)
	{
		TerminateThread( Thread, 0 );
		VirtualFreeEx(
			Process,
			Buf,
			0,
			MEM_RELEASE
			);
	}

	for (int i = 0;
	     i < Argc;
	     i += 1)
	{
		size_t l = strlen( Argv[ i ] ) + 1;

		memcpy( CmdLineBuf + Pos, Argv[ i ], l );
		Pos += l;
	}

	CmdLineBuf[ Pos ] = '\0';

	CmdBuf = VirtualAllocEx(
		Process,
		0,
		Len,
		MEM_COMMIT,
		PAGE_READWRITE
		);

	if (!CmdBuf)
	{
		free( CmdLineBuf );
		TerminateThread( Thread, 0 );
		VirtualFreeEx(
			Process,
			Buf,
			0,
			MEM_RELEASE
			);
	}

	if (!WriteProcessMemory(
		Process,
		CmdBuf,
		CmdLineBuf,
		Len,
		&Written))
	{
		wprintf(L"Couldn't write process memory (%lu).\n",
			GetLastError());

		free( CmdLineBuf );
		TerminateThread( Thread, 0 );
		VirtualFreeEx(
			Process,
			Buf,
			0,
			MEM_RELEASE
			);

		return FALSE;
	}

	if (!NT_SUCCESS(NtQueryInformationThread(
		Thread,
		ThreadBasicInformation,
		&ThreadInfo,
		sizeof( ThreadInfo ),
		&ReturnLength)))
	{
		wprintf(L"Couldn't query thread information.\n");

		free( CmdLineBuf );
		TerminateThread( Thread, 0 );
		VirtualFreeEx(
			Process,
			Buf,
			0,
			MEM_RELEASE
			);

		return FALSE;
	}

	if (!WriteProcessMemory(
		Process,
		(PUCHAR)ThreadInfo.TebBaseAddress + FIELD_OFFSET( TEB, NtTib.ArbitraryUserPointer ),
		&CmdBuf,
		sizeof( CmdBuf ),
		&Written))
	{
		wprintf(L"Couldn't write process memory (%lu).\n",
			GetLastError());

		free( CmdLineBuf );
		TerminateThread( Thread, 0 );
		VirtualFreeEx(
			Process,
			Buf,
			0,
			MEM_RELEASE
			);

		return FALSE;
	}

	free( CmdLineBuf );

	ResumeThread( Thread );

	WaitForSingleObject(
		Thread,
		INFINITE
		);

	CloseHandle(
		Thread
		);

	VirtualFreeEx(
		Process,
		Buf,
		0,
		MEM_RELEASE
		);

	return TRUE;
}

int
__cdecl
main(
	int ac,
	char **av
	)
{
	HANDLE  Process;
	CHAR    DllName[ MAX_PATH ];
	ULONG   Pid = 0;

	if (!ac)
		return 0;

	for (int i = 0;
	     i < ac - 1;
	     i += 1)
	{
		if (!_stricmp( av[ i ], "-p" ))
		{
			Pid = strtoul( av[ i + 1 ], 0, 0 );
			i += 1;
			break;
		}
	}

	if (!Pid)
	{
		ULONG NumProcesses;

		if (!FindProcessPid( L"nwn2server.exe", &Pid, &NumProcesses ))
		{
			wprintf(L"Usage: %S -p <nwn2server-pid> [... arguments]\n",
				av[0]);

			if (NumProcesses == 0)
				wprintf(L"\nnwn2server.exe is not running.\nManually specify the pid with the -p option if the server process is running.\n");
			else
				wprintf(L"\nMultiple nwn2server.exe instances are running.\nManually specify the pid with the -p option.");

			return 1;
		}
	}

	if (!GetCurrentDirectoryA(
		MAX_PATH,
		DllName))
	{
		wprintf(L"Couldn't determine current directory (%lu).\n",
			GetLastError());

		return 1;
	}

	if (FAILED(StringCbCatA(
		DllName,
		sizeof( DllName ),
		"\\nwn2srvutil_remote.dll")))
	{
		wprintf(L"Current directory pathname exceeded limits.\n");
		return 1;
	}

	if (_access(
		DllName,
		00))
	{
		wprintf(L"nwn2srvutil_remote.dll must be contained in the current directory.\n -> %S\n",
			DllName);

		return 1;
	}

	Process = OpenProcess(
		PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION | PROCESS_CREATE_THREAD | PROCESS_SET_INFORMATION | PROCESS_QUERY_INFORMATION,
		FALSE,
		Pid);

	if (!Process)
	{
		wprintf(L"OpenProcess fails (%lu).  Ensure that you provided a correct process id that you have full rights to.\n",
			GetLastError());

		return 1;
	}

	if (!InjectProcessDll(
		Process,
		DllName,
		ac,
		av))
	{
		wprintf(L"Failed to inject code.  Ensure that you have full access to the process.\n");

		CloseHandle(
			Process
			);

		return 1;
	}

	CloseHandle(
		Process
		);

	wprintf(L"Request sent successfully.\n");

	return 0;
}



