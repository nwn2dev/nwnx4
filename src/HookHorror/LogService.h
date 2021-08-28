//
// Created by Adriana on 7/4/2021.
//

#ifndef HOOKHORROR_LOGSERVICE_H
#define HOOKHORROR_LOGSERVICE_H

#include <grpcpp/grpcpp.h>
#include "../misc/log.h"
#include "Service.h"
#include "LogConfig.h"
#include "LogServiceImpl.h"

namespace HookHorror = NWNX4::HookHorror;

namespace NWNX4::HookHorror::Log {
    class LogService final: public HookHorror::Service {
    public:
        LogService(const char* const nwnxHome);
    protected:
        const char* const GetServiceName() { return "log service"; }
        void SetupBuilder(grpc::ServerBuilder& builder);
    private:
        LogServiceImpl service;
    };
}

#endif //HOOKHORROR_LOGSERVICE_H
