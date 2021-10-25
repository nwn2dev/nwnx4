#include "Pickpocket.h"
#include "../../NWN2Lib/NWN2.h"
#include "../../NWN2Lib/NWN2Common.h"
#include <detours/detours.h>
#include <bit>
#include <cassert>
#include <charconv>
#include <optional>
#include <string_view>

namespace
{

	// 1.23
	// g_pVirtualMachine
	constexpr uintptr_t NWN2_OFFSET_CVIRTUALMACHINE = 0x00864424;
	// CVirtualMachine::ExecuteScript
	constexpr uintptr_t NWN2_OFFSET_EXECUTESCRIPT = 0x0072B380;
	// CNWSCreature::AIActionPickPocket
	constexpr uintptr_t NWN2_OFFSET_AIACTIONPICKPOCKET = 0x005CF5E0;

	struct ActionNode
	{
		std::byte pad_0004[68]; //0x000
		NWN::OBJECTID targetObjectId; //0x044
	};
	struct CVirtualMachine
	{

	};

	using CVirtualMachine_ExecuteScript_t = BOOL(__thiscall*)(CVirtualMachine* thisVM,
		const NWN::CExoString& scriptName, NWN::OBJECTID objectId, uint32_t unknown1, uint32_t unknown2);
	CVirtualMachine_ExecuteScript_t CVirtualMachine_ExecuteScript =
		std::bit_cast<CVirtualMachine_ExecuteScript_t>(NWN2_OFFSET_EXECUTESCRIPT);

	using CNWSCreature_AIActionPickPocket_t = int(__thiscall*)(NWN::CNWSCreature* thisPtr,
		ActionNode& actionNode);
	CNWSCreature_AIActionPickPocket_t CNWSCreature_AIActionPickPocket =
		std::bit_cast<CNWSCreature_AIActionPickPocket_t>(NWN2_OFFSET_AIACTIONPICKPOCKET);
	CNWSCreature_AIActionPickPocket_t CNWSCreature_AIActionPickPocket_Trampoline = nullptr;

	auto ScriptName = std::string();
	NWN::OBJECTID TargetOid = NWN::INVALIDOBJID;
	bool HaltPickPocket = false;

	CVirtualMachine* GetNwn2VirtualMachine()
	{
		const auto vm = std::bit_cast<CVirtualMachine**>(NWN2_OFFSET_CVIRTUALMACHINE);
		return *vm;
	}

	int __fastcall CNWSCreature_AIActionPickPocket_Hook(
		NWN::CNWSCreature* thisPtr, void* /*edx*/, ActionNode& actionNode)
	{
		HaltPickPocket = false;
		TargetOid = actionNode.targetObjectId;

		const NWN::CExoString script = {
			.m_sString = ScriptName.data(),
			.m_nBufferLength = std::size(ScriptName) + 1
		};
		const auto vm = GetNwn2VirtualMachine();
		assert(vm);
		CVirtualMachine_ExecuteScript(vm, script, thisPtr->m_idSelf, 1, 0);

		TargetOid = NWN::INVALIDOBJID;

		if (HaltPickPocket)
		{
			// The script wants to prevent pickpocketing
			return 3;
		}
		else
		{
			// Allow pickpocketing by default
			return CNWSCreature_AIActionPickPocket_Trampoline(thisPtr, actionNode);
		}
	}

	void DetourAttachTransaction(PVOID* ppPointer,
		PVOID pDetour,
		PDETOUR_TRAMPOLINE* ppRealTrampoline)
	{
		try
		{
			constexpr char errorMsg[] = "DetourAttachTransaction failure";
			if (DetourTransactionBegin() != NO_ERROR)
			{
				throw std::runtime_error(errorMsg);
			}
			if (DetourUpdateThread(GetCurrentThread()) != NO_ERROR)
			{
				throw std::runtime_error(errorMsg);
			}
			if (DetourAttachEx(ppPointer, pDetour, ppRealTrampoline, nullptr, nullptr) != NO_ERROR)
			{
				throw std::runtime_error(errorMsg);
			}
			if (DetourTransactionCommit() != NO_ERROR)
			{
				throw std::runtime_error(errorMsg);
			}
		}
		catch (const std::runtime_error &)
		{
			DetourTransactionAbort();
			throw;
		}
	}

