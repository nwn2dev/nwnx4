#include <io.h>
#include <process.h>
#include <stdlib.h>
#include <string.h>
#include <strsafe.h>
#include <windows.h>
#include <windowsx.h>

#define IDC_SENDMESSAGE_EDIT   0x3FC
#define IDC_PLAYERLIST_LISTBOX 0x3FE
#define IDC_SHUTDOWN_BUTTON    0x3FF
#define IDC_SENDMESSAGE_BUTTON 0x400
#define IDC_BOOT_BUTTON        0x401
#define IDC_BANNAME_BUTTON     0x402
#define IDC_BANCD_BUTTON       0x403
#define IDC_BANIP_BUTTON       0x404

unsigned __stdcall MainThreadProc(__in void* Arg);

HMODULE g_Nwn2srvutilDll;

BOOLEAN
WINAPI
DllMain(__in PVOID DllHandle, __in ULONG Reason, __in_opt PCONTEXT Context)
{
	switch (Reason) {

		case DLL_PROCESS_ATTACH: {
			WCHAR FileName[MAX_PATH + 1];

			g_Nwn2srvutilDll = (HMODULE)DllHandle;

			if (GetModuleFileNameW((HMODULE)DllHandle, FileName, MAX_PATH)) {
				HANDLE Thread;

				//				LoadLibrary( FileName );

				Thread = (HANDLE)_beginthreadex(
				    0, 0, MainThreadProc, ((PNT_TIB)(NtCurrentTeb()))->ArbitraryUserPointer, 0, 0);

				if (Thread)
					CloseHandle(Thread);
			}
		} break;
	}

	return TRUE;
}

BOOL CALLBACK FindServerGuiWindowEnumProc(__in HWND hwnd, __in LPARAM lParam)
{
	DWORD Pid;
	WCHAR ClassName[256];

	GetWindowThreadProcessId(hwnd, &Pid);

	if (Pid != GetCurrentProcessId())
		return TRUE;

	if (GetClassName(hwnd, ClassName, 256)) {
		if (!wcscmp(ClassName, L"Exo - BioWare Corp., (c) 1999 - Generic Blank Application")) {
			*(HWND*)lParam = hwnd;
			return FALSE;
		}
	}

	return TRUE;
}

HWND FindServerGuiWindow()
{
	HWND hwnd = 0;

	EnumWindows(FindServerGuiWindowEnumProc, (LPARAM)&hwnd);

	return hwnd;
}

int BroadcastServerMessage(__in const char* Message)
{
	HWND SrvWnd;
	HWND SendMsgEdit;
	HWND SendMsgButton;

	SrvWnd = FindServerGuiWindow();

	if (!SrvWnd)
		return -1;

	SendMsgEdit   = GetDlgItem(SrvWnd, IDC_SENDMESSAGE_EDIT);
	SendMsgButton = GetDlgItem(SrvWnd, IDC_SENDMESSAGE_BUTTON);

	if (!SendMsgEdit)
		return -2;

	if (!SendMsgButton)
		return -3;

	SetWindowTextA(SendMsgEdit, Message);
	SendMessage(SendMsgButton, BM_CLICK, 0, 0);
	SetWindowTextA(SendMsgEdit, "");

	return 0;
}

bool SelectPlayerListBox(__in HWND ListBox, __in const char* PlayerName)
{
	int Index
	    = (int)(SendMessageA(ListBox, LB_FINDSTRINGEXACT, (WPARAM)((int)-1), (LPARAM)PlayerName));

	if (Index == LB_ERR)
		return false;

	return (ListBox_SetCurSel(ListBox, Index) != LB_ERR);
}

int BootPlayer(__in const char* PlayerName)
{
	HWND SrvWnd;
	HWND PlayerListListBox;
	HWND BootButton;

	SrvWnd = FindServerGuiWindow();

	if (!SrvWnd)
		return -1;

	PlayerListListBox = GetDlgItem(SrvWnd, IDC_PLAYERLIST_LISTBOX);
	BootButton        = GetDlgItem(SrvWnd, IDC_BOOT_BUTTON);

	if (!PlayerListListBox)
		return -2;

	if (!BootButton)
		return -3;

	if (!SelectPlayerListBox(PlayerListListBox, PlayerName))
		return -4;

	SendMessage(BootButton, BM_CLICK, 0, 0);

	return 0;
}

