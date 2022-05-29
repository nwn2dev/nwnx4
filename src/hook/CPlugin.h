#ifndef NWNX4_CPLUGIN_H
#define NWNX4_CPLUGIN_H

#include <windows.h>
#include <functional>
#include <string>

class CPlugin {
public:
	struct InitInfo {
		const char* dll_path;
		const char* nwnx_path;
		const char* nwn2_install_path;
		const char* nwn2_home_path;
		const char* nwn2_module_path;
	};

	CPlugin(HINSTANCE hDLL, const InitInfo& initInfo);
	~CPlugin();

	bool GetIsLoaded() const {
		return m_instancePtr != nullptr;
	}

	std::string GetID() const;
	std::string GetInfo() const;
	std::string GetVersion() const;

	int GetInt(const char* sFunction, const char* sParam1, int nParam2);
	void SetInt(const char* sFunction, const char* sParam1, int nParam2, int nValue);
	float GetFloat(const char* sFunction, const char* sParam1, int nParam2);
	void SetFloat(const char* sFunction, const char* sParam1, int nParam2, float fValue);
	void GetString(const char* sFunction, const char* sParam1, int nParam2, char* result, size_t resultSize);
	void SetString(const char* sFunction, const char* sParam1, int nParam2, char* sValue);
private:
	void* m_instancePtr = nullptr;

	// clang-format off
	typedef void*       (__cdecl NewPluginFn)   (InitInfo info);
	typedef void        (__cdecl DeletePluginFn)(void* cplugin);
	typedef const char* (__cdecl GetInfoFn)     ();
	typedef const char* (__cdecl GetVersionFn)  ();
	typedef const char* (__cdecl GetIDFn)       (void* cplugin);
	typedef int32_t     (__cdecl GetIntFn)      (void* cplugin, const char* sFunction, const char* sParam1, int32_t nParam2);
	typedef void        (__cdecl SetIntFn)      (void* cplugin, const char* sFunction, const char* sParam1, int32_t nParam2, int32_t nValue);
	typedef float       (__cdecl GetFloatFn)    (void* cplugin, const char* sFunction, const char* sParam1, int32_t nParam2);
	typedef void        (__cdecl SetFloatFn)    (void* cplugin, const char* sFunction, const char* sParam1, int32_t nParam2, float fValue);
	typedef void        (__cdecl GetStringFn)   (void* cplugin, const char* sFunction, const char* sParam1, int32_t nParam2, char* result, size_t resultSize);
	typedef void        (__cdecl SetStringFn)   (void* cplugin, const char* sFunction, const char* sParam1, int32_t nParam2, const char* sValue);
	// clang-format on
	struct {
		uint32_t abiVersion;
		std::function<NewPluginFn> newPlugin;
		std::function<DeletePluginFn> deletePlugin;
		std::function<GetInfoFn> getInfo;
		std::function<GetVersionFn> getVersion;
		std::function<GetIDFn> getID;
		std::function<GetIntFn> getInt;
		std::function<SetIntFn> setInt;
		std::function<GetFloatFn> getFloat;
		std::function<SetFloatFn> setFloat;
		std::function<GetStringFn> getString;
		std::function<SetStringFn> setString;
	} m_dll;
};

#endif //NWNX4_CPLUGIN_H
