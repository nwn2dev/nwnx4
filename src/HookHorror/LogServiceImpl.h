//
// Created by Adriana on 6/4/2021.
//

#ifndef HOOKHORROR_LOGSERVICEIMPL_H
#define HOOKHORROR_LOGSERVICEIMPL_H

#include "../misc/log.h"
#include "proto/log.grpc.pb.h"

namespace proto = NWNX4::HookHorror::Proto;

namespace NWNX4::HookHorror::Log {
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

#endif //HOOKHORROR_LOGSERVICEIMPL_H
