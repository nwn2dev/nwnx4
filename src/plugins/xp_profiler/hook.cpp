/***************************************************************************
    NWNX Profiler - Hoooking
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

#include "hook.h"
#include "hash.h"
#include "profiler.h"

/***************************************************************************
    Declarations
***************************************************************************/
#define MAX_CALLDEPTH 128
#define MAX_SCRIPTNAME_LENGTH 64

extern Profiler* plugin;

void (*RunScriptNextHook)();

hash_table scriptHash;

int iColumn;
int iCallDepth;
unsigned int iLongestScriptName = 0;
unsigned int iScriptCounter;
unsigned int iTotalRuntime;
unsigned int iTotalLast;
bool emptyScript;
char scriptName[MAX_CALLDEPTH][MAX_SCRIPTNAME_LENGTH+1];
char sFormatString[40];
LARGE_INTEGER liFrequency;
LARGE_INTEGER liLast[MAX_CALLDEPTH];
LARGE_INTEGER liLastStatistic;

struct sScriptData 
{
	unsigned long ulCalls;
	DWORD dwElapsedTime;
	char updated;
};

/***************************************************************************
    Hooking functions
***************************************************************************/

void printer(char *string, void *data)
{
	sScriptData* scriptData = (sScriptData*)data;

	int iLen = strlen(string);
	int iMsec = (int)scriptData->dwElapsedTime / 1000;

	if (scriptData->updated)
		sprintf_s(sFormatString, "%%-%ds  %%10d msec %%6d calls *| ", iLongestScriptName);
	else
		sprintf_s(sFormatString, "%%-%ds  %%10d msec %%6d calls  | ", iLongestScriptName);
	sFormatString[39] = 0;

	plugin->logger->Info(sFormatString, string, iMsec, scriptData->ulCalls);

	iTotalRuntime += iMsec;
	scriptData->updated = FALSE;
}

void FlushStatistics(DWORD dwStatisticMsec)
{
	plugin->logger->Info("\nCurrent statistics");
	plugin->logger->Info("---------------------------------------------------------------------------");
	enumerate(&scriptHash, printer);
	plugin->logger->Info("---------------------------------------------------------------------------");
	plugin->logger->Info("Elapsed time                : %d msec", dwStatisticMsec);
	plugin->logger->Info("Runtime delta               : %d msec", iTotalRuntime - iTotalLast);
	plugin->logger->Info("Total cumulative runtime    : %d msec", iTotalRuntime);
	plugin->logger->Info("Total number of scriptcalls : %d\n", iScriptCounter);
	iTotalLast = iTotalRuntime;
	iTotalRuntime = 0;
	//fflush(profiler.m_fFile);
}

void StopTimer()
{
	if (emptyScript)
		return;

	sScriptData* scriptData;
	DWORD dwPerfElapsed;
	DWORD dwStatisticMsec;

	LARGE_INTEGER liCurrent;
	QueryPerformanceCounter(&liCurrent);
	dwPerfElapsed = (DWORD) (((liCurrent.QuadPart - liLast[iCallDepth].QuadPart) * 1000000) / liFrequency.QuadPart);

	//fprintf(profiler.m_fFile, "(stop %s, %d microsec)\n", scriptName[iCallDepth], dwPerfElapsed);

	scriptData = (sScriptData*)lookup(scriptName[iCallDepth], &scriptHash);
	if (!scriptData)
	{
		scriptData = new sScriptData;
		scriptData->dwElapsedTime = dwPerfElapsed;
		scriptData->ulCalls = 1;
		scriptData->updated = TRUE;
	}
	else
	{
		scriptData->dwElapsedTime += dwPerfElapsed;
		scriptData->ulCalls++;
		scriptData->updated = TRUE;
	}
	insert(scriptName[iCallDepth], (void*)scriptData, &scriptHash);	

	dwStatisticMsec = (DWORD) (((liCurrent.QuadPart - liLastStatistic.QuadPart) * 1000) / liFrequency.QuadPart);
	if (dwStatisticMsec > 10000)
	{
		QueryPerformanceCounter(&liLastStatistic);
		FlushStatistics(dwStatisticMsec);
	}

	if (iCallDepth > -1)
		iCallDepth--;
}

void myRunScript(char *str)
{
	if (str != NULL)
	{
		if (iCallDepth < MAX_CALLDEPTH - 1)
			iCallDepth++;
		else
			plugin->logger->Info("Maximum call depth reached!");

		emptyScript = false;
		strncpy(scriptName[iCallDepth], str, MAX_SCRIPTNAME_LENGTH);
		scriptName[iCallDepth][MAX_SCRIPTNAME_LENGTH] = 0x0;
		unsigned int iScriptLength = strlen(scriptName[iCallDepth]);
		if(iScriptLength > iLongestScriptName) iLongestScriptName = iScriptLength;
		iScriptCounter++;

		if (plugin->m_LogLevel == plugin->logCallstack)
		{
			plugin->logger->Info("%s (calldepth %d)", str, iCallDepth);
			//fflush(profiler.m_fFile);
		}
		QueryPerformanceCounter(&liLast[iCallDepth]);
	}
	else
		emptyScript = true;
}

