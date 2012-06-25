#include <libbase/err.hpp>
#include <libbase/str.hpp>

namespace Base {

	ustring ErrAsStr(DWORD err, PCWSTR lib) {
		HMODULE mod = nullptr;
		if (err && lib) {
			mod = ::LoadLibraryExW(lib, nullptr, DONT_RESOLVE_DLL_REFERENCES); //LOAD_LIBRARY_AS_DATAFILE
		}
		PWSTR buf = nullptr;
		::FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS | ((mod) ? FORMAT_MESSAGE_FROM_HMODULE : FORMAT_MESSAGE_FROM_SYSTEM), mod, err,
		                 MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), //GetSystemDefaultLangID(),
		                 (PWSTR)&buf, 0, nullptr);

		if (mod)
			::FreeLibrary(mod);

		if (!buf) {
			if (lib) {
				return ErrAsStr(err);
			} else {
				WCHAR out[MAX_PATH];
				_snwprintf(out, sizeofa(out), L"[0x%x] Unknown error", err);
				return ustring(out);
			}
		}

		WCHAR out[MAX_PATH_LEN];
		_snwprintf(out, sizeofa(out), L"[0x%x] %s", err, buf);
		::LocalFree(buf);

		ustring ret(out);
		return ret.erase(ret.size() - 2);
	}

	ustring ErrAsStrWmi(HRESULT err) {
		return ErrAsStr(err, L"wmiutils.dll");
	}

}
