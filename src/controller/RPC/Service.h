//
// Created by Adriana on 8/26/2021.
//

#ifndef MESON_BUILD_DEBUG_SERVICE_H
#define MESON_BUILD_DEBUG_SERVICE_H

#include <future>
#include <grpcpp/grpcpp.h>
#include "../misc/log.h"

namespace NWNX4::RPC {
    class Service {
    public:
        ~Service();
        void Start();
        std::future<void> StartAsync();
        void Shutdown();
    protected:
        bool enabled = false;
        bool enableSsl = false;
        bool verifySsl = true;
        ::std::string url;
        ::std::shared_ptr<::grpc::ServerCredentials> sslCredentials;
        ::std::unique_ptr<::grpc::Server> server;
        virtual const char* const GetServiceName() { return "RPC service"; }
        virtual void SetupBuilder(::grpc::ServerBuilder& builder) { }
        void Init();
    };
}

#endif //MESON_BUILD_DEBUG_SERVICE_H
