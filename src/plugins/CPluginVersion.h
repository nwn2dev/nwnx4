#ifndef NWNX4_CPLUGINVERSION_H
#define NWNX4_CPLUGINVERSION_H

#include <windows.h>
#include "../misc/log.h"
#include "CPlugin.h"

// ABI Version: 1.0
struct CPluginInitInfoV1_0 {
	const char* dllPath;
	const char* nwnInstallHomePath;
	const char* nwnxHomePath;
};

typedef void* (WINAPI* CallNewV1_0_)(CPluginInitInfoV1_0*);

class CPluginVersion: public CPlugin {
public:
	CPluginVersion(HINSTANCE hDLL, CPluginInitInfoV1_0* initInfo);
private:
	void New(CPluginInitInfoV1_0* initInfo);

	CallNewV1_0_ CallNewV1_0;
};

#endif //NWNX4_CPLUGINVERSION_H
