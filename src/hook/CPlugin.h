#ifndef NWNX4_CPLUGIN_H
#define NWNX4_CPLUGIN_H

#include <functional>
#include <string>
#include <windows.h>

class CPlugin {
public:
	struct InitInfo {
		const char* dll_path;
		const char* nwnx_user_path;
		const char* nwn2_install_path;
		const char* nwn2_home_path;
		const char* nwn2_module_path;
		const char* nwnx_install_path;
	};

	CPlugin(HINSTANCE hDLL, const InitInfo& initInfo);
	~CPlugin();

	bool GetIsLoaded() const { return m_instancePtr != nullptr; }

	inline std::string GetID() const { return std::string(m_dll.getID(m_instancePtr)); }
	inline std::string GetInfo() const
	{
		if (m_dll.getInfo)
			return std::string(m_dll.getInfo());
		return GetID();
	}
	inline std::string GetVersion() const
	{
		if (m_dll.getVersion)
			return std::string(m_dll.getVersion());
		return "unknown";
	}

	inline int GetInt(const char* sFunction, const char* sParam1, int nParam2)
	{
		if (m_dll.getInt) {
			return m_dll.getInt(m_instancePtr, sFunction, sParam1, nParam2);
		}
		return 0;
	}
	inline void SetInt(const char* sFunction, const char* sParam1, int nParam2, int nValue)
	{
		if (m_dll.setInt) {
			m_dll.setInt(m_instancePtr, sFunction, sParam1, nParam2, nValue);
		}
	}
	inline float GetFloat(const char* sFunction, const char* sParam1, int nParam2)
	{
		if (m_dll.getFloat) {
			return m_dll.getFloat(m_instancePtr, sFunction, sParam1, nParam2);
		}
		return 0.0;
	}
	inline void SetFloat(const char* sFunction, const char* sParam1, int nParam2, float fValue)
	{
		if (m_dll.setFloat) {
			m_dll.setFloat(m_instancePtr, sFunction, sParam1, nParam2, fValue);
		}
	}
	inline void GetString(
	    const char* sFunction, const char* sParam1, int nParam2, char* result, size_t resultSize)
	{
		if (m_dll.getString) {
			return m_dll.getString(m_instancePtr, sFunction, sParam1, nParam2, result, resultSize);
		}
	}
	inline void SetString(const char* sFunction, const char* sParam1, int nParam2, char* sValue)
	{
		if (m_dll.setString) {
			m_dll.setString(m_instancePtr, sFunction, sParam1, nParam2, sValue);
		}
	}
	inline size_t GetGFFSize(const char* sVarName)
	{
		if (m_dll.getGFFSize) {
			return m_dll.getGFFSize(m_instancePtr, sVarName);
		}
		return 0;
	}
	inline void GetGFF(const char* sVarName, uint8_t* result, size_t resultSize)
	{
		if (m_dll.getGFF) {
			m_dll.getGFF(m_instancePtr, sVarName, result, resultSize);
		}
	}
	inline void SetGFF(const char* sVarName, const uint8_t* gffData, size_t gffDataSize)
	{
		if (m_dll.setGFF) {
			m_dll.setGFF(m_instancePtr, sVarName, gffData, gffDataSize);
		}
	}

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
	typedef size_t      (__cdecl GetGFFSizeFn)  (void* cplugin, const char* sVarName);
	typedef void        (__cdecl GetGFFFn)      (void* cplugin, const char* sVarName, uint8_t* result, size_t resultSize);
	typedef void        (__cdecl SetGFFFn)      (void* cplugin, const char* sVarName, const uint8_t* gffData, size_t gffDataSize);
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
		std::function<GetGFFSizeFn> getGFFSize;
		std::function<GetGFFFn> getGFF;
		std::function<SetGFFFn> setGFF;
	} m_dll;
};

#endif // NWNX4_CPLUGIN_H
