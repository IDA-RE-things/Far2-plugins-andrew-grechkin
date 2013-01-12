#ifndef _LIBBASE_PCSTR_HPP_
#define _LIBBASE_PCSTR_HPP_

#include <libbase/std.hpp>

extern "C" {
	long long __MINGW_NOTHROW wcstoll(const wchar_t * __restrict__, wchar_t** __restrict__, int);
	unsigned long long __MINGW_NOTHROW wcstoull(const wchar_t * __restrict__, wchar_t ** __restrict__, int);
}

namespace Base {
	namespace Str {

		const DWORD NORM_STOP_ON_NULL = 0x10000000;

		inline bool is_empty(PCSTR str) {
			return str == nullptr || *str == 0;
		}

		inline bool is_empty(PCWSTR str) {
			return str == nullptr || *str == 0;
		}

		inline size_t length(PCSTR in) {
			return ::strlen(in);
		}

		inline size_t length(PCWSTR in) {
			return ::wcslen(in);
		}

		inline int compare(PCSTR in1, PCSTR in2) {
			return ::strcmp(in1, in2);
		}

		inline int compare(PCSTR in1, PCSTR in2, size_t n) {
			return ::strncmp(in1, in2, n);
		}

		inline int compare(PCWSTR in1, PCWSTR in2) {
			return ::wcscmp(in1, in2);
			//	return ::wcscoll(in1, in2);
		}

		inline int compare(PCWSTR in1, PCWSTR in2, size_t n) {
			return ::wcsncmp(in1, in2, n);
		}

		inline int compare_cs(PCSTR in1, PCSTR in2) {
			return ::CompareStringA(0, SORT_STRINGSORT, in1, -1, in2, -1) - CSTR_EQUAL;
		}

		inline int compare_cs(PCSTR in1, PCSTR in2, size_t n) {
			return ::CompareStringA(0, NORM_STOP_ON_NULL | SORT_STRINGSORT, in1, n, in2, n) - CSTR_EQUAL;
		}

		inline int compare_cs(PCWSTR in1, PCWSTR in2) {
			return ::CompareStringW(0, SORT_STRINGSORT, in1, -1, in2, -1) - CSTR_EQUAL;
		}

		inline int compare_cs(PCWSTR in1, PCWSTR in2, size_t n) {
			return ::CompareStringW(0, NORM_STOP_ON_NULL | SORT_STRINGSORT, in1, n, in2, n) - CSTR_EQUAL;
		}

		inline int compare_ci(PCSTR in1, PCSTR in2) {
			//	return ::_stricmp(in1, in2);
			return ::CompareStringA(0, NORM_IGNORECASE | SORT_STRINGSORT, in1, -1, in2, -1) - CSTR_EQUAL;
		}

		inline int compare_ci(PCSTR in1, PCSTR in2, size_t n) {
			return ::CompareStringA(0, NORM_IGNORECASE | NORM_STOP_ON_NULL | SORT_STRINGSORT, in1, n, in2, n) - CSTR_EQUAL;
		}

		inline int compare_ci(PCWSTR in1, PCWSTR in2) {
			//	return ::_wcsicmp(in1, in2);
			//	return ::_wcsicoll(lhs.first.c_str(), rhs.first.c_str()) < 0;
			//	return fsf.LStricmp(lhs.first.c_str(), rhs.first.c_str()) < 0;
			return ::CompareStringW(0, NORM_IGNORECASE | SORT_STRINGSORT, in1, -1, in2, -1) - CSTR_EQUAL;
		}

		inline int compare_ci(PCWSTR in1, PCWSTR in2, size_t n) {
			return ::CompareStringW(0, NORM_IGNORECASE | NORM_STOP_ON_NULL | SORT_STRINGSORT, in1, n, in2, n) - CSTR_EQUAL;
		}

		inline PSTR copy(PSTR dest, PCSTR src) {
			return ::strcpy(dest, src);
		}

		inline PWSTR copy(PWSTR dest, PCWSTR src) {
			return ::wcscpy(dest, src);
		}

		inline PSTR copy(PSTR dest, PCSTR src, size_t size) {
			return ::strncpy(dest, src, size);
		}

		inline PWSTR copy(PWSTR dest, PCWSTR src, size_t size) {
			return ::wcsncpy(dest, src, size);
		}

		inline PSTR cat(PSTR dest, PCSTR src) {
			return ::strcat(dest, src);
		}

		inline PWSTR cat(PWSTR dest, PCWSTR src) {
			return ::wcscat(dest, src);
		}

		inline PSTR cat(PSTR dest, PCSTR src, size_t size) {
			return ::strncat(dest, src, size);
		}

		inline PWSTR cat(PWSTR dest, PCWSTR src, size_t size) {
			return ::wcsncat(dest, src, size);
		}

		inline PSTR find(PCSTR where, PCSTR what) {
			return ::strstr(where, what);
		}

