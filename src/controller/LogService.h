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

class LogService {
public:
    LogService(SimpleIniConfig *config);
    ~LogService();
    std::future<void> asyncStart();
    void shutdown();

private:
    std::string url;
    std::string privateKey;
    std::string certificate;
    std::string caRoot;
    std::unique_ptr<grpc::Server> logServer;
};

#endif //NWNX4_LOGSERVICE_H
