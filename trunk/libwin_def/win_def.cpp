#include "win_def.h"

///====================================================================================== WinSysInfo
WinSysInfo::WinSysInfo() {
	if (IsWOW64())
		::GetNativeSystemInfo((LPSYSTEM_INFO)this);
	else
		::GetSystemInfo((LPSYSTEM_INFO)this);
}

size_t WinSysInfo::Uptime(size_t /*del*/) {
	return 0;//::GetTickCount64() / del;
}

///===================================================================================== Binary type
NamedValues<DWORD> BinaryType[] = {
	{L"UNKNOWN", (DWORD)-1},
	{L"x32", SCS_32BIT_BINARY},
	{L"x64", SCS_64BIT_BINARY},
	{L"dos", SCS_DOS_BINARY},
	{L"os2x16", SCS_OS216_BINARY},
	{L"pif", SCS_PIF_BINARY},
	{L"posix", SCS_POSIX_BINARY },
	{L"x16", SCS_WOW_BINARY},
};

///========================================================================================= WinProc
/// Обертка хэндла процесса
ustring WinProcess::User() {
	DWORD size = MAX_PATH;
	WCHAR buf[size];
	::GetUserNameW(buf, &size);
	return buf;
}

ustring WinProcess::FullPath() {
	WCHAR tmp[MAX_PATH];
	size_t sz = ::GetModuleFileNameW(nullptr, tmp, sizeofa(tmp));
	if (sz > sizeofa(tmp)) {
		WCHAR Result[sz];
		::GetModuleFileNameW(nullptr, Result, sizeofa(Result));
		return Result;
	}
	return tmp;
}
