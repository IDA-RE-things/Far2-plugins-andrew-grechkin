#ifndef _LIBBASE_PATH_HPP_
#define _LIBBASE_PATH_HPP_

#include <libbase/std.hpp>
#include <libbase/str.hpp>

namespace Base {

	ustring Canonicalize(PCWSTR path);
	inline ustring Canonicalize(const ustring &path) {
		return Canonicalize(path.c_str());
	}

	ustring Expand(PCWSTR path);
	inline ustring Expand(const ustring &path) {
		return Expand(path.c_str());
	}

	ustring UnExpand(PCWSTR path);
	inline ustring UnExpand(const ustring &path) {
		return UnExpand(path.c_str());
	}

	ustring MakeGoodPath(PCWSTR path);
	inline ustring MakeGoodPath(const ustring path) {
		return MakeGoodPath(path.c_str());
	}

	ustring get_fullpath(PCWSTR path);
	inline ustring get_fullpath(const ustring &path) {
		return get_fullpath(path.c_str());
	}

	ustring PathNice(PCWSTR path);
	inline ustring PathNice(const ustring &path) {
		return Canonicalize(Expand(path.c_str()));
	}

	ustring path_compact(PCWSTR path, size_t size);
	inline ustring path_compact(const ustring &path, size_t size) {
		return path_compact(path.c_str(), size);
	}

	ustring& ensure_end_path_separator(ustring &path, WCHAR sep = PATH_SEPARATOR_C);

	ustring& ensure_no_end_path_separator(ustring &path);

	ustring Secure(PCWSTR path);
	inline ustring Secure(const ustring &path) {
		return Secure(path.c_str());
	}

	ustring Validate(PCWSTR path);
	inline ustring Validate(const ustring &path) {
		return Validate(path.c_str());
	}

	ustring SlashAdd(const ustring &path, WCHAR sep = PATH_SEPARATOR_C);
	ustring SlashDel(const ustring &path);

	bool IsPathUnix(PCWSTR path);
	inline bool IsPathUnix(const ustring &path) {
		return IsPathUnix(path.c_str());
	}

	ustring ExtractFile(const ustring &path);

	ustring ExtractPath(const ustring &path);

	ustring GetSpecialPath(int csidl, bool create = true);

	bool MaskMatch(PCWSTR path, PCWSTR mask, DWORD flags = 0);

	ustring MakePath(PCWSTR path, PCWSTR name);
	inline ustring MakePath(const ustring &path, const ustring &name) {
		return MakePath(path.c_str(), name.c_str());
	}

	ustring PathUnix(PCWSTR path);
	inline ustring PathUnix(const ustring &path) {
		return PathUnix(path.c_str());
	}

	ustring PathWin(PCWSTR path);
	inline ustring PathWin(const ustring &path) {
		return PathWin(path.c_str());
	}

	ustring GetWorkDirectory();

	bool SetWorkDirectory(PCWSTR path);
	inline bool SetWorkDirectory(const ustring &path) {
		return SetWorkDirectory(path.c_str());
	}

	ustring get_root(PCWSTR path);
	inline ustring get_root(const ustring &path) {
		return get_root(path.c_str());
	}

	bool is_path_mask(PCWSTR path);
	inline bool is_path_mask(const ustring &path) {
		return is_path_mask(path.c_str());
	}

	bool is_valid_filename(PCWSTR name);
	inline bool is_valid_filename(const ustring &name) {
		return is_valid_filename(name.c_str());
	}

	ustring remove_path_prefix(const ustring &path, PCWSTR pref = PATH_PREFIX_NT);

	ustring ensure_path_prefix(const ustring &path, PCWSTR pref = PATH_PREFIX_NT);

	ustring get_path_from_mask(const ustring &mask);

	ustring TempDir();

	ustring TempFile(PCWSTR path);
	inline ustring TempFile(const ustring &path) {
		return TempFile(path.c_str());
	}

	bool substr_match(const ustring& str, size_t pos, PCWSTR mstr);

	void locate_path_root(const ustring& path, size_t& path_root_len, bool& is_unc_path);

	ustring extract_file_name(const ustring& path);

	///===================================================================================== SysPath
	namespace SysPath {
		ustring Winnt();
		ustring Sys32();
		ustring SysNative();
		ustring InetSrv();
		ustring Dns();
		ustring Temp();

		ustring Users();
	}

	///====================================================================================== SysApp
	namespace SysApp {
		ustring appcmd();
	//	ustring dnscmd();
	}

}

#endif
