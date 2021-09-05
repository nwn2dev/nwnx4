//
// Created by Adriana on 8/28/2021.
//

#ifndef XP_RPC_RPCCLIENT_H
#define XP_RPC_RPCCLIENT_H

#include <grpcpp/grpcpp.h>
#include "../../RPC/proto/NWScript/nwnx.grpc.pb.h"

namespace proto = NWNX4::RPC::Proto;

class RpcClient {
public:
    RpcClient() { }
    RpcClient(std::string url): url_(url) {
        Build(url);
    }
    void Build(std::string url);
    std::string url_;
    std::unique_ptr<proto::NWScript::NWNXService::Stub> stub_;
};

#endif //XP_RPC_RPCCLIENT_H
