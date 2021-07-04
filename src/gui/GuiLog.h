#ifndef MAIN_GUI_LOG_H
#define MAIN_GUI_LOG_H

#include "stdwx.h"
#include "../misc/log.h"

class GuiLog: public LogNWNX {
public:
    GuiLog();
protected:
    void LogStr(const char* message);
};

#endif //MAIN_GUI_LOG_H
