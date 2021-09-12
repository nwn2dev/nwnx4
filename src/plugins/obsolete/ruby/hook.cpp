/***************************************************************************
    NWNX Ruby - Hoooking
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

#include "hook.h"
#include "nwnxruby.h"
#include <stdarg.h>

/***************************************************************************
    Declarations
***************************************************************************/

extern Ruby* plugin;

/***************************************************************************
    Functions
***************************************************************************/

dword pServThis = 0;
dword pScriptThis = 0;
dword pServInternal = 0;
dword **g_pVirtualMachine;

int (__fastcall *CNWVirtualMachineCommands_ExecuteCommand)(void *pCommands, int edx, dword nCommandID, int arg_8);
int (__fastcall *CVirtualMachine_StackPopInteger)(void *pVM, int edx, int *buf);
int (__fastcall *CVirtualMachine_StackPopFloat)(void *pVM, int edx, float *buf);
int (__fastcall *CVirtualMachine_StackPopString)(void *pVM, int edx, CExoString *buf);
int (__fastcall *CVirtualMachine_StackPopVector)(void *pVM, int edx, Vector *buf);
int (__fastcall *CVirtualMachine_StackPopObject)(void *pVM, int edx, dword *buf);
int (__fastcall *CVirtualMachine_StackPopEngineStructure)(void *pVM, int edx, dword nStructType, void **buf);

int (__fastcall *CVirtualMachine_StackPushInteger)(void *pVM, int edx, int value);
int (__fastcall *CVirtualMachine_StackPushFloat)(void *pVM, int edx, float value);
int (__fastcall *CVirtualMachine_StackPushString)(void *pVM, int edx, CExoString *value);
int (__fastcall *CVirtualMachine_StackPushVector)(void *pVM, int edx, Vector value);
int (__fastcall *CVirtualMachine_StackPushObject)(void *pVM, int edx, dword value);
int (__fastcall *CVirtualMachine_StackPushEngineStructure)(void *pVM, int edx, dword nStructType, void *value);

/***************************************************************************
    Wrappers
***************************************************************************/

int StackPopInteger(int *buf)
{
	return CVirtualMachine_StackPopInteger(*g_pVirtualMachine, 0, buf);
}

int StackPopFloat(float *buf)
{
	return CVirtualMachine_StackPopFloat(*g_pVirtualMachine, 0, buf);
}

int StackPopString(char **buf)
{
	CExoString *str = (CExoString *) malloc(sizeof(CExoString));
	str->Text = NULL;
	str->Length = 0;
	int retval = CVirtualMachine_StackPopString(*g_pVirtualMachine, 0, str);
	if (!str->Text)
		*buf = "";
	else
		*buf = str->Text;
	free(str);
	return retval;
}

int StackPopVector(Vector *buf)
{
	return CVirtualMachine_StackPopVector(*g_pVirtualMachine, 0, buf);
}

int StackPopObject(dword *buf)
{
	return CVirtualMachine_StackPopObject(*g_pVirtualMachine, 0, buf);
}

int StackPopEngineStructure(dword nStructType, void **buf)
{
	return CVirtualMachine_StackPopEngineStructure(*g_pVirtualMachine, 0, nStructType, buf);
}

int StackPushInteger(int value)
{
	return CVirtualMachine_StackPushInteger(*g_pVirtualMachine, 0, value);
}

int StackPushFloat(float value)
{
	return CVirtualMachine_StackPushFloat(*g_pVirtualMachine, 0, value);
}

int StackPushString(char *value)
{
	return CVirtualMachine_StackPushString(*g_pVirtualMachine, 0, (CExoString *) &value);
}

int StackPushVector(Vector value)
{
	return CVirtualMachine_StackPushVector(*g_pVirtualMachine, 0, value);
}

int StackPushObject(dword value)
{
	return CVirtualMachine_StackPushObject(*g_pVirtualMachine, 0, value);
}

int StackPushEngineStructure(dword nStructType, void *value)
{
	return CVirtualMachine_StackPushEngineStructure(*g_pVirtualMachine, 0, nStructType, value);
}

dword GetObjectSelf()
{
	CVirtualMachine *pVM = (CVirtualMachine *) *g_pVirtualMachine;
	return pVM->ObjectID[pVM->RecursionLevel];
}

void *GetCommandsPtr()
{
	return *(void **)(*(dword *)(g_pVirtualMachine) + 0x3A0);
}

void VM_ExecuteCommand(dword nCommandID, int nArgsCount)
{
	CNWVirtualMachineCommands_ExecuteCommand(GetCommandsPtr(), 0, nCommandID, nArgsCount);
}

/***************************************************************************
    Functions
***************************************************************************/

int HookFunctions()
{

	*(dword*)&CNWVirtualMachineCommands_ExecuteCommand = 0x0067C8A0;
	*(dword*)&CVirtualMachine_StackPopInteger = 0x0067CFD0;
	*(dword*)&CVirtualMachine_StackPopFloat = 0x0067D030;
	*(dword*)&CVirtualMachine_StackPopString = 0x0067D190;
	*(dword*)&CVirtualMachine_StackPopVector = 0x0067D090;
	*(dword*)&CVirtualMachine_StackPopObject = 0x0067D2F0;
	*(dword*)&CVirtualMachine_StackPopEngineStructure = 0x0067D240;

	*(dword*)&CVirtualMachine_StackPushInteger = 0x0067D000;
	*(dword*)&CVirtualMachine_StackPushFloat = 0x0067D060;
	*(dword*)&CVirtualMachine_StackPushString = 0x0067D1E0;
	*(dword*)&CVirtualMachine_StackPushVector = 0x0067D110;
	*(dword*)&CVirtualMachine_StackPushObject = 0x0067D320;
	*(dword*)&CVirtualMachine_StackPushEngineStructure = 0x0067D2A0;

	InitConstants();

	return true;
}

void InitConstants()
{
	*(dword*)&g_pVirtualMachine = 0x00864424;
}