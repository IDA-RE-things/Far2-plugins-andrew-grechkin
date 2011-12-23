#include "win_def.h"

namespace {
	///================================================================================= shlwapi_dll
	struct shlwapi_dll {
		typedef HRESULT (WINAPI *FPathMatchSpecExW)(PCWSTR, PCWSTR, DWORD);
		typedef WINBOOL (WINAPI *FPathCanonicalizeW)(LPWSTR ,LPCWSTR);
		typedef WINBOOL (WINAPI *FPathUnExpandEnvStringsW)(LPCWSTR, LPWSTR, UINT);
		typedef WINBOOL (WINAPI *FPathCompactPathExW)(LPWSTR, LPCWSTR, UINT, DWORD);
		typedef WINBOOL (WINAPI *FPathIsDirectoryEmptyW)(LPCWSTR);

		FPathMatchSpecExW PathMatchSpecExW;
		FPathCanonicalizeW PathCanonicalizeW;
		FPathUnExpandEnvStringsW PathUnExpandEnvStringsW;
		FPathCompactPathExW PathCompactPathExW;
		FPathIsDirectoryEmptyW PathIsDirectoryEmptyW;

		static shlwapi_dll & inst() {
			static shlwapi_dll ret;
			return ret;
		}

		~shlwapi_dll() {
			::FreeLibrary(m_hnd);
		}

	private:
		shlwapi_dll():
			m_hnd(::LoadLibraryW(L"shlwapi.dll")) {
			GET_DLL_FUNC(PathMatchSpecExW);
			GET_DLL_FUNC(PathCanonicalizeW);
			GET_DLL_FUNC(PathUnExpandEnvStringsW);
			GET_DLL_FUNC(PathCompactPathExW);
			GET_DLL_FUNC(PathIsDirectoryEmptyW);
//			PathMatchSpecExW = (FPathMatchSpecExW)get_function("PathMatchSpecExW");
//			PathCanonicalizeW = (FPathCanonicalizeW)get_function("PathCanonicalizeW");
//			PathUnExpandEnvStringsW = (FPathUnExpandEnvStringsW)get_function("PathUnExpandEnvStringsW");
//			PathCompactPathExW = (FPathCompactPathExW)get_function("PathCompactPathExW");
//			PathIsDirectoryEmptyW = (FPathIsDirectoryEmptyW)get_function("PathIsDirectoryEmptyW");
		}

		FARPROC get_function(PCSTR name) const {
			return ::GetProcAddress(m_hnd, name);
		}

		HMODULE m_hnd;
	};
}

namespace Directory {
	bool is_empty(PCWSTR path) {
		return shlwapi_dll::inst().PathIsDirectoryEmptyW(path);
	}
}

bool MaskMatch(PCWSTR path, PCWSTR mask, DWORD flags) {
	return shlwapi_dll::inst().PathMatchSpecExW(path, mask, flags) == S_OK;
}

ustring	Canonicalize(PCWSTR path) {
	WCHAR ret[MAX_PATH_LEN];
	return shlwapi_dll::inst().PathCanonicalizeW(ret, path) ?  ustring(ret) : ustring();
}

ustring	UnExpand(PCWSTR path) {
//	bool	unx = IsPathUnix(path);
//	if (unx)
//		Result.PathWin();
	WCHAR ret[MAX_PATH_LEN];
	return shlwapi_dll::inst().PathUnExpandEnvStringsW(path, ret, sizeofa(ret)) ? ustring(ret) : ustring();
//	return unx ? Result.PathUnix() : Result;
//	return ustring();
}

ustring	path_compact(PCWSTR path, size_t size) {
	auto_array<WCHAR> ret(MAX_PATH_LEN);
	return shlwapi_dll::inst().PathCompactPathExW(ret.data(), path, size, 0) ? ustring(ret.data()) : ustring();
}
