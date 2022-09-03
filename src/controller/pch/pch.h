#pragma once

#include <cstring>
#include <cwchar>
#include <future>
#include <filesystem>
#include <cassert>

#include <winsock2.h>
#include <windows.h>
#include <WS2tcpip.h>

#include <detours/detours.h>

#include "../../misc/log.h"
#include "../../misc/shmem.h"
#include "../../misc/ini.h"
