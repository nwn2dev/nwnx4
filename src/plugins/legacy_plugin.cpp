/***************************************************************************
    NWNX Plugin - Plugins are derived from this class
    Copyright (C) 2007 Ingmar Stieger (Papillon, papillon@blackdagger.com)

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

#include "../plugins/legacy_plugin.h"
#include <algorithm>
#include <filesystem>
#include <cassert>

bool LegacyPlugin::Init(char* parameter)
{
	return true;
}

void LegacyPlugin::ProcessQueryFunction(std::string_view function, char* buffer)
{
	if (function == "GET_SUBCLASS")
		nwnxcpy(buffer, subClass.c_str());
	else if (function == "GET_VERSION")
		nwnxcpy(buffer, version.c_str());
	else if (function == "GET_DESCRIPTION")
		nwnxcpy(buffer, description.c_str());
}

void LegacyPlugin::GetFunctionClass(char* fClass)
{
	assert(fClass);
	fClass[0] = '\0';
}

const char* LegacyPlugin::GetPluginFileName()
{
	return pluginFileName.c_str();
}

const char* LegacyPlugin::GetPluginFullPath()
{
	return pluginFullPath.c_str();
}

void LegacyPlugin::SetPluginFullPath(std::string_view p)
{
	const auto fullPath = std::filesystem::path(p);
	if (fullPath.has_stem())
	{
		pluginFullPath = fullPath.string();
		pluginFileName = fullPath.stem().string();
	}
	else
	{
		throw std::runtime_error("SetPluginFullPath: input path has no stem.");
	}
}

void LegacyPlugin::nwnxcpy(char* buffer, std::string_view response)
{
	assert(buffer);
	constexpr auto maxSize = static_cast<size_t>(MAX_BUFFER - 1);
	const auto r = response.substr(0, maxSize);
	std::ranges::copy(r, buffer);
	buffer[std::size(r)] = '\0';
}

void LegacyPlugin::nwnxcpy(char* buffer, const char* response, size_t len)
{
	assert(buffer);
	assert(response);
	nwnxcpy(buffer, { response, len });
}