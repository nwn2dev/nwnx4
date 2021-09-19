/***************************************************************************
    NWNX RPC - Middleware between server and microservices.
    Copyright (C) 2021 ihatemundays

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 ***************************************************************************/
#if !defined(RPCPLUGIN_H_INCLUDED)
#define RPCPLUGIN_H_INCLUDED

#define DLLEXPORT extern "C" __declspec(dllexport)

#include <filesystem>
#include <windows.h>
#include <windowsx.h>
#include <specstrings.h>
#include <yaml-cpp/yaml.h>
#include "../plugin.h"
#include "../../misc/log.h"
#include "../../RPC/LogClient.h"
#include "proto/NWScript/nwnx.grpc.pb.h"
#include "RpcClient.h"

class RpcPlugin: public Plugin {
public:
    RpcPlugin();
    ~RpcPlugin();

    void GetFunctionClass(char* fClass);
    bool Init(char* nwnxhome);

    int GetInt(char* sFunction, char* sParam1, int nParam2);
    void SetInt(char* sFunction, char* sParam1, int nParam2, int nValue);
    float GetFloat(char* sFunction, char* sParam1, int nParam2);
    void SetFloat(char* sFunction, char* sParam1, int nParam2, float fValue);
    char* GetString(char* sFunction, char* sParam1, int nParam2);
    void SetString(char* sFunction, char* sParam1, int nParam2, char* sValue);
private:
    bool logged;
    NWNX4::RPC::Log::LogClient* logger_;
    std::map<std::string, RpcClient> clients_;
    RpcClient* GetRpcClient(char* sFunction);
};

#endif
