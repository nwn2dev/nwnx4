#pragma once
#define _CRT_SECURE_NO_DEPRECATE
#define DLLEXPORT extern "C" __declspec(dllexport)
// Remembing to define _CRT_RAND_S prior
// to inclusion statement
#define _CRT_RAND_S

#include <memory>

#ifdef WIN32
#include <windows.h>
#endif

#include "../plugin.h"
#include "../../misc/log.h"
#include "../../misc/ini.h"


class CPickpocket : public Plugin
{
public:
	static constexpr char FunctionClass[] = "PICK";

public:
	CPickpocket();
	~CPickpocket();
	
	bool Init(char* nwnxhome) final;
	int GetInt(char* sFunction, char* sParam1, int nParam2) final;
	void SetInt(char* sFunction, char* sParam1, int nParam2, int nValue) final;
	float GetFloat(char* sFunction, char* sParam1, int nParam2) final;
	void SetFloat(char* sFunction, char* sParam1, int nParam2, float fValue) final;
	char* GetString(char* sFunction, char* sParam1, int nParam2) final;
	void SetString(char* sFunction, char* sParam1, int nParam2, char* sValue) final;
	void GetFunctionClass(char* fClass) final;

private:
	std::unique_ptr<LogNWNX> logger;
	std::unique_ptr<SimpleIniConfig> config;

	std::string execScript;
};
