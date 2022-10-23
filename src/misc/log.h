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

#include "../misc/ini.h"
#include <cstdarg>
#include <filesystem>
#include <fstream>
#include <string>

enum class LogLevel {
	none    = 0,
	error   = 1,
	warning = 2,
	info    = 3,
	debug   = 4,
	trace   = 5,
};
LogLevel ParseLogLevel(const std::string& level);

class LogNWNX {
public:
	LogNWNX(LogLevel level = LogLevel::info);
	LogNWNX(std::string, LogLevel level = LogLevel::info);

	LogLevel Level() const { return m_level; }

	void SetLogLevel(LogLevel level) { m_level = level; }
	void SetLogLevel(const std::string& level) { m_level = ParseLogLevel(level); }
	void Configure(const SimpleIniConfig* config)
	{
		std::string lvl;
		if (config->Read("log_level", &lvl) || config->Read("loglevel", &lvl))
			SetLogLevel(lvl);

		config->Read("log_buffered", &m_buffered);

		std::string maxsize;
		if (config->Read("log_max_file_size", &maxsize) && maxsize.size() > 0) {
			size_t mult = 1;
			switch (maxsize.back()) {
				case 'g':
				case 'G':
					mult *= 1024;
				case 'm':
				case 'M':
					mult *= 1024;
				case 'k':
				case 'K':
					mult *= 1024;
					maxsize.pop_back();
			}
			try {
				m_maxFileSize = std::stoul(maxsize) * mult;
			} catch (std::invalid_argument e) {
			}
		}
	}

	void Trace(_Printf_format_string_ const char* format, ...)
	{
		va_list args;
		va_start(args, format);
		Log(LogLevel::trace, format, args);
		va_end(args);
	}
	void Debug(_Printf_format_string_ const char* format, ...)
	{
		va_list args;
		va_start(args, format);
		Log(LogLevel::debug, format, args);
		va_end(args);
	}
	void Info(_Printf_format_string_ const char* format, ...)
	{
		va_list args;
		va_start(args, format);
		Log(LogLevel::info, format, args);
		va_end(args);
	}
	void Warn(_Printf_format_string_ const char* format, ...)
	{
		va_list args;
		va_start(args, format);
		Log(LogLevel::warning, format, args);
		va_end(args);
	}
	void Err(_Printf_format_string_ const char* format, ...)
	{
		va_list args;
		va_start(args, format);
		Log(LogLevel::error, format, args);
		va_end(args);
	}
	virtual void LogStr(const char* message);

protected:
	LogLevel m_level;
	size_t m_maxFileSize = 0;

	std::filesystem::path m_ofPath;
	std::ofstream m_ofStream;
	size_t m_counter = 0;

	bool m_buffered = false;

	virtual void Log(LogLevel level, const char* format, va_list args);

	void RotateFile();
};

#endif
