#ifndef NWNX4_SCORCOHOOK_H
#define NWNX4_SCORCOHOOK_H

// #include <windows.h>
// #include <string.h>
// #include <stdio.h>
// #include <detours/detours.h>
#include "../misc/log.h"
#include "../plugins/database/dbplugin.h" // For compatibility with current SQL plugins implementation

/// Hooks SCORCO functions, returns true on success
bool SCORCOHook(LogNWNX* logger);

/// Set DB plugin pointer for forwarding legacy queries
void SCORCOSetLegacyDBPlugin(DBPlugin* dbplugin);

#endif