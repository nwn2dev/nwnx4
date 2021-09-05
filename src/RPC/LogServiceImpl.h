//
// Created by Adriana on 6/4/2021.
//

#ifndef RPC_LOGSERVICEIMPL_H
#define RPC_LOGSERVICEIMPL_H

#include "../misc/log.h"
#include "proto/log.grpc.pb.h"

namespace proto = NWNX4::RPC::Proto;

namespace NWNX4 {
    namespace RPC {
        namespace Log {
            class LogServiceImpl final: public proto::Log::LogService::Service {
            public:
                ::grpc::Status Trace(::grpc::ServerContext* context, const ::google::protobuf::StringValue* request, ::google::protobuf::Empty* response);
                ::grpc::Status Debug(::grpc::ServerContext* context, const ::google::protobuf::StringValue* request, ::google::protobuf::Empty* response);
                ::grpc::Status Info(::grpc::ServerContext* context, const ::google::protobuf::StringValue* request, ::google::protobuf::Empty* response);
                ::grpc::Status Warn(::grpc::ServerContext* context, const ::google::protobuf::StringValue* request, ::google::protobuf::Empty* response);
                ::grpc::Status Err(::grpc::ServerContext* context, const ::google::protobuf::StringValue* request, ::google::protobuf::Empty* response);
                ::grpc::Status LogStr(::grpc::ServerContext* context, const ::google::protobuf::StringValue* request, ::google::protobuf::Empty* response);
            };
        }
    }
}

#endif //RPC_LOGSERVICEIMPL_H
