#include "Pickpocket.h"
#include "detours/detours.h"

// 1.13
// g_pVirtualMachine
//#define NWN2_OFFSET_CVIRTUALMACHINE 0x00856FA4
// CVirtualMachine::RunScript(with script params)
//#define NWN2_OFFSET_RUNSCRIPT		0x00747090
// CNWSCreature::AIActionPickPocket
//#define NWN2_OFFSET_AIACTIONPICKPOCKET	0x004983D0

// 1.21
// g_pVirtualMachine
//#define NWN2_OFFSET_CVIRTUALMACHINE 0x0085D2A4
// CVirtualMachine::RunScript(with script params)
//#define NWN2_OFFSET_RUNSCRIPT		0x0074D500
// CNWSCreature::AIActionPickPocket
//#define NWN2_OFFSET_AIACTIONPICKPOCKET	0x00499B20


// 1.22
// g_pVirtualMachine
//#define NWN2_OFFSET_CVIRTUALMACHINE 0x0085D2A4
// CVirtualMachine::RunScript(with script params)
//#define NWN2_OFFSET_RUNSCRIPT		0x0074D370
// CNWSCreature::AIActionPickPocket
//#define NWN2_OFFSET_AIACTIONPICKPOCKET	0x00499AD0

// 1.23
// g_pVirtualMachine
#define NWN2_OFFSET_CVIRTUALMACHINE 0x00864424
// CVirtualMachine::RunScript(with script params)
#define NWN2_OFFSET_RUNSCRIPT		0x0072B050
// CNWSCreature::AIActionPickPocket
#define NWN2_OFFSET_AIACTIONPICKPOCKET	0x005CF5E0


struct CExoArrayList 
{
	void *buf;
	int length;
	int arraylength;
};
struct CExoString
{
	char *buf;
	int len; 
};

//struct CScriptParameterWrapper
//{
//	enum ParameterType
//	{
//		PT_INT = 0,
//		PT_FLOAT,
//		PT_STRING,
//		PT_OBJECTTAG,
//		PT_UNKNOWN
//	};
//
//	void        * __VFN_table;
//	int           m_iIntParameter;
//	float         m_fFloatParameter;
//	CExoString    m_cStringParameter;
//	ParameterType m_eType;
//};

CExoArrayList g_scriptArray = {NULL, 0, 0};
CExoString g_scriptVar;

char pszScript[128];


int (__stdcall *pRunScript)(CExoString *, unsigned long oid, CExoArrayList const &varArray, int unk, unsigned int Enum) = (int (__stdcall *)(CExoString *, unsigned long oid, CExoArrayList const &varArray, int unk, unsigned int Enum))NWN2_OFFSET_RUNSCRIPT;

int (__stdcall *Orig_AIActionPickPocket)(void *pActionNode) = (int (__stdcall *)(void *))NWN2_OFFSET_AIACTIONPICKPOCKET;


DWORD dwThisOID = -1;
DWORD dwTargetOID = -1;
DWORD dwHalt = 0;
int __stdcall AIActionPickPocket(void *pActionNode)
{
	_asm
	{
		pushad

		mov			eax, [ecx + 0xA0]
		mov			dwThisOID, eax
		
		mov			eax, pActionNode
		mov			eax, [eax + 0x44]
		mov			dwTargetOID, eax

		mov			dwHalt,	0
		
		mov			ecx, NWN2_OFFSET_CVIRTUALMACHINE
		mov			ecx, [ecx]
	}
	
	pRunScript(&g_scriptVar, dwThisOID, g_scriptArray, 1, 0);

	_asm
	{
		popad
		leave
	}
	if(dwHalt != 0)
	{
		_asm
		{
			mov eax, 3
			ret 4
		}
	}

	_asm
	{
		jmp Orig_AIActionPickPocket
	}
}
int HookFunctions()
{
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	int detour_success = DetourAttach(&(PVOID&)Orig_AIActionPickPocket, AIActionPickPocket) == 0;
	detour_success |= DetourTransactionCommit() == 0;

	return detour_success;
}

CPickpocket* plugin;

DLLEXPORT Plugin* GetPluginPointerV2()
{
	return plugin;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		plugin = new CPickpocket;

		TCHAR szPath[MAX_PATH];
		GetModuleFileName(hModule, szPath, MAX_PATH);
		plugin->SetPluginFullPath(szPath);
	}
	else if (ul_reason_for_call == DLL_PROCESS_DETACH)
	{
		delete plugin;
		plugin = NULL;
	}
	return TRUE;
}

CPickpocket::CPickpocket(void)
{
	header = _T(
		"NWNX OnPickpocket Plugin V.0.0.2\n" \
		"by Merlin Avery (GodBeastX)\n" \
		"big help from Skywing\n\n" \
		"FOR USE WITH 1.23 NWN2SERVER ONLY!\n");

	description = _T("This plugin provides script access onpickpocketting.");

	subClass = _T("PICK");
	version = _T("0.0.1");
}

CPickpocket::~CPickpocket(void)
{
}

bool CPickpocket::Init(TCHAR* nwnxhome)
{
	/* Log file */
	wxString logfile(nwnxhome); 
	logfile.append(wxT("\\"));
	logfile.append(GetPluginFileName());
	logfile.append(wxT(".txt"));
	logger = new wxLogNWNX(logfile, wxString(header.c_str()));

	/* Ini file */
	wxString inifile(nwnxhome); 
	inifile.append(wxT("\\"));
	inifile.append(GetPluginFileName());
	inifile.append(wxT(".ini"));
	wxLogTrace(TRACE_VERBOSE, wxT("* reading inifile %s"), inifile);

	config = new wxFileConfig(wxEmptyString, wxEmptyString, 
		inifile, wxEmptyString, wxCONFIG_USE_LOCAL_FILE|wxCONFIG_USE_NO_ESCAPE_CHARACTERS);

	if (!config->Read(wxT("script"), &execScript) )
	{
		wxLogMessage(wxT("* 'script' not found in ini"));
		return false;
	}

	strncpy(pszScript, execScript, 127);
	g_scriptVar.len = (int)strlen(pszScript);
	g_scriptVar.buf = pszScript;

	if(!HookFunctions())
	{
		wxLogMessage(wxT("* Hooking error."));
		return false;
	}

	wxLogMessage(wxT("* Plugin initialized."));

	return true;
}

int CPickpocket::GetInt(char* sFunction, char* sParam1, int nParam2)
{
	int iFuncID = atoi(sFunction);

	switch(iFuncID)
	{
		case 0: return dwTargetOID;
		case 1: dwHalt = nParam2; break;
	}
	return 0;
}
void CPickpocket::SetInt(char* sFunction, char* sParam1, int nParam2, int nValue)
{
	int iFuncID = atoi(sFunction);
}
float CPickpocket::GetFloat(char* sFunction, char* sParam1, int nParam2)
{
	return 0.0f;
}
void CPickpocket::SetFloat(char* sFunction, char* sParam1, int nParam2, float fValue)
{
}
char* CPickpocket::GetString(char* sFunction, char* sParam1, int nParam2)
{
	int iFuncID = atoi(sFunction);

	return "";
}
void CPickpocket::SetString(char* sFunction, char* sParam1, int nParam2, char* sValue)
{
	int iFuncID = atoi(sFunction);
}
void CPickpocket::GetFunctionClass(TCHAR* fClass)
{
	_tcsncpy_s(fClass, 128, wxT("PICK"), 4); 
}
