#pragma once

#include <cassert>
#include <cstring>
#include <cwchar>
#include <filesystem>
#include <future>

#include <WS2tcpip.h>
#include <windows.h>
#include <winsock2.h>

#include <detours/detours.h>

#include "../../misc/ini.h"
#include "../../misc/log.h"
#include "../../misc/shmem.h"
