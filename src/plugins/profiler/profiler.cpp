/***************************************************************************
    NWNX Profiler - Profiler plugin
	Copyright (C) 2003 Ingmar Stieger (Papillon, papillon@blackdagger.com)
    Copyright (C) 2007 virusman (virusman@virusman.ru)

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

#include "profiler.h"
#include "hook.h"
#include <cassert>

/***************************************************************************
    NWNX and DLL specific functions
***************************************************************************/

Profiler* plugin;

DLLEXPORT Plugin* GetPluginPointerV2()
{
	return plugin;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		plugin = new Profiler();

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
    Implementation of Functions Plugin
***************************************************************************/

Profiler::Profiler()
{
	description =
		"This plugin provides script profiling functionality.";

	subClass = "PROFILER";
	version = "1.0.0";
}

Profiler::~Profiler()
{
	logger->Info("* Plugin unloaded.");
}

bool Profiler::Init(char* nwnxhome)
{
	assert(GetPluginFileName());

	/* Log file */
	std::string logfile(nwnxhome);
	logfile.append("\\");
	logfile.append(GetPluginFileName());
	logfile.append(".txt");
	logger = new LogNWNX(logfile);


	logger->Info("NWNX Profiler Plugin V.1.0.0");
	logger->Info("(c) 2003 Ingmar Stieger (papillon@blackdagger.com)");
	logger->Info("(c) 2007 virusman (virusman@virusman.ru)");
	logger->Info("visit us at http://www.nwnx.org");

	LoadConfiguration(nwnxhome);
	HookRunScript();

	logger->Info("* Plugin initialized.");
	return true;
}

void Profiler::GetFunctionClass(char* fClass)
{
	strcpy(fClass, "PROFILER");
}

void Profiler::LoadConfiguration(char* nwnxhome)
{
	std::string inifile(nwnxhome);
	inifile.append("\\");
	inifile.append(GetPluginFileName());
	inifile.append(".ini");
	logger->Trace("* reading inifile %s", inifile.c_str());

	config = new SimpleIniConfig(inifile);
	logger->Configure(config);

	config->Read("LogLevel", &m_LogLevel);
	config->Read("scriptparts", &log_scriptparts);

	logger->Info("* Log level: ");
	switch (m_LogLevel)
	{
	case logStats:
		logger->Info("Only overall statistics will be logged.");
		break;
	case logCallstack:
		logger->Info("Script callstack will be logged.");
		break;
	}
	logger->Info("* scriptparts = %d", log_scriptparts);

}