void myRunScriptPart(char *str)
{
	if (str != NULL)
	{
		if (iCallDepth < MAX_CALLDEPTH - 1)
			iCallDepth++;
		else
			plugin->logger->Info("Maximum call depth reached!");

		emptyScript = false;
		
		if(plugin->log_scriptparts==1)
		{
			scriptName[iCallDepth][0] = '>';
			scriptName[iCallDepth][1] = 0x0;
			strncat(scriptName[iCallDepth], str, MAX_SCRIPTNAME_LENGTH-1);
			scriptName[iCallDepth][MAX_SCRIPTNAME_LENGTH] = 0x0;
		}
		else
		{
			strncpy(scriptName[iCallDepth], str, MAX_SCRIPTNAME_LENGTH);
			scriptName[iCallDepth][MAX_SCRIPTNAME_LENGTH] = 0x0;
		}

		unsigned int iScriptLength = strlen(scriptName[iCallDepth]);
		if(iScriptLength > iLongestScriptName) iLongestScriptName = iScriptLength;

		iScriptCounter++;

		if (plugin->m_LogLevel == plugin->logCallstack)
		{
			plugin->logger->Info("%s (calldepth %d, scriptpart)", str, iCallDepth);
			//fflush(profiler.m_fFile);
		}
		QueryPerformanceCounter(&liLast[iCallDepth]);
	}
	else
		emptyScript = true;
}

void __declspec(naked) RunScriptHookProc()
{
	__asm {

		push ecx	  // save register contents
		push edx
		push ebx
		push esi
		push edi
		push ebp	  // prolog 1
		mov ebp, esp  // prolog 2

		// fetch script name
		lea ecx, dword ptr ss:[ecx+0x3EC]
		mov eax, dword ptr ss:[ecx]
		mov ebx, dword ptr ss:[esp+0x1C] //arg 1
		test ebx, ebx
		jnz scriptpart

		cmp byte ptr ds:[eax],0
		je invalidscript

		push eax
		call myRunScript
		add esp, 4
		jmp original

scriptpart:
		//add ecx, 0x18
		mov eax, dword ptr ss:[ecx]
		test eax, eax
		je invalidscript
		push eax
		call myRunScriptPart
		add esp, 4

original:
		pop ebp		// restore register contents
		pop edi		
		pop esi
		pop ebx
		pop edx
		pop ecx

		mov eax, dword ptr ss:[esp+0x4] // arg 1
		push eax
		call RunScriptNextHook // call original function

		// save return value of StartConditional() script
		push eax 
		push 0
		call StopTimer
		jmp cleanup

invalidscript:
		pop ebp		// restore register contents
		pop edi		
		pop esi
		pop ebx
		pop edx
		pop ecx

		mov eax, dword ptr ss:[esp+0x4] // arg 1
		push eax
		call RunScriptNextHook // call original function

		// save return value of StartConditional() script
		push eax 
		push 0

cleanup:
		// cleanup stack
		add esp, 0x8
		pop eax
		add esp, 0x4
		push eax

		// put return value of StartConditional() script in EAX
		sub esp, 0x8
		pop eax
		add esp, 0x4

		retn
	}
}

DWORD FindHookRunScript()
{
	char* ptr = (char*) 0x400000;
	while (ptr < (char*) 0x800000)
	{
		if ((ptr[0x3] == (char) 0x53) &&
			(ptr[0x4] == (char) 0x55) &&
			(ptr[0x5] == (char) 0x56) &&
			(ptr[0x6] == (char) 0x8B) &&
			(ptr[0x7] == (char) 0xF1) &&
			(ptr[0x8] == (char) 0x8B) &&
			(ptr[0x9] == (char) 0x86) &&
			(ptr[0xA] == (char) 0x90) && 
			(ptr[0xB] == (char) 0x01) &&
			(ptr[0xC] == (char) 0x00) &&
			(ptr[0xD] == (char) 0x00) &&
			(ptr[0xE] == (char) 0x8B) &&
			(ptr[0xF] == (char) 0xAE)
			)

			return (DWORD) ptr;
		else
			ptr++;

	}
	return NULL;
}

void Release()
{
	LARGE_INTEGER liCurrent;
	QueryPerformanceCounter(&liCurrent);
	DWORD dwStatisticMsec = (DWORD) (((liCurrent.QuadPart - liLastStatistic.QuadPart) * 1000) / liFrequency.QuadPart);
	FlushStatistics(dwStatisticMsec);	
}


void HookRunScript()
{
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	int success = false;

	DWORD old_RunScript = FindHookRunScript();
	*(DWORD*)&RunScriptNextHook = old_RunScript; 
	if (old_RunScript)
		success = DetourAttach(&(PVOID&)RunScriptNextHook, RunScriptHookProc)==0;
	DetourTransactionCommit();

	// Performance analysis variables
	iColumn = 0;
	iTotalRuntime = 0;
	iTotalLast = 0;
	iScriptCounter = 0;
	iCallDepth = -1;
	construct_table(&scriptHash, 2048);
	QueryPerformanceFrequency(&liFrequency);
	QueryPerformanceCounter(&liLastStatistic);

	if (success)
		plugin->logger->Info("* RunScript hooked (symbol: >).");
	else
		plugin->logger->Info("* Could not find RunScript function or hook failed: %x", old_RunScript);

	return;
}


/*
int HookFunctions()
{
	int success = 0;
	DWORD org_Get  = FindGetPCobjByOID();
	*(dword*)&pGetPCobj = org_Get;
	DWORD org_GetPlayerObj = FindGetPlayerObj();
	
	if (org_Get)
		logger->Info(wxT("GetPCobjByOID found at 0x%x"), org_Get);
	else
		logger->Info(wxT("GetPCobjByOID NOT FOUND!"));

	if (org_Get)
		logger->Info(wxT("GetPlayerObj found at 0x%x"), org_GetPlayerObj);
	else
		logger->Info(wxT("GetPlayerObj NOT FOUND!"));

	if(!org_Get || !org_GetPlayerObj)
		return NULL;

	pServThis = *(dword*)(org_GetPlayerObj + 0x5);

	if (!(pServThis))
	{
		logger->Info(wxT("Error initializing variables"));
		return NULL;
	}
	return true;
}*/
