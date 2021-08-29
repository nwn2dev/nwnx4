//
// Created by Adriana on 8/28/2021.
//

#include "RpcClient.h"

void RpcClient::Build(std::string url) {
    url_ = url;
    std::shared_ptr<grpc::ChannelInterface> channel(
            grpc::CreateChannel(
                    url,
                    grpc::SslCredentials(grpc::SslCredentialsOptions())));
    stub_ = proto::NWScript::NWNXService::NewStub(channel);
}
