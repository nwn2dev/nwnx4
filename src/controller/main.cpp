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
#include "../misc/windows_utils.h"

std::unique_ptr<NWNXController> controller;
std::unique_ptr<LogNWNX> logger;
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

std::unordered_map<std::string, std::string> parse_command_line(int argc, char *argv[]){
	std::unordered_map<std::string, std::string> ret;
	for(int i = 1 ; i < argc ; i++){
		std::string arg{argv[i]};

		if(arg == "-serviceno" || arg == "-userdir"){
			// Arguments in the form -flag value
			if(++i >= argc){
				std::cerr << "Error: Missing value for argument " << arg << std::endl;
				throw "Argument error";
			}
			ret.insert({arg, std::string{argv[i]}});
		}
		else if(arg.size() >= 1 && arg[0] == '-'){
			// Arguments in the form -flag
			ret.insert({arg, ""});
		}
		else{
			std::cerr << "Error: Command-line arguments must start with '-'. Found " << arg << std::endl;
			throw "Argument error";
		}
	}
	return ret;
}


int main(int argc,char *argv[])
{
	const auto args = parse_command_line(argc, argv);

	// Initialize logger with decent defaults
	LogLevel logLevel = args.contains("-verbose") ? LogLevel::trace : LogLevel::info;
	logger = std::make_unique<LogNWNX>(logLevel);

	if(args.size() == 0){
		logger->Err("No command line parameters specified.");
		logger->Err("Use -help for a list of valid parameters.");
		return 1;
	}

	// Print help
	if(args.contains("-help") || args.contains("-h")){
		logger->Info("Valid parameters are:");
		logger->Info("   -verbose            Increase log verbosity");
		logger->Info("   -userdir <PATH>     Set NWNX4 user dir path instead of using %%CD%%");
		logger->Info("");
		logger->Info("   -interactive        Start in interactive mode");
		logger->Info("");
		logger->Info("   -serviceno <NUM>    Specify service instance number");
		logger->Info("   -startservice       Start the NWNX service");
		logger->Info("   -stopservice        Stop the NWNX service");
		logger->Info("   -installservice     Install the NWNX service");
		logger->Info("   -uninstallservice   Uninstall the NWNX service");
		return 0;
	}

	// Move inside NWNX4 UserDir
	if(args.contains("-userdir")){
		auto userDir = std::filesystem::path{args.at("-userdir")};
		if(!std::filesystem::exists(userDir)){
			std::filesystem::create_directories(userDir);
		}
		SetCurrentDirectoryA(args.at("-userdir").c_str());
	}

	// Parse service number (may be used later)
	serviceNo = 1;
	if(args.contains("-serviceno")){
		try{
			serviceNo = std::stoi(args.at("-serviceno"));
		}
		catch(std::invalid_argument){
			std::cerr << "Error: Service number '" << args.at("-serviceno") << "' is not an int" << std::endl;
			return 1;
		}
	}

	if(args.contains("-interactive") || args.contains("-runservice")){
		logger->Info("NWNX4 " NWNX_VERSION_INFO);
		logger->Info("(c) 2008 by Ingmar Stieger (Papillon)");
		logger->Info("Contribute to the project at https://github.com/nwn2dev/nwnx4 !");

		if(args.contains("-runservice")){
			// Redirect logs to a file instead
			logger = std::make_unique<LogNWNX>("nwnx_controller.txt", logLevel);

			// userdir not explicitely set, move to the exe dir
			if(!args.contains("-userdir")){
				char executablePath[MAX_PATH] = {0};
				if(GetModuleFileNameA(nullptr, executablePath, MAX_PATH) == 0){
					auto errInfo = GetLastErrorInfo();
					logger->Warn("Could not get executable path: Error %d: %s", errInfo.first, errInfo.second);
				}
				else{
					SetCurrentDirectoryA(executablePath);
				}
			}
		}

		// open ini file
		std::string inifile("nwnx.ini");
		logger->Info("Reading ini file '%s'", inifile.c_str());
		auto config = SimpleIniConfig(inifile);

		// Reconfigure logger according to INI config
		logger->Configure(&config);

		controller = std::make_unique<NWNXController>(&config);

		if (args.contains("-interactive"))
		{
			// start in interactive mode
			logger->Info("Running in interactive mode.");
			logger->Info("Press enter to stop the controller.");
			logger->Info("NWNX4 plugins will continue working inside the nwn2server process.");
			start_worker();
			getc(stdin);
		}
		else if (args.contains("-runservice"))
		{
			// start as service
			wchar_t serviceName[64];
			swprintf(serviceName, 64, L"NWNX4-%d", serviceNo);

			SERVICE_TABLE_ENTRY DispatchTable[] = {{ serviceName, NWNXServiceStart}, { nullptr, nullptr }};
			if (!StartServiceCtrlDispatcher(DispatchTable))
			{
				auto errInfo = GetLastErrorInfo();
				logger->Err("* StartServiceCtrlDispatcher (%d): %s", errInfo.first, errInfo.second);
			}
		}
		else
			assert(0);
	}
	else if(args.contains("-startservice")){
		if(!StartNWNXService(serviceNo)){
			logger->Err("Failed to start service");
			return 1;
		}
	}
	else if(args.contains("-stopservice")){
		if(!StopNWNXService(serviceNo)){
			logger->Err("Failed to stop service");
			return 1;
		}
	}
	else if(args.contains("-installservice")){
		if(!installservice(serviceNo)){
			logger->Err("Failed to install service");
			return 1;
		}
	}
	else if(args.contains("-uninstallservice")){
		if(!uninstallservice(serviceNo)){
			logger->Err("Failed to uninstall service");
			return 1;
		}
	}
	else{
		logger->Err("No action specified. Use -interactive or -runservice to start the server. -help to print all commands.");
		return 1;
	}
	return 0;
}