		inline PCSTR find(PCSTR where, CHAR what) {
			return ::strchr(where, what);
		}

		inline PWSTR find(PCWSTR where, PCWSTR what) {
			return ::wcsstr(where, what);
		}

		inline PCWSTR find(PCWSTR where, WCHAR what) {
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

		inline size_t span(PCSTR str, PCSTR strCharSet) {
			return ::strcspn(str, strCharSet);
		}

		inline size_t span(PCWSTR str, PCWSTR strCharSet) {
			return ::wcscspn(str, strCharSet);
		}

		///=================================================================================================
		inline PWSTR to_upper(PWSTR buf, size_t len) {
			::CharUpperBuffW(buf, len);
			return buf;
		}

		inline PWSTR to_upper(PWSTR s1) {
			return to_upper(s1, length(s1));
		}

		inline PWSTR to_lower(PWSTR buf, size_t len) {
			::CharLowerBuffW(buf, len);
			return buf;
		}

		inline PWSTR to_lower(PWSTR s1) {
			return to_lower(s1, length(s1));
		}

		inline PSTR fill(PSTR in, CHAR ch) {
			return ::_strset(in, ch);
		}

		inline PWSTR fill(PWSTR in, WCHAR ch) {
			return ::_wcsset(in, ch);
		}

		inline PSTR reverse(PSTR in) {
			return ::_strrev(in);
		}

		inline PWSTR reverse(PWSTR in) {
			return ::_wcsrev(in);
		}

		//inline PWSTR AssignStr(PCWSTR src) {
		//	size_t len = Len(src) + 1;
		//	PWSTR dest;
		//	Memory::Alloc(dest, len * sizeof(WCHAR));
		//	Copy(dest, src, len);
		//	return dest;
		//}

		inline size_t convert(PCSTR from, UINT cp) {
			return ::MultiByteToWideChar(cp, 0, from, -1, nullptr, 0);
		}

		inline size_t convert(PWSTR to, size_t size, PCSTR from, UINT cp) {
			return ::MultiByteToWideChar(cp, 0, from, -1, to, (int)size);
		}

		inline size_t convert(PCWSTR from, UINT cp) {
			return ::WideCharToMultiByte(cp, 0, from, -1, nullptr, 0, nullptr, nullptr);
		}

		inline size_t convert(PSTR to, size_t size, PCWSTR from, UINT cp) {
			return ::WideCharToMultiByte(cp, 0, from, -1, to, (int)size, nullptr, nullptr);
		}

		///=========================================================================================
		inline PSTR convert_num(PSTR to, int64_t num, ssize_t base = 10) {
			return ::_i64toa(num, to, base); //lltoa
		}

		inline PWSTR convert_num(PWSTR to, int64_t num, ssize_t base = 10) {
			return ::_i64tow(num, to, base); //lltow
		}

		///=========================================================================================
		inline int64_t as_int64(PCSTR in) {
			return _atoi64(in);
		}

		inline uint32_t as_uint32(PCSTR in, int base = 10) {
			PSTR end_ptr;
			return ::strtoul(in, &end_ptr, base);
		}

		inline int32_t as_int32(PCSTR in, int base = 10) {
			PSTR end_ptr;
			return ::strtol(in, &end_ptr, base);
		}

		inline double as_double(PCSTR in) {
			PSTR end_ptr;
			return ::strtod(in, &end_ptr);
		}

		inline uint64_t as_uint64(PCWSTR in, int base = 10) {
			//	return _wtoi64(in);
			PWSTR end_ptr;
			return ::wcstoull(in, &end_ptr, base);
		}

		inline int64_t as_int64(PCWSTR in, int base = 10) {
			//	return _wtoi64(in);
			PWSTR end_ptr;
			return ::wcstoll(in, &end_ptr, base);
		}

		inline uint32_t as_uint32(PCWSTR in, int base = 10) {
			PWSTR end_ptr;
			return ::wcstoul(in, &end_ptr, base);
		}

		inline int32_t as_int32(PCWSTR in, int base = 10) {
			PWSTR end_ptr;
			return ::wcstol(in, &end_ptr, base);
		}

		inline double as_double(PCWSTR in) {
			PWSTR end_ptr;
			return ::wcstod(in, &end_ptr);
		}

	}

	///======================================================================================= Types
	template<typename Type>
	struct NamedValues {
		PCWSTR name;
		Type value;

		static PCWSTR GetName(const NamedValues<Type> dim[], size_t size, const Type & in) {
			for (size_t i = 0; i < size; ++i) {
				if (dim[i].value == in) {
					return dim[i].name;
				}
			}
			return L"unknown";
		}

		static Type GetValue(const NamedValues<Type> dim[], size_t size, PCWSTR name) {
			for (size_t i = 0; i < size; ++i) {
				if (Str::compare(dim[i].name, name)) {
					return dim[i].value;
				}
			}
			return 0;
		}
	};

}

#endif
