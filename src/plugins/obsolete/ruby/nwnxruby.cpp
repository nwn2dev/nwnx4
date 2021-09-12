/***************************************************************************
	NWNX Ruby
	Copyright (C) 2010 virusman (virusman@virusman.ru)

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

#include "nwnxruby.h"
#include "hook.h"
#include "ruby_int.h"
#include <cassert>

/***************************************************************************
    NWNX and DLL specific functions
***************************************************************************/

Ruby* plugin;

DLLEXPORT LegacyPlugin* GetPluginPointer()
{
	return plugin;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		plugin = new Ruby();

		char szPath[MAX_PATH];
		GetModuleFileName(hModule, szPath, MAX_PATH);
		plugin->SetPluginFullPath(szPath);
	}
	else if (ul_reason_for_call == DLL_PROCESS_DETACH)
	{
		delete plugin;
	}
    return TRUE;
}


/***************************************************************************
    Implementation of Chat Plugin
***************************************************************************/

Ruby::Ruby()
{
	header =
		"NWNX4 Ruby Plugin V.0.1.0\n"
		"(c) 2010 by virusman (virusman@virusman.ru)\n"
		"visit us at http://www.nwnx.org\n";

	description = "This plugin provides some testing.";

	subClass = "RUBY";
	version = "0.1.0";
}

Ruby::~Ruby()
{
	logger->Info("* Plugin unloaded.");
}

bool Ruby::Init(char* nwnxhome)
{
	assert(GetPluginFileName());

	/* Log file */
	std::string logfile(nwnxhome);
	logfile.append("\\");
	logfile.append(GetPluginFileName());
	logfile.append(".txt");
	logger = new LogNWNX(logfile);
	logger->Info(header.c_str());

	ruby_init();
	ruby_script("embedded");
	ruby_init_loadpath();
	RubyInt_DefineConstants();

	//rb_eval_string("puts \"NWNX Ruby Initialized\"\n");

/*	char *preload = (char*)((*nwnxConfig)[confKey]["preload"].c_str());
	if (strlen(preload) > 0)
	{
		Log(0, "Preloading: %s\n", preload);
		rb_require(preload);
	}*/

	cNWScript = RubyInt_InitNWScript();
	rb_include_module(rb_cObject, cNWScript);

	if (HookFunctions())
	{
		//bHooked=1;
		Log(0,"* Module loaded successfully.\n");
	}

	//logger->Info(wxT("* Plugin initialized."));
	return true;
}

char *Ruby::Eval(char *value)
{
	//Evaluate Ruby expression (protected)
	try
	{
		VALUE retval;
		char *c_retval;
		rb_eval_string("Thread.current[:nwnx_context] = true");
		retval = rb_eval_string_protect(value, &nError);
		rb_eval_string("Thread.current[:nwnx_context] = false");
		if(nError)
		{
			Log(0, "Error %d while evaluating a Ruby expression: %s\n", nError, value);
			return NULL;
		}
		if(retval!=Qnil)
		{
			retval = rb_funcall(retval, rb_intern("to_s"), 0);
			c_retval = rb_string_value_ptr(&retval);
			if(c_retval)
			{
				char *buf = (char *) malloc(strlen(c_retval)+1);
				strcpy(buf, c_retval);
				return buf;
			}
		}
		return NULL;
	}
	catch(...)
	{
		Log(0, "Caught a C++ exception while evaluating a Ruby expression: %s\n", value);
		return NULL;
	}
	/*else
	return RSTRING(rb_cvar_get(cNWScript, rb_intern(RUBY_RETVAL)))->ptr;*/
}

const char* Ruby::DoRequest(char *gameObject, char* request, char* parameters)
{
	Log(2,"(S) Request: \"%s\"\n",request);
	Log(3,"(S) Params:  \"%s\"\n",parameters);

	if(strncmp(request, "EVAL", 4) == 0)
	{
		return Eval(parameters);
	}
	return NULL;
}

void Ruby::GetFunctionClass(char* fClass)
{
	strncpy_s(fClass, 128, "RUBY", 5);
}

void Ruby::Log(int priority, const char *pcMsg, ...)
{
	va_list argList;
	char acBuffer[2048];

	if (TRUE)
	{  
		// build up the string
		va_start(argList, pcMsg);
		_vsnprintf(acBuffer, 2047, pcMsg, argList);
		acBuffer[2047] = 0;
		va_end(argList);

		logger->Info(acBuffer);
	}
}