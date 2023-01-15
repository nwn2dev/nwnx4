/***************************************************************************
    NWNX Controller - Windows Service handling functions
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

#include "service.h"
#include "../misc/windows_utils.h"

extern std::unique_ptr<LogNWNX> logger;
SERVICE_STATUS_HANDLE NWNXServiceStatusHandle;
SERVICE_STATUS NWNXServiceStatus;

extern int serviceNo;
extern void start_worker(void);
extern std::unique_ptr<NWNXController> controller;

SC_HANDLE getSCManager()
{
	// Return a handle to the SC Manager database.
	SC_HANDLE schSCManager = OpenSCManager(nullptr, // local machine
	                                       nullptr, // ServicesActive database
	                                       SC_MANAGER_ALL_ACCESS); // full access rights

	if (schSCManager == nullptr) {
		auto errInfo = GetLastErrorInfo();
		logger->Err("Failed to connect to service manager. Error %d: %s", errInfo.first,
		            errInfo.second);
	}

	return schSCManager;
}

BOOL installservice(int serviceNo)
{
	SC_HANDLE schSCManager, schService;
	SERVICE_DESCRIPTION sdBuf;

	wchar_t szPath[MAX_PATH], cmdLine[MAX_PATH * 2];
	wchar_t serviceName[64];
	wchar_t displayName[64];

	logger->Info("Installing NWNX Service %d...", serviceNo);

	schSCManager = getSCManager();
	if (nullptr == schSCManager)
		return FALSE;

	if (!GetModuleFileName(nullptr, szPath, MAX_PATH)) {
		auto errInfo = GetLastErrorInfo();
		logger->Err("GetModuleFileName failed. Error %d: %s", errInfo.first, errInfo.second);
		return FALSE;
	}

	wchar_t userDir[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, userDir);

	swprintf(serviceName, 64, L"NWNX4-%d", serviceNo);
	swprintf(displayName, 64, L"NWNX4 Service %d", serviceNo);
	swprintf(cmdLine, MAX_PATH, L"\"%s\" -serviceno %d -userdir \"%s\" -runservice", szPath,
	         serviceNo, userDir);
	// non-const cast: we can assume ChangeServiceConfig2 does not write to this
	sdBuf.lpDescription = (wchar_t*)L"Neverwinter Nights Extender 4 service instance";

	schService = CreateService(schSCManager, // SCManager database
	                           serviceName, // name of service
	                           displayName, // service name to display
	                           SERVICE_ALL_ACCESS, // desired access
	                           SERVICE_WIN32_OWN_PROCESS, // service type
	                           SERVICE_DEMAND_START, // start type
	                           SERVICE_ERROR_NORMAL, // error control type
	                           cmdLine, // path to service's binary
	                           nullptr, // no load ordering group
	                           nullptr, // no tag identifier
	                           nullptr, // no dependencies
	                           nullptr, // LocalSystem account
	                           nullptr); // no password

	if (schService == nullptr) {
		auto errInfo = GetLastErrorInfo();
		logger->Err("CreateService failed. Error %d: %s", errInfo.first, errInfo.second);
		return FALSE;
	}

	if (!ChangeServiceConfig2(schService, // handle to service
	                          SERVICE_CONFIG_DESCRIPTION, // change: description
	                          &sdBuf)) // value: new description
	{
		logger->Err("ChangeServiceConfig2 failed");
		return FALSE;
	}

	CloseServiceHandle(schService);
	logger->Info("Installed service %ls with userdir %ls", serviceName, userDir);
	return TRUE;
}

BOOL uninstallservice(int serviceNo)
{
	SC_HANDLE schSCManager, schService;
	wchar_t serviceName[64];

	swprintf(serviceName, 64, L"NWNX4-%d", serviceNo);
	logger->Info("Uninstalling NWNX Service %d...", serviceNo);

	schSCManager = getSCManager();
	if (nullptr == schSCManager)
		return FALSE;

	schService = OpenService(schSCManager, // SCManager database
	                         serviceName, // name of service
	                         DELETE); // only need DELETE access

	if (schService == nullptr) {
		auto errInfo = GetLastErrorInfo();
		logger->Err("OpenService failed. Error %d: %s", errInfo.first, errInfo.second);
		return FALSE;
	}

	if (!DeleteService(schService)) {
		auto errInfo = GetLastErrorInfo();
		logger->Err("DeleteService failed. Error %d: %s", errInfo.first, errInfo.second);
		return FALSE;
	} else
		logger->Info("DeleteService succeeded");

	CloseServiceHandle(schService);
	return TRUE;
}

void WINAPI NWNXServiceStart(DWORD argc, LPTSTR* argv)
{
	DWORD status;
	DWORD specificError;
	wchar_t serviceName[64];

	NWNXServiceStatus.dwServiceType             = SERVICE_WIN32;
	NWNXServiceStatus.dwCurrentState            = SERVICE_START_PENDING;
	NWNXServiceStatus.dwControlsAccepted        = SERVICE_ACCEPT_STOP;
	NWNXServiceStatus.dwWin32ExitCode           = 0;
	NWNXServiceStatus.dwServiceSpecificExitCode = 0;
	NWNXServiceStatus.dwCheckPoint              = 0;
	NWNXServiceStatus.dwWaitHint                = 0;

	swprintf(serviceName, L"NWNX4-%d", serviceNo);
	NWNXServiceStatusHandle = RegisterServiceCtrlHandler(serviceName, NWNXServiceCtrlHandler);

	if (NWNXServiceStatusHandle == (SERVICE_STATUS_HANDLE)0) {
		auto errInfo = GetLastErrorInfo();
		logger->Err("RegisterServiceCtrlHandler failed. Error %d: %s", errInfo.first,
		            errInfo.second);
		return;
	}

	// Initialization code goes here.
	status = NWNXServiceInitialization(argc, argv, &specificError);

	// Handle error condition
	if (status != NO_ERROR) {
		NWNXServiceStatus.dwCurrentState            = SERVICE_STOPPED;
		NWNXServiceStatus.dwCheckPoint              = 0;
		NWNXServiceStatus.dwWaitHint                = 0;
		NWNXServiceStatus.dwWin32ExitCode           = status;
		NWNXServiceStatus.dwServiceSpecificExitCode = specificError;

		SetServiceStatus(NWNXServiceStatusHandle, &NWNXServiceStatus);
		return;
	}

	// Initialization complete - report running status.
	NWNXServiceStatus.dwCurrentState = SERVICE_RUNNING;
	NWNXServiceStatus.dwCheckPoint   = 0;
	NWNXServiceStatus.dwWaitHint     = 0;

	if (!SetServiceStatus(NWNXServiceStatusHandle, &NWNXServiceStatus)) {
		auto errInfo = GetLastErrorInfo();
		status       = errInfo.first;
		logger->Err("SetServiceStatus error. Error %d: %s", errInfo.first, errInfo.second);
	}

	// This is where the service does its work.
	start_worker();
}

// Stub initialization function.
DWORD NWNXServiceInitialization(DWORD argc, LPTSTR* argv, DWORD* specificError)
{
	argv;
	argc;
	specificError;
	return (0);
}

VOID WINAPI NWNXServiceCtrlHandler(DWORD Opcode)
{
	switch (Opcode) {
		case SERVICE_CONTROL_STOP:

			// Notify the SCM immediately that we received the request and are
			// working on stopping.

			NWNXServiceStatus.dwWin32ExitCode = 0;
			NWNXServiceStatus.dwCurrentState  = SERVICE_STOP_PENDING;
			NWNXServiceStatus.dwCheckPoint    = 0;
			NWNXServiceStatus.dwWaitHint
			    = (DWORD)(controller->getGracefulShutdownTimeout() + 6) * 1000;

			if (!SetServiceStatus(NWNXServiceStatusHandle, &NWNXServiceStatus)) {
				auto errInfo = GetLastErrorInfo();
				logger->Err("SetServiceStatus error. Error %d: %s", errInfo.first, errInfo.second);
			}

			// Cleanly shutdown the server process.

			controller->notifyServiceShutdown();
			controller->killServerProcess();

			NWNXServiceStatus.dwWin32ExitCode = 0;
			NWNXServiceStatus.dwCurrentState  = SERVICE_STOPPED;
			NWNXServiceStatus.dwCheckPoint    = 0;
			NWNXServiceStatus.dwWaitHint      = 0;

			if (!SetServiceStatus(NWNXServiceStatusHandle, &NWNXServiceStatus)) {
				auto errInfo = GetLastErrorInfo();
				logger->Err("SetServiceStatus error. Error %d: %s", errInfo.first, errInfo.second);
			}

			logger->Info("Service successfully stopped.");
			return;

		case SERVICE_CONTROL_INTERROGATE:
			// Fall through to send current status.
			break;

		default:;
			logger->Err("Unrecognized opcode %ld", Opcode);
	}

	// Send current status.
	if (!SetServiceStatus(NWNXServiceStatusHandle, &NWNXServiceStatus)) {
		auto errInfo = GetLastErrorInfo();
		logger->Err("SetServiceStatus error. Error %d: %s", errInfo.first, errInfo.second);
	}
	return;
}

BOOL StartNWNXService(int serviceNo)
{
	SC_HANDLE schService;
	SC_HANDLE schSCManager;
	SERVICE_STATUS_PROCESS ssStatus;
	DWORD dwOldCheckPoint;
	DWORD dwStartTickCount;
	DWORD dwWaitTime;
	DWORD dwBytesNeeded;
	wchar_t serviceName[64];

	schSCManager = getSCManager();
	if (nullptr == schSCManager)
		return FALSE;

	swprintf(serviceName, L"NWNX4-%d", serviceNo);
	schService = OpenService(schSCManager, // SCManager database
	                         serviceName, // name of service
	                         SERVICE_ALL_ACCESS);

	if (schService == nullptr) {
		auto errInfo = GetLastErrorInfo();
		logger->Err("OpenService failed. Error %d: %s", errInfo.first, errInfo.second);
		return FALSE;
	}

	if (!StartService(schService, // handle to service
	                  0, // number of arguments
	                  nullptr)) // no arguments
	{
		return 0;
	} else {
		logger->Info("Starting NWNX service %d...", serviceNo);
	}

	// Check the status until the service is no longer start pending.

	if (!QueryServiceStatusEx(schService, // handle to service
	                          SC_STATUS_PROCESS_INFO, // info level
	                          (LPBYTE)&ssStatus, // address of structure
	                          sizeof(SERVICE_STATUS_PROCESS), // size of structure
	                          &dwBytesNeeded)) // if buffer too small
	{
		return 0;
	}

	// Save the tick count and initial checkpoint.

	dwStartTickCount = GetTickCount();
	dwOldCheckPoint  = ssStatus.dwCheckPoint;

	while (ssStatus.dwCurrentState == SERVICE_START_PENDING) {
		// Do not wait longer than the wait hint. A good interval is
		// one tenth the wait hint, but no less than 1 second and no
		// more than 10 seconds.

		dwWaitTime = ssStatus.dwWaitHint / 10;

		if (dwWaitTime < 1000)
			dwWaitTime = 1000;
		else if (dwWaitTime > 10000)
			dwWaitTime = 10000;

		Sleep(dwWaitTime);

		// Check the status again.

		if (!QueryServiceStatusEx(schService, // handle to service
		                          SC_STATUS_PROCESS_INFO, // info level
		                          (LPBYTE)&ssStatus, // address of structure
		                          sizeof(SERVICE_STATUS_PROCESS), // size of structure
		                          &dwBytesNeeded)) // if buffer too small
			break;

		if (ssStatus.dwCheckPoint > dwOldCheckPoint) {
			// The service is making progress.
			dwStartTickCount = GetTickCount();
			dwOldCheckPoint  = ssStatus.dwCheckPoint;
		} else {
			if (GetTickCount() - dwStartTickCount > ssStatus.dwWaitHint) {
				// No progress made within the wait hint
				break;
			}
		}
	}

	CloseServiceHandle(schService);

	if (ssStatus.dwCurrentState == SERVICE_RUNNING) {
		logger->Info("Service successfully started.");
		return 1;
	} else {
		logger->Err("Service not started.");
		logger->Err("Current State: %d", ssStatus.dwCurrentState);
		logger->Err("Exit Code: %d", ssStatus.dwWin32ExitCode);
		logger->Err("Service Specific Exit Code: %d", ssStatus.dwServiceSpecificExitCode);
		logger->Err("Check Point: %d", ssStatus.dwCheckPoint);
		logger->Err("Wait Hint: %d", ssStatus.dwWaitHint);
		return 0;
	}
}

// This function attempts to stop a service. It allows the caller to
// specify whether dependent services should also be stopped. It also
// accepts a timeout value, to prevent a scenario in which a service
// shutdown hangs, then the application stopping the service hangs.
//
// If the operation is successful, returns ERROR_SUCCESS. Otherwise,
// returns a system error code.

DWORD StopNWNXService(int serviceNo)
{
	SC_HANDLE schService;
	SC_HANDLE schSCManager;
	SERVICE_STATUS_PROCESS ssStatus;
	DWORD dwStartTime = GetTickCount();
	DWORD dwBytesNeeded;
	DWORD dwTimeout = 10000; // msec
	wchar_t serviceName[64];

	swprintf(serviceName, L"NWNX4-%d", serviceNo);

	schSCManager = getSCManager();
	if (nullptr == schSCManager)
		return FALSE;

	schService = OpenService(schSCManager, // SCManager database
	                         serviceName, SERVICE_ALL_ACCESS);

	if (schService == nullptr) {
		auto errInfo = GetLastErrorInfo();
		logger->Err("OpenService failed. Error %d: %s", errInfo.first, errInfo.second);
		return FALSE;
	}

	logger->Info("Stopping NWNX service %d...", serviceNo);

	// Make sure the service is not already stopped
	if (!QueryServiceStatusEx(schService, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssStatus,
	                          sizeof(SERVICE_STATUS_PROCESS), &dwBytesNeeded))
		return GetLastError();

	if (ssStatus.dwCurrentState == SERVICE_STOPPED)
		return ERROR_SUCCESS;

	// If a stop is pending, just wait for it
	while (ssStatus.dwCurrentState == SERVICE_STOP_PENDING) {
		Sleep(ssStatus.dwWaitHint);
		if (!QueryServiceStatusEx(schService, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssStatus,
		                          sizeof(SERVICE_STATUS_PROCESS), &dwBytesNeeded))
			return GetLastError();

		if (ssStatus.dwCurrentState == SERVICE_STOPPED)
			return ERROR_SUCCESS;

		if (GetTickCount() - dwStartTime > dwTimeout)
			return ERROR_TIMEOUT;
	}

	// Send a stop code to the main service
	SERVICE_STATUS ss;
	if (!ControlService(schService, SERVICE_CONTROL_STOP, &ss))
		return GetLastError();

	// Wait for the service to stop
	while (ss.dwCurrentState != SERVICE_STOPPED) {
		Sleep(ss.dwWaitHint);
		if (!QueryServiceStatusEx(schService, SC_STATUS_PROCESS_INFO, (LPBYTE)&ss,
		                          sizeof(SERVICE_STATUS), &dwBytesNeeded))
			return GetLastError();

		if (ss.dwCurrentState == SERVICE_STOPPED)
			break;

		if (GetTickCount() - dwStartTime > dwTimeout)
			return ERROR_TIMEOUT;
	}

	// Return success
	return ERROR_SUCCESS;
}
