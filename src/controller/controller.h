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

#if !defined(CONTROLLER_H_INCLUDED)
#define CONTROLLER_H_INCLUDED

#include "../misc/ini.h"
#include "../misc/log.h"
#include "../misc/shmem.h"
#include "udp.h"
#include <cstring>
#include <cwchar>
#include <detours/detours.h>
#include <filesystem>
#include <processenv.h>

#define arrayof(x)             (sizeof(x) / sizeof(x[0]))
#define IDC_SENDMESSAGE_EDIT   0x3FC
#define IDC_SENDMESSAGE_BUTTON 0x400

class NWNXController {
public:
	NWNXController(SimpleIniConfig* config);
	~NWNXController();

	void startServerProcess();
	void notifyServiceShutdown();
	void killServerProcess(bool graceful = true);
	void restartServerProcess();
	void shutdownServerProcess();
	void ping();
	int getGracefulShutdownTimeout() const { return gracefulShutdownTimeout; };

	std::string parameters;
	std::string gracefulShutdownMessage;
	bool processWatchdog;
	bool gamespyWatchdog;
	int gamespyPort;
	int gamespyInterval;
	int gamespyRetries;
	int gamespyTolerance;
	int gracefulShutdownTimeout;
	int gracefulShutdownMessageWait;
	long restartDelay;
	long gamespyDelay;

private:
	SimpleIniConfig* config;

	std::filesystem::path m_nwnx4UserDir;
	std::filesystem::path m_nwnx4InstallDir;
	std::filesystem::path m_nwn2InstallDir;

	std::unique_ptr<CUDP> udp;
	STARTUPINFOA si;
	PROCESS_INFORMATION pi;

	unsigned long tick;
	bool initialized;
	bool shuttingDown;

	void populateUserDir();
	void setupTempDirectories();

	bool startServerProcessInternal();
	bool checkProcessActive();
	void runProcessWatchdog();
	void runGamespyWatchdog();

	typedef struct _FIND_SERVER_GUI_WINDOW_PARAM {
		HWND hwnd;
		ULONG processId;
	} FIND_SERVER_GUI_WINDOW_PARAM, *PFIND_SERVER_GUI_WINDOW_PARAM;

	static BOOL CALLBACK findServerGuiWindowEnumProc(HWND hwnd, LPARAM lParam);
	static HWND findServerGuiWindow(ULONG processId);
	bool performGracefulShutdown();
	bool broadcastServerMessage(const char* message);
};

#endif
