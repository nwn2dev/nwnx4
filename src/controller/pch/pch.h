#pragma once

#include <cstring>
#include <cwchar>
#include <future>

#include <winsock2.h>
#include <windows.h>
#include <WS2tcpip.h>

#include <detours/detours.h>
#include <grpcpp/grpcpp.h>

#include "../../misc/log.h"
#include "../../misc/shmem.h"
#include "../../misc/ini.h"

// gRPC generated files
#include "proto/log.grpc.pb.h"
#include "proto/log.pb.h"
#include "proto/NWScript/nwnx.grpc.pb.h"
#include "proto/NWScript/nwnx.pb.h"
#include "proto/NWScript/types.grpc.pb.h"
#include "proto/NWScript/types.pb.h"