//
// Created by Adriana on 6/4/2021.
//

#include "LogServiceImpl.h"

extern LogNWNX* logger;

namespace NWNX4::HookHorror::Log {
    ::grpc::Status LogServiceImpl::Trace(::grpc::ServerContext* context, const ::google::protobuf::StringValue* request, ::google::protobuf::Empty* response) {
        logger->Trace(request->value().c_str());

        return ::grpc::Status::OK;
    }

    ::grpc::Status LogServiceImpl::Debug(::grpc::ServerContext* context, const ::google::protobuf::StringValue* request, ::google::protobuf::Empty* response) {
        logger->Debug(request->value().c_str());

        return ::grpc::Status::OK;
    }

    ::grpc::Status LogServiceImpl::Info(::grpc::ServerContext* context, const ::google::protobuf::StringValue* request, ::google::protobuf::Empty* response) {
        logger->Info(request->value().c_str());

        return ::grpc::Status::OK;
    }

    ::grpc::Status LogServiceImpl::Warn(::grpc::ServerContext* context, const ::google::protobuf::StringValue* request, ::google::protobuf::Empty* response) {
        logger->Warn(request->value().c_str());

        return ::grpc::Status::OK;
    }

    ::grpc::Status LogServiceImpl::Err(::grpc::ServerContext* context, const ::google::protobuf::StringValue* request, ::google::protobuf::Empty* response) {
        logger->Err(request->value().c_str());

        return ::grpc::Status::OK;
    }

    ::grpc::Status LogServiceImpl::LogStr(::grpc::ServerContext* context, const ::google::protobuf::StringValue* request, ::google::protobuf::Empty* response) {
        logger->LogStr(request->value().c_str());

        return ::grpc::Status::OK;
    }
}