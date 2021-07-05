//
// Created by Adriana on 7/4/2021.
//

#ifndef NWNX4_GUILOGWORKER_H
#define NWNX4_GUILOGWORKER_H

#include "LogServiceImpl.h"
#include <grpcpp/grpcpp.h>

class GuiLogWorker: public wxThread {
public:
    GuiLogWorker(std::string guiLogServiceUrl, GuiLog *guiLogger);
    ~GuiLogWorker();
    virtual void *Entry();

private:
    std::string serviceUrl;
    GuiLog *logger;
};

#endif //NWNX4_GUILOGWORKER_H
