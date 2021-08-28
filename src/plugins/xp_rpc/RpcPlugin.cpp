//
// Created by Adriana on 7/5/2021.
//

#include "RpcPlugin.h"

#define PLUGIN_VERSION "0.0.1"

RpcPlugin *plugin;

/***************************************************************************
    NWNX and DLL specific functions
***************************************************************************/

DLLEXPORT Plugin* GetPluginPointerV2()
{
    return plugin;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
        plugin = new RpcPlugin();

        char szPath[MAX_PATH];
        GetModuleFileNameA(hModule, szPath, MAX_PATH);
        plugin->SetPluginFullPath(szPath);
    } else if (ul_reason_for_call == DLL_PROCESS_DETACH) {
        delete plugin;
    }

    return true;
}

void RpcPlugin::GetFunctionClass(char* fClass)
{
    strncpy_s(fClass, 128, "RPC", 8);
}

RpcPlugin::RpcPlugin(): logged(false) {
    header =
        "NWNX RPC (hook_horror) Plugin " PLUGIN_VERSION "\n" \
		"(c) 2021 by ihatemundays \n";

    description = "This reduces NWN2Server start times by using links instead of copying module data.";

    subClass = "RPC";
    version  = PLUGIN_VERSION;
}

RpcPlugin::~RpcPlugin() {
    logger->Info("* Plugin unloaded.");
}

bool RpcPlugin::Init(char* nwnxhome) {
    auto yaml = YAML::LoadFile("xp_rpc.yml");
    grpc::CreateChannel


    return true;
}

void RpcPlugin::SetString(char* sFunction, char* sParam1, int nParam2, char* sValue) {
}

char* RpcPlugin::GetString(char* sFunction, char* sParam1, int nParam2) {
    return nullptr;
}
