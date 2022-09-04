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

#include "controller.h"
#include "../misc/log.h"

VOID WINAPI NWNXServiceCtrlHandler (DWORD opcode); 
DWORD NWNXServiceInitialization(DWORD argc, LPTSTR *argv, DWORD *specificError); 

SC_HANDLE getSCManager();
BOOL installservice(int serviceNo);
BOOL uninstallservice(int serviceNo);
void WINAPI NWNXServiceStart (DWORD argc, LPTSTR *argv);
DWORD NWNXServiceInitialization(DWORD argc, LPTSTR *argv, DWORD *specificError);
VOID WINAPI NWNXServiceCtrlHandler(DWORD Opcode);
BOOL StartNWNXService(int serviceNo);
DWORD StopNWNXService(int serviceNo);
