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

#if !defined(LEGACY_PLUGIN_H_INCLUDED)
#define LEGACY_PLUGIN_H_INCLUDED

#define NOMINMAX
#include <string>
#include <string_view>
#include <windows.h>

#define MAX_BUFFER 64 * 1024

class LegacyPlugin {
public:
	LegacyPlugin()          = default;
	virtual ~LegacyPlugin() = default;

	LegacyPlugin(const LegacyPlugin&)            = delete;
	LegacyPlugin& operator=(const LegacyPlugin&) = delete;
	LegacyPlugin(LegacyPlugin&&)                 = delete;
	LegacyPlugin& operator=(LegacyPlugin&&)      = delete;

	// Called when a plugin DLL gets loaded.
	virtual bool Init(char*);

	// Called when a request is made from NWScript
	virtual const char* DoRequest(char* gameObject, char* request, char* parameters) = 0;

	// Process query functions like GET_VERSION, ...
	void ProcessQueryFunction(std::string_view function, char* buffer);

	// Return the function class of the plugin in fClass
	virtual void GetFunctionClass(char* fClass);

	// Plugin file name functions
	const char* GetPluginFileName();
	const char* GetPluginFullPath();
	void SetPluginFullPath(std::string_view p);

	// Copy a plugin response into the buffer provided by NWN
	void nwnxcpy(char* buffer, std::string_view response);
	void nwnxcpy(char* buffer, const char* response, size_t len);

protected:
	std::string header;
	std::string subClass;
	std::string version;
	std::string description;

private:
	std::string pluginFileName;
	std::string pluginFullPath;
};

#endif
