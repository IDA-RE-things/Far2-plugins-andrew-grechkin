#ifndef WIN_DEF_STD_HPP
#define WIN_DEF_STD_HPP

#if (_WIN32_WINNT < 0x0600)
#undef _WIN32_WINNT
#endif
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif

#if (WINVER < 0x0600)
#undef WINVER
#endif
#ifndef WINVER
#define WINVER 0x0600
#endif

#if (_WIN32_IE < 0x0600)
#undef _WIN32_IE
#endif
#ifndef _WIN32_IE
#define _WIN32_IE 0x0600
#endif

//#define WIN32_LEAN_AND_MEAN
#define NOMCX
#define NOIME

#include <stdint.h>
#include <windows.h>
#include <algorithm>

#ifdef __x86_64__
#define nullptr 0ll
#else
#define nullptr 0
#endif

typedef const void * PCVOID;

PCWSTR const EMPTY_STR = L"";
PCWSTR const PATH_SEPARATOR = L"\\"; // Path separator in the file system
PCWSTR const PATH_SEPARATORS = L"\\/";
PCWSTR const SPACE = L" ";
PCWSTR const PATH_PREFIX_NT = L"\\\\?\\"; // Prefix to put ahead of a long path for Windows API
PCWSTR const NETWORK_PATH_PREFIX = L"\\\\";

const WCHAR PATH_SEPARATOR_C = L'\\';
const WCHAR STR_END = L'\0';
const WCHAR SPACE_C = L' ';

const uint32_t BOM_UTF32le = 0x0000FEFF;
const uint32_t BOM_UTF32be = 0xFFFE0000;
const uint32_t BOM_UTF16le = 0xFEFF;
const uint32_t BOM_UTF16be = 0xFFFE;
const uint32_t BOM_UTF8 = 0xBFBBEF00;

const size_t MAX_PATH_LEN = 32772;
const UINT CP_UTF16le = 1200;
const UINT CP_UTF16be = 1201;
const UINT CP_UTF32le = 1202;
const UINT CP_UTF32be = 1203;
const UINT CP_AUTODETECT = (UINT)-1;
const UINT DEFAULT_CP = CP_UTF8;

#define DEFINE_FUNC(name) F##name name
#define GET_DLL_FUNC(name) name = (F##name)get_function(#name)
#define GET_DLL_FUNC_NT(name) name = (F##name)get_function_nt(#name)

#define THIS_FILE filename_only(__FILE__)

#ifndef sizeofa
#define sizeofa(array) (sizeof(array)/sizeof(0[array]))
#endif

#ifndef sizeofe
#define sizeofe(array) (sizeof(0[array]))
#endif

template <typename Type, size_t N>
size_t lengthof(Type(&)[N]) {
	return N;
}

inline uint64_t HighLow64(uint32_t high, uint32_t low) {
	return ((uint64_t)high) << 32 | low;
}

inline uint32_t HighPart64(uint64_t arg64) {
	return (uint32_t)(arg64 >> 32);
}

inline uint32_t LowPart64(uint64_t arg64) {
	return (uint32_t)(arg64 & 0xFFFFFFFF);
}

inline PCSTR filename_only(PCSTR path, char ch = '\\') {
	return (strrchr((path), ch) ?: (path) - 1) + 1;
}

inline PCWSTR filename_only(PCWSTR path, WCHAR ch = L'\\') {
	return (wcsrchr((path), ch) ?: (path) - 1) + 1;
}

///=================================================================================================
inline PCSTR as_cstr(PSTR str, int64_t num, int base = 10) {
	return ::_i64toa(num, str, base); //lltoa
}

inline PCWSTR as_cstr(PWSTR str, int64_t num, int base = 10) {
	return ::_i64tow(num, str, base); //lltow
}

///=================================================================================================
extern "C" {
	long long __MINGW_NOTHROW	wcstoll(const wchar_t * __restrict__, wchar_t** __restrict__, int);
	unsigned long long __MINGW_NOTHROW wcstoull(const wchar_t * __restrict__, wchar_t ** __restrict__, int);
}

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

///=========================================================================================== Types
template <typename Type>
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
			if (Eq(dim[i].name, name)) {
				return dim[i].value;
			}
		}
		return 0;
	}
};

///====================================================================================== Uncopyable
/// Базовый класс для private наследования классами, объекты которых не должны копироваться
class Uncopyable {
	typedef Uncopyable this_type;
protected:
	~Uncopyable() {
	}
	Uncopyable() {
	}
private:
	Uncopyable(const this_type &);
	this_type & operator =(const this_type &);
};

#endif
