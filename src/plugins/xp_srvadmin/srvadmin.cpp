/***************************************************************************
    NWNX SrvAdmin - NWN2Server Administrative GUI Functions
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

#include "srvadmin.h"
#include <cassert>

#define PLUGIN_VERSION "0.0.4"

/*
 * Control identifiers for the server GUI admin window.
 */

#define IDC_ELC_CHECKBOX        0x3EF
#define IDC_SENDMESSAGE_EDIT    0x3FC
#define IDC_PLAYERPASSWORD_EDIT 0x3F9
#define IDC_DMPASSWORD_EDIT     0x3FA
#define IDC_ADMINPASSWORD_EDIT  0x3FB
#define IDC_PLAYERLIST_LISTBOX  0x3FE
#define IDC_SHUTDOWN_BUTTON     0x3FF
#define IDC_SENDMESSAGE_BUTTON  0x400
#define IDC_BOOT_BUTTON         0x401
#define IDC_BANNAME_BUTTON      0x402
#define IDC_BANCD_BUTTON        0x403
#define IDC_BANIP_BUTTON        0x404

Plugin* plugin;

/***************************************************************************
    NWNX and DLL specific functions
***************************************************************************/

DLLEXPORT Plugin* GetPluginPointerV2() { return plugin; }

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
		plugin = new SrvAdmin();

		char szPath[MAX_PATH];
		GetModuleFileNameA(hModule, szPath, MAX_PATH);
		plugin->SetPluginFullPath(szPath);
	} else if (ul_reason_for_call == DLL_PROCESS_DETACH) {
		delete plugin;
	}
	return TRUE;
}

/***************************************************************************
    Implementation of SrvAdmin Plugin
***************************************************************************/

SrvAdmin::SrvAdmin()
    : mainGuiWindow(NULL)
{
	header = "NWNX SrvAdmin Plugin " PLUGIN_VERSION "\n"
	         "(c) 2008 by Skywing \n"
	         "Visit NWNX at: http://www.nwnx.org\n";

	description = "This plugin allows the server admin GUI functions to be called from script.";

	subClass = "SRVADMIN";
	version  = PLUGIN_VERSION;
}

SrvAdmin::~SrvAdmin() { logger->Info("* Plugin unloaded."); }

bool SrvAdmin::Init(char* nwnxhome)
{
	assert(GetPluginFileName());

	/* Log file */
	std::string logfile(nwnxhome);
	logfile.append("\\");
	logfile.append(GetPluginFileName());
	logfile.append(".txt");
	logger = new LogNWNX(logfile);
	logger->Info(header.c_str());

	logger->Info("* Plugin initialized.");
	return true;
}

void SrvAdmin::GetFunctionClass(char* fClass) { strncpy_s(fClass, 128, "SRVADMIN", 8); }

void SrvAdmin::SetString(char* sFunction, char* sParam1, int nParam2, char* sValue)
{
	logger->Trace("* Plugin SetString(0x%x, %s, %d, %s)", 0x0, sParam1, nParam2, sValue);

	std::string function(sFunction);

	if (function == "") {
		logger->Info("* Function not specified.");
		return;
	}

	if (function == "SHUTDOWNNWN2SERVER") {
		logger->Info("* Shutting down nwn2server...");
		ShutdownNwn2server();
	} else if (function == "BROADCASTSERVERMESSAGE") {
		logger->Info("* Broadcasting server message: %s ", sParam1);
		BroadcastServerMessage(sParam1);
	} else if (function == "BOOTPLAYER") {
		logger->Info("* Booting player: %s", sParam1);
		BootPlayer(sParam1);
	} else if (function == "BANPLAYERNAME") {
		logger->Info("* Banning player by account name: %s", sParam1);
		BanPlayerName(sParam1);
	} else if (function == "BANPLAYERIP") {
		logger->Info("* Banning player by IP address: %s", sParam1);
		BanPlayerIP(sParam1);
	} else if (function == "BANPLAYERCDKEY") {
		logger->Info("* Banning player by CD-Key: %s", sParam1);
		BanPlayerCDKey(sParam1);
	} else if (function == "SETELC") {
		bool EnableELC;

		if (sParam1) {
			if (!strcmp(sParam1, "true"))
				EnableELC = true;
			else
				EnableELC = false;

			logger->Info("* Setting ELC to: %s", EnableELC ? "enabled" : "disabled");

			SetELC(EnableELC);
		}
	} else if (function == "SETPLAYERPASSWORD") {
		logger->Info("* Setting player password to: %s", sParam1);
		SetPlayerPassword(sParam1);
	} else if (function == "SETDMPASSWORD") {
		logger->Info("* Setting DM password to: %s", sParam1);
		SetDMPassword(sParam1);
	} else if (function == "SETADMINPASSWORD") {
		logger->Info("* Setting admin password to: %s", sParam1);
		SetAdminPassword(sParam1);
	}
}

