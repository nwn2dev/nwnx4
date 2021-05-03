/***************************************************************************
    NWNX Controller - Controls the server process
    Copyright (C) 2006 Ingmar Stieger (Papillon, papillon@nwnx.org)
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

#include "stdwx.h"
#include "controller.h"

NWNXController::NWNXController(wxFileConfig *config)
{
    USES_CONVERSION;
	this->config = config;

	tick = 0;
	initialized = false;
	shuttingDown = false;
	processWatchdog = true;
	gamespyWatchdog = true;
	gamespyPort = 5121;
	gamespyInterval = 30;
	restartDelay = 5;
	gamespyRetries = 0;
	gamespyTolerance = 5;
	gamespyDelay = 30;
    gracefulShutdownTimeout = 10;
	gracefulShutdownMessageWait = 5;
	ZeroMemory(&si, sizeof(si));
	ZeroMemory(&pi, sizeof(pi));

	config->Read(wxT("restartDelay"), &restartDelay);
	config->Read(wxT("processWatchdog"), &processWatchdog);
	config->Read(wxT("gamespyWatchdog"), &gamespyWatchdog);
	config->Read(wxT("gamespyInterval"), &gamespyInterval);
	config->Read(wxT("gamespyTolerance"), &gamespyTolerance);
	config->Read(wxT("gamespyDelay"), &gamespyDelay);
	config->Read(wxT("gracefulShutdownTimeout"), &gracefulShutdownTimeout);
	config->Read(wxT("gracefulShutdownMessage"), &gracefulShutdownMessage);
	config->Read(wxT("gracefulShutdownMessageWait"), &gracefulShutdownMessageWait);

	if (!config->Read(wxT("parameters"), &parameters) )
	{
		wxLogMessage(wxT("Parameter setting not found in nwnx.ini. Starting server with empty commandline."));
	}
	parameters.Prepend(wxT(" "));

	if (gamespyWatchdog)
	{
		config->Read(wxT("gamespyPort"), &gamespyPort);
		try
		{
			udp = new CUDP("localhost", gamespyPort);
		}
		catch (std::bad_alloc)
		{
			udp = NULL;
		}
	}

	if (!config->Read(wxT("nwn2"), &nwnhome))
	{
		wxLogMessage(wxT("* NWN2 home directory not found. Check your nwnx.ini file."));
		return;
	}
}

NWNXController::~NWNXController()
{
	killServerProcess();

	if (udp)
		delete udp;
}


/***************************************************************************
    NWNServer related functions
***************************************************************************/

void NWNXController::startServerProcess()
{
	killServerProcess(false);

	while (!shuttingDown && !startServerProcessInternal())
	{
		killServerProcess(false);
		wxLogMessage(wxT( "! Error: Failed to start server process, retrying in 5000ms..." ));
		Sleep(5000);
	}
}

void NWNXController::notifyServiceShutdown()
{
	shuttingDown = true;
}

