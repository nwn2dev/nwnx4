#ifndef NWNX4_CPLUGINVERSION_H
#define NWNX4_CPLUGINVERSION_H

#include <windows.h>
#include "../misc/log.h"
#include "CPlugin.h"

// ABI Version: 1
struct CPluginInitInfoV1 {
	const char* dllPath;
	const char* nwnInstallHomePath;
	const char* nwnxHomePath;
};

typedef void* (WINAPI* CallNewV1_)(CPluginInitInfoV1*);

class CPluginVersion: public CPlugin {
public:
	CPluginVersion(HINSTANCE hDLL, CPluginInitInfoV1* initInfo);
private:
	void New(CPluginInitInfoV1* initInfo);

	CallNewV1_ CallNewV1;
};

#endif //NWNX4_CPLUGINVERSION_H
