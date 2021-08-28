//
// Created by Adriana on 7/5/2021.
//

#include "RpcPlugin.h"

#define PLUGIN_VERSION "0.0.1"

RpcPlugin *plugin;

LogNWNX* logger;

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
        "NWNX RPC (w/ HookHorror) Plugin " PLUGIN_VERSION "\n" \
		"(c) 2021 by ihatemundays \n";

    description = "A better way to build NWN2 microservices.";

    subClass = "RPC";
    version  = PLUGIN_VERSION;
}

RpcPlugin::~RpcPlugin() {
    logger->Info("* Plugin unloaded.");
}

bool RpcPlugin::Init(char* nwnxhome) {
    // Build backup log.
    std::string logfile(nwnxhome);
    logfile += "\\";
    logfile += GetPluginFileName();
    logfile += ".txt";
    logger = new LogNWNX(logfile);

    // Build log client; default is to use backup logger.
    logger_ = new NWNX4::HookHorror::Log::LogClient(nwnxhome);
    logger_->Info(header.c_str());

    return true;
}

void RpcPlugin::SetString(char* sFunction, char* sParam1, int nParam2, char* sValue) {
}

char* RpcPlugin::GetString(char* sFunction, char* sParam1, int nParam2) {
    return nullptr;
}
