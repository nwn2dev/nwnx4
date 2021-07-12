//
// Created by Adriana on 7/4/2021.
//

#include "LogService.h"

extern LogNWNX* logger;

LogService::LogService(SimpleIniConfig *config) {
    std::string logServicePrivateKeyPath;
    std::string logServiceCertPath;
    std::string logServiceCARootPath;

    config->Read("logServiceUrl", &url);

    if (
            config->Read("logServicePrivateKeyPath", &logServicePrivateKeyPath) &&
            std::filesystem::exists(std::filesystem::path(logServicePrivateKeyPath)) &&
            config->Read("logServiceCertPath", &logServiceCertPath) &&
            std::filesystem::exists(std::filesystem::path(logServiceCertPath))) {
        std::ifstream _privateKey(logServicePrivateKeyPath);
        std::ifstream _certificate(logServiceCertPath);
        privateKey = std::string(
                (std::istreambuf_iterator<char>(_privateKey)),
                (std::istreambuf_iterator<char>()));
        certificate = std::string(
                (std::istreambuf_iterator<char>(_certificate)),
                (std::istreambuf_iterator<char>()));
    }

    if (config->Read("logServiceCARootPath", &logServiceCARootPath) &&
        std::filesystem::exists(std::filesystem::path(logServiceCARootPath))) {
        std::ifstream _caRoot(logServicePrivateKeyPath);
        caRoot = std::string(
                (std::istreambuf_iterator<char>(_caRoot)),
                (std::istreambuf_iterator<char>()));
    }
}

LogService::~LogService() {
    if (logServer != nullptr) {
        shutdown();
    }
}

std::future<void> LogService::asyncStart() {
    return std::async(std::launch::async, [&]() {
        LogServiceImpl logService;
        grpc::ServerBuilder builder;

        if (privateKey != "" && certificate != "") {  // SSL security if provided.
            grpc::SslServerCredentialsOptions::PemKeyCertPair pkcp = {privateKey, certificate};
            grpc::SslServerCredentialsOptions sslOptions(GRPC_SSL_REQUEST_AND_REQUIRE_CLIENT_CERTIFICATE_AND_VERIFY);

            if (caRoot != "") {
                sslOptions.pem_root_certs = caRoot;
            }

            sslOptions.pem_key_cert_pairs.push_back(pkcp);
            std::shared_ptr<grpc::ServerCredentials> sslCredentials = grpc::SslServerCredentials(sslOptions);
            builder.AddListeningPort(url, sslCredentials);
            logger->Info("Added log service port with SSL.", url.c_str());
        } else {  // Else insecure, but still secretive.
            builder.AddListeningPort(url, grpc::InsecureServerCredentials());
            logger->Info("Added log service port without SSL.", url.c_str());
        }
        builder.RegisterService(&logService);
        logServer = builder.BuildAndStart();
        logger->Info("Started log service at %s.", url.c_str());
        logServer->Wait();
        logger->Info("Shutdown log service.");
    });
}

void LogService::shutdown() {
    logServer->Shutdown();
}
