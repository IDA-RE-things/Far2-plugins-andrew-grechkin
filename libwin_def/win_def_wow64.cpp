#include "win_def.h"

///=========================================================================================== Win64
/// Функции работы с WOW64
namespace	Win64 {
bool		WowDisable(PVOID &oldValue) {
	typedef BOOL (WINAPI * PFN_WOW64)(PVOID*);
	PFN_WOW64 func = (PFN_WOW64) ::GetProcAddress(::GetModuleHandleW(L"kernel32.dll"), "Wow64DisableWow64FsRedirection");
	if (func != NULL) {
		return	(func)(&oldValue) != 0;
	}
	return	false;
}
bool		WowEnable(PVOID &oldValue) {
	typedef BOOL (WINAPI * PFN_WOW64)(PVOID*);
	PFN_WOW64 func = (PFN_WOW64) ::GetProcAddress(::GetModuleHandleW(L"kernel32.dll"), "Wow64RevertWow64FsRedirection");
	if (func != NULL) {
		return	(func)(&oldValue) != 0;
	}
	return	false;
}
bool		IsWOW64() {
	typedef BOOL (WINAPI * PFN_ISWOW64)(HANDLE, PBOOL);
	PFN_ISWOW64 func = (PFN_ISWOW64) ::GetProcAddress(::GetModuleHandleW(L"kernel32"), "IsWow64Process");
	if (func != NULL) {
		BOOL	Result = false;
		if (func(::GetCurrentProcess(), &Result) != 0) {
			return	Result != 0;
		}
	}
	return	false;
}
}

