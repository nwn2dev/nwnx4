//
// Created by Adriana on 8/26/2021.
//

#include "Service.h"

using NWNX4::HookHorror;

void Service::start() {
    init();
}

std::future<void> Service::asyncStart() {
    return std::async(std::launch::async, [&]() {
        init();
    });
}

void Service::shutdown() {
    if (server != nullptr) {
        server->Shutdown();
    }
    server = null;
}

void Service::init() {
    ::grpc::ServerBuilder builder;
    auto serviceUrl = url.c_str();
    auto serviceName(getServiceName());

    if (enableSsl) {  // SSL security if provided.
        ::grpc::SslServerCredentialsOptions sslOptions(verifySsl ? GRPC_SSL_REQUEST_CLIENT_CERTIFICATE_AND_VERIFY : GRPC_SSL_REQUEST_CLIENT_CERTIFICATE_BUT_DONT_VERIFY);
        builder.AddListeningPort(serviceUrl, sslCredentials);
        logger->Info("Added %s port with SSL.", serviceName);
    } else {  // Else insecure, but still secretive.
        builder.AddListeningPort(serviceName, ::grpc::InsecureServerCredentials());
        logger->Info("Added %s port without SSL.", serviceName);
    }
    setupBuilder(builder);
    server = builder.BuildAndStart();
    logger->Info("Started %s at %s.", serviceName, serviceUrl);
    server->Wait();
    logger->Info("Shuting down %s.", serviceName);
}

