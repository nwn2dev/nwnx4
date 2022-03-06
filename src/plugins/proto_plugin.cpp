#include "proto_plugin.h"

extern LogNWNX* logger;

ProtoPlugin::ProtoPlugin(string pluginPath, HINSTANCE hDLL) {
	// Setup the plugin full path
	auto fileName = _strdup(pluginPath.c_str());
	SetPluginFullPath(fileName);

	// Read each of the descriptors
	typedef char* (WINAPI* CallGetDescriptor_)();
	void* CallGetHeaderDescriptor = GetProcAddress(hDLL, "GetHeaderDescriptor");
	if (CallGetHeaderDescriptor != nullptr) {
		header = reinterpret_cast<CallGetDescriptor_>(CallGetHeaderDescriptor)();
	}
	void* CallGetDescriptionDescriptor = GetProcAddress(hDLL, "GetDescriptionDescriptor");
	if (CallGetDescriptionDescriptor != nullptr) {
		description = reinterpret_cast<CallGetDescriptor_>(CallGetDescriptionDescriptor)();
	}
	void* CallGetSubClassDescriptor = (CallGetDescriptor_)GetProcAddress(hDLL, "GetSubClassDescriptor");
	if (CallGetSubClassDescriptor != nullptr) {
		subClass = reinterpret_cast<CallGetDescriptor_>(CallGetSubClassDescriptor)();
	}
	auto CallGetVersionDescriptor = (CallGetDescriptor_)GetProcAddress(hDLL, "GetVersionDescriptor");
	if (CallGetVersionDescriptor != nullptr) {
		version = reinterpret_cast<CallGetDescriptor_>(CallGetVersionDescriptor)();
	}

	// Get each of the function pointers from the DLL
	void* callInit = GetProcAddress(hDLL, "Init");
	if (callInit != nullptr) {
		CallInit = reinterpret_cast<CallInit_>(callInit);
	}
	void* callGetFunctionClass = GetProcAddress(hDLL, "GetFunctionClass");
	if (callGetFunctionClass != nullptr) {
		CallGetFunctionClass = reinterpret_cast<CallGetFunctionClass_>(callGetFunctionClass);
	}
	void* callGetInt = GetProcAddress(hDLL, "GetInt");
	if (callGetInt != nullptr) {
		CallGetInt = reinterpret_cast<CallGetInt_>(callGetInt);
	}
	void* callSetInt = GetProcAddress(hDLL, "SetInt");
	if (callSetInt != nullptr) {
		CallSetInt = reinterpret_cast<CallSetInt_>(callSetInt);
	}
	void* callGetFloat = GetProcAddress(hDLL, "GetFloat");
	if (callGetFloat != nullptr) {
		CallGetFloat = reinterpret_cast<CallGetFloat_>(callGetFloat);
	}
	void* callSetFloat = GetProcAddress(hDLL, "SetFloat");
	if (callSetFloat != nullptr) {
		CallSetFloat = reinterpret_cast<CallSetFloat_>(callSetFloat);
	}
	void* callGetString = GetProcAddress(hDLL, "GetString");
	if (callGetString != nullptr) {
		CallGetString = reinterpret_cast<CallGetString_>(callGetString);
	}
	void* callSetString = GetProcAddress(hDLL, "SetString");
	if (callSetString != nullptr) {
		CallSetString = reinterpret_cast<CallSetString_>(callSetString);
	}
}

bool ProtoPlugin::Init(char* nwnxHome) {
	try {
		if (CallInit == nullptr) {
			return true;
		}

		return CallInit(nwnxHome);
	} catch (const std::exception& exception) {
		logger->Warn("Calling Init() failed: %s", exception.what());
	}

	return false;
}

void ProtoPlugin::GetFunctionClass(char* fClass) {
	try {
		if (CallGetFunctionClass == nullptr) {
			return;
		}

		CallGetFunctionClass(fClass);
	} catch (const std::exception& exception) {
		logger->Warn("Calling GetFunctionClass() failed: %s", exception.what());
	}
}

int ProtoPlugin::GetInt(char* sFunction, char* sParam1, int nParam2) {
	try {
		if (CallGetInt == nullptr) {
			return 0;
		}

		return CallGetInt(sFunction, sParam1, nParam2);
	} catch (const std::exception& exception) {
		logger->Warn("Calling GetInt() failed: %s", exception.what());
	}

	return 0;
}

void ProtoPlugin::SetInt(char* sFunction, char* sParam1, int nParam2, int nValue) {
	try {
		if (CallSetInt == nullptr) {
			return;
		}

		CallSetInt(sFunction, sParam1, nParam2, nValue);
	} catch (const std::exception& exception) {
		logger->Warn("Calling SetInt() failed: %s", exception.what());
	}
}

float ProtoPlugin::GetFloat(char* sFunction, char* sParam1, int nParam2) {
	try {
		if (CallGetFloat == nullptr) {
			return 0.0;
		}

		return CallGetFloat(sFunction, sParam1, nParam2);
	} catch (const std::exception& exception) {
		logger->Warn("Calling GetFloat() failed: %s", exception.what());
	}

	return 0.0;
}

void ProtoPlugin::SetFloat(char* sFunction, char* sParam1, int nParam2, float fValue) {
	try {
		if (CallSetFloat == nullptr) {
			return;
		}

		CallSetFloat(sFunction, sParam1, nParam2, fValue);
	} catch (const std::exception& exception) {
		logger->Warn("Calling SetFloat() failed: %s", exception.what());
	}
}

char* ProtoPlugin::GetString(char* sFunction, char* sParam1, int nParam2) {
	try {
		if (CallGetString == nullptr) {
			return nullptr;
		}

		return CallGetString(sFunction, sParam1, nParam2);
	} catch (const std::exception& exception) {
		logger->Warn("Calling GetString() failed: %s", exception.what());
	}

	return nullptr;
}

void ProtoPlugin::SetString(char* sFunction, char* sParam1, int nParam2, char* sValue) {
	try {
		if (CallSetString == nullptr) {
			return;
		}

		CallSetString(sFunction, sParam1, nParam2, sValue);
	} catch (const std::exception& exception) {
		logger->Warn("Calling SetFloat() failed: %s", exception.what());
	}
}
