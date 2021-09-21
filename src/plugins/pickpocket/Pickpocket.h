#pragma once
#define _CRT_SECURE_NO_DEPRECATE
#define DLLEXPORT extern "C" __declspec(dllexport)
// Remembing to define _CRT_RAND_S prior
// to inclusion statement
#define _CRT_RAND_S

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>

#ifdef WIN32
#include <windows.h>
#endif

#include "../plugin.h"
#include "../../misc/log.h"
#include "../../misc/ini.h"


class CPickpocket : public Plugin
{
public:
	CPickpocket(void);
	virtual ~CPickpocket(void);
	
public:
	bool Init(char* nwnxhome);
	int GetInt(char* sFunction, char* sParam1, int nParam2);
	void SetInt(char* sFunction, char* sParam1, int nParam2, int nValue);
	float GetFloat(char* sFunction, char* sParam1, int nParam2);
	void SetFloat(char* sFunction, char* sParam1, int nParam2, float fValue);
	char* GetString(char* sFunction, char* sParam1, int nParam2);
	void SetString(char* sFunction, char* sParam1, int nParam2, char* sValue);
	void GetFunctionClass(char* fClass);

public:
	LogNWNX* logger;
	SimpleIniConfig* config;

	std::string execScript;
};
