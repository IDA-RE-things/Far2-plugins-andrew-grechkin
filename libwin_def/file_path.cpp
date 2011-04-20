#include "win_def.h"

#include <wchar.h>

extern "C" {
	BOOL WINAPI SHGetSpecialFolderPathW(HWND, LPWSTR, int, BOOL);
	LWSTDAPI PathMatchSpecExW(PCWSTR pszFile, PCWSTR pszSpec, DWORD dwFlags);
}

bool			MaskMatch(PCWSTR path, PCWSTR mask, DWORD flags) {
	return ::PathMatchSpecExW(path, mask, flags) == S_OK;
}

AutoUTF	MakePath(PCWSTR path, PCWSTR name) {
	AutoUTF	Result(PathNice(SlashAdd(path)));
	return AddWordEx(Result, name, PATH_SEPARATOR);
}

AutoUTF	Canonicalize(PCWSTR path) {
	WCHAR ret[MAX_PATH_LEN];
	return	::PathCanonicalizeW(ret, path) ?  AutoUTF(ret) : AutoUTF();
}

AutoUTF	Expand(PCWSTR path) {
	WCHAR ret[MAX_PATH_LEN];
	return ::ExpandEnvironmentStringsW(path, ret, sizeofa(ret)) ? AutoUTF(ret) : AutoUTF();
}

AutoUTF	UnExpand(PCWSTR path) {
//	bool	unx = IsPathUnix(path);
//	if (unx)
//		Result.PathWin();
	WCHAR ret[MAX_PATH_LEN];
	return ::PathUnExpandEnvStringsW(path, ret, sizeofa(ret)) ? AutoUTF(ret) : AutoUTF();
//	return	unx ? Result.PathUnix() : Result;
//	return	AutoUTF();
}

AutoUTF get_fullpath(PCWSTR path) {
	WCHAR buf[MAX_PATH_LEN];
	::GetFullPathNameW(path, sizeofa(buf), buf, nullptr);
	return ensure_path_prefix(AutoUTF(buf));
}

AutoUTF MakeGoodPath(PCWSTR path) {
	return get_fullpath(Expand(path));
}

AutoUTF	PathNice(PCWSTR path) {
	return	Canonicalize(Expand(path));
}

AutoUTF	path_compact(PCWSTR path, size_t size) {
	auto_array<WCHAR> ret(MAX_PATH_LEN);
	if (::PathCompactPathExW(ret, path, size, 0))
		return	AutoUTF(ret);
	return AutoUTF();
}

AutoUTF& ensure_end_path_separator(AutoUTF &path, WCHAR sep) {
	if (!Find(PATH_SEPARATORS, path[path.size() - 1])) {
		path += sep;
	}
	return path;
}

AutoUTF& ensure_no_end_path_separator(AutoUTF &path) {
	if (Find(PATH_SEPARATORS, path[path.size() - 1])) {
		path.erase(path.size() - 1);
	}
	return path;
}

AutoUTF	SlashAdd(const AutoUTF &path, WCHAR sep) {
	AutoUTF	ret(path);
	return	ensure_end_path_separator(ret, sep);
}

AutoUTF	SlashDel(const AutoUTF &path) {
	AutoUTF	ret(path);
	return	ensure_no_end_path_separator(ret);
}

bool				IsPathUnix(PCWSTR path) {
	return	Find(path, L'/') != nullptr;
}

AutoUTF	ExtractFile(const AutoUTF &path) {
	return	path.substr(path.find_last_of(PATH_SEPARATORS));
}

AutoUTF	ExtractPath(const AutoUTF &path) {
	return	path.substr(0, path.find_last_of(PATH_SEPARATORS));
}

AutoUTF	PathUnix(PCWSTR path) {
	AutoUTF	Result(path);
	return	ReplaceAll(Result, L"\\", L"/");
}

AutoUTF	PathWin(PCWSTR path) {
	AutoUTF	Result(path);
	return	ReplaceAll(Result, L"/", L"\\");
}

AutoUTF	Secure(PCWSTR path) {
	AutoUTF	Result(path);
	ReplaceAll(Result, L"..", L"");
	ReplaceAll(Result, L"%", L"");
	return	Validate(Result);
}

AutoUTF	Validate(PCWSTR path) {
	AutoUTF	Result(Canonicalize(Expand(path)));
	ReplaceAll(Result, L"...", L"");
	ReplaceAll(Result, L"\\\\", L"\\");
	if (Result == L"\\")
		Result.clear();
	return	Result;
}

AutoUTF	GetSpecialPath(int csidl, bool create) {
	auto_array<WCHAR> ret(MAX_PATH_LEN);
	if (::SHGetSpecialFolderPathW(nullptr, ret, csidl, create))
		return	AutoUTF(ret);
	return	AutoUTF();
}

AutoUTF	GetWorkDirectory() {
	WCHAR	Result[::GetCurrentDirectoryW(0, nullptr)];
	::GetCurrentDirectoryW(sizeofa(Result), Result);
	return	Result;
}
bool	SetWorkDirectory(PCWSTR path) {
	return Empty(path) ? false : ::SetCurrentDirectoryW(path);
}

