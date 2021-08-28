//
// Created by Adriana on 7/4/2021.
//

#ifndef NWNX4_LOGSERVICE_H
#define NWNX4_LOGSERVICE_H

#include <filesystem>
#include <future>
#include <grpcpp/grpcpp.h>
#include "LogServiceImpl.h"
#include "../misc/log.h"
#include "../misc/HookHorror/Service.h"

class LogService: public ::NWNX4::HookHorror::Service {
public:
    explicit LogService(SimpleIniConfig *config);
protected:
    void setupBuilder(::grpc::ServerBuilder& builder);
private:
    LogServiceImpl service;
};

#endif //NWNX4_LOGSERVICE_H
