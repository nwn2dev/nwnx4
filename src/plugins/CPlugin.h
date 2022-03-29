#ifndef NWNX4_CPLUGIN_H
#define NWNX4_CPLUGIN_H

#include <windows.h>
#include "../misc/log.h"

typedef char* (WINAPI* CallGetPluginName_)();
typedef char* (WINAPI* CallGetPluginVersion_)();
typedef bool (WINAPI* CallDelete_)(void*);
typedef int (WINAPI* CallGetInt_)(void*, char*, char*, int);
typedef void (WINAPI* CallSetInt_)(void*, char*, char*, int, int);
typedef float (WINAPI* CallGetFloat_)(void*, char*, char*, int);
typedef void (WINAPI* CallSetFloat_)(void*, char*, char*, int, float);
typedef char* (WINAPI* CallGetString_)(void*, char*, char*, int);
typedef void (WINAPI* CallSetString_)(void*, char*, char*, int, char*);

class CPlugin {
public:
	CPlugin() {}
	CPlugin(HINSTANCE hDLL);
	~CPlugin();

	std::string GetPluginId();
	bool Ready();
	bool Test(char* pluginName);

	int GetInt(char* sFunction, char* sParam1, int nParam2);
	void SetInt(char* sFunction, char* sParam1, int nParam2, int nValue);
	float GetFloat(char* sFunction, char* sParam1, int nParam2);
	void SetFloat(char* sFunction, char* sParam1, int nParam2, float fValue);
	char* GetString(char* sFunction, char* sParam1, int nParam2);
	void SetString(char* sFunction, char* sParam1, int nParam2, char* sValue);
protected:
	void* plugin = nullptr;

	char* pluginName_ = nullptr;
	char* pluginVersion_ = nullptr;

	bool Delete();  // Called on destructor
	void SetPluginName();
	void SetPluginVersion();

	CallGetPluginName_ CallGetPluginName;
	CallGetPluginVersion_ CallGetPluginVersion;

	CallDelete_ CallDelete;
	CallGetInt_ CallGetInt;
	CallSetInt_ CallSetInt;
	CallGetFloat_ CallGetFloat;
	CallSetFloat_ CallSetFloat;
	CallGetString_ CallGetString;
	CallSetString_ CallSetString;
};

#endif //NWNX4_CPLUGIN_H
