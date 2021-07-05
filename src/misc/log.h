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

#if !defined(LOG_H_INCLUDED)
#define LOG_H_INCLUDED

#include <string>
#include <fstream>
#include "../misc/ini.h"
#include <cstdarg>


enum class LogLevel {
    none = 0,
    error = 1,
    warning = 2,
    info = 3,
    debug = 4,
    trace = 5,
};
LogLevel ParseLogLevel(const std::string& level);


class LogNWNX
{
public:
    LogNWNX(LogLevel level = LogLevel::info);
    LogNWNX(std::string, LogLevel level = LogLevel::info);

    void SetLogLevel(LogLevel level) {
        m_level = level;
    }
    void SetLogLevel(const std::string& level) {
        m_level = ParseLogLevel(level);
    }
    void Configure(const SimpleIniConfig* config) {
        std::string lvl;
        if (config->Read("loglevel", &lvl))
            SetLogLevel(lvl);
    }

    void Trace(_Printf_format_string_ const char* format, ...){
        va_list args;
        va_start(args, format);
        Log(LogLevel::trace, format, args);
        va_end(args);
    }
    void Debug(_Printf_format_string_ const char* format, ...){
        va_list args;
        va_start(args, format);
        Log(LogLevel::debug, format, args);
        va_end(args);
    }
    void Info(_Printf_format_string_ const char* format, ...){
        va_list args;
        va_start(args, format);
        Log(LogLevel::info, format, args);
        va_end(args);
    }
    void Warn(_Printf_format_string_ const char* format, ...){
        va_list args;
        va_start(args, format);
        Log(LogLevel::warning, format, args);
        va_end(args);
    }
    void Err(_Printf_format_string_ const char* format, ...){
        va_list args;
        va_start(args, format);
        Log(LogLevel::error, format, args);
        va_end(args);
    }
protected:
    std::ofstream m_ofStream;
    LogLevel m_level;

    virtual void Log(LogLevel level, const char* format, va_list args);
    virtual void LogStr(const char* message);
};


#endif
