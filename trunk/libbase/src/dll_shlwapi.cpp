#include <libbase/std.hpp>
#include <libbase/memory.hpp>
#include <libbase/dll.hpp>

namespace {

	///================================================================================= shlwapi_dll
	struct shlwapi_dll: private Base::DynamicLibrary {
		typedef HRESULT (WINAPI *FPathMatchSpecExW)(PCWSTR, PCWSTR, DWORD);
		typedef WINBOOL (WINAPI *FPathCanonicalizeW)(PWSTR, PCWSTR);
		typedef WINBOOL (WINAPI *FPathCompactPathExW)(PWSTR, PCWSTR, UINT, DWORD);
		typedef WINBOOL (WINAPI *FPathIsDirectoryEmptyW)(PCWSTR);
		typedef WINBOOL (WINAPI *FPathUnExpandEnvStringsW)(PCWSTR, PWSTR, UINT);

		DEFINE_FUNC(PathMatchSpecExW);DEFINE_FUNC(PathCanonicalizeW);DEFINE_FUNC(PathCompactPathExW);DEFINE_FUNC(PathIsDirectoryEmptyW);DEFINE_FUNC(PathUnExpandEnvStringsW);

		static shlwapi_dll & inst() {
			static shlwapi_dll ret;
			return ret;
		}

	private:
		shlwapi_dll() :
			DynamicLibrary(L"shlwapi.dll") {
			GET_DLL_FUNC(PathMatchSpecExW);
			GET_DLL_FUNC(PathCanonicalizeW);
			GET_DLL_FUNC(PathCompactPathExW);
			GET_DLL_FUNC(PathIsDirectoryEmptyW);
			GET_DLL_FUNC(PathUnExpandEnvStringsW);
		}
	};
}

///================================================================================================
namespace Base {

	namespace Directory {
		bool is_empty(PCWSTR path) {
			return shlwapi_dll::inst().PathIsDirectoryEmptyW(path);
		}
	}

	bool MaskMatch(PCWSTR path, PCWSTR mask, DWORD flags) {
		return shlwapi_dll::inst().PathMatchSpecExW(path, mask, flags) == S_OK;
	}

	ustring Canonicalize(PCWSTR path) {
		WCHAR ret[MAX_PATH_LEN];
		return shlwapi_dll::inst().PathCanonicalizeW(ret, path) ? ustring(ret) : ustring();
	}

	ustring UnExpand(PCWSTR path) {
		//	bool	unx = IsPathUnix(path);
		//	if (unx)
		//		Result.PathWin();
		WCHAR ret[MAX_PATH_LEN];
		return shlwapi_dll::inst().PathUnExpandEnvStringsW(path, ret, sizeofa(ret)) ? ustring(ret) : ustring();
		//	return unx ? Result.PathUnix() : Result;
		//	return ustring();
	}

	ustring path_compact(PCWSTR path, size_t size) {
		auto_array<WCHAR> ret(MAX_PATH_LEN);
		return shlwapi_dll::inst().PathCompactPathExW(ret.data(), path, size, 0) ? ustring(ret.data()) : ustring();
	}

}