bool NWNXController::startServerProcessInternal()
{
    USES_CONVERSION;
    SHARED_MEMORY shmem;
	wxString nwnexe(wxT("\\nwn2server.exe"));

#ifdef DEBUG
	wchar_t* pszHookDLLPath = L"hhHook.dll";			// Debug DLL
#else
	wchar_t* pszHookDLLPath = L"hhHook.dll";			// Release DLL
#endif

	ZeroMemory(&si, sizeof(si));
	ZeroMemory(&pi, sizeof(pi));
	si.cb = sizeof(si);

	wxLogTrace(TRACE_VERBOSE, wxT("Starting server executable %s in %s"), nwnhome + nwnexe, nwnhome);

	TCHAR szDllPath[MAX_PATH];
	LPWSTR pszFilePart = NULL;

	//	if (!GetFullPathName(pszHookDLLPath, arrayof(szDllPath), szDllPath, &pszFilePart))
	if (!GetFullPathName(pszHookDLLPath, arrayof(szDllPath), szDllPath, &pszFilePart))
	{
		wxLogMessage(wxT("Error: %s could not be found."), pszHookDLLPath);
		return false;
	}

	SECURITY_ATTRIBUTES SecurityAttributes;
	SecurityAttributes.nLength = sizeof(SECURITY_ATTRIBUTES);
	SecurityAttributes.bInheritHandle = TRUE;
	SecurityAttributes.lpSecurityDescriptor = 0;

	shmem.ready_event = CreateEvent(&SecurityAttributes, TRUE, FALSE, 0);
	if(!shmem.ready_event)
	{ 
		wxLogMessage(wxT("CreateEvent failed (%d)"), GetLastError());
		return false;
	}

	wxLogTrace(TRACE_VERBOSE, wxT("Starting: %s"), nwnhome + nwnexe);
	wxLogTrace(TRACE_VERBOSE, wxT("with %s"), szDllPath);

	DWORD dwFlags = CREATE_DEFAULT_ERROR_MODE | CREATE_SUSPENDED;
	SetLastError(0);

	auto wexecutable = A2T((nwnhome + nwnexe).c_str());
	auto wparameters = A2T(parameters.c_str());
	auto wnwnhome = A2T(nwnhome.c_str());
    auto wdllPath = T2A(szDllPath);

	if (!DetourCreateProcessWithDll(wexecutable, wparameters,
                                    NULL, NULL, TRUE, dwFlags, NULL, wnwnhome,
                                    &si, &pi, wdllPath, NULL))
	{
		wxLogMessage(wxT("DetourCreateProcessWithDll failed: %d"), GetLastError());
		CloseHandle( shmem.ready_event );
		ZeroMemory( &pi, sizeof( PROCESS_INFORMATION ) );
		return false;
    }

	GUID my_guid =
	{ /* d9ab8a40-f4cc-11d1-b6d7-006097b010e3 */
		0xd9ab8a40,
		0xf4cc,
		0x11d1,
		{0xb6, 0xd7, 0x00, 0x60, 0x97, 0xb0, 0x10, 0xe3}
	};

	GetCurrentDirectory(MAX_PATH, shmem.nwnx_home);
	wxLogTrace(TRACE_VERBOSE, wxT("NWNX home directory set to %s"), shmem.nwnx_home);

	DetourCopyPayloadToProcess(pi.hProcess, my_guid, &shmem, sizeof(SHARED_MEMORY));

	// Start the main thread running and wait for it to signal that it has read
	// configuration data and started up it's end of any IPC mechanisms that we
	// might rely on.

	ResumeThread(pi.hThread);
	switch(WaitForSingleObject(shmem.ready_event, 60000))
	{
		case WAIT_TIMEOUT:
			wxLogMessage(wxT("! Error: Server did not initialize properly (timeout)."));
			CloseHandle( shmem.ready_event );
			CloseHandle( pi.hProcess );
			CloseHandle( pi.hThread );
			ZeroMemory( &pi, sizeof( PROCESS_INFORMATION ) );
			return false;
			break;
		case WAIT_FAILED:
			wxLogMessage(wxT("! Error: Server did not initialize properly (wait failed)."));
			CloseHandle( shmem.ready_event );
			CloseHandle( pi.hProcess );
			CloseHandle( pi.hThread );
			ZeroMemory( &pi, sizeof( PROCESS_INFORMATION ) );
			return false;
			break;
		case WAIT_OBJECT_0:
			CloseHandle(shmem.ready_event);
			wxLogMessage(wxT("* Success: Server initialized properly."));
			break;
	}

    DWORD dwResult = 0;
    if (!GetExitCodeProcess(pi.hProcess, &dwResult)) 
	{
		wxLogMessage(wxT("GetExitCodeProcess failed: %d\n"), GetLastError());
		return false;
    }
	
	// Reset the count of ping probes to zero for purposes of initial load time
	// GameSpy ping allowances.
	tick = 0;

	// Reset GameSpy failed response count.
	gamespyRetries = 0;

	wxLogMessage(wxT("* Hook installed and initialized successfully"));
	initialized = true;

	return true;
}

void NWNXController::restartServerProcess()
{
    USES_CONVERSION;
	// Kill any leftovers
	if (checkProcessActive())
		killServerProcess(true);
	
	// Run maintenance command
	wxString restartCmd;
	config->Read(wxT("restartCmd"), &restartCmd);
	if (restartCmd != wxT(""))
	{
		PROCESS_INFORMATION pi;
		STARTUPINFO si;

		ZeroMemory(&si,sizeof(si));
		si.cb = sizeof(si);
		wxLogMessage(wxT("* Starting maintenance file %s"), restartCmd);
		restartCmd.Prepend(wxT("cmd.exe /c "));
		if (CreateProcess(NULL, A2T(restartCmd.c_str()), NULL, NULL,FALSE, NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi))
		{
			WaitForSingleObject( pi.hProcess, INFINITE );
			CloseHandle( pi.hProcess );
			CloseHandle( pi.hThread );
		}
	}

	// Finally restart the server
	wxLogMessage(wxT("* Waiting %d seconds before restarting the server."), restartDelay);
	Sleep(restartDelay * 1000);
	wxLogMessage(wxT("* Restarting server."));
	startServerProcess();
}

void NWNXController::killServerProcess(bool graceful)
{
	if (!pi.hProcess)
		return;

	// If we are doing a graceful shutdown, then let's try to poke the
	// server GUI closed so that players are cleanly saved and logged out of
	// the server.

	if (graceful)
	{
		wxLogMessage(wxT( "* Telling server to stop itself..." ));
		if (!performGracefulShutdown())
			wxLogMessage(wxT( "* WARNING: Failed to gracefully shutdown the server process." ));
	}

	// Mark us as not initialized.

	shutdownServerProcess();
	TerminateProcess(pi.hProcess, -1);
	CloseHandle(pi.hProcess);
	if (pi.hThread)
		CloseHandle(pi.hThread);
	ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
}

void NWNXController::shutdownServerProcess()
{
	initialized = false;
}