	void HookFunctions()
	{
		auto aiPickPocketAction = CNWSCreature_AIActionPickPocket;
		DetourAttachTransaction(std::bit_cast<PVOID*>(&aiPickPocketAction),
			CNWSCreature_AIActionPickPocket_Hook,
			std::bit_cast<PDETOUR_TRAMPOLINE*>(&CNWSCreature_AIActionPickPocket_Trampoline));
	}

	std::optional<int> StringToInt(std::string_view s, int base = 10)
	{
		auto intval = int();
		const auto [p, ec] = std::from_chars(s.data(), s.data() + std::size(s), intval, base);
		if (ec == std::errc())
		{
			return intval;
		}
		else
		{
			return {};
		}
	}

	auto plugin = std::unique_ptr<CPickpocket>();
}

DLLEXPORT Plugin* GetPluginPointerV2()
{
	return plugin.get();
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		plugin = std::make_unique<CPickpocket>();

		char szPath[MAX_PATH];
		GetModuleFileNameA(hModule, szPath, MAX_PATH);
		plugin->SetPluginFullPath(szPath);
	}
	else if (ul_reason_for_call == DLL_PROCESS_DETACH)
	{
		plugin.reset();
	}
	return TRUE;
}

CPickpocket::CPickpocket()
{
	header =
		"NWNX OnPickpocket Plugin V.0.0.2\n" \
		"by Merlin Avery (GodBeastX)\n" \
		"big help from Skywing\n\n" \
		"FOR USE WITH 1.23 NWN2SERVER ONLY!\n";

	description = "This plugin provides script access on pickpocketting.";

	subClass = FunctionClass;
	version = "0.0.1";
}

CPickpocket::~CPickpocket(void)
{
}

bool CPickpocket::Init(char* nwnxhome)
{
	/* Log file */
	std::string logfile(nwnxhome);
	logfile.append("\\");
	logfile.append(GetPluginFileName());
	logfile.append(".txt");
	logger = std::make_unique<LogNWNX>(logfile);
	logger->Info(header.c_str());

	/* Ini file */
	std::string inifile(nwnxhome);
	inifile.append("\\");
	inifile.append(GetPluginFileName());
	inifile.append(".ini");
	logger->Trace("* reading inifile %s", inifile.c_str());

	config = std::make_unique<SimpleIniConfig>(inifile);
	logger->Configure(config.get());

	if (!config->Read("script", &execScript) )
	{
		logger->Info("* 'script' not found in ini");
		return false;
	}

	if (!execScript.empty())
	{
		ScriptName = execScript;

		try
		{
			HookFunctions();
		}
		catch (const std::runtime_error&)
		{
			logger->Info("* Hooking error.");
			return false;
		}
	}

	logger->Info("* Plugin initialized.");

	return true;
}

int CPickpocket::GetInt(char* sFunction, [[maybe_unused]] char* sParam1, int nParam2)
{
	const auto functionId = StringToInt(sFunction);
	if (functionId)
	{
		switch (*functionId)
		{
		case 0: return static_cast<int>(TargetOid);
		case 1: HaltPickPocket = nParam2; break;
		}
	}

	return 0;
}
void CPickpocket::SetInt([[maybe_unused]] char* sFunction,
	[[maybe_unused]] char* sParam1,
	[[maybe_unused]] int nParam2,
	[[maybe_unused]] int nValue)
{

}
float CPickpocket::GetFloat([[maybe_unused]] char* sFunction,
	[[maybe_unused]] char* sParam1,
	[[maybe_unused]] int nParam2)
{
	return 0.0f;
}
void CPickpocket::SetFloat([[maybe_unused]] char* sFunction,
	[[maybe_unused]] char* sParam1,
	[[maybe_unused]] int nParam2,
	[[maybe_unused]] float fValue)
{

}
char* CPickpocket::GetString([[maybe_unused]] char* sFunction,
	[[maybe_unused]] char* sParam1,
	[[maybe_unused]] int nParam2)
{
	return nullptr;
}
void CPickpocket::SetString([[maybe_unused]] char* sFunction,
	[[maybe_unused]] char* sParam1,
	[[maybe_unused]] int nParam2,
	[[maybe_unused]] char* sValue)
{

}
void CPickpocket::GetFunctionClass(char* fClass)
{
	static constexpr auto cls = std::string_view(FunctionClass);
	strncpy_s(fClass, 128, cls.data(), std::size(cls));
}
