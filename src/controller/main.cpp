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

#include "stdwx.h"
#include "controller.h"
#include "service.h"
#include "wx/fileconf.h"

enum actions { no_action, run_interactive, run_service };
BOOL STARTUP_ACTION;

NWNXController* controller;

wxLogNWNX* logger;
int serviceNo;

void start_worker(void);
DWORD WINAPI workerProcessThread(LPVOID);
void process_command_line(int argc,char *argv[]);
int main(int argc,char *argv[]);

const wxString header = 
	wxT("\nNWN Extender 4 Server Controller V.0.0.9\n") \
	wxT("(c) 2008 by Ingmar Stieger (Papillon)\n") \
	wxT("visit us at http://www.nwnx.org\n");

void start_worker()
{
	DWORD dwThreadId;
	HANDLE hThread; 

	wxLogTrace(TRACE_VERBOSE, wxT("Starting worker..."));

	// Start the worker thread
	hThread = CreateThread(NULL, 0, workerProcessThread, NULL, 0, &dwThreadId);                
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
	wxString logfile = wxT("nwnx_controller.txt");

	// decide on log target (depending on whether
	// we run interactive or as a service). 
	logger = NULL;
	for (int i = 0; i < argc; i++)
	{
		if (
			(_stricmp(argv[i], "-interactive") == 0) ||
			(_stricmp(argv[i], "-help") == 0)			
			) 
		{
			logger = new wxLogNWNX(NULL, header);
			break;
		}
	}
	if (argc == 1) 
		logger = new wxLogNWNX();
	if (!logger)
		logger = new wxLogNWNX(logfile, header);

	if (argc == 1) 
	{
		wxLogError(wxT("No command line parameters specified.\nUse -help for a list of valid parameters."));
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
			wxLogMessage(wxT("Valid parameters are:\n"));
			wxLogMessage(wxT("   -serviceno          Specify service instance number"));
			wxLogMessage(wxT("   -startservice       Start the NWNX service"));
			wxLogMessage(wxT("   -stopservice        Stop the NWNX service"));
			wxLogMessage(wxT("   -installservice     Install the NWNX service"));
			wxLogMessage(wxT("   -uninstallservice   Uninstall the NWNX service"));
			wxLogMessage(wxT("   -interactive        Start in interactive mode"));

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
    USES_CONVERSION;

	// init
	STARTUP_ACTION = no_action;
	serviceNo = 1;

	// Set the current working directory to the executeables base directory
	TCHAR path_buffer[_MAX_PATH];
	TCHAR drive[_MAX_DRIVE];
	TCHAR dir[_MAX_DIR];
	GetModuleFileName(NULL, path_buffer, MAX_PATH);
	_wsplitpath( path_buffer, drive, dir, NULL, NULL );
	_wmakepath(path_buffer, drive, dir, NULL, NULL);
	SetCurrentDirectory(path_buffer);

	// set up logging and process command line parameters
	process_command_line(argc, argv);

	// open ini file
	std::cout << "here!";
	wxString inifile(wxT("nwnx.ini")); 
	wxLogTrace(TRACE_VERBOSE, wxT("Reading inifile %s"), inifile);
	wxFileConfig* config = new wxFileConfig(wxEmptyString, wxEmptyString, 
		inifile, wxEmptyString, wxCONFIG_USE_RELATIVE_PATH|wxCONFIG_USE_NO_ESCAPE_CHARACTERS);

	// Setup temporary directories
	wxString tempPath;
	config->Read(wxT("nwn2temp"), &tempPath);
	if (tempPath != wxT(""))
	{
		SetEnvironmentVariable(wxT("TEMP"), A2T(tempPath.c_str()));
		SetEnvironmentVariable(wxT("TMP"), A2T(tempPath.c_str()));
	}

	controller = new NWNXController(config);

	if (STARTUP_ACTION == run_interactive)
	{
		// start in interactive mode
		wxLogMessage(wxT("Running in interactive mode."));
		wxLogMessage(wxT("Press enter to stop the controller."));
		wxLogMessage(wxT("NWNX will continue to run within nwnserver."));
		start_worker();
		getc(stdin);
	}
	else if (STARTUP_ACTION == run_service)
	{
		// start as service 
		TCHAR serviceName[64];
		_stprintf_s(serviceName, 64, wxT("NWNX4-%d"), serviceNo);

		SERVICE_TABLE_ENTRY DispatchTable[] = {{ serviceName, NWNXServiceStart}, { NULL, NULL }}; 
		if (!StartServiceCtrlDispatcher(DispatchTable)) 
		{ 
			wxLogError(wxT("* StartServiceCtrlDispatcher (%d)"), GetLastError()); 
		} 

	}
	return 0;
}
