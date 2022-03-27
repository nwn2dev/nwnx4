#include "CPluginVersion.h"

extern LogNWNX* logger;

CPluginVersion::CPluginVersion(HINSTANCE hDLL, CPluginInitInfoV1_0 *initInfo): CPlugin(hDLL) {
	void* callNew = GetProcAddress(hDLL, "NWNXCPlugin_New");
	if (callNew != nullptr) {
		CallNewV1_0 = reinterpret_cast<CallNewV1_0_>(callNew);
	}

	// Create new version of the C plugin (V1.0)
	New(initInfo);
}

void CPluginVersion::New(CPluginInitInfoV1_0* initInfo) {
	try {
		if (CallNewV1_0 == nullptr) {
			return;
		}

		plugin = CallNewV1_0(initInfo);
		logger->Info("  * C plugin established at %p", plugin);
	} catch (const std::exception& exception) {
		logger->Warn("  * Calling New() failed: %s", exception.what());
	}
}
