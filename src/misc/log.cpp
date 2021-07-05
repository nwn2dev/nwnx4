/***************************************************************************
    NWNX Log - Logging functions
    Copyright (C) 2006 Ingmar Stieger (Papillon, papillon@nwnx.org)

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

#include "log.h"
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <locale>
#include <string>

LogLevel ParseLogLevel(const std::string& level){
    try {
        return (LogLevel)std::stoi(level);
    }
    catch (std::invalid_argument e) {}

    std::string lower(level);
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    if(lower == "none")
        return LogLevel::none;
    if(lower == "err" || lower == "error")
        return LogLevel::error;
    if(lower == "warn" || lower == "warning")
        return LogLevel::warning;
    if(lower == "info" || lower == "information")
        return LogLevel::info;
    if(lower == "dbg" || lower == "debug")
        return LogLevel::debug;
    if(lower == "trc" || lower == "trace")
        return LogLevel::trace;
    return LogLevel::info;
}


LogNWNX::LogNWNX(LogLevel level)
{
    m_level = level;
}
LogNWNX::LogNWNX(std::string filePath, LogLevel level)
{
    m_level = level;

    if(m_level == LogLevel::none)
        return;

    m_ofStream.open(filePath, std::ofstream::out | std::ofstream::app);
    if (!m_ofStream) {
        std::cerr << "Canot open log file: " << filePath << std::endl;
    }
}

void LogNWNX::Log(LogLevel level, const char* format, va_list a){
    if(level > m_level)
        return;

    auto t = time(nullptr);
    char nowStr[22];// 2021-04-24 13:37:05:
    strftime(nowStr, 22, "%Y-%m-%d %H:%M:%S: ", localtime(&t));

    std::string fmt;
    fmt += nowStr;
    switch(level){
        case LogLevel::error:   fmt += "ERROR:"; break;
        case LogLevel::warning: fmt += "WARN: "; break;
        case LogLevel::info:    fmt += "Info: "; break;
        case LogLevel::debug:   fmt += "Dbg:  "; break;
        case LogLevel::trace:   fmt += "Trace:"; break;
        default: return;
    }
    fmt += format;

    va_list args;
    va_copy(args, a);
    size_t len = std::vsnprintf(nullptr, 0, fmt.c_str(), args);
    va_end(args);
    std::vector<char> vec(len + 1);
    va_copy(args, a);
    std::vsnprintf(&vec[0], len + 1, fmt.c_str(), args);
    va_end(args);
    LogStr(&vec[0]);
}

void LogNWNX::LogStr(const char* message){
    if (m_ofStream) {
        m_ofStream << message << std::endl;
        m_ofStream.flush();
    }
    else {
        std::cout << message << std::endl;
        std::cout.flush();
    }
}
