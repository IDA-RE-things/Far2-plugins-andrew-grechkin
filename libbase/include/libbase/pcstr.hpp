#ifndef _LIBBASE_PCSTR_HPP_
#define _LIBBASE_PCSTR_HPP_

#include <libbase/std.hpp>

namespace Base {

	const DWORD NORM_STOP_ON_NULL = 0x10000000;

	inline bool is_str_empty(PCSTR str) {
		return str == nullptr || *str == 0;
	}

	inline bool is_str_empty(PCWSTR str) {
		return str == nullptr || *str == 0;
	}

	inline size_t get_str_len(PCSTR in) {
		return ::strlen(in);
	}

	inline size_t get_str_len(PCWSTR in) {
		return ::wcslen(in);
	}

	inline int compare_str(PCSTR in1, PCSTR in2) {
		return ::strcmp(in1, in2);
	}

	inline int compare_str(PCSTR in1, PCSTR in2, size_t n) {
		return ::strncmp(in1, in2, n);
	}

	inline int compare_str(PCWSTR in1, PCWSTR in2) {
		return ::wcscmp(in1, in2);
		//	return ::wcscoll(in1, in2);
	}

	inline int compare_str(PCWSTR in1, PCWSTR in2, size_t n) {
		return ::wcsncmp(in1, in2, n);
	}

	inline int compare_str_cs(PCSTR in1, PCSTR in2) {
		return ::CompareStringA(0, SORT_STRINGSORT, in1, -1, in2, -1) - CSTR_EQUAL;
	}

	inline int compare_str_cs(PCSTR in1, PCSTR in2, size_t n) {
		return ::CompareStringA(0, NORM_STOP_ON_NULL | SORT_STRINGSORT, in1, n, in2, n) - CSTR_EQUAL;
	}

	inline int compare_str_cs(PCWSTR in1, PCWSTR in2) {
		return ::CompareStringW(0, SORT_STRINGSORT, in1, -1, in2, -1) - CSTR_EQUAL;
	}

	inline int compare_str_cs(PCWSTR in1, PCWSTR in2, size_t n) {
		return ::CompareStringW(0, NORM_STOP_ON_NULL | SORT_STRINGSORT, in1, n, in2, n) - CSTR_EQUAL;
	}

	inline int compare_str_ci(PCSTR in1, PCSTR in2) {
		//	return ::_stricmp(in1, in2);
		return ::CompareStringA(0, NORM_IGNORECASE | SORT_STRINGSORT, in1, -1, in2, -1) - CSTR_EQUAL;
	}

	inline int compare_str_ci(PCSTR in1, PCSTR in2, size_t n) {
		return ::CompareStringA(0, NORM_IGNORECASE | NORM_STOP_ON_NULL | SORT_STRINGSORT, in1, n, in2, n) - CSTR_EQUAL;
	}

	inline int compare_str_ci(PCWSTR in1, PCWSTR in2) {
		//	return ::_wcsicmp(in1, in2);
		//	return ::_wcsicoll(lhs.first.c_str(), rhs.first.c_str()) < 0;
		//	return fsf.LStricmp(lhs.first.c_str(), rhs.first.c_str()) < 0;
		return ::CompareStringW(0, NORM_IGNORECASE | SORT_STRINGSORT, in1, -1, in2, -1) - CSTR_EQUAL;
	}

	inline int compare_str_ci(PCWSTR in1, PCWSTR in2, size_t n) {
		return ::CompareStringW(0, NORM_IGNORECASE | NORM_STOP_ON_NULL | SORT_STRINGSORT, in1, n, in2, n) - CSTR_EQUAL;
	}

	inline PSTR copy_str(PSTR dest, PCSTR src) {
		return ::strcpy(dest, src);
	}

	inline PWSTR copy_str(PWSTR dest, PCWSTR src) {
		return ::wcscpy(dest, src);
	}

	inline PSTR copy_str(PSTR dest, PCSTR src, size_t size) {
		return ::strncpy(dest, src, size);
	}