int BanPlayerName(__in const char* PlayerName)
{
	HWND SrvWnd;
	HWND PlayerListListBox;
	HWND BanNameButton;

	SrvWnd = FindServerGuiWindow();

	if (!SrvWnd)
		return -1;

	PlayerListListBox = GetDlgItem(SrvWnd, IDC_PLAYERLIST_LISTBOX);
	BanNameButton     = GetDlgItem(SrvWnd, IDC_BANNAME_BUTTON);

	if (!PlayerListListBox)
		return -2;

	if (!BanNameButton)
		return -3;

	if (!SelectPlayerListBox(PlayerListListBox, PlayerName))
		return -4;

	SendMessage(BanNameButton, BM_CLICK, 0, 0);

	return 0;
}

int BanPlayerIP(__in const char* PlayerName)
{
	HWND SrvWnd;
	HWND PlayerListListBox;
	HWND BanIPButton;

	SrvWnd = FindServerGuiWindow();

	if (!SrvWnd)
		return -1;

	PlayerListListBox = GetDlgItem(SrvWnd, IDC_PLAYERLIST_LISTBOX);
	BanIPButton       = GetDlgItem(SrvWnd, IDC_BANIP_BUTTON);

	if (!PlayerListListBox)
		return -2;

	if (!BanIPButton)
		return -3;

	if (!SelectPlayerListBox(PlayerListListBox, PlayerName))
		return -4;

	SendMessage(BanIPButton, BM_CLICK, 0, 0);

	return 0;
}

int BanPlayerCDKey(__in const char* PlayerName)
{
	HWND SrvWnd;
	HWND PlayerListListBox;
	HWND BanCDButton;

	SrvWnd = FindServerGuiWindow();

	if (!SrvWnd)
		return -1;

	PlayerListListBox = GetDlgItem(SrvWnd, IDC_PLAYERLIST_LISTBOX);
	BanCDButton       = GetDlgItem(SrvWnd, IDC_BANCD_BUTTON);

	if (!PlayerListListBox)
		return -2;

	if (!BanCDButton)
		return -3;

	if (!SelectPlayerListBox(PlayerListListBox, PlayerName))
		return -4;

	SendMessage(BanCDButton, BM_CLICK, 0, 0);

	return 0;
}

int ShutdownNwn2server()
{
	HWND SrvWnd;

	SrvWnd = FindServerGuiWindow();

	if (!SrvWnd)
		return -1;

	SendMessage(SrvWnd, WM_CLOSE, 0, 0);

	return 0;
}

int Nwn2srvutilMain(int argc, char** argv)
{
	if (!argc)
		return 0;

	for (int i = 0; i < argc; i += 1) {
		if (!_stricmp(argv[i], "-msg")) {
			if (i < argc - 1) {
				BroadcastServerMessage(argv[i + 1]);
				i += 1;
			}
		} else if (!_stricmp(argv[i], "-boot")) {
			if (i < argc - 1) {
				BootPlayer(argv[i + 1]);
				i += 1;
			}
		} else if (!_stricmp(argv[i], "-banname")) {
			if (i < argc - 1) {
				BanPlayerName(argv[i + 1]);
				i += 1;
			}
		} else if (!_stricmp(argv[i], "-banip")) {
			if (i < argc - 1) {
				BanPlayerIP(argv[i + 1]);
				i += 1;
			}
		} else if (!_stricmp(argv[i], "-bancd")) {
			if (i < argc - 1) {
				BanPlayerCDKey(argv[i + 1]);
				i += 1;
			}
		} else if (!_stricmp(argv[i], "-shutdown")) {
			ShutdownNwn2server();
		}
	}

	return 0;
}

unsigned __stdcall MainThreadProc(__in void* Arg)
{
	char* Args  = (char*)Arg;
	char** Argv = 0;
	int Argc    = 1;

	while (*Args) {
		Argc += 1;

		Args += strlen(Args) + 1;
	}

	Argv = (char**)malloc(Argc * sizeof(char*));

	if (Argv) {
		Args = (char*)Arg;
		Argc = 0;

		while (*Args) {
			Argv[Argc++] = Args;

			Args += strlen(Args) + 1;
		}

		Argv[Argc] = 0;

		Nwn2srvutilMain(Argc, Argv);

		free(Argv);
	}

	VirtualFree(Arg, 0, MEM_RELEASE);

	FreeLibraryAndExitThread(g_Nwn2srvutilDll, 0);
}
