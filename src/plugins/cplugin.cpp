#include "cplugin.h"

extern LogNWNX* logger;

CPlugin::CPlugin(HINSTANCE hDLL) {
	void* callDelete = GetProcAddress(hDLL, "NWNXCPlugin_Delete");
	if (callDelete != nullptr) {
		CallDelete = reinterpret_cast<CallDelete_>(callDelete);
	}

	void* callGetPluginName = GetProcAddress(hDLL, "NWNXCPlugin_GetPluginName");
	if (callGetPluginName != nullptr) {
		CallGetPluginName = reinterpret_cast<CallGetPluginName_>(callGetPluginName);
	}

	void* callGetPluginVersion = GetProcAddress(hDLL, "NWNXCPlugin_GetPluginVersion");
	if (callGetPluginVersion != nullptr) {
		CallGetPluginVersion = reinterpret_cast<CallGetPluginVersion_>(callGetPluginVersion);
	}

	void* callGetInt = GetProcAddress(hDLL, "NWNXCPlugin_GetInt");
	if (callGetInt != nullptr) {
		CallGetInt = reinterpret_cast<CallGetInt_>(callGetInt);
	}

	void* callSetInt = GetProcAddress(hDLL, "NWNXCPlugin_SetInt");
	if (callSetInt != nullptr) {
		CallSetInt = reinterpret_cast<CallSetInt_>(callSetInt);
	}

	void* callGetFloat = GetProcAddress(hDLL, "NWNXCPlugin_GetFloat");
	if (callGetFloat != nullptr) {
		CallGetFloat = reinterpret_cast<CallGetFloat_>(callGetFloat);
	}

	void* callSetFloat = GetProcAddress(hDLL, "NWNXCPlugin_SetFloat");
	if (callSetFloat != nullptr) {
		CallSetFloat = reinterpret_cast<CallSetFloat_>(callSetFloat);
	}

	void* callGetString = GetProcAddress(hDLL, "NWNXCPlugin_GetString");
	if (callGetString != nullptr) {
		CallGetString = reinterpret_cast<CallGetString_>(callGetString);
	}

	void* callSetString = GetProcAddress(hDLL, "NWNXCPlugin_SetString");
	if (callSetString != nullptr) {
		CallSetString = reinterpret_cast<CallSetString_>(callSetString);
	}

	SetPluginName();
	SetPluginVersion();
}

CPlugin::~CPlugin() {
	if (!Delete()) {
		logger->Err("Cannot delete plugin for %s@%s", GetPluginName(), GetPluginVersion());
	}
}

bool CPlugin::Delete() {
	try {
		if (CallDelete == nullptr) {
			return false;
		}

		return CallDelete(plugin);
	} catch (const std::exception& exception) {
		logger->Warn("Calling Delete() failed: %s", exception.what());
	}

	return false;
}

void CPlugin::SetPluginName() {
	try {
		if (plugin == nullptr) {
			throw std::exception("Plugin missing");
		}

		if (CallGetPluginName == nullptr) {
			return;
		}

		pluginName = CallGetPluginName();
	} catch (const std::exception& exception) {
		logger->Warn("Calling SetPluginName() failed: %s", exception.what());
	}
}

char* CPlugin::GetPluginName() {
	return pluginName;
}

void CPlugin::SetPluginVersion() {
	try {
		if (plugin == nullptr) {
			throw std::exception("Plugin missing");
		}

		if (CallGetPluginVersion == nullptr) {
			return;
		}

		pluginVersion = CallGetPluginVersion();
	} catch (const std::exception& exception) {
		logger->Warn("Calling SetPluginVersion() failed: %s", exception.what());
	}
}

char* CPlugin::GetPluginVersion() {
	return pluginVersion;
}

int CPlugin::GetInt(char* sFunction, char* sParam1, int nParam2) {
	try {
		if (plugin == nullptr) {
			throw std::exception("Plugin missing");
		}

		if (CallGetInt == nullptr) {
			return 0;
		}

		return CallGetInt(plugin, sFunction, sParam1, nParam2);
	} catch (const std::exception& exception) {
		logger->Warn("Calling GetInt() failed: %s", exception.what());
	}

	return 0;
}

void CPlugin::SetInt(char* sFunction, char* sParam1, int nParam2, int nValue) {
	try {
		if (plugin == nullptr) {
			throw std::exception("Plugin missing");
		}

		if (CallSetInt == nullptr) {
			return;
		}

		CallSetInt(plugin, sFunction, sParam1, nParam2, nValue);
	} catch (const std::exception& exception) {
		logger->Warn("Calling SetInt() failed: %s", exception.what());
	}
}

float CPlugin::GetFloat(char* sFunction, char* sParam1, int nParam2) {
	try {
		if (plugin == nullptr) {
			throw std::exception("Plugin missing");
		}

		if (CallGetFloat == nullptr) {
			return 0.0;
		}

		return CallGetFloat(plugin, sFunction, sParam1, nParam2);
	} catch (const std::exception& exception) {
		logger->Warn("Calling GetFloat() failed: %s", exception.what());
	}

	return 0.0;
}

void CPlugin::SetFloat(char* sFunction, char* sParam1, int nParam2, float fValue) {
	try {
		if (plugin == nullptr) {
			throw std::exception("Plugin missing");
		}

		if (CallSetFloat == nullptr) {
			return;
		}

		CallSetFloat(plugin, sFunction, sParam1, nParam2, fValue);
	} catch (const std::exception& exception) {
		logger->Warn("Calling SetFloat() failed: %s", exception.what());
	}
}

char* CPlugin::GetString(char* sFunction, char* sParam1, int nParam2) {
	try {
		if (plugin == nullptr) {
			throw std::exception("Plugin missing");
		}

		if (CallGetString == nullptr) {
			return nullptr;
		}

		return CallGetString(plugin, sFunction, sParam1, nParam2);
	} catch (const std::exception& exception) {
		logger->Warn("Calling GetString() failed: %s", exception.what());
	}

	return nullptr;
}

void CPlugin::SetString(char* sFunction, char* sParam1, int nParam2, char* sValue) {
	try {
		if (plugin == nullptr) {
			throw std::exception("Plugin missing");
		}

		if (CallSetString == nullptr) {
			return;
		}

		CallSetString(plugin, sFunction, sParam1, nParam2, sValue);
	} catch (const std::exception& exception) {
		logger->Warn("Calling SetString() failed: %s", exception.what());
	}
}

// ABI Version: 1
void CPluginV1::New(CPluginInitInfoV1* initInfo) {
	try {
		if (CallNew == nullptr) {
			return;
		}

		plugin = CallNew(initInfo);
	} catch (const std::exception& exception) {
		logger->Warn("Calling Delete() failed: %s", exception.what());
	}
}
