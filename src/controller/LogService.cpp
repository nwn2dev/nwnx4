//
// Created by Adriana on 7/4/2021.
//

#include "LogService.h"

extern LogNWNX* logger;

LogService::LogService(std::string logServiceUrl) {
    this->logServiceUrl = logServiceUrl;
}

LogService::~LogService() {
}

std::future<void> LogService::asyncStart() {
    return std::async(std::launch::async, [](std::string logServiceUrl) {
        LogServiceImpl logService;
        grpc::ServerBuilder builder;
        builder.AddListeningPort(logServiceUrl, grpc::InsecureServerCredentials());
        builder.RegisterService(&logService);
        std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
        logger->Info("Started log service at %s.", logServiceUrl.c_str());
        server->Wait();
    }, logServiceUrl);
}
