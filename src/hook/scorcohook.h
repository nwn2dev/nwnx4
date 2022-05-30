#ifndef NWNX4_SCORCOHOOK_H
#define NWNX4_SCORCOHOOK_H

// #include <windows.h>
// #include <string.h>
// #include <stdio.h>
// #include <detours/detours.h>
#include "../misc/log.h"
#include "../plugins/database/dbplugin.h" // For compatibility with current SQL plugins implementation

int SCORCOHook(LogNWNX* logger);


void SCORCOSetLegacyDBPlugin(DBPlugin* dbplugin);

#endif