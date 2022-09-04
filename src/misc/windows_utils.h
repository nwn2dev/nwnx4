#pragma once

#include <windows.h>
#include <utility>

inline std::pair<DWORD, const char*> GetLastErrorInfo(){
	char* lpMsgBuf;
	DWORD dw = GetLastError();
	FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_MAX_WIDTH_MASK | FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr, dw, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPSTR)&lpMsgBuf, 0, nullptr);
	return {dw, lpMsgBuf};
}