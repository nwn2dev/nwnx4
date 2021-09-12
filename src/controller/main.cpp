/***************************************************************************
    NWNX Controller - Main function
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

#include "controller.h"
#include "service.h"
#include "../nwnx_version.h"

enum actions { no_action, run_interactive, run_service };
BOOL STARTUP_ACTION;

NWNXController* controller;

LogNWNX* logger;
int serviceNo;

void start_worker(void);
DWORD WINAPI workerProcessThread(LPVOID);
void process_command_line(int argc,char *argv[]);
int main(int argc,char *argv[]);

void start_worker()
{
	DWORD dwThreadId;
	HANDLE hThread;

	logger->Trace("Starting worker...");

	// Start the worker thread
	hThread = CreateThread(nullptr, 0, workerProcessThread, nullptr, 0, &dwThreadId);
}

// The thread that does the actual work
DWORD WINAPI workerProcessThread(LPVOID lpParam)
{
	controller->startServerProcess();

	// Endless loop
	while (1)
	{
		Sleep(1000);
		controller->ping();
	}
	return 0;
}

/***************************************************************************
	Entry Point / main()
***************************************************************************/

void process_command_line(int argc,char *argv[])
{
	std::string logfile = "nwnx_controller.txt";
	LogLevel logLevel = LogLevel::info;

	// Search for verbosity arg
	for (int i = 1; i < argc; i++)
	{
		if (_stricmp(argv[i], "-verbose") == 0){
			logLevel = LogLevel::trace;
		}
	}

	// decide on log target (depending on whether
	// we run interactive or as a service).
	logger = nullptr;
	for (int i = 0; i < argc; i++)
	{
		if (
			(_stricmp(argv[i], "-interactive") == 0) ||
			(_stricmp(argv[i], "-help") == 0)
			)
		{
			logger = new LogNWNX(logLevel);
			break;
		}
	}
	if (argc == 1)
		logger = new LogNWNX(logLevel);
	if (!logger)
		logger = new LogNWNX(logfile, logLevel);

	logger->Info("");
	logger->Info("NWNX4 Server Controller version " NWNX_VERSION_INFO);
	logger->Info("(c) 2008 by Ingmar Stieger (Papillon)");
	logger->Info("visit us at http://www.nwnx.org");

	if (argc == 1)
	{
		logger->Err("No command line parameters specified.");
		logger->Err("Use -help for a list of valid parameters.");
		return;
	}

	// scan for service number parameter
	for (int i = 0; i < argc; i++)
	{
		if (_stricmp(argv[i], "-serviceno") == 0)
		{
			if (argc > i)
			{
				// next parameter is the number
				i++;
				if (argv[i])
				{
					serviceNo = atoi(argv[i]);
					if ((serviceNo < 1) || (serviceNo > 1024))
						serviceNo = 1;
				}
			}
		}
	}

	for (int i = 0; i < argc; i++)
	{
		if (_stricmp(argv[i], "-help") == 0)
		{
			logger->Info("Valid parameters are:");
			logger->Info("   -serviceno          Specify service instance number");
			logger->Info("   -startservice       Start the NWNX service");
			logger->Info("   -stopservice        Stop the NWNX service");
			logger->Info("   -installservice     Install the NWNX service");
			logger->Info("   -uninstallservice   Uninstall the NWNX service");
			logger->Info("   -interactive        Start in interactive mode");

			STARTUP_ACTION = no_action;
		}

		if (_stricmp(argv[i], "-interactive") == 0)
		{
			STARTUP_ACTION = run_interactive;
		}

		if (_stricmp(argv[i], "-startservice") == 0)
		{
			StartNWNXService(serviceNo);
		}
		if (_stricmp(argv[i], "-stopservice") == 0)
		{
			StopNWNXService(serviceNo);
		}

		if (_stricmp(argv[i], "-runservice") == 0)
		{
			// if called from Service Control Manager...
			STARTUP_ACTION = run_service;
		}

		if (_stricmp(argv[i], "-installservice") == 0)
		{
			installservice(serviceNo);
		}

		if (_stricmp(argv[i], "-uninstallservice") == 0)
		{
			uninstallservice(serviceNo);
		}
	}
}



int main(int argc,char *argv[])
{
    // init
	STARTUP_ACTION = no_action;
	serviceNo = 1;

	// Set the current working directory to the executeables base directory
	wchar_t path_buffer[_MAX_PATH];
	wchar_t drive[_MAX_DRIVE];
	wchar_t dir[_MAX_DIR];
	GetModuleFileName(nullptr, path_buffer, MAX_PATH);
	_wsplitpath( path_buffer, drive, dir, nullptr, nullptr);
	_wmakepath(path_buffer, drive, dir, nullptr, nullptr);
	SetCurrentDirectory(path_buffer);

	// set up logging and process command line parameters
	process_command_line(argc, argv);

	// open ini file
	std::string inifile("nwnx.ini");
	logger->Trace("Reading ini file '%s'", inifile.c_str());
	auto config = new SimpleIniConfig(inifile);
	logger->Configure(config);
	controller = new NWNXController(config);

	if (STARTUP_ACTION == run_interactive)
	{
		// start in interactive mode
		logger->Info("Running in interactive mode.");
		logger->Info("Press enter to stop the controller.");
		logger->Info("NWNX will continue to run within nwnserver.");
		start_worker();
		getc(stdin);
	}
	else if (STARTUP_ACTION == run_service)
	{
		// start as service
		wchar_t serviceName[64];
		swprintf(serviceName, 64, L"NWNX4-%d", serviceNo);

		SERVICE_TABLE_ENTRY DispatchTable[] = {{ serviceName, NWNXServiceStart}, { nullptr, nullptr }};
		if (!StartServiceCtrlDispatcher(DispatchTable))
		{
			logger->Err("* StartServiceCtrlDispatcher (%d)", GetLastError());
		}

	}
	else {
		logger->Err("No action specified. Use -interactive or -runservice to start the server.");
	}
	return 0;
}
