//
// Created by Adriana on 6/4/2021.
//

#include "LogServiceImpl.h"

extern LogNWNX* logger;

::grpc::Status LogServiceImpl::logTrace(::grpc::ServerContext* context, const ::google::protobuf::StringValue* request, ::google::protobuf::BoolValue* response) {
    logger->Trace(request->value().c_str());
    response->set_value(true);

    return ::grpc::Status::OK;
}


::grpc::Status LogServiceImpl::logDebug(::grpc::ServerContext* context, const ::google::protobuf::StringValue* request, ::google::protobuf::BoolValue* response) {
    logger->Debug(request->value().c_str());
    response->set_value(true);

    return ::grpc::Status::OK;
}

::grpc::Status LogServiceImpl::logInfo(::grpc::ServerContext* context, const ::google::protobuf::StringValue* request, ::google::protobuf::BoolValue* response) {
    logger->Info(request->value().c_str());
    response->set_value(true);

    return ::grpc::Status::OK;
}

::grpc::Status LogServiceImpl::logWarn(::grpc::ServerContext* context, const ::google::protobuf::StringValue* request, ::google::protobuf::BoolValue* response) {
    logger->Warn(request->value().c_str());
    response->set_value(true);

    return ::grpc::Status::OK;
}

::grpc::Status LogServiceImpl::logErr(::grpc::ServerContext* context, const ::google::protobuf::StringValue* request, ::google::protobuf::BoolValue* response) {
    logger->Err(request->value().c_str());
    response->set_value(true);

    return ::grpc::Status::OK;
}
