//
// Created by Adriana on 8/27/2021.
//

#ifndef RPC_LOGCLIENT_H
#define RPC_LOGCLIENT_H

#include <filesystem>
#include <grpcpp/grpcpp.h>
#include "../misc/ini.h"
#include "../misc/log.h"
#include "proto/log.grpc.pb.h"
#include "LogConfig.h"

namespace proto = NWNX4::RPC::Proto;

namespace NWNX4 {
    namespace RPC {
        namespace Log {
            class LogClient final: public LogNWNX {
            public:
                LogClient(const char* const nwnxHome, LogLevel level = LogLevel::info): LogNWNX(level) {
                    auto config = new LogConfig(nwnxHome);

                    if (!config->GetEnabled()) {
                        return;
                    }

                    useLogService = true;

                    if (config->GetEnableSsl()) {
                        std::shared_ptr<grpc::ChannelInterface> channel(
                                grpc::CreateChannel(
                                        config->GetUrl(),
                                        grpc::SslCredentials(grpc::SslCredentialsOptions())));
                        stub = proto::Log::LogService::NewStub(channel);

                        return;
                    }

                    std::shared_ptr<grpc::ChannelInterface> channel(
                            grpc::CreateChannel(config->GetUrl(), grpc::InsecureChannelCredentials()));
                    stub = proto::Log::LogService::NewStub(channel);
                }
                void LogStr(const char* message);
            private:
                bool useLogService = false;
                std::unique_ptr<proto::Log::LogService::Stub> stub;
            };
        }
    }
}

#endif //RPC_LOGCLIENT_H
