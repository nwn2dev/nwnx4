//
// Created by Adriana on 6/5/2021.
//

#include "Rpc.h"

#define PLUGIN_VERSION "0.0.1"

Rpc* plugin;

DLLEXPORT Plugin* GetPluginPointerV2()
{
    return plugin;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
        plugin = new Rpc();
    } else if (ul_reason_for_call == DLL_PROCESS_DETACH) {
        delete plugin;
    }

    return TRUE;
}

Rpc::Rpc() {
    header =
            "NWNX RPC/hook_horror Plugin " PLUGIN_VERSION "\n" \
		"(c) 2021 by ihatemundays \n" \
		"Visit NWNX at: http://www.nwnx.org\n";

    description = "RPC message broker for external microservices and awesomeness.";

    subClass = "RPC";
    version  = PLUGIN_VERSION;
}

Rpc::~Rpc() {}

bool Rpc::Init(char* nwnxhome) {
    return true;
}

void Rpc::GetFunctionClass(char* fClass) {
    strncpy_s(fClass, 128, "RPC", 3);
}
