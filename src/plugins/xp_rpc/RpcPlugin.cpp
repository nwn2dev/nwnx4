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
        "NWNX RPC Plugin " PLUGIN_VERSION "\n" \
		"(c) 2021 by ihatemundays \n";

    description = "A better way to build NWN2 microservices.";

    subClass = "RPC";
    version  = PLUGIN_VERSION;
}

RpcPlugin::~RpcPlugin() {
    logger->Info("* RPC plugin unloaded.");
}

bool RpcPlugin::Init(char* nwnxhome) {
    // Build backup log.
    std::string logfile(nwnxhome);
    logfile += "\\";
    logfile += GetPluginFileName();
    logfile += ".txt";
    logger = new LogNWNX(logfile);

    // Build log client; default is to use backup logger.
    logger_ = new NWNX4::RPC::Log::LogClient(nwnxhome);
    logger_->Info(header.c_str());

    // Setup yaml map for services.
    std::string configfile(nwnxhome);
    configfile += "\\";
    configfile += GetPluginFileName();
    configfile += ".yml";

    if (!std::filesystem::exists(std::filesystem::path(configfile))) {
        logger_->Warn("There is no config file at %s.", configfile.c_str());

        return false;
    }

    // Get config.
    logger_->Info("Get config at %s.", configfile.c_str());
    auto config = YAML::LoadFile(configfile);
    auto services = config["clients"];

    if (!services.IsMap()) {
        logger_->Warn("Clients must be a map.");

        return false;
    }

    for (auto element: config["clients"]) {
        clients_.insert(std::make_pair(element.first.as<std::string>(), element.second.as<RpcClient>()));
    }

    logger_->Info("Installed %d client(s) into xp_rpc.", clients_.size());

    return true;
}

int RpcPlugin::GetInt(char* sFunction, char* sParam1, int nParam2) {
    auto client = GetRpcClient(sFunction);

    logger_->Info("Started NWNXGetInt.");

    if (client == nullptr) {
        logger_->Warn("%s client not found.", sFunction);

        return 0;
    }

    logger_->Info("Invoked NWNXGetInt.");

    grpc::ClientContext context;
    proto::NWScript::NWNXGetIntRequest request;
    request.set_sfunction(sFunction);
    request.set_sparam1(sParam1);
    request.set_nparam2(nParam2);
    proto::NWScript::Int response;
    client->stub_->NWNXGetInt(&context, request, &response);

    return response.value();
}

void RpcPlugin::SetInt(char* sFunction, char* sParam1, int nParam2, int nValue) {
    auto client = GetRpcClient(sFunction);

    logger_->Info("Started NWNXSetInt.");

    if (client == nullptr) {
        logger_->Warn("%s client not found.", sFunction);

        return;
    }

    logger_->Info("Invoked NWNXSetInt.");

    grpc::ClientContext context;
    proto::NWScript::NWNXSetIntRequest request;
    request.set_sfunction(sFunction);
    request.set_sparam1(sParam1);
    request.set_nparam2(nParam2);
    request.set_nvalue(nValue);
    proto::NWScript::Void response;
    client->stub_->NWNXSetInt(&context, request, &response);
}

float RpcPlugin::GetFloat(char* sFunction, char* sParam1, int nParam2) {
    auto client = GetRpcClient(sFunction);

    logger_->Info("Started NWNXGetFloat.");

    if (client == nullptr) {
        logger_->Warn("%s client not found.", sFunction);

        return 0.0;
    }

    logger_->Info("Invoked NWNXGetFloat.");

    grpc::ClientContext context;
    proto::NWScript::NWNXGetFloatRequest request;
    request.set_sfunction(sFunction);
    request.set_sparam1(sParam1);
    request.set_nparam2(nParam2);
    proto::NWScript::Float response;
    client->stub_->NWNXGetFloat(&context, request, &response);

    return response.value();
}

void RpcPlugin::SetFloat(char* sFunction, char* sParam1, int nParam2, float fValue) {
    auto client = GetRpcClient(sFunction);

    logger_->Info("Started NWNXSetFloat.");

    if (client == nullptr) {
        logger_->Warn("%s client not found.", sFunction);

        return;
    }

    logger_->Info("Invoked NWNXSetFloat.");

    grpc::ClientContext context;
    proto::NWScript::NWNXSetFloatRequest request;
    request.set_sfunction(sFunction);
    request.set_sparam1(sParam1);
    request.set_nparam2(nParam2);
    request.set_fvalue(fValue);
    proto::NWScript::Void response;
    client->stub_->NWNXSetFloat(&context, request, &response);
}

char* RpcPlugin::GetString(char* sFunction, char* sParam1, int nParam2) {
    auto client = GetRpcClient(sFunction);

    logger_->Info("Started NWNXGetString.");

    if (client == nullptr) {
        logger_->Warn("%s client not found.", sFunction);

        return "";
    }

    logger_->Info("Invoked NWNXGetString.");

    grpc::ClientContext context;
    proto::NWScript::NWNXGetStringRequest request;
    request.set_sfunction(sFunction);
    request.set_sparam1(sParam1);
    request.set_nparam2(nParam2);
    proto::NWScript::String response;
    client->stub_->NWNXGetString(&context, request, &response);

    return (char*) response.value().c_str();
}

void RpcPlugin::SetString(char* sFunction, char* sParam1, int nParam2, char* sValue) {
    auto client = GetRpcClient(sFunction);

    logger_->Info("Started NWNXSetString.");

    if (client == nullptr) {
        logger_->Warn("%s client not found.", sFunction);

        return;
    }

    logger_->Info("Invoked NWNXSetString.");

    grpc::ClientContext context;
    proto::NWScript::NWNXSetStringRequest request;
    request.set_sfunction(sFunction);
    request.set_sparam1(sParam1);
    request.set_nparam2(nParam2);
    request.set_svalue(sValue);
    proto::NWScript::Void response;
    client->stub_->NWNXSetString(&context, request, &response);
}

RpcClient* RpcPlugin::GetRpcClient(char* sFunction) {
    try {
        return &clients_.at(sFunction);
    } catch (std::out_of_range exception) {
        logger_->Err("Service %s not available.", sFunction);
    }

    return nullptr;
}

namespace YAML {
    template<>
    struct convert<RpcClient> {
        static Node encode(const RpcClient& rhs) {
            Node node;
            node.as<std::string>();

            return node;
        }

        static bool decode(const Node& node, RpcClient& rhs) {
            rhs.Build(node.as<std::string>());

            return true;
        }
    };
}