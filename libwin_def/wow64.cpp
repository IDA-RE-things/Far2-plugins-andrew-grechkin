#include "win_def.h"

namespace {
	///================================================================================ kernel32_dll
	struct kernel32_dll {
		typedef BOOL (WINAPI *FIsWow64Process)(HANDLE hProcess, PBOOL Wow64Process);
		typedef BOOL (WINAPI *FWow64DisableWow64FsRedirection)(PVOID * OldValue);
		typedef BOOL (WINAPI *FWow64RevertWow64FsRedirection)(PVOID OldValue);

		DEFINE_FUNC(IsWow64Process);
		DEFINE_FUNC(Wow64DisableWow64FsRedirection);
		DEFINE_FUNC(Wow64RevertWow64FsRedirection);

		static kernel32_dll & inst() {
			static kernel32_dll ret;
			return ret;
		}

		~kernel32_dll() {
			::FreeLibrary(m_hnd);
		}

	private:
		kernel32_dll():
			m_hnd(::LoadLibraryW(L"kernel32.dll")) {
			GET_DLL_FUNC(IsWow64Process);
			GET_DLL_FUNC(Wow64DisableWow64FsRedirection);
			GET_DLL_FUNC(Wow64RevertWow64FsRedirection);
		}

		FARPROC get_function(PCSTR name) const {
			return ::GetProcAddress(m_hnd, name);
		}

		HMODULE m_hnd;
	};
}

bool is_WOW64() {
	if (kernel32_dll::inst().IsWow64Process) {
		BOOL Result = false;
		if (kernel32_dll::inst().IsWow64Process(::GetCurrentProcess(), &Result) != 0) {
			return Result != 0;
		}
	}
	return false;
}

bool disable_WOW64(PVOID & oldValue) {
	if (kernel32_dll::inst().Wow64DisableWow64FsRedirection) {
		return kernel32_dll::inst().Wow64DisableWow64FsRedirection(&oldValue) != 0;
	}
	return false;
}

bool enable_WOW64(PVOID & oldValue) {
	if (kernel32_dll::inst().Wow64RevertWow64FsRedirection) {
		return kernel32_dll::inst().Wow64RevertWow64FsRedirection(&oldValue) != 0;
	}
	return false;
}