AutoUTF	TempDir() {
	WCHAR	buf[MAX_PATH];
	buf[0] = 0;
	::GetTempPathW(sizeofa(buf), buf);
	return	AutoUTF(buf);
}

AutoUTF	TempFile(PCWSTR path) {
	WCHAR	buf[MAX_PATH];
	WCHAR	pid[32];
	buf[0] = 0;
	Num2Str(pid, ::GetCurrentProcessId());
	::GetTempFileNameW(path, pid, 0, buf);
	return	buf;
}

bool is_path_mask(PCWSTR path) {
	PCWSTR pos = nullptr;
	if ((pos = Find(path, L'*')) || (pos = Find(path, L'?')))
		return	pos != (path + 2);
	return false;
}

bool is_valid_filename(PCWSTR name) {
	return	!(Eq(name, L".") || Eq(name, L"..") || Eq(name, L"..."));
}

AutoUTF remove_path_prefix(const AutoUTF &path, PCWSTR pref) {
	if (!path.empty() && path.find(pref) == 0)
		return path.substr(Len(pref));
	return	path;
}

AutoUTF ensure_path_prefix(const AutoUTF &path, PCWSTR pref) {
	if (path.size() > 1 &&
		(path[0] != L'\\' || path[1] != L'\\') &&
		path.find(pref) == AutoUTF::npos)
		return AutoUTF(pref) += path;
	return	path;
}

AutoUTF	get_path_from_mask(const AutoUTF &mask) {
	AutoUTF	tmp(remove_path_prefix(mask));
	AutoUTF::size_type pos = std::min(tmp.find_first_of(L"?*"), tmp.find_last_of(L"\\/"));
	if (pos != AutoUTF::npos) {
		tmp.erase(pos);
	}
	return	ensure_path_prefix(tmp);
}

AutoUTF get_root(PCWSTR path) {
	WCHAR ret[MAX_PATH];
	if (::GetVolumePathNameW(path, ret, sizeofa(ret)))
		return AutoUTF(ret);
	return AutoUTF(path);
}

///========================================================================================= SysPath
namespace	SysPath {
	AutoUTF	Winnt() {
		const int CSIDL_WINDOWS = 0x0024;
		AutoUTF ret(GetSpecialPath(CSIDL_WINDOWS));
		return	ensure_end_path_separator(ret);
	}
	AutoUTF	Sys32() {
		const int CSIDL_SYSTEM = 0x0025;
		AutoUTF ret(GetSpecialPath(CSIDL_SYSTEM));
		return	ensure_end_path_separator(ret);
	}

	AutoUTF	SysNative() {
		return	PathNice(IsWOW64() ? L"%SystemRoot%\\sysnative\\" : L"%SystemRoot%\\system32\\");
	}

	AutoUTF	InetSrv() {
		return	MakePath(Sys32().c_str(), L"inetsrv\\");
	}

	AutoUTF	Dns() {
		return	MakePath(SysNative().c_str(), L"dns\\");
	}

	AutoUTF	Temp() {
		return	PathNice(L"%TEMP%\\");
	}

	AutoUTF	Users() {
		AutoUTF	ret = PathNice(L"%PUBLIC%\\..\\");
		return	(ret.empty() || (ret == L"\\")) ? PathNice(L"%ALLUSERSPROFILE%\\..\\") : ret;
	}
}

///========================================================================================== SysApp
namespace	SysApp {
	AutoUTF	appcmd() {
		return	SysPath::InetSrv() + L"appcmd.exe ";
	}
}

///=================================================================================================
bool substr_match(const AutoUTF& str, size_t pos, PCWSTR mstr) {
	size_t mstr_len = Len(mstr);
	if ((pos > str.size()) || (pos + mstr_len > str.size())) {
		return false;
	}
	return wmemcmp(str.c_str() + pos, mstr, mstr_len) == 0;
}

void locate_path_root(const AutoUTF& path, size_t& path_root_len, bool& is_unc_path) {
	unsigned prefix_len = 0;
	is_unc_path = false;
	if (substr_match(path, 0, L"\\\\")) {
		if (substr_match(path, 2, L"?\\UNC\\")) {
			prefix_len = 8;
			is_unc_path = true;
		} else if (substr_match(path, 2, L"?\\") || substr_match(path, 2, L".\\")) {
			prefix_len = 4;
		} else {
			prefix_len = 2;
			is_unc_path = true;
		}
	}
	if ((prefix_len == 0) && !substr_match(path, 1, L":\\")) {
		path_root_len = 0;
	} else {
		AutoUTF::size_type p = path.find(L'\\', prefix_len);
		if (p == AutoUTF::npos) {
			p = path.size();
		}
		if (is_unc_path) {
			p = path.find(L'\\', p + 1);
			if (p == AutoUTF::npos) {
				p = path.size();
			}
		}
		path_root_len = p;
	}
}

AutoUTF extract_file_name(const AutoUTF& path) {
	size_t pos = path.rfind(L"\\");
	if (pos == AutoUTF::npos) {
		pos = 0;
	} else {
		pos++;
	}
	size_t path_root_len;
	bool is_unc_path;
	locate_path_root(path, path_root_len, is_unc_path);
	if ((pos <= path_root_len) && (path_root_len != 0))
		return AutoUTF();
	else
		return path.substr(pos);
}
