#include "CPluginVersion.h"

extern LogNWNX* logger;

CPluginVersion::CPluginVersion(HINSTANCE hDLL, CPluginInitInfoV1 *initInfo): CPlugin(hDLL) {
	void* callNew = GetProcAddress(hDLL, "NWNXCPlugin_New");
	if (callNew != nullptr) {
		CallNewV1 = reinterpret_cast<CallNewV1_>(callNew);
	}

	// Create new version of the C plugin (V1)
	New(initInfo);
}

void CPluginVersion::New(CPluginInitInfoV1* initInfo) {
	try {
		if (CallNewV1 == nullptr) {
			return;
		}

		plugin = CallNewV1(initInfo);

		if (plugin == nullptr) {
			logger->Warn("  * Invalid plugin returned from New() call");
		}
	} catch (const std::exception& exception) {
		logger->Warn("  * Calling New() failed: %s", exception.what());
	}
}
