#ifndef MAIN_GUI_LOG_H
#define MAIN_GUI_LOG_H

#include "../misc/log.h"

class GuiLog: public LogNWNX {
public:
    GuiLog();
    void LogStr(const char* message);
};

#endif //MAIN_GUI_LOG_H
