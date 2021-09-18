//
// Created by Adriana on 8/27/2021.
//

#include "LogClient.h"

extern LogNWNX* logger;

namespace NWNX4 {
    namespace RPC {
        namespace Log {
            void LogClient::LogStr(const char* message) {
                if (!useLogService) {
                    logger->LogStr(message);

                    return;
                }

                // Use stub to log the string.
                grpc::ClientContext context;
                google::protobuf::StringValue request;
                request.set_value(message);
                google::protobuf::Empty response;
                stub->LogStr(&context, request, &response);
            }
        }
    }
}
