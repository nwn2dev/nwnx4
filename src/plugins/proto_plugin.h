#ifndef NWNX4_PROTO_PLUGIN_H
#define NWNX4_PROTO_PLUGIN_H

#include <windows.h>
#include "plugin.h"
#include "../misc/log.h"

typedef bool (WINAPI* CallInit_)(char* nwnxHome);
typedef void (WINAPI* CallGetFunctionClass_)(char* fClass);
typedef int (WINAPI* CallGetInt_)(char* sFunction, char* sParam1, int nParam2);
typedef void (WINAPI* CallSetInt_)(char* sFunction, char* sParam1, int nParam2, int nValue);
typedef float (WINAPI* CallGetFloat_)(char* sFunction, char* sParam1, int nParam2);
typedef void (WINAPI* CallSetFloat_)(char* sFunction, char* sParam1, int nParam2, float fValue);
typedef char* (WINAPI* CallGetString_)(char* sFunction, char* sParam1, int nParam2);
typedef void (WINAPI* CallSetString_)(char* sFunction, char* sParam1, int nParam2, char* sValue);

class ProtoPlugin: public Plugin {
public:
	ProtoPlugin(string pluginPath, HINSTANCE hDLL);
	bool Init(char* nwnxhome) override;
	void GetFunctionClass(char* fClass) override;
	int GetInt(char* sFunction, char* sParam1, int nParam2) override;
	void SetInt(char* sFunction, char* sParam1, int nParam2, int nValue) override;
	float GetFloat(char* sFunction, char* sParam1, int nParam2) override;
	void SetFloat(char* sFunction, char* sParam1, int nParam2, float fValue) override;
	char* GetString(char* sFunction, char* sParam1, int nParam2) override;
	void SetString(char* sFunction, char* sParam1, int nParam2, char* sValue) override;
private:
	CallInit_ CallInit;
	CallGetFunctionClass_ CallGetFunctionClass;
	CallGetInt_ CallGetInt;
	CallSetInt_ CallSetInt;
	CallGetFloat_ CallGetFloat;
	CallSetFloat_ CallSetFloat;
	CallGetString_ CallGetString;
	CallSetString_ CallSetString;
};

#endif //NWNX4_PROTO_PLUGIN_H
