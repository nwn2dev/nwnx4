//
// Created by Adriana on 8/26/2021.
//

#ifndef MESON_BUILD_DEBUG_SERVICE_H
#define MESON_BUILD_DEBUG_SERVICE_H

#include <future>
#include <grpcpp/grpcpp.h>

namespace NWNX4::HookHorror {
    class Service {
    public:
        ~Service();
        void start();
        std::future<void> asyncStart();
        void shutdown();
    protected:
        bool enableSsl = false;
        bool verifySsl = true;
        ::std::string url;
        ::std::shared_ptr<::grpc::ServerCredentials> sslCredentials;
        std::unique_ptr<grpc::Server> server;
        virtual char* getServiceName() { return 'HookHorror service'; }
        virtual void setupBuilder(::grpc::ServerBuilder& builder) { }
        void init();
    };
}

#endif //MESON_BUILD_DEBUG_SERVICE_H
