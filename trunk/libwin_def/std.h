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

//#define NTSIGNATURE(a) ((LPVOID)((BYTE *)(a) + ((PIMAGE_DOS_HEADER)(a))->e_lfanew))
//#define HighLow64(high, low) (((uint64_t)(high) << 32) | (low))
//#define HighPart64(arg64) ((DWORD)((arg64) >> 32))
//#define LowPart64(arg64) ((DWORD)((arg64) & 0xFFFFFFFF))

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

///======================================================================================= Singleton
/// Паттерн Singleton
template <typename T>
struct Singleton_p : private T {
	typedef T implementation_type;
	typedef Singleton_p<T> this_type;
	static T & instance() {
		static this_type instance;
		return instance;
	}
private:
	~Singleton_p() {
	}
	Singleton_p() {
	}
};

///========================================================================================= Command
/// Паттерн Command
struct Command_p {
	virtual ~Command_p() {
	}
	virtual bool Execute() const = 0;
};

struct NullCommand: public Command_p {
	virtual bool Execute() const {
		return true;
	}
};

#endif
