//
// Created by Adriana on 7/4/2021.
//

#include "stdwx.h"
#include "GuiLogWorker.h"

GuiLogWorker::GuiLogWorker(std::string guiLogServiceUrl, GuiLog *guiLogger) {
    serviceUrl = guiLogServiceUrl;
    logger = guiLogger;
}

GuiLogWorker::~GuiLogWorker() {
}

void *GuiLogWorker::Entry() {
    LogServiceImpl service;
    grpc::ServerBuilder builder;
    builder.AddListeningPort(serviceUrl, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    logger->Info("Started GUI log service at %s.", serviceUrl.c_str());
    server->Wait();

    return nullptr;
}