BOOL CALLBACK NWNXController::findServerGuiWindowEnumProc(__in HWND hwnd, __in LPARAM lParam)
{
	DWORD pid;
	WCHAR className[256];
	PFIND_SERVER_GUI_WINDOW_PARAM param;

	param = reinterpret_cast< PFIND_SERVER_GUI_WINDOW_PARAM >( lParam );

	// Ignore windows that do not match the right nwn2server process id.
	GetWindowThreadProcessId(hwnd, &pid);
	if (pid != param->processId)
		return TRUE;

	// Ignore windows that are not of the class of the main server window GUI.
	if (GetClassNameW(hwnd, className, 256))
	{
		if (!wcscmp(className, L"Exo - BioWare Corp., (c) 1999 - Generic Blank Application"))
		{
			param->hwnd = hwnd;
			return FALSE;
		}
	}

	return TRUE;
}

HWND NWNXController::findServerGuiWindow(ULONG processId)
{
	FIND_SERVER_GUI_WINDOW_PARAM param;

	param.hwnd = 0;
	param.processId = processId;

	EnumWindows(findServerGuiWindowEnumProc, (LPARAM)&param);

	return param.hwnd;
}

bool NWNXController::performGracefulShutdown()
{
	HWND serverGuiWindow;

	// Can't perform a graceful shutdown without a process ID.
	if (!pi.dwProcessId || !pi.hProcess)
		return false;

	// Try and locate the server's administrative GUI window.
	serverGuiWindow = findServerGuiWindow(pi.dwProcessId);

	if (!serverGuiWindow)
		return false;

	// If we have a graceful shutdown message then transmit it before we
	// initiate shutdown.
	if (gracefulShutdownMessage != wxT( "" )) 
	{
		wxLogMessage(wxT( "* Sending shutdown server message and waiting %d seconds."), gracefulShutdownMessageWait);
		broadcastServerMessage(gracefulShutdownMessage.c_str());
		Sleep(gracefulShutdownMessageWait * 1000);
	}

	// Post a WM_CLOSE to the admin interface window, which initiates a clean
	// server shutdown without the blocking confirmation message box (if there
	// were any players present).
	if (!PostMessage(serverGuiWindow, WM_CLOSE, 0, 0))
		return false;

	// Wait for the server process to exit in the timeout interval.
	return (WaitForSingleObject(pi.hProcess, gracefulShutdownTimeout * 1000) == WAIT_OBJECT_0);
}

bool NWNXController::broadcastServerMessage(const TCHAR *message)
{
	HWND srvWnd;
	HWND sendMsgEdit;
	HWND sendMsgButton;
	DWORD_PTR result;

	if (!pi.dwProcessId)
		return false;

	srvWnd = findServerGuiWindow(pi.dwProcessId);

	if (!srvWnd)
		return false;

	sendMsgEdit   = GetDlgItem(srvWnd, IDC_SENDMESSAGE_EDIT);
	sendMsgButton = GetDlgItem(srvWnd, IDC_SENDMESSAGE_BUTTON);

	if (!sendMsgEdit)
		return false;

	if (!sendMsgButton)
		return false;

	SendMessageTimeout(
		sendMsgEdit,
		WM_SETTEXT,
		0,
		reinterpret_cast< LPARAM >( message ),
		SMTO_NORMAL,
		1000,
		&result
		);
	SendMessageTimeout(
		sendMsgButton,
		BM_CLICK,
		0,
		0,
		SMTO_NORMAL,
		1000,
		&result
		);
	SendMessageTimeoutA(
		sendMsgEdit,
		WM_SETTEXT,
		0,
		reinterpret_cast< LPARAM >( "" ),
		SMTO_NORMAL,
		1000,
		&result
		);

	return true;
}

/***************************************************************************
    Watchdog related functions
***************************************************************************/

void NWNXController::ping()
{
	if (initialized)
	{
		if (processWatchdog)
			runProcessWatchdog();
		if (gamespyWatchdog && (tick % gamespyInterval == 0) && (tick > (unsigned long) gamespyDelay))
			runGamespyWatchdog();
		tick++;
	}
}

bool NWNXController::checkProcessActive()
{
	if (!pi.hProcess)
		return false;

	return (WaitForSingleObject( pi.hProcess, 0 ) == WAIT_TIMEOUT);
}

void NWNXController::runProcessWatchdog()
{
	if (checkProcessActive() == false)
	{
		wxLogMessage(wxT("* Server process has gone away."));
		restartServerProcess();
	}
}

void NWNXController::runGamespyWatchdog()
{	
	int ret;
	char buffer[2048];

	udp->sendMessage("BNLM");
	Sleep(UDP_WAIT);
	ret = udp->getMessage(buffer, 2048);

	if (ret == 0)
	{
		// No reply from server
		wxLogMessage(wxT("* Warning: Server did not answer gamespy query. %d retries left."),
			gamespyTolerance - gamespyRetries);
		gamespyRetries++;
	}
	else
	{
		// Server answered
		gamespyRetries = 0;
	}

	if (gamespyRetries > gamespyTolerance)
	{
		// Restart server
		wxLogMessage(wxT("* Server did not answer the last %d gamespy queries."), gamespyTolerance);
		gamespyRetries = 0;
		restartServerProcess();
	}
}