	inline PWSTR copy_str(PWSTR dest, PCWSTR src, size_t size) {
		return ::wcsncpy(dest, src, size);
	}

	inline PSTR cat_str(PSTR dest, PCSTR src) {
		return ::strcat(dest, src);
	}

	inline PWSTR cat_str(PWSTR dest, PCWSTR src) {
		return ::wcscat(dest, src);
	}

	inline PSTR cat_str(PSTR dest, PCSTR src, size_t size) {
		return ::strncat(dest, src, size);
	}

	inline PWSTR cat_str(PWSTR dest, PCWSTR src, size_t size) {
		return ::wcsncat(dest, src, size);
	}

	inline PSTR find_str(PCSTR where, PCSTR what) {
		return ::strstr(where, what);
	}

	inline PCSTR find_str(PCSTR where, CHAR what) {
		return ::strchr(where, what);
	}

	inline PWSTR find_str(PCWSTR where, PCWSTR what) {
		return ::wcsstr(where, what);
	}

	inline PCWSTR find_str(PCWSTR where, WCHAR what) {
		return ::wcschr(where, what);
	}

	//inline PSTR RFind(PCSTR where, PCSTR what) {
	//	PCSTR last1 = where + Len(where);
	//	PCSTR last2 = what + Len(what);
	//	last2 = std::find_end(where, last1, what, last2);
	//	return (last1 == last2) ? nullptr : const_cast<PSTR>(last2);
	//}
	//
	//inline PSTR RFind(PCSTR where, CHAR what) {
	//	return ::strrchr(where, what);
	//}
	//
	//inline PWSTR RFind(PCWSTR where, PCWSTR what) {
	//	PCWSTR last1 = where + Len(where);
	//	PCWSTR last2 = what + Len(what);
	//	last2 = std::find_end(where, last1, what, last2);
	//	return (last1 == last2) ? nullptr : const_cast<PWSTR>(last2);
	//}
	//
	//inline PWSTR RFind(PCWSTR where, WCHAR what) {
	//	return ::wcsrchr(where, what);
	//}

	inline size_t span_str(PCSTR str, PCSTR strCharSet) {
		return ::strcspn(str, strCharSet);
	}

	inline size_t span_str(PCWSTR str, PCWSTR strCharSet) {
		return ::wcscspn(str, strCharSet);
	}

	///=================================================================================================
	inline PWSTR to_upper_str(PWSTR buf, size_t len) {
		::CharUpperBuffW(buf, len);
		return buf;
	}

	inline PWSTR to_upper_str(PWSTR s1) {
		return to_upper_str(s1, get_str_len(s1));
	}

	inline PWSTR to_lower_str(PWSTR buf, size_t len) {
		::CharLowerBuffW(buf, len);
		return buf;
	}

	inline PWSTR to_lower_str(PWSTR s1) {
		return to_lower_str(s1, get_str_len(s1));
	}

	inline PSTR fill_str(PSTR in, CHAR ch) {
		return ::_strset(in, ch);
	}

	inline PWSTR fill_str(PWSTR in, WCHAR ch) {
		return ::_wcsset(in, ch);
	}

	inline PSTR reverse_str(PSTR in) {
		return ::_strrev(in);
	}

	inline PWSTR reverse_str(PWSTR in) {
		return ::_wcsrev(in);
	}

	//inline PWSTR AssignStr(PCWSTR src) {
	//	size_t len = Len(src) + 1;
	//	PWSTR dest;
	//	Memory::Alloc(dest, len * sizeof(WCHAR));
	//	Copy(dest, src, len);
	//	return dest;
	//}

	inline size_t convert_cp(PCSTR from, UINT cp, PWSTR to = nullptr, size_t size = 0) {
		return ::MultiByteToWideChar(cp, 0, from, -1, to, (int)size);
	}

	inline size_t convert_cp(PCWSTR from, UINT cp, PSTR to = nullptr, size_t size = 0) {
		return ::WideCharToMultiByte(cp, 0, from, -1, to, (int)size, nullptr, nullptr);
	}

}

#endif
