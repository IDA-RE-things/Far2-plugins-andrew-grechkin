#include "win_def.h"


///=========================================================================================== Win64
/// Функции работы с WOW64
namespace	Win64 {
bool		WowDisable(PVOID &oldValue) {
	typedef BOOL (WINAPI * PFUNC)(PVOID*);
	PFUNC	func = (PFUNC) ::GetProcAddress(::GetModuleHandleW(L"kernel32.dll"), "Wow64DisableWow64FsRedirection");
	if (func != nullptr) {
		return	(func)(&oldValue) != 0;
	}
	return	false;
}
bool		WowEnable(PVOID &oldValue) {
	typedef BOOL (WINAPI * PFUNC)(PVOID*);
	PFUNC	func = (PFUNC) ::GetProcAddress(::GetModuleHandleW(L"kernel32.dll"), "Wow64RevertWow64FsRedirection");
	if (func != nullptr) {
		return	(func)(&oldValue) != 0;
	}
	return	false;
}
bool		IsWOW64() {
	typedef BOOL (WINAPI * PFUNC)(HANDLE, PBOOL);
	PFUNC	func = (PFUNC) ::GetProcAddress(::GetModuleHandleW(L"kernel32"), "IsWow64Process");
	if (func != nullptr) {
		BOOL	Result = false;
		if (func(::GetCurrentProcess(), &Result) != 0) {
			return	Result != 0;
		}
	}
	return	false;
}
}
