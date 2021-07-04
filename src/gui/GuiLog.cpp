#include "GuiLog.h"

GuiLog::GuiLog(): LogNWNX() {}

void GuiLog::LogStr(const char* message) {
    wxLogMessage(message);
}
