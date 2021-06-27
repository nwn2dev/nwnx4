//
// Created by Adriana on 6/4/2021.
//

#ifndef NWNX4_LOGSERVICEIMPL_H
#define NWNX4_LOGSERVICEIMPL_H

#include "GuiLog.h"
#include "log.grpc.pb.h"

class LogServiceImpl final: public Gui::LogService::Service {
public:
    ::grpc::Status logTrace(::grpc::ServerContext* context, const ::google::protobuf::StringValue* request, ::google::protobuf::BoolValue* response);
    ::grpc::Status logDebug(::grpc::ServerContext* context, const ::google::protobuf::StringValue* request, ::google::protobuf::BoolValue* response);
    ::grpc::Status logInfo(::grpc::ServerContext* context, const ::google::protobuf::StringValue* request, ::google::protobuf::BoolValue* response);
    ::grpc::Status logWarn(::grpc::ServerContext* context, const ::google::protobuf::StringValue* request, ::google::protobuf::BoolValue* response);
    ::grpc::Status logErr(::grpc::ServerContext* context, const ::google::protobuf::StringValue* request, ::google::protobuf::BoolValue* response);
};


#endif //NWNX4_LOGSERVICEIMPL_H
