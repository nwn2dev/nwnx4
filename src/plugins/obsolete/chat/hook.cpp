/***************************************************************************
    NWNX Chat - Hoooking
    Copyright (C) 2006-2007 virusman (virusman@virusman.ru)

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
#include "chat.h"

/***************************************************************************
    Declarations
***************************************************************************/

extern Chat* plugin;

void (*ChatNextHook)();

void (*pRunScript)();

dword pServThis = 0;
dword pList = 0;
dword pScriptThis = 0;
char *pChatThis = 0;
dword * (*pGetPCobj)();
void (*pChat)(int mode, int id, char **msg, int to);
bool post123 = 0;

char scriptRun = 0;
char *lastMsg;
char lastIDs[32];

unsigned long lastRet;

/***************************************************************************
    Hooking functions
***************************************************************************/

DWORD FindChat()
{
	char* ptr = (char*) 0x400000;
	while (ptr < (char*) 0x730000)
	{
		if ((ptr[0] == (char) 0x83) &&
			(ptr[1] == (char) 0xEC) &&
			(ptr[8] == (char) 0x53) &&
			(ptr[9] == (char) 0x55) &&
			(ptr[0xA] == (char) 0x56) &&
			(ptr[0xB] == (char) 0x8B) &&
			(ptr[0xC] == (char) 0xE9) &&
			(ptr[0xD] == (char) 0x8B) &&
			(ptr[0xE] == (char) 0x48) &&
			(ptr[0xF] == (char) 0x04) &&
			(ptr[0x10] == (char) 0x57) &&
			(ptr[0x11] == (char) 0xBE) &&
			(ptr[0x12] == (char) 0x01)
			)
			return (DWORD) ptr;
		else
			ptr++;
	}
	ptr = (char*) 0x400000;
	while (ptr < (char*) 0x730000)
	{
		if ((ptr[0] == (char) 0x83) &&
			(ptr[1] == (char) 0xEC) &&
			(ptr[8] == (char) 0x53) &&
			(ptr[9] == (char) 0x55) &&
			(ptr[0xA] == (char) 0x56) &&
			(ptr[0xB] == (char) 0x8B) &&
			(ptr[0xC] == (char) 0xD9) &&
			(ptr[0xD] == (char) 0x8B) &&
			(ptr[0xE] == (char) 0x48) &&
			(ptr[0xF] == (char) 0x04) &&
			(ptr[0x10] == (char) 0x57) &&
			(ptr[0x11] == (char) 0x89) &&
			(ptr[0x12] == (char) 0x5C)
			)
		{
			post123 = 1;
			return (DWORD) ptr;
		}
		else
			ptr++;
	}
	return NULL;
}


DWORD FindRunScript()
{ //83 EC 0C 8B 54 24 18 33 C0 56
	char* ptr = (char*) 0x400000;
	// runscript moved again in 1.06 at  0x737120
	// bumped limit. Perhaps parameterize this limit?
	while (ptr < (char*) 0x800000)
	{
		if ((ptr[0] == (char) 0x83) &&
			(ptr[1] == (char) 0xEC) &&
			(ptr[2] == (char) 0x0C) &&
			(ptr[3] == (char) 0x8B) &&
			(ptr[4] == (char) 0x54) &&
			(ptr[5] == (char) 0x24) &&
			(ptr[6] == (char) 0x18) &&
			(ptr[7] == (char) 0x33) &&
			(ptr[8] == (char) 0xC0) &&
			(ptr[9] == (char) 0x56)
			)
			return (DWORD) ptr;
		else
			ptr++;
	}
	return NULL;
}

DWORD FindGetPCobjByOID()
{ //83 EC 0C 8B 54 24 18 33 C0 56
	char* ptr = (char*) 0x400000;
	while (ptr < (char*) 0x730000)
	{
        if ((ptr[0] == (char) 0x53) &&
            (ptr[1] == (char) 0x8B) &&
            (ptr[2] == (char) 0x5C) &&
            (ptr[3] == (char) 0x24) &&
            (ptr[4] == (char) 0x08) &&
            (ptr[5] == (char) 0x81) &&
            (ptr[6] == (char) 0xfb) &&
            (ptr[7] == (char) 0x00) &&
            (ptr[8] == (char) 0x00) &&
            (ptr[9] == (char) 0x00) &&
            (ptr[10] == (char) 0x7f)
            )
			return (DWORD) ptr;
		else
			ptr++;
	}
	return NULL;
}

