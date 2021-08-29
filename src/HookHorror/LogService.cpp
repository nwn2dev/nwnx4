//
// Created by Adriana on 7/4/2021.
//

#include "LogService.h"

namespace NWNX4::HookHorror::Log {
    LogService::LogService(const char* const nwnxHome) {
        auto config = new LogConfig(nwnxHome);

        if (!config->GetEnabled()) {
            return;
        }

        enabled = true;
        url = config->GetUrl();  // Get the URL from the configuration.

        // If no SSL security, no more setup.
        if (!config->GetEnableSsl()) {
            return;
        }

        enableSsl = true;
        grpc::SslServerCredentialsOptions::PemKeyCertPair pkcp = { config->GetSslKey(), config->GetSslCertificate() };
        grpc::SslServerCredentialsOptions sslOptions(config->GetVerifySsl() ?
            GRPC_SSL_REQUEST_CLIENT_CERTIFICATE_AND_VERIFY :
            GRPC_SSL_REQUEST_CLIENT_CERTIFICATE_BUT_DONT_VERIFY);
        sslOptions.pem_root_certs = config->GetHasSslRootCertificates() ?
            config->GetSslRootCertificates() :
            "";
        sslOptions.pem_key_cert_pairs.push_back(pkcp);
        sslCredentials = ::grpc::SslServerCredentials(sslOptions);
    }

    void LogService::SetupBuilder(::grpc::ServerBuilder& builder) {
        builder.RegisterService(&service);
    }
}
