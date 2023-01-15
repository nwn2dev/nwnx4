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
#include <algorithm>
#include <fstream>
#include <iostream>
#include <locale>
#include <string>
#include <vector>

LogLevel ParseLogLevel(const std::string& level)
{
	try {
		return (LogLevel)std::stoi(level);
	} catch (std::invalid_argument e) {
	}

	std::string lower(level);
	std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
	if (lower == "none")
		return LogLevel::none;
	else if (lower == "err" || lower == "error")
		return LogLevel::error;
	else if (lower == "warn" || lower == "warning")
		return LogLevel::warning;
	else if (lower == "info" || lower == "information")
		return LogLevel::info;
	else if (lower == "dbg" || lower == "debug")
		return LogLevel::debug;
	else if (lower == "trc" || lower == "trace")
		return LogLevel::trace;
	return LogLevel::info;
}

LogNWNX::LogNWNX(LogLevel level) { m_level = level; }
LogNWNX::LogNWNX(std::string filePath, LogLevel level)
{
	m_level = level;

	if (m_level != LogLevel::none) {
		// Open / create log file
		m_ofPath = filePath;
		m_ofStream.open(m_ofPath, std::ofstream::out | std::ofstream::app);
		if (!m_ofStream) {
			std::cerr << "Cannot open log file: " << m_ofPath << std::endl;
		}
	}
}

void LogNWNX::Log(LogLevel level, const char* format, va_list a)
{
	if (level > m_level)
		return;

	auto t = time(nullptr);
	char nowStr[26]; // 2022-10-09T19:48:01+0200
	strftime(nowStr, 26, "%FT%T%z", localtime(&t));

	// RFC 3339 compat fix
	for (size_t i = 25; i > 22; i--)
		nowStr[i] = nowStr[i - 1];
	nowStr[22] = ':';

	std::string fmt;
	fmt += nowStr;
	fmt += ": ";
	switch (level) {
		case LogLevel::error:
			fmt += "ERROR: ";
			break;
		case LogLevel::warning:
			fmt += " WARN: ";
			break;
		case LogLevel::info:
			fmt += " Info: ";
			break;
		case LogLevel::debug:
			fmt += "  Dbg: ";
			break;
		case LogLevel::trace:
			fmt += "Trace: ";
			break;
		default:
			return;
	}
	fmt += format;

	va_list args;
	va_copy(args, a);
	size_t len = std::vsnprintf(nullptr, 0, fmt.c_str(), args);
	std::vector<char> vec(len + 1);
	std::vsnprintf(&vec[0], len + 1, fmt.c_str(), args);
	va_end(args);
	LogStr(&vec[0]);
}

void LogNWNX::LogStr(const char* message)
{
	if (m_ofStream.is_open()) {
		if (m_maxFileSize > 0 && m_ofStream.tellp() > m_maxFileSize) {
			RotateFile();
		}

		m_ofStream << message << std::endl;
		if (!m_buffered)
			m_ofStream.flush();
	} else {
		std::cout << message << std::endl;
		if (!m_buffered)
			std::cout.flush();
	}
}

void LogNWNX::RotateFile()
{
	m_ofStream.close();

	// Count number of rotated files
	size_t freeSlot = 1;
	while (std::filesystem::exists(m_ofPath.string() + "." + std::to_string(freeSlot))) {
		freeSlot++;
	}

	// Move log.1 -> log.2
	for (; freeSlot >= 2; freeSlot--) {
		std::filesystem::rename(m_ofPath.string() + "." + std::to_string(freeSlot - 1),
		                        m_ofPath.string() + "." + std::to_string(freeSlot));
	}

	// Move log -> log.1
	std::filesystem::rename(m_ofPath, m_ofPath.string() + ".1");

	// Reopen new log file
	m_ofStream.open(m_ofPath, std::ofstream::out | std::ofstream::app);
	if (!m_ofStream) {
		std::cerr << "Cannot open log file: " << m_ofPath << std::endl;
	}
}