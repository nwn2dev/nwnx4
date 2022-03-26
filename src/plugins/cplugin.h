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

	char* GetPluginName();
	char* GetPluginVersion();

	int GetInt(char* sFunction, char* sParam1, int nParam2);
	void SetInt(char* sFunction, char* sParam1, int nParam2, int nValue);
	float GetFloat(char* sFunction, char* sParam1, int nParam2);
	void SetFloat(char* sFunction, char* sParam1, int nParam2, float fValue);
	char* GetString(char* sFunction, char* sParam1, int nParam2);
	void SetString(char* sFunction, char* sParam1, int nParam2, char* sValue);
protected:
	bool Delete();  // Called on destructor

	void* plugin = nullptr;

	char* pluginName = nullptr;
	char* pluginVersion = nullptr;

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

// ABI Version: 1
struct CPluginInitInfoV1 {
	const char* dllPath;
	const char* nwnInstallHomePath;
	const char* nwnxHomePath;
};

typedef void* (WINAPI* CallNewV1_)(CPluginInitInfoV1*);

class CPluginV1: public CPlugin {
public:
	CPluginV1(HINSTANCE hDLL, CPluginInitInfoV1* initInfo): CPlugin(hDLL) {
		void* callNew = GetProcAddress(hDLL, "NWNXCPlugin_New");
		if (callNew != nullptr) {
			CallNew = reinterpret_cast<CallNewV1_>(callNew);
		}

		// Create new version of the C plugin (V1)
		New(initInfo);
	};
private:
	void New(CPluginInitInfoV1* initInfo);

	CallNewV1_ CallNew;
};

#endif //NWNX4_CPLUGIN_H
