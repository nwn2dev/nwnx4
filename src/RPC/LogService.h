//
// Created by Adriana on 7/4/2021.
//

#ifndef RPC_LOGSERVICE_H
#define RPC_LOGSERVICE_H

#include <grpcpp/grpcpp.h>
#include "../misc/log.h"
#include "Service.h"
#include "LogConfig.h"
#include "LogServiceImpl.h"

namespace RPC = NWNX4::RPC;

namespace NWNX4 {
    namespace RPC {
        namespace Log {
            class LogService final: public RPC::Service {
            public:
                LogService(const char* const nwnxHome);
            protected:
                const char* const GetServiceName() { return "log service"; }
                void SetupBuilder(grpc::ServerBuilder& builder);
            private:
                LogServiceImpl service;
            };
        }
    }
}

#endif //RPC_LOGSERVICE_H
