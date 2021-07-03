/***************************************************************************
    NWNX Time - Timer functions plugin
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

#include "time_plugin.h"
#include <cassert>

/***************************************************************************
    NWNX and DLL specific functions
***************************************************************************/

Timer* plugin;
LogNWNX* logger;

DLLEXPORT Plugin* GetPluginPointerV2()
{
	return plugin;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		plugin = new Timer();

		char szPath[MAX_PATH];
		GetModuleFileNameA(hModule, szPath, MAX_PATH);
		plugin->SetPluginFullPath(szPath);
	}
	else if (ul_reason_for_call == DLL_PROCESS_DETACH)
	{
		delete plugin;
	}
    return TRUE;
}


/***************************************************************************
    Implementation of Timer Plugin
***************************************************************************/

Timer::Timer()
{
	description = 
		"This plugin provides highly accurate timers.";

	subClass = "TIME";
	version = "0.0.2";

	QueryPerformanceFrequency(&liFrequency);
}

Timer::~Timer()
{
	logger->Info("* Plugin unloaded.");
}

bool Timer::Init(char* nwnxhome)
{
	assert(GetPluginFileName());

	/* Log file */
	std::string logfile(nwnxhome);
	logfile += "\\";
	logfile += GetPluginFileName();
	logfile += ".txt";
	logger = new LogNWNX(logfile);

	logger->Info("* Plugin initialized.");

	logger->Info("NWNX Timer Plugin V.0.0.2");
	logger->Info("(c) 2007 by Ingmar Stieger (Papillon)");
	logger->Info("visit us at http://www.nwnx.org");
	return true;
}

void Timer::GetFunctionClass(char* fClass)
{
	strncpy(fClass, "TIME", 5);
}

void Timer::SetString(char* sFunction, char* sParam1, int nParam2, char* sValue)
{
	logger->Trace("* Plugin SetString(0x%x, %s, %d, %s)", 0x0, sParam1, nParam2, sValue);

	std::string function(sFunction);
	std::string timerName(sParam1);

	if (function == "")
	{
		logger->Info("* Function not specified.");
		return;
	}

	if (timerName == "")
	{
		logger->Info("* Timer name not specified.");
		return;
	}

	if (function == "START")
	{
		logger->Info("o Starting timer %s", timerName.c_str());
		StartTimer(timerName);
	}
	else if (function == "STOP")
	{
		LONGLONG result = StopTimer(timerName);
		logger->Info("o Stopping timer %s: %I64i µs / %.3f msec / %.3f sec\n", timerName.c_str(), result, (float) result / 1000, (float) result / 1000 / 1000);
	}
}

char* Timer::GetString(char* sFunction, char* sParam1, int nParam2)
{
	logger->Trace("* Plugin GetString(0x%x, %s, %d)", 0x0, sParam1, nParam2);

	std::string function(sFunction);
	std::string timerName(sParam1);

	if (function == "")
	{
		logger->Info("* Function not specified.");
		return NULL;
	}

	if (timerName == "")
	{
		logger->Info("* Timer name not specified.");
	}

	if (function == "QUERY")
	{
		LONGLONG result = PeekTimer(timerName);
		logger->Info("o Elapsed timer %s: %I64i µs / %.3f msec / %.3f sec\n", timerName.c_str(), result, (float) result / 1000, (float) result / 1000 / 1000);
		sprintf_s(returnBuffer, MAX_BUFFER, "%I64i", result);
	}
	else if (function == "STOP")
	{
		LONGLONG result = StopTimer(timerName);
		logger->Info("o Stopping timer %s: %I64i µs / %.3f msec / %.3f sec\n", timerName.c_str(), result, (float) result / 1000, (float) result / 1000 / 1000);
		sprintf_s(returnBuffer, MAX_BUFFER, "%I64i", result);
	}
	else
	{
		// Process generic functions
		std::string query = ProcessQueryFunction(function.c_str());
		if (query != "")
		{
			sprintf_s(returnBuffer, MAX_BUFFER, "%s", query);
		}
		else
		{
			logger->Info("* Unknown function '%s' called.", function.c_str());
			return NULL;
		}
	}

	return returnBuffer;
}

void Timer::StartTimer(std::string name)
{
	LARGE_INTEGER liLast;
	QueryPerformanceCounter(&liLast);
	timers[name] = liLast;
}

LONGLONG Timer::StopTimer(std::string name)
{
	LONGLONG result = PeekTimer(name);
	timers.erase(name);
	return result;
}

LONGLONG Timer::PeekTimer(std::string name)
{
	LARGE_INTEGER liCurrent;
	QueryPerformanceCounter(&liCurrent);

	TimerHashMap::iterator it = timers.find(name);
	if (it != timers.end())
		return (((liCurrent.QuadPart - it->second.QuadPart) * 1000000) / liFrequency.QuadPart);
	else
		return 0;
}
