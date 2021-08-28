//
// Created by Adriana on 8/27/2021.
//

#include "LogConfig.h"

namespace NWNX4::HookHorror::Log {
    LogConfig::LogConfig(const char* const nwnxHome) {
        // Check nwnx.ini file exists.
        std::string nwnxIniPath(nwnxHome);
        nwnxIniPath += "\\nwnx.ini";

        if (!std::filesystem::exists(nwnxIniPath)) {
            return;
        }

        auto config = new SimpleIniConfig(nwnxIniPath);

        // No URL, no log service.
        if (!config->Read("logServiceUrl", &url)) {
            return;
        }

        enabled = true;

        // No SSL key or certificate, no SSL security.
        std::string sslKeyPath;
        std::string sslCertPath;

        if (!(
            config->Read("logServiceSslKeyPath", &sslKeyPath) &&
            std::filesystem::exists(std::filesystem::path(sslKeyPath)) &&
            config->Read("logServiceSslCertPath", &sslCertPath) &&
            std::filesystem::exists(std::filesystem::path(sslCertPath)))) {
            return;
        }

        enableSsl = true;
        std::ifstream key(sslKeyPath);
        std::ifstream cert(sslCertPath);
        sslKey = std::string(
            (std::istreambuf_iterator<char>(key)),
            (std::istreambuf_iterator<char>()));
        sslCertificate = std::string(
            (std::istreambuf_iterator<char>(cert)),
            (std::istreambuf_iterator<char>()));

        // Verify SSL setting exists and apply directly.
        config->Read("logServiceSslVerify", &verifySsl);

        // No SSL root certificate path, no need to set it up.
        std::string sslRootCertsPath;

        if (!(
            config->Read("logServiceSslRootCertsPath", &sslRootCertsPath) &&
            std::filesystem::exists(std::filesystem::path(sslRootCertsPath)))) {
            return;
        }

        hasSslRootCertificates = true;
        std::ifstream rootCerts(sslRootCertsPath);
        sslRootCertificates = std::string(
            (std::istreambuf_iterator<char>(rootCerts)),
            (std::istreambuf_iterator<char>()));
    }
}