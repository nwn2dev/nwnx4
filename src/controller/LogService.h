//
// Created by Adriana on 7/4/2021.
//

#ifndef NWNX4_LOGSERVICE_H
#define NWNX4_LOGSERVICE_H

#include <future>
#include <grpcpp/grpcpp.h>
#include "LogServiceImpl.h"
#include "../misc/log.h"

class LogService {
public:
    LogService(std::string logServiceUrl);
    ~LogService();
    std::future<void> asyncStart();

private:
    std::string logServiceUrl;
};

#endif //NWNX4_LOGSERVICE_H
