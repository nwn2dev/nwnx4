//
// Created by Adriana on 8/26/2021.
//

#ifndef HOOKHORROR_LOGCONFIG_H
#define HOOKHORROR_LOGCONFIG_H

#include <filesystem>
#include "../misc/ini.h"

namespace NWNX4::HookHorror::Log {
    class LogConfig final {
    public:
        LogConfig(const char* const nwnxHome);
        bool GetEnabled() { return enabled; }
        bool GetEnableSsl() { return enableSsl; }
        bool GetVerifySsl() { return verifySsl; }
        bool GetHasSslRootCertificates() { return hasSslRootCertificates; }
        const char* const GetUrl() { return url.c_str(); }
        const char* const GetSslKey() { return sslKey.c_str(); }
        const char* const GetSslCertificate() { return sslCertificate.c_str(); }
        const char* const GetSslRootCertificates() { return sslRootCertificates.c_str(); }
    private:
        bool enabled = false;
        bool enableSsl = false;
        bool verifySsl = true;
        bool hasSslRootCertificates = false;
        std::string url;
        std::string sslKey;
        std::string sslCertificate;
        std::string sslRootCertificates;
    };
}

#endif //HOOKHORROR_LOGCONFIG_H