char* SrvAdmin::GetString(char* sFunction, char* sParam1, int nParam2)
{
	logger->Trace("* Plugin GetString(0x%x, %s, %d)", 0x0, sParam1, nParam2);
	return NULL;
}

/*
 * Window message interface functions.
 */

BOOL CALLBACK SrvAdmin::FindServerGuiWindowEnumProc(__in HWND hwnd, __in LPARAM lParam)
{
	DWORD Pid;
	WCHAR ClassName[256];

	GetWindowThreadProcessId(hwnd, &Pid);

	if (Pid != GetCurrentProcessId())
		return TRUE;

	if (GetClassNameW(hwnd, ClassName, 256)) {
		if (!wcscmp(ClassName, L"Exo - BioWare Corp., (c) 1999 - Generic Blank Application")) {
			*(HWND*)lParam = hwnd;
			return FALSE;
		}
	}

	return TRUE;
}

HWND SrvAdmin::FindServerGuiWindow()
{
	HWND hwnd;

	if (mainGuiWindow)
		return mainGuiWindow;

	hwnd = 0;

	EnumWindows(FindServerGuiWindowEnumProc, (LPARAM)&hwnd);

	if (hwnd)
		mainGuiWindow = hwnd;

	return hwnd;
}

int SrvAdmin::BroadcastServerMessage(__in const char* Message)
{
	HWND SrvWnd;
	HWND SendMsgEdit;
	HWND SendMsgButton;

	if (!Message)
		return -4;

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

bool SrvAdmin::SelectPlayerListBox(__in HWND ListBox, __in const char* PlayerName)
{
	int Index
	    = (int)(SendMessageA(ListBox, LB_FINDSTRINGEXACT, (WPARAM)((int)-1), (LPARAM)PlayerName));

	if (Index == LB_ERR)
		return false;

	return (ListBox_SetCurSel(ListBox, Index) != LB_ERR);
}

bool SrvAdmin::SendVkKeyStroke(__in HWND ControlWindow, __in UINT VkCode)
{
	UINT ScanCode;

	//
	// Map the scan code and send a WM_KEYDOWN/WM_KEYUP.
	//

	ScanCode = MapVirtualKey(VkCode, MAPVK_VK_TO_VSC);

	SendMessageW(ControlWindow, WM_KEYDOWN, (WPARAM)(VkCode),
	             (LPARAM)((ScanCode << 16) | (0 << 30) | (0 << 31)));
	SendMessageW(ControlWindow, WM_KEYUP, (WPARAM)(VkCode),
	             (LPARAM)((ScanCode << 16) | (1 << 30) | (1 << 31)));

	return true;
}

bool SrvAdmin::SetTabbingTextField(__in HWND ControlWindow, __in const char* TextContents)
{
	//
	// Sets the text content of a control subclassed by nwn2server!TabbingProc,
	// and synthesizes the appropriate key-down message to realize it.
	//
	// TabbingProc accepts both VK_RETURN and VK_TAB and does not use the scan
	// codes.  It also does not require WM_KEYUP, but we do the whole smash so
	// that we're compatible with any other subclasses or hooks that someone
	// else might install.
	//

	//
	// First, set the text contents.
	//
	// N.B.  Comparison with TRUE is carefully chosen as that is valid for all
	//       of the standard control implementations for WM_SETTEXT.
	//

	if ((BOOL)(SendMessageA(ControlWindow, WM_SETTEXT, 0, (LPARAM)TextContents)) != TRUE)
		return false;

	//
	// Now that the text contents are set, fake a return keypress so that the
	// contents are realized and saved to the internal state.
	//

	return SendVkKeyStroke(ControlWindow, VK_RETURN);
}

int SrvAdmin::BootPlayer(__in const char* PlayerName)
{
	HWND SrvWnd;
	HWND PlayerListListBox;
	HWND BootButton;

	if (!PlayerName)
		return -5;

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

int SrvAdmin::BanPlayerName(__in const char* PlayerName)
{
	HWND SrvWnd;
	HWND PlayerListListBox;
	HWND BanNameButton;

	if (!PlayerName)
		return -5;

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

int SrvAdmin::BanPlayerIP(__in const char* PlayerName)
{
	HWND SrvWnd;
	HWND PlayerListListBox;
	HWND BanIPButton;

	if (!PlayerName)
		return -5;

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

int SrvAdmin::BanPlayerCDKey(__in const char* PlayerName)
{
	HWND SrvWnd;
	HWND PlayerListListBox;
	HWND BanCDButton;

	if (!PlayerName)
		return -5;

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

int SrvAdmin::ShutdownNwn2server()
{
	HWND SrvWnd;

	SrvWnd = FindServerGuiWindow();

	if (!SrvWnd)
		return -1;

	PostMessage(SrvWnd, WM_CLOSE, 0, 0);

	return 0;
}

int SrvAdmin::SetELC(__in bool EnableELC)
{
	HWND SrvWnd;
	HWND ELCCheckbox;
	UINT CheckState;

	SrvWnd = FindServerGuiWindow();

	if (!SrvWnd)
		return -1;

	ELCCheckbox = GetDlgItem(SrvWnd, IDC_ELC_CHECKBOX);

	if (!ELCCheckbox)
		return -2;

	CheckState = (EnableELC) ? BST_CHECKED : BST_UNCHECKED;

	SendMessage(ELCCheckbox, BM_SETCHECK, (WPARAM)CheckState, 0);

	return 0;
}

int SrvAdmin::SetPlayerPassword(__in const char* PlayerPassword)
{
	HWND SrvWnd;
	HWND Edit;

	SrvWnd = FindServerGuiWindow();

	if (!SrvWnd)
		return -1;

	Edit = GetDlgItem(SrvWnd, IDC_PLAYERPASSWORD_EDIT);

	if (!Edit)
		return -2;

	return SetTabbingTextField(Edit, PlayerPassword) ? 0 : -3;
}

int SrvAdmin::SetDMPassword(__in const char* DMPassword)
{
	HWND SrvWnd;
	HWND Edit;

	SrvWnd = FindServerGuiWindow();

	if (!SrvWnd)
		return -1;

	Edit = GetDlgItem(SrvWnd, IDC_DMPASSWORD_EDIT);

	if (!Edit)
		return -2;

	return SetTabbingTextField(Edit, DMPassword) ? 0 : -3;
}

int SrvAdmin::SetAdminPassword(__in const char* AdminPassword)
{
	HWND SrvWnd;
	HWND Edit;

	SrvWnd = FindServerGuiWindow();

	if (!SrvWnd)
		return -1;

	Edit = GetDlgItem(SrvWnd, IDC_ADMINPASSWORD_EDIT);

	if (!Edit)
		return -2;

	return SetTabbingTextField(Edit, AdminPassword) ? 0 : -3;
}
