//
// Created by Adriana on 8/26/2021.
//

#include "Service.h"

extern LogNWNX* logger;

namespace NWNX4 {
    namespace RPC {
        Service::~Service() {
            server->Shutdown();
        }

        void Service::Start() {
            Init();
        }

        std::future<void> Service::StartAsync() {
            return std::async(std::launch::async, [&]() {
                Init();
            });
        }

        void Service::Shutdown() {
            if (server != nullptr) {
                server->Shutdown();
            }
            server = nullptr;
        }

        void Service::Init() {
            if (!enabled) {
                return;
            }

            ::grpc::ServerBuilder builder;
            auto serviceUrl = url.c_str();
            auto serviceName(GetServiceName());

            if (enableSsl) {  // SSL security if provided.
                builder.AddListeningPort(serviceUrl, sslCredentials);
                logger->Info("Added %s port with SSL.", serviceName);
            } else {  // Else insecure, but still secretive.
                builder.AddListeningPort(serviceUrl, ::grpc::InsecureServerCredentials());
                logger->Info("Added %s port without SSL.", serviceName);
            }
            SetupBuilder(builder);
            std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
            logger->Info("Started %s at %s.", serviceName, serviceUrl);
            server->Wait();
            logger->Info("Shutting down %s.", serviceName);
        }
    }
}