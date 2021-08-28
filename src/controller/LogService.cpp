//
// Created by Adriana on 7/4/2021.
//

#include "LogService.h"

extern LogNWNX* logger;

LogService::LogService(SimpleIniConfig *config) {
    std::string sslKeyPath;
    std::string sslCertPath;
    std::string sslRootCertsPath;

    config->Read("logServiceUrl", &url);

    if (
            config->Read("logServiceSslKeyPath", &sslKeyPath) &&
            std::filesystem::exists(std::filesystem::path(sslKeyPath)) &&
            config->Read("logServiceSslCertPath", &sslCertPath) &&
            std::filesystem::exists(std::filesystem::path(sslCertPath))) {
        std::ifstream key(sslKeyPath);
        std::ifstream cert(sslCertPath);
        auto _key(std::string(
            (std::istreambuf_iterator<char>(key)),
            (std::istreambuf_iterator<char>())));
        auto _cert(std::string(
            (std::istreambuf_iterator<char>(cert)),
            (std::istreambuf_iterator<char>())));
        grpc::SslServerCredentialsOptions::PemKeyCertPair pkcp = {_key, _cert};
        grpc::SslServerCredentialsOptions sslOptions;

        if (
            config->Read("logServiceSslRootCertsPath", &sslRootCertsPath) &&
            std::filesystem::exists(std::filesystem::path(sslRootCertsPath))) {
            std::ifstream rootCerts(sslRootCertsPath);
            sslOptions.pem_root_certs = std::string(
                (std::istreambuf_iterator<char>(rootCerts)),
                (std::istreambuf_iterator<char>()));
        } else {
            sslOptions.pem_root_certs = "";
        }

        sslOptions.pem_key_cert_pairs.push_back(pkcp);
        sslCredentials = ::grpc::SslServerCredentials(sslOptions);

        config->Read("logServiceSslVerify", &verifySsl);
        enableSsl = true;
    }
}

void LogService::setupBuilder(::grpc::ServerBuilder& builder) {
    builder.RegisterService(&service);
}