void ChatHookProc(const int mode, const int id, const char **msg, const int to)
{
	_asm { pushad }
	// wxLogDebug("Wow, cool. The Chat function has been called.");
	if(!pChatThis) 
		_asm { mov pChatThis, ecx }
	if (!scriptRun)
		lastRet = (unsigned long)plugin->ChatProc(mode, id, msg, to);
	_asm { popad }
	_asm { leave }
	if (!scriptRun && lastRet)
	{
		if(post123)
		{
			_asm { mov eax, lastRet }
			_asm { retn 0x18 }
		}
		else
		{
			_asm { mov eax, lastRet }
			_asm { retn 0x14 }
		}

	}
	_asm { jmp ChatNextHook }
}

void RunScript(char * sname, int ObjID)
{
  int sptr[4];
  sptr[1] = strlen(sname);
  _asm {
    lea  edx, sptr
    mov  eax, sname
    mov  [edx], eax
	push 0
    push 1
    push ObjID
    push edx
    mov ecx, pScriptThis
    mov ecx, [ecx]
  }
  scriptRun = 1;
  pRunScript();
  scriptRun = 0;
}

int SendMsg(const int mode, const int id, char *msg, const int to)
{
	int nRetVal;
	if(pChat && pChatThis && msg)
	{
		char **msgl = &msg;
		//char xz=0;
		//_asm { mov ecx, pChatThis }
		_asm {
		    push 0 // extra ;)
			push to
			push msgl
			push id
			push mode
			mov ecx, pChatThis 
			call [pChat]
		    mov nRetVal, eax
		}
		return nRetVal;
		//pChat(mode, id, &msg, to);
	}
	else return 0;
}
unsigned long* GetPCobj(dword OID)
{
	_asm {
		mov  ecx, pServThis
		mov  ecx, [ecx]
		mov  ecx, [ecx+4]
		mov  ecx, [ecx+4]
		push OID
		call pGetPCobj
	}
	//return pGetPCobj();
}

unsigned long GetID(dword OID)
{
	dword *pcObj = GetPCobj(OID);
	if(!pcObj) return 0x7F000000;
	return *(pcObj+1); // +1 dword = +4
}
int HookFunctions()
{
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	int success = 0;
	DWORD org_Chat = FindChat();
	*(dword*)&pChat = org_Chat;
	DWORD org_Run  = FindRunScript();
	*(dword*)&pRunScript = org_Run;
	DWORD org_Get  = FindGetPCobjByOID();
	*(dword*)&pGetPCobj = org_Get;
	
	//HookCode((PVOID) hookAt, SetLocalStringHookProc, (PVOID*) &SetLocalStringNextHook);

	// if (org_Chat)
	// 	wxLogMessage("ChatFunc found at 0x%x", org_Chat);
	// else
	// 	wxLogMessage("ChatFunc NOT FOUND!");

	// if (org_Run)
	// 	wxLogMessage("RunScript found at 0x%x", org_Run);
	// else
	// 	wxLogMessage("RunScript NOT FOUND!");

	// if (org_Get)
	// 	wxLogMessage("GetPCobjByOID found at 0x%x", org_Get);
	// else
	// 	wxLogMessage("GetPCobjByOID NOT FOUND!");

	if(!(org_Chat&&org_Run&&org_Get))
		return NULL;

	*(dword*)&ChatNextHook = org_Chat;
	//success = HookCode((PVOID) org_Chat, ChatHookProc, (PVOID*) &ChatNextHook);
	success = DetourAttach(&(PVOID&)ChatNextHook, ChatHookProc)==0;
	DetourTransactionCommit();

	pServThis = *(dword*)(org_Chat + 0x33);
	pScriptThis = pServThis - 8;

	if (!(pServThis&&pScriptThis))
	{
		// wxLogMessage("Error initializing variables");
		return NULL;
	}

	// if (success)
	// 	wxLogMessage("Chat function is hooked.");
	// else
	if(!success)
	{
		// wxLogMessage("Failed to hook Chat function.");
		return NULL;
	}

	return true;
}