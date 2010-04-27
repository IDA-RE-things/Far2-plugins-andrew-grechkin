/**
	win_def
	Main windows application include, always include first
	@classes	(WinMem, WinStr, CStr, WinFlag, WinBit, WinTimer, WinErrorCheck, Uncopyable, WinCOM)
	@author		© 2009 Andrew Grechkin
	@link		()
	@link		(ole32) for WinCom
**/

#ifndef WIN_DEF_HPP
#define WIN_DEF_HPP

#if (_WIN32_WINNT < 0x0501)
#undef _WIN32_WINNT
#endif
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif

#if (WINVER < 0x0501)
#undef WINVER
#endif
#ifndef WINVER
#define WINVER 0x0501
#endif

#if (_WIN32_IE < 0x0600)
#undef _WIN32_IE
#endif
#ifndef _WIN32_IE
#define _WIN32_IE 0x0600
#endif

#include <windows.h>
#include <shlwapi.h>
#include <stdint.h>
#include <stdio.h>

typedef struct _PERFORMANCE_INFORMATION {
	DWORD cb;
	SIZE_T CommitTotal;
	SIZE_T CommitLimit;
	SIZE_T CommitPeak;
	SIZE_T PhysicalTotal;
	SIZE_T PhysicalAvailable;
	SIZE_T SystemCache;
	SIZE_T KernelTotal;
	SIZE_T KernelPaged;
	SIZE_T KernelNonpaged;
	SIZE_T PageSize;
	DWORD HandleCount;
	DWORD ProcessCount;
	DWORD ThreadCount;
} PERFORMANCE_INFORMATION, *PPERFORMANCE_INFORMATION, PERFORMACE_INFORMATION, *PPERFORMACE_INFORMATION;

EXTERN_C {
	WINBASEAPI VOID WINAPI GetNativeSystemInfo(LPSYSTEM_INFO);
	WINBASEAPI ULONGLONG WINAPI GetTickCount64();
	WINBASEAPI DWORD WINAPI GetProcessId(HANDLE Process);
	_CRTIMP int __cdecl _snwprintf(wchar_t*, size_t, const wchar_t*, ...);
}

///===================================================================================== definitions
#define MAX_PATH_LENGTH			32768
#define STR_END					L'\0'
#define SPACE					L" "
#define SPACE_C					L' '
#define PATH_SEPARATOR			L"\\" // Path separator in the file system
#define PATH_SEPARATOR_C		L'\\' // Path separator in the file system
#define PATH_PREFIX				L"\\\\?\\" // Prefix to put ahead of a long path for Windows API
#define NET_PREFIX				L"\\\\"
#define NORM_M_PREFIX(m)        (*(LPDWORD)m==0x5c005c)
#define REV_M_PREFIX(m)         (*(LPDWORD)m==0x2f002f)

#define NTSIGNATURE(a) ((LPVOID)((BYTE *)a + ((PIMAGE_DOS_HEADER)a)->e_lfanew))

#ifndef sizeofa
#define sizeofa(array)			(sizeof(array)/sizeof(array[0]))
#endif
#ifndef sizeofe
#define sizeofe(array)			(sizeof(array[0]))
#endif

#ifndef NORM_STOP_ON_NULL
#define NORM_STOP_ON_NULL 0x10000000
#endif

#ifdef NoStdNew
inline void*		operator new(size_t size) {
	return	::HeapAlloc(::GetProcessHeap(), HEAP_ZERO_MEMORY, size);
}
inline void*		operator new[](size_t size) {
	return ::operator new(size);
}
inline void			operator delete(void *in) {
	::HeapFree(::GetProcessHeap(), 0, (PVOID)in);
}
inline void 		operator delete[](void *ptr) {
	::operator	delete(ptr);
}
#endif

#define BOM_UTF32le		0x0000FEFF
#define BOM_UTF32be		0xFFFE0000
#define BOM_UTF16le		0xFEFF
#define BOM_UTF16be		0xFFFE
#define BOM_UTF8		0xBFBBEF

#define CP_UTF16le		1200
#define CP_UTF16be		1201
#define CP_UTF32le		1202
#define CP_UTF32be		1203
#define CP_AUTODETECT	((UINT)-1)
#define	DEFAULT_CHAR_CP	CP_UTF8

///=========================================================================================== Types
typedef const void			*PCVOID;

template <typename Type>
struct		NamedValues {
	Type	value;
	PCWSTR	name;

	static	PCWSTR	GetName(NamedValues<Type> dim[], size_t size, const Type &in) {
		for (size_t i = 0; i < size; ++i) {
			if (dim[i].value == in)
				return	dim[i].name;
		}
		return	L"unknown";
	}
};

class		MyUI64 {
	ULARGE_INTEGER	m_data;
public:
	MyUI64(uint64_t in = 0ull) {
		m_data.QuadPart = in;
	}
	MyUI64(DWORD low, DWORD high) {
		m_data.LowPart = low;
		m_data.HighPart = high;
	}
	MyUI64(const ULARGE_INTEGER &in) {
		m_data.QuadPart = in.QuadPart;
	}
	operator	uint64_t() const {
		return	m_data.QuadPart;
	}
};

///=================================================================================================
inline void			XchgByte(WORD &inout) {
	inout = inout >> 8 | inout << 8;
}
inline void			XchgWord(DWORD &inout) {
	inout = inout >> 16 | inout << 16;
}

uintmax_t			Mega2Bytes(size_t in);
size_t				Bytes2Mega(uintmax_t in);

///=================================================================================== Reverse bytes
template<typename Type>
inline Type			ReverseBytes(const Type &in) {
	Type	Result;
	size_t	size = sizeof(Type) - 1;
	char	*sta = (char*)(&in), *end = (char*)(&Result);
	for (size_t i = 0; i <= size; ++i) {
		end[size-i] = sta[i];
	}
	return	(Result);
}

template <typename Type>
inline const Type	&Min(const Type &a, const Type &b) {
	return	(a < b) ? a : b;
}
template <typename Type>
inline	const Type	&Max(const Type &a, const Type &b) {
	return	(a < b) ? b : a;
}
template <typename Type>
inline	void		Swp(Type &x, Type &y) {
	Type tmp(x);
	x = y;
	y = tmp;
}

///========================================================================================= WinFlag
/// Класс проверки и установки флагов
template<typename Type>
class		WinFlag {
public:
	static bool		Check(Type in, Type flag) {
		return	flag == (in & flag);
	}
	static Type		&Set(Type &in, Type flag) {
		return	in |= flag;
	}
	static Type		&UnSet(Type &in, Type flag) {
		return	in &= ~flag;
	}
};

///========================================================================================= WinFlag
/// Класс проверки и установки битов
template<typename Type>
class		WinBit {
public:
	static	size_t	BIT_LIMIT() {
		return	sizeof(Type) * 8;
	}
	static	bool	BadBit(size_t in) {
		return	(in < 0) || (in >= BIT_LIMIT());
	}
	static	size_t	Limit(size_t in) {
		return	(in == 0) ? BIT_LIMIT() : Min<int>(in, BIT_LIMIT());
	}

	static	bool	Check(Type in, size_t bit) {
		if (BadBit(bit))
			return	false;
		Type tmp = 1;
		tmp <<= bit;
		return	(in & tmp);
	}
	static	Type	&Set(Type &in, size_t bit) {
		if (BadBit(bit))
			return	in;
		Type	tmp = 1;
		tmp <<= bit;
		in |= tmp;
		return	in;
	}
	static	Type	&UnSet(Type &in, size_t bit) {
		if (BadBit(bit))
			return	in;
		Type	tmp = 1;
		tmp <<= bit;
		in &= ~tmp;
		return	in;
	}
};

///========================================================================================== WinCom
/// Класс инициализации COM (объекты создавать запрещено, нужно использовать фукцию IsOK)
class		WinCOM {
	bool		m_err;

	WinCOM(const WinCOM&);
	WinCOM(): m_err(true) {
		if (m_err) {
			m_err = !SUCCEEDED(::CoInitializeEx(NULL, COINIT_MULTITHREADED));
		}
	}
public:
	~WinCOM() {
		if (IsOK()) {
			::CoUninitialize();
			the().m_err = true;
		}
	}
	static WinCOM	&the() {
		static WinCOM com;
		return	com;
	}
	static bool		IsOK() {
		return	!the().m_err;
	}
};

///====================================================================================== Uncopyable
/// Базовый класс для наследования классами, объекты которых не должны копироваться
class		Uncopyable {
	Uncopyable(const Uncopyable&);
	Uncopyable &operator=(const Uncopyable&);
protected:
	~Uncopyable() {
	}
	Uncopyable() {
	}
};

///======================================================================================== Auto_ptr

///====================================================================================== Shared_ptr
template <typename Type>
class	Shared_ptr {
	template <typename Pti>
	class	Pointee {
		size_t	m_ref;
		Pti		*m_ptr;
	public:
		Pointee(Pti *ptr): m_ref(1), m_ptr(ptr) {
		}
		void	delRef() {
			--m_ref;
			if (m_ref == 0 && m_ptr)
				delete	m_ptr;
		}
		void	addRef() {
			++m_ref;
		}
		friend class Shared_ptr;
	};
	Pointee<Type>	*data;
public:
	~Shared_ptr() {
		release();
	}
	Shared_ptr(): data(new Pointee<Type>(NULL)) {
	}
	Shared_ptr(Type *ptr): data(new Pointee<Type>(ptr)) {
	}
	Shared_ptr(const Shared_ptr<Type> &rhs): data(rhs.data) {
		data->addRef();
	}

	Shared_ptr<Type>&	operator=(const Shared_ptr<Type> &rhs) {
		if (this != &rhs) {
			release();
			data = rhs.data;
			data->addRef();
		}
		return	*this;
	}
	Type*				get() const {
		return	data->m_ptr;
	}
	void				release() {
		data->delRef();
	}
	void				reset(Type* ptr) {
		release();
		data = new Pointee<Type>(ptr);
	}

	operator			bool() const {
		return	data->m_ptr;
	}
	operator			Type*() const {
		return	data->m_ptr;
	}
	Type*				operator->() const {
		return data->m_ptr;
	}
	Type&				operator*() {
		return *(data->m_ptr);
	}
};

///=================================================================================== WinErrorCheck
/// Базовый класс для проверки и хранения кода ошибки
class		WinErrorCheck {
	DWORD	mutable	m_err;
protected:
	~WinErrorCheck() {
	}
	WinErrorCheck(): m_err(NO_ERROR) {
	}
public:
	DWORD			err() const {
		return	(DWORD)m_err;
	}
	DWORD			err(DWORD err) const {
		return	(DWORD)(m_err = err);
	}
	bool			IsOK() const {
		return	m_err == NO_ERROR;
	}
	bool			ChkSucc(bool in) const {
		if (!in)
			err(::GetLastError());
		else
			err(NO_ERROR);
		return	in;
	}
	template<typename Type>
	void			SetIfFail(Type &in, const Type &value) {
		if (m_err != NO_ERROR)
			in = value;
	}
};

///========================================================================================== WinMem
/// Функции работы с кучей
namespace	WinMem {
template <typename Type>
inline bool			Alloc(Type &in, size_t size) {
	in = static_cast<Type>(::HeapAlloc(::GetProcessHeap(), HEAP_ZERO_MEMORY, size));
	return	 in != NULL;
}
inline PVOID		Alloc(size_t size) {
	return	::HeapAlloc(::GetProcessHeap(), HEAP_ZERO_MEMORY, size);
}

template <typename Type>
inline bool			Realloc(Type &in, size_t size) {
	if (in != NULL)
		in = static_cast<Type>(::HeapReAlloc(::GetProcessHeap(), HEAP_ZERO_MEMORY, static_cast<PVOID>(in), size));
	else
		in = static_cast<Type>(::HeapAlloc(::GetProcessHeap(), HEAP_ZERO_MEMORY, size));
	return	 in != NULL;
}
inline PVOID		Realloc(PVOID in, size_t size) {
	return	::HeapReAlloc(::GetProcessHeap(), HEAP_ZERO_MEMORY, in, size);
}

template <typename Type>
inline bool			Free(Type &in) {
	bool	Result = ::HeapFree(::GetProcessHeap(), 0, (PVOID)in) != 0;
	in = NULL;
	return	Result;
}
inline void			Free(PVOID in) {
	::HeapFree(::GetProcessHeap(), 0, in);
}

inline size_t		Size(PCVOID in) {
	return	(in) ? ::HeapSize(::GetProcessHeap(), 0, in) : 0;
}

inline bool			Cmp(PCVOID m1, PCVOID m2, size_t size) {
	return	::memcmp(m1, m2, size) == 0;
}

inline PVOID		Copy(PVOID dest, PCVOID sour, size_t size) {
	return	::memcpy(dest, sour, size);
}
inline PVOID		Fill(PVOID in, size_t size, char fill) {
	return	::memset(in, (int)fill, size);
}
inline void			Zero(PVOID in, size_t size) {
	Fill(in, size, 0);
}
template<typename Type>
inline void			Fill(Type &in, char fill) {
	Fill(&in, sizeof(in), fill);
}
template<typename Type>
inline void			Zero(Type &in) {
	Fill(&in, sizeof(in), 0);
}
}

///========================================================================================= WinTime
struct		WinTime: public FILETIME {
	void			Init(const ULARGE_INTEGER &in) {
		dwLowDateTime	= in.LowPart;
		dwHighDateTime	= in.HighPart;
	}
public:
	WinTime() {
		now();
	}
	WinTime(const uint64_t &in) {
		ULARGE_INTEGER tmp;
		tmp.QuadPart = in;
		Init(tmp);
	}
	WinTime(const ULARGE_INTEGER &in) {
		Init(in);
	}
	operator		FILETIME() const {
		return	*this;
	}
	operator		ULARGE_INTEGER() const {
		ULARGE_INTEGER	Result;
		Result.LowPart = this->dwLowDateTime;
		Result.HighPart = this->dwHighDateTime;
		return	Result;
	}
	operator		uint64_t() const {
		ULARGE_INTEGER	Result;
		Result.LowPart = this->dwLowDateTime;
		Result.HighPart = this->dwHighDateTime;
		return	Result.QuadPart;
	}
	void			now() {
		::GetSystemTimeAsFileTime(this);
	}

	const WinTime&	operator=(const ULARGE_INTEGER & in) {
		Init(in);
		return	*this;
	}
	WinTime&		operator+=(const uint64_t & in) {
		ULARGE_INTEGER tmp = *this;
		tmp.QuadPart += in * Second();
		Init(tmp);
		return	*this;
	}
	WinTime&		operator-=(const uint64_t & in) {
		ULARGE_INTEGER tmp = *this;
		tmp.QuadPart -= in * Second();
		Init(tmp);
		return	*this;
	}
	WinTime			operator+(const uint64_t &in) {
		ULARGE_INTEGER tmp = *this;
		tmp.QuadPart += in * Second();
		return	WinTime(tmp);
	}
	uint64_t		operator-(const WinTime &in) {
		ULARGE_INTEGER tmp = *this;
		tmp.QuadPart -= ((ULARGE_INTEGER)in).QuadPart;
		return	tmp.QuadPart / Second();
	}

	static uint64_t	MiliSecond() {
		return	10000ULL;
	}
	static uint64_t	Second() {
		return	10000000ULL;
	}
	static uint64_t	Minute() {
		return	600000000ULL;
	}
	static uint64_t	Hour() {
		return	36000000000ULL;
	}
	static uint64_t	Day() {
		return	864000000000ULL;
	}
	static uint64_t	Week() {
		return	6048000000000ULL;
	}
	static uint64_t	SecPerDay() {
		return	60ULL * 60 * 24;
	}
	static uint64_t	SecPerHour() {
		return	60ULL * 60;
	}
};

///========================================================================================= WinTime
class		WinSysTime: public SYSTEMTIME {
public:
	WinSysTime() {
		WinMem::Zero(*this);
		Now();
	}
	void			Now(bool isLocal = false) {
		if (isLocal)
			::GetLocalTime(this);
		else
			::GetSystemTime(this);
	}
	void			AddYear(long in) {
		wYear += in;
	}
};

///====================================================================== Функции работы с символами
inline WORD			GetType(WCHAR in) {
	WORD	Result[2] = {0};
	::GetStringTypeW(CT_CTYPE1, &in, 1, Result);
	return	Result[0];
}
inline bool			IsEol(WCHAR in) {
	return	in == L'\r' || in == L'\n';
}
inline bool			IsSpace(WCHAR in) {
//	return	in == L' ' || in == L'\t';
	return	WinFlag<WORD>::Check(GetType(in), C1_SPACE);
}
inline bool			IsPrint(WCHAR in) {
	return	!WinFlag<WORD>::Check(GetType(in), C1_CNTRL);
}
inline bool			IsCntrl(WCHAR in) {
//	return	in == L' ' || in == L'\t';
	return	WinFlag<WORD>::Check(GetType(in), C1_CNTRL);
}
inline bool			IsUpper(WCHAR in) {
//	return	::IsCharUpperW(in);
	return	WinFlag<WORD>::Check(GetType(in), C1_UPPER);
}
inline bool			IsLower(WCHAR in) {
//	return	::IsCharLowerW(in);
	return	WinFlag<WORD>::Check(GetType(in), C1_LOWER);
}
inline bool			IsAlpha(WCHAR in) {
//	return	::IsCharAlphaW(in);
	return	WinFlag<WORD>::Check(GetType(in), C1_ALPHA);
}
inline bool			IsAlNum(WCHAR in) {
//	return	::IsCharAlphaW(in);
	WORD	Result = GetType(in);
	return	WinFlag<WORD>::Check(Result, C1_ALPHA) || WinFlag<WORD>::Check(Result, C1_DIGIT);
}
inline bool			IsDigit(WCHAR in) {
//	return	::IsCharAlphaNumeric(in);
	return	WinFlag<WORD>::Check(GetType(in), C1_DIGIT);
}
inline bool			IsXDigit(WCHAR in) {
//	return	::IsCharAlphaNumeric(in);
	return	WinFlag<WORD>::Check(GetType(in), C1_XDIGIT);
}
inline bool			IsPunct(WCHAR in) {
//	return	::IsCharAlphaNumeric(in);
	return	WinFlag<WORD>::Check(GetType(in), C1_PUNCT);
}

///====================================================================== Функции работы со строками
inline size_t		Len(PCSTR in) {
	return	::strlen(in);
}
inline size_t		Len(PCWSTR in) {
	return	::wcslen(in);
}
inline bool			Empty(PCSTR in) {
	return	Len(in) == 0;
}
inline bool			Empty(PCWSTR in) {
	return	Len(in) == 0;
}

inline int			Cmp(PCSTR in1, PCSTR in2) {
//	return	::strcmp(in1, in2);
	return	::CompareStringA(0, SORT_STRINGSORT, in1, -1, in2, -1) - CSTR_EQUAL;
}
inline int			Cmp(PCSTR in1, PCSTR in2, size_t n) {
//	return	::strncmp(in1, in2, n);
	return	::CompareStringA(0, NORM_STOP_ON_NULL | SORT_STRINGSORT, in1, n, in2, n) - CSTR_EQUAL;
}
inline int			Cmp(PCWSTR in1, PCWSTR in2) {
//	return	::wcscmp(in1, in2);
	return	::CompareStringW(0, SORT_STRINGSORT, in1, -1, in2, -1) - CSTR_EQUAL;
}
inline int			Cmp(PCWSTR in1, PCWSTR in2, size_t n) {
//	return	::wcsncmp(in1, in2, n);
	return	::CompareStringW(0, NORM_STOP_ON_NULL | SORT_STRINGSORT, in1, n, in2, n) - CSTR_EQUAL;
}
inline int			Cmpi(PCSTR in1, PCSTR in2) {
//	return	::_stricmp(in1, in2);
	return	::CompareStringA(0, NORM_IGNORECASE | SORT_STRINGSORT, in1, -1, in2, -1) - CSTR_EQUAL;
}
inline int			Cmpi(PCSTR in1, PCSTR in2, size_t n) {
	return	::CompareStringA(0, NORM_IGNORECASE | NORM_STOP_ON_NULL | SORT_STRINGSORT, in1, n, in2, n) - CSTR_EQUAL;
}
inline int			Cmpi(PCWSTR in1, PCWSTR in2) {
//	return	::_wcsicmp(in1, in2);
	return	::CompareStringW(0, NORM_IGNORECASE | SORT_STRINGSORT, in1, -1, in2, -1) - CSTR_EQUAL;
}
inline int			Cmpi(PCWSTR in1, PCWSTR in2, size_t n) {
	return	::CompareStringW(0, NORM_IGNORECASE | NORM_STOP_ON_NULL | SORT_STRINGSORT, in1, n, in2, n) - CSTR_EQUAL;
}
inline bool			Eq(PCSTR in1, PCSTR in2) {
	return	Cmp(in1, in2) == 0;
}
inline bool			Eq(PCWSTR in1, PCWSTR in2) {
	return	Cmp(in1, in2) == 0;
}
inline bool			Eqi(PCSTR in1, PCSTR in2) {
	return	Cmpi(in1, in2) == 0;
}
inline bool			Eqi(PCWSTR in1, PCWSTR in2) {
	return	Cmpi(in1, in2) == 0;
}

inline PSTR			Copy(PSTR dest, PCSTR src) {
	return	::strcpy(dest, src);
}
inline PWSTR		Copy(PWSTR dest, PCWSTR src) {
	return	::wcscpy(dest, src);
}
inline PSTR			Copy(PSTR dest, PCSTR src, size_t size) {
	return	::strncpy(dest, src, size);
}
inline PWSTR		Copy(PWSTR dest, PCWSTR src, size_t size) {
	return	::wcsncpy(dest, src, size);
}

inline PSTR			Cat(PSTR dest, PCSTR src) {
	return	::strcat(dest, src);
}
inline PWSTR		Cat(PWSTR dest, PCWSTR src) {
	return	::wcscat(dest, src);
}
inline PWSTR		Cat(PWSTR dest, PCWSTR src, size_t size) {
	return	::wcsncat(dest, src, size);
}

inline PWSTR		Find(PCWSTR where, PCWSTR what) {
	return	::wcsstr(where, what);
}
inline PWSTR		RFind(PCWSTR where, PCWSTR what) {
	return	::wcsstr(where, what);
}
inline PCSTR		CharFirst(PCSTR in, CHAR ch) {
	return	::strchr(in, ch);
}
inline PCWSTR		CharFirst(PCWSTR in, WCHAR ch) {
	return	::wcschr(in, ch);
}
inline PSTR			CharFirst(PSTR in, CHAR ch) {
	return	::strchr(in, ch);
}
inline PWSTR		CharFirst(PWSTR in, WCHAR ch) {
	return	::wcschr(in, ch);
}

inline PCSTR		CharLast(PCSTR in, CHAR ch) {
	return	::strrchr(in, ch);
}
inline PCWSTR		CharLast(PCWSTR in, WCHAR ch) {
	return	::wcsrchr(in, ch);
}
inline PSTR			CharLast(PSTR in, CHAR ch) {
	return	::strrchr(in, ch);
}
inline PWSTR		CharLast(PWSTR in, WCHAR ch) {
	return	::wcsrchr(in, ch);
}

inline PWSTR		CharFirstOf(PCWSTR in, PCWSTR mask) {
	return	(PWSTR)(in + ::wcscspn(in, mask));
}
inline PWSTR		CharFirstNotOf(PCWSTR in, PCWSTR mask) {
	return	(PWSTR)(in + ::wcsspn(in, mask));
}
inline PWSTR		CharLastOf(PCWSTR in, PCWSTR mask) {
	size_t	len = Len(mask);
	for (size_t i = Len(in); i > 0; --i) {
		for (size_t j = 0; j < len; ++j) {
			if (in[i-1] == mask[j])
				return	(PWSTR)&in[i-1];
		}
	}
	return	NULL;
}
inline PWSTR		CharLastNotOf(PCWSTR in, PCWSTR mask) {
	size_t	len = Len(mask);
	for (size_t i = Len(in); i > 0; --i) {
		for (size_t j = 0; j < len; ++j) {
			if (in[i-1] == mask[j])
				break;
			if (j == len - 1)
				return	(PWSTR)&in[i-1];
		}
	}
	return	NULL;
}

inline LONGLONG		AsLongLong(PCSTR in) {
	return	_atoi64(in);
}
inline ULONG		AsULong(PCSTR in, int base = 10) {
	PSTR	end_ptr;
	return	::strtoul(in, &end_ptr, base);
}
inline long			AsLong(PCSTR in, int base = 10) {
	PSTR	end_ptr;
	return	::strtol(in, &end_ptr, base);
}
inline UINT			AsUInt(PCSTR in, int base = 10) {
	return	(unsigned int)AsULong(in, base);
}
inline int			AsInt(PCSTR in, int base = 10) {
	return	(int)AsLong(in, base);
}

inline LONGLONG		AsLongLong(PCWSTR in) {
	return	_wtoi64(in);
}
inline ULONG		AsULong(PCWSTR in, int base = 10) {
	PWSTR	end_ptr;
	return	::wcstoul(in, &end_ptr, base);
}
inline long			AsLong(PCWSTR in, int base = 10) {
	PWSTR	end_ptr;
	return	::wcstol(in, &end_ptr, base);
}
inline UINT			AsUInt(PCWSTR in, int base = 10) {
	return	(unsigned int)AsULong(in, base);
}
inline int			AsInt(PCWSTR in, int base = 10) {
	return	(int)AsLong(in, base);
}

inline WCHAR		ToUpper(WCHAR in) {
	::CharUpperBuffW(&in, 1);
	return	in;
}
inline WCHAR 		ToLower(WCHAR in) {
	::CharLowerBuffW(&in, 1);
	return	in;
}
inline PWSTR		ToUpper(PWSTR buf, size_t len) {
	::CharUpperBuffW(buf, len);
	return	buf;
}
inline PWSTR		ToLower(PWSTR buf, size_t len) {
	::CharLowerBuffW(buf, len);
	return	buf;
}
inline PWSTR		ToUpper(PWSTR s1) {
	return	ToUpper(s1, Len(s1));
}
inline PWSTR		ToLower(PWSTR s1) {
	return	ToLower(s1, Len(s1));
}

inline PSTR			Fill(PSTR in, CHAR ch) {
	return	::_strset(in, ch);
}
inline PWSTR		Fill(PWSTR in, WCHAR ch) {
	return	::_wcsset(in, ch);
}

inline PSTR			Reverse(PSTR in) {
	return	::_strrev(in);
}
inline PWSTR		Reverse(PWSTR in) {
	return	::_wcsrev(in);
}

namespace	WinStr {
inline PWSTR		Assign(PCWSTR src) {
	size_t	len = Len(src) + 1;
	PWSTR	dest;
	WinMem::Alloc(dest, len * sizeof(WCHAR));
	Copy(dest, src, len);
	return	dest;
}
inline bool			Free(PCWSTR in) {
	return	WinMem::Free(in);
}
}

inline PSID			GetSid() {
	SID_IDENTIFIER_AUTHORITY NtAuthority = {SECURITY_NT_AUTHORITY};
	PSID	AdministratorsGroup = NULL;

	::AllocateAndInitializeSid(&NtAuthority, 2,
							   SECURITY_BUILTIN_DOMAIN_RID,
							   DOMAIN_ALIAS_RID_ADMINS,
							   0, 0, 0, 0, 0, 0,
							   &AdministratorsGroup);
	return	AdministratorsGroup;
}
inline bool			IsUserAdmin() {
	BOOL	Result = false;
	PSID	AdministratorsGroup = GetSid();
	if (AdministratorsGroup) {
		if (CheckTokenMembership(NULL, AdministratorsGroup, &Result)) {
			Result = true;
		}
		::FreeSid(AdministratorsGroup);
	}
	return	Result;
}

///======================================================================================= WinBuffer
/// Обертка буфера
template<typename Type>
class		WinBuf {
	Type*		m_buf;
	size_t		m_size;

	bool			Free() {
		return	WinMem::Free(m_buf);
	}
public:
	~WinBuf() {
		Free();
	}
	WinBuf(): m_buf(NULL), m_size(0) {
	}
	WinBuf(size_t size, bool bytes = false): m_buf(NULL), m_size((bytes) ? size : size * sizeof(Type)) {
		WinMem::Alloc(m_buf, m_size);
	}
	WinBuf(const WinBuf &in): m_buf(NULL), m_size(in.m_size) {
		if (WinMem::Alloc(m_buf, m_size)) {
			WinMem::Copy(m_buf, in.m_buf, m_size);
		}
	}
	const WinBuf	&operator=(const WinBuf & in) {
		if (this != &in) {
			Free();
			m_size = in.m_size;
			if (WinMem::Alloc(m_buf, m_size)) {
				WinMem::Copy(m_buf, in.m_buf, m_size);
			}
		}
		return	*this;
	}

	operator		Type*() {
		return	m_buf;
	}
	Type*			data() const {
		return	m_buf;
	}
	Type*			operator->() const {
		return	m_buf;
	}

	size_t			size() const {
		return	m_size;
	}
	size_t			capacity() const {
		return	m_size / sizeof(Type);
	}
	bool			reserve(size_t size, bool bytes = false) {
		if (!bytes) {
			size *= sizeof(Type);
		}
		if (m_size < size) {
			m_size = size;
			return	WinMem::Realloc(m_buf, m_size);
		}
		return	false;
	}
	void			zero() {
		WinMem::Zero(m_buf, m_size);
	}
};

///============================================================================================ CStr
/// Строки с счетчиком ссылок, передача по значению обходится очень дешево
inline size_t		Convert(PCSTR from, UINT cp, PWSTR to = NULL, size_t size = 0) {
	return	::MultiByteToWideChar(cp, 0, from, -1, to, (int)size);
}
inline size_t		Convert(PCWSTR from, UINT cp, PSTR to = NULL, size_t size = 0) {
	return	::WideCharToMultiByte(cp, 0, from, -1, to, (int)size, NULL, NULL);
}
UINT				CheckUnicode(const PVOID buf, size_t size);
UINT				IsUTF8(const PVOID buf, size_t size);
UINT				GetCP(HANDLE hFile, bool bUseHeuristics, bool &bSignatureFound);

#ifdef NoStlString
#include "win_autostr.h"
#else
#include "win_autoutf.h"
#endif

class		CStrMW {
	class	MzsData {
		PWSTR		m_data;
		size_t		m_capa;
		size_t		m_size;
	public:
		~MzsData() {
			delete[]	m_data;
		}
		explicit	MzsData(PCWSTR in);
		friend class CStrMW;
	};
	Shared_ptr<MzsData>	m_str;
public:
	CStrMW(PCWSTR in = L""): m_str(new MzsData(in)) {
	}
	const CStrMW	&operator=(const CStrMW &in) {
		m_str = in.m_str;
		return	*this;
	}

	size_t			size() const {
		return	m_str->m_size;
	}
	size_t			capacity() const {
		return	m_str->m_capa;
	}
	PCWSTR			c_str() const {
		return	m_str->m_data;
	}
	PCWSTR			operator[](int index) const;
};

inline AutoUTF		Err(HRESULT err = ::GetLastError(), PCWSTR lib = NULL) {
	HMODULE	mod = NULL;
	if (err != 0 && lib) {
		mod = ::LoadLibraryExW(lib, NULL, LOAD_LIBRARY_AS_DATAFILE);
	}
	PWSTR	buf = NULL;
	::FormatMessageW(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | ((mod) ?  FORMAT_MESSAGE_FROM_HMODULE : FORMAT_MESSAGE_FROM_SYSTEM),
		mod,
		err,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(PWSTR)&buf, 0, NULL);
	AutoUTF	Result((buf) ? buf : L"Unknown error\r\n");
	::LocalFree(buf);
	Result[(int)Result.size()-2] = L'\0';
	if (mod)
		::FreeLibrary(mod);
	return	Result;
}
inline AutoUTF		ErrWmi(HRESULT err) {
	return	Err(err, L"wmiutils.dll");
}

inline void			mbox(PCSTR text, PCSTR capt = "") {
	::MessageBoxA(NULL, text, capt, MB_OK);
}
inline void			mbox(PCWSTR text, PCWSTR capt = L"") {
	::MessageBoxW(NULL, text, capt, MB_OK);
}
inline void			mbox(HRESULT err, PCWSTR lib = NULL) {
	::MessageBoxW(NULL, Err(err, lib).c_str(), L"Error", MB_OK);
}

inline CStrA		oem(PCWSTR in) {
	return	w2cp(in, CP_OEMCP);
}
inline CStrA		oem(const AutoUTF &in) {
	return	w2cp(in.c_str(), CP_OEMCP);
}
inline CStrA		utf8(PCWSTR in) {
	return	w2cp(in, CP_UTF8);
}
inline CStrA		utf8(const AutoUTF &in) {
	return	w2cp(in.c_str(), CP_UTF8);
}
inline AutoUTF		utf16(PCSTR in) {
	return	cp2w(in, CP_UTF16le);
}
inline AutoUTF		utf16(const CStrA &in) {
	return	cp2w(in.c_str(), CP_UTF16le);
}
inline AutoUTF		u2w(const CStrA &in) {
	return	cp2w(in.c_str(), CP_UTF8);
}

inline void			Num2Str(PWSTR str, ssize_t in, int base = 10) {
	::_i64tow(in, str, base);
}
inline AutoUTF		Num2Str(int in, int base = 10) {
	WCHAR	buf[32];
	Num2Str(buf, in, base);
	return	buf;
}
inline AutoUTF		Num2Str(ssize_t in, int base = 10) {
	WCHAR	buf[64];
	Num2Str(buf, in, base);
	return	buf;
}
inline AutoUTF		Num2Str(size_t in, int base = 10) {
	WCHAR	buf[64];
	Num2Str(buf, in, base);
	return	buf;
}

CStrA				Hash2Str(PBYTE buf, size_t size);
CStrA				Hash2StrNum(PBYTE buf, size_t size);
bool				Str2Hash(const CStrA &str, PVOID &hash, ULONG &size);

AutoUTF&			ReplaceAll(AutoUTF& str, const AutoUTF &from, const AutoUTF &to);
AutoUTF				ReplaceAllOut(const AutoUTF& str, const AutoUTF &from, const AutoUTF &to);

///===================================================================================== Console out
int					consoleout(WCHAR in, DWORD nStdHandle = STD_OUTPUT_HANDLE);
int					consoleout(PCWSTR in, DWORD nStdHandle = STD_OUTPUT_HANDLE);
int					consoleout(PCSTR in, DWORD nStdHandle = STD_OUTPUT_HANDLE);
int					consoleout(const AutoUTF &in, DWORD nStdHandle = STD_OUTPUT_HANDLE/*STD_ERROR_HANDLE*/);
int					printf(PCWSTR format, ...);

enum	WinLogLevel {
	LOG_TRACE =	-3,
	LOG_DEBUG,
	LOG_VERBOSE,
	LOG_INFO,
	LOG_ERROR,
};

extern int			logLevel;
void				setLogLevel(WinLogLevel lvl);
void				logDebug(PCWSTR message, ...);
void				logVerbose(PCWSTR message, ...);
void				logCounter(PCWSTR message, ...);
void				logInfo(PCWSTR message, ...);
void				logFile(WIN32_FIND_DATA FileData);
void				logError(PCWSTR message, ...);

///========================================================================================== WinEnv
namespace	WinEnv {
AutoUTF		Get(PCWSTR name);
bool		Set(PCWSTR name, PCWSTR val);
bool		Add(PCWSTR name, PCWSTR val);
bool		Del(PCWSTR name);
}

///============================================================================================ Exec
extern DWORD		EXEC_TIMEOUT;
extern DWORD		EXEC_TIMEOUT_DX;
bool				Exec(const AutoUTF &cmd);
int					Exec(const AutoUTF &cmd, CStrA &out);
int					Exec(const AutoUTF &cmd, CStrA &out, const CStrA &in);
int					ExecWait(const AutoUTF &cmd, DWORD wait = EXEC_TIMEOUT);

///============================================================================================ path
inline AutoUTF		Canonicalize(PCWSTR path) {
	WCHAR	Result[MAX_PATH_LENGTH];
	::PathCanonicalizeW(Result, path);
	return	Result;
}
inline AutoUTF		Canonicalize(const AutoUTF &path) {
	return	Canonicalize(path.c_str());
}
inline AutoUTF		Expand(PCWSTR path) {
	DWORD	size = ::ExpandEnvironmentStringsW(path, NULL, 0);
	if (size) {
		WCHAR	Result[::ExpandEnvironmentStringsW(path, NULL, 0)];
		if (::ExpandEnvironmentStringsW(path, Result, size))
			return	Result;
	}
	return	AutoUTF();
}
inline AutoUTF		Expand(const AutoUTF &path) {
	return	Expand(path.c_str());
}
inline AutoUTF		PathNice(PCWSTR path) {
	return	Canonicalize(Expand(path));
}
inline AutoUTF		PathNice(const AutoUTF &path) {
	return	PathNice(path.c_str());
}

AutoUTF				Secure(PCWSTR path);
AutoUTF				Secure(const AutoUTF &path);
AutoUTF				UnExpand(PCWSTR path);
AutoUTF				UnExpand(const AutoUTF &path);
AutoUTF				Validate(PCWSTR path);
AutoUTF				Validate(const AutoUTF &path);

inline AutoUTF		SlashAdd(PCWSTR path, WCHAR add = PATH_SEPARATOR_C) {
	AutoUTF	Result(path);
	if (!Empty(path) && Result[Result.size() - 1] != L'\\')
		Result += add;
	return	Result;
}
inline AutoUTF		SlashAdd(const AutoUTF &path, WCHAR add = PATH_SEPARATOR_C) {
	return	SlashAdd(path.c_str());
}
AutoUTF				SlashAddNec(PCWSTR path);
AutoUTF				SlashAddNec(const AutoUTF &path);
AutoUTF				SlashDel(PCWSTR path);
AutoUTF				SlashDel(const AutoUTF &path);

bool				IsPathMask(PCWSTR path);
bool				IsPathMask(const AutoUTF &path);
bool				IsPathUnix(PCWSTR path);
bool				IsPathUnix(const AutoUTF &path);

AutoUTF				ExtractFile(PCWSTR path, WCHAR sep = PATH_SEPARATOR_C);
AutoUTF				ExtractFile(const AutoUTF &path, WCHAR sep = PATH_SEPARATOR_C);
AutoUTF				ExtractPath(PCWSTR path, WCHAR sep = PATH_SEPARATOR_C);
AutoUTF				ExtractPath(const AutoUTF &path, WCHAR sep = PATH_SEPARATOR_C);
AutoUTF				GetPathFromMask(PCWSTR mask);
AutoUTF				GetPathFromMask(const AutoUTF &mask);
AutoUTF				GetSpecialPath(int csidl, bool create = true);

inline AutoUTF		MakePath(PCWSTR path, PCWSTR name) {
	AutoUTF	Result(SlashAdd(PathNice(path)));
	Result += name;
	return	Result;
}
inline AutoUTF		MakePath(const AutoUTF &path, const AutoUTF &name) {
	return	MakePath(path.c_str(), name.c_str());
}
AutoUTF				PathUnix(PCWSTR path);
AutoUTF				PathUnix(const AutoUTF &path);
AutoUTF				PathWin(PCWSTR path);
AutoUTF				PathWin(const AutoUTF &path);

AutoUTF				GetWorkDirectory();
bool				SetWorkDirectory(PCWSTR path);
bool				SetWorkDirectory(const AutoUTF &path);

///========================================================================================= SysPath
namespace	SysPath {
AutoUTF				Winnt();
AutoUTF				Sys32();
AutoUTF				SysNative();
AutoUTF				InetSrv();
AutoUTF				Dns();

AutoUTF				Users();
AutoUTF				UsersRoot();
AutoUTF				FtpRoot();

AutoUTF				Temp();

#ifdef ISPMGR
AutoUTF				OpenSsl();
AutoUTF				OpenSslConf();
AutoUTF				UserHome(const AutoUTF &name);
AutoUTF				FtpUserHome(const AutoUTF &name);
#endif
}

///========================================================================================== SysApp
namespace	SysApp {
AutoUTF				appcmd();
AutoUTF				dnscmd();
#ifdef ISPMGR
AutoUTF				openssl();
AutoUTF				openssl_conf();
#endif
}

///===================================================================================== File system
/// Работа с файловой системой (неокончено)
typedef		WIN32_FILE_ATTRIBUTE_DATA	FileInfo;
class		WinFilePos {
	LARGE_INTEGER	m_pos;
public:
	WinFilePos(LARGE_INTEGER pos): m_pos(pos) {
	}
	WinFilePos(LONGLONG pos = 0LL) {
		m_pos.QuadPart = pos;
	}
	operator		LONGLONG() const {
		return	m_pos.QuadPart;
	}
	operator		LARGE_INTEGER() const {
		return	m_pos;
	}
	operator		PLARGE_INTEGER() const {
		return	(PLARGE_INTEGER)&m_pos;
	}
};
inline bool			FileValidName(PCWSTR path) {
	return	!(Eq(path, L".") || Eq(path, L"..") || Eq(path, L"..."));
}

inline bool			FileOpenRead(PCWSTR	path, HANDLE &hFile) {
	hFile = ::CreateFileW(path, FILE_READ_DATA, FILE_SHARE_DELETE | FILE_SHARE_READ, NULL, OPEN_EXISTING,
						  FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS, NULL);
	return	hFile && hFile != INVALID_HANDLE_VALUE;
}
inline bool			FileOpenAttr(PCWSTR	path, HANDLE &hFile) {
	hFile = ::CreateFileW(path, FILE_READ_ATTRIBUTES, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING,
						  FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS, NULL);
	return	hFile && hFile != INVALID_HANDLE_VALUE;
}
inline bool			FileClose(HANDLE hFile) {
	return	::CloseHandle(hFile) != 0;
}

inline AutoUTF		TempDir() {
	WCHAR	buf[::GetTempPath(0, NULL)];
	::GetTempPathW(sizeofa(buf), buf);
	return	buf;
}
inline AutoUTF		TempFile(PCWSTR s) {
	WCHAR	buf[MAX_PATH];
	::GetTempFileNameW(TempDir().c_str(), s, 0, buf);
	return	buf;
}

inline bool			HardLink(PCWSTR path, PCWSTR newfile) {
	return	::CreateHardLinkW(newfile, path, NULL) != 0;
}
inline size_t 		NumberOfLinks(PCWSTR path) {
	size_t	Result = 0;
	HANDLE	hFile;
	if (FileOpenAttr(path, hFile)) {
		BY_HANDLE_FILE_INFORMATION	info;
		if (::GetFileInformationByHandle(hFile, &info)) {
			Result = info.nNumberOfLinks;
		}
		FileClose(hFile);
	}
	return	Result;
}
inline size_t 		FileInode(PCWSTR path) {
	size_t	Result = 0;
	HANDLE	hFile;
	if (FileOpenAttr(path, hFile)) {
		BY_HANDLE_FILE_INFORMATION	info;
		if (::GetFileInformationByHandle(hFile, &info)) {
			Result = info.nNumberOfLinks;
		}
		FileClose(hFile);
	}
	return	Result;
}

inline DWORD		Attributes(PCWSTR path) {
	return	::GetFileAttributesW(path);
}
inline bool			Attributes(PCWSTR path, DWORD attr) {
	return	::SetFileAttributesW(path, attr) != 0;
}

inline FileInfo		FileGetInfo(PCWSTR	path) {
	FileInfo	info;
	::GetFileAttributesExW(path, GetFileExInfoStandard, &info);
	return	info;
}
inline uint64_t		FileSize(PCWSTR	path) {
	FileInfo	info;
	::GetFileAttributesExW(path, GetFileExInfoStandard, &info);
	return	((uint64_t)info.nFileSizeHigh) << 32 | info.nFileSizeLow;
}
inline uint64_t		FileSize(HANDLE	hFile) {
	LARGE_INTEGER	size;
	::GetFileSizeEx(hFile, &size);
	return	size.QuadPart;
}
inline bool			FilePos(HANDLE hFile, const WinFilePos &pos, DWORD m = FILE_BEGIN) {
	return	::SetFilePointerEx(hFile, pos, NULL, m) != 0;
}
inline LONGLONG		FilePos(HANDLE hFile) {
	WinFilePos	pos;
	return	::SetFilePointerEx(hFile, WinFilePos(0LL), pos, FILE_CURRENT);
}

inline FILETIME		FileTimeCr(PCWSTR path) {
	FileInfo	info;
	::GetFileAttributesExW(path, GetFileExInfoStandard, &info);
	return	info.ftCreationTime;
}
inline FILETIME		FileTimeAc(PCWSTR path) {
	FileInfo	info;
	::GetFileAttributesExW(path, GetFileExInfoStandard, &info);
	return	info.ftLastAccessTime;
}
inline FILETIME		FileTimeWr(PCWSTR path) {
	FileInfo	info;
	::GetFileAttributesExW(path, GetFileExInfoStandard, &info);
	return	info.ftLastWriteTime;
}

inline bool			IsExist(PCWSTR path) {
	return	(::GetFileAttributesW(path) != INVALID_FILE_ATTRIBUTES) || (::GetLastError() != ERROR_FILE_NOT_FOUND);
}
inline bool			IsExist(const AutoUTF &path) {
	return	IsExist(path.c_str());
}
inline bool			IsDir(PCWSTR path) {
	DWORD	Attr = Attributes(path);
	return	(Attr != INVALID_FILE_ATTRIBUTES) && (Attr & FILE_ATTRIBUTE_DIRECTORY);
}
inline bool			IsDir(const AutoUTF &path) {
	return	IsDir(path.c_str());
}
bool				IsDirEmpty(PCWSTR path);
inline bool			IsDirEmpty(const AutoUTF &path) {
	return	IsDirEmpty(path.c_str());
}

inline bool			IsJunc(PCWSTR path) {
	return	(::GetFileAttributesW(path) & FILE_ATTRIBUTE_REPARSE_POINT) != 0;
}

bool				DirCreate(PCWSTR path);
inline bool			DirCreate(const AutoUTF &path) {
	return	DirCreate(path.c_str());
}
bool				FileCreate(PCWSTR path, PCWSTR name, PCSTR content);
inline bool			FileCreate(const AutoUTF &path, const AutoUTF &name, PCSTR content) {
	return	FileCreate(path.c_str(), name.c_str(), content);
}
bool				DelDir(PCWSTR path);
inline bool			DelDir(const AutoUTF &path) {
	return	DelDir(path.c_str());
}
inline bool			DelFile(PCWSTR path) {
	return	(::DeleteFileW(path) != 0) ?
		   true :
		   (::SetFileAttributesW(path, FILE_ATTRIBUTE_NORMAL) ||
			::DeleteFileW(path));
}
inline bool			DelFile(const AutoUTF &path) {
	return	DelFile(path.c_str());
}
bool				Del2(PCWSTR path);
bool				Recycle(PCWSTR path);
inline bool			Recycle(const AutoUTF &path) {
	return	Recycle(path.c_str());
}

inline bool			Copy(PCWSTR path, PCWSTR dest) {
	return	::CopyFileW(path, dest, true) != 0;
}
inline bool			Copy(const AutoUTF &path, const AutoUTF &dest) {
	return	Copy(path.c_str(), dest.c_str());
}
inline bool			Move(PCWSTR path, PCWSTR dest) {
	return	::MoveFileW(path, dest) != 0;
}
inline bool			Move(const AutoUTF &path, const AutoUTF &dest) {
	return	Move(path.c_str(), dest.c_str());
}
inline bool			Move(PCWSTR path, PCWSTR dest, bool copy) {
	return	::MoveFileExW(path, dest, (copy) ? MOVEFILE_COPY_ALLOWED : 0) != 0;
}
inline bool			Move(const AutoUTF &path, const AutoUTF &dest, bool copy) {
	return	Move(path.c_str(), dest.c_str(), copy);
}

inline bool			FileRead(HANDLE hFile, PBYTE buf, DWORD &size) {
	return	::ReadFile(hFile, buf, size, &size, NULL) != 0;
}
bool				FileRead(PCWSTR	path, CStrA &buf);
bool				FileWrite(PCWSTR path, PCVOID buf, size_t size, bool rewrite = false);

AutoUTF				GetDrives();

class		WinFileId {
	DWORD	m_vol_sn;
	DWORD	m_node_low;
	DWORD	m_node_high;
	DWORD	m_links;
public:
	WinFileId(): m_vol_sn(0), m_node_low(0), m_node_high(0), m_links(0)  {
	}
	WinFileId(const BY_HANDLE_FILE_INFORMATION &info):
			m_vol_sn(info.dwVolumeSerialNumber),
			m_node_low(info.nFileIndexLow),
			m_node_high(info.nFileIndexHigh),
			m_links(info.nNumberOfLinks)  {

	}
	WinFileId(HANDLE hFile): m_vol_sn(0), m_node_low(0), m_node_high(0), m_links(0) {
		Load(hFile);
	}
	WinFileId(PCWSTR path): m_vol_sn(0), m_node_low(0), m_node_high(0), m_links(0)  {
		HANDLE	hFile = ::CreateFileW(path, 0, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING,
									 FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS, NULL);
		Load(hFile);
		::CloseHandle(hFile);
	}

	bool			Load(PCWSTR path) {
		HANDLE	hFile;
		if (FileOpenAttr(path, hFile)) {
			bool	Result = Load(hFile);
			::CloseHandle(hFile);
			return	Result;
		}
		return	false;
	}
	bool			Load(HANDLE hFile) {
		if (!m_links) {
			BY_HANDLE_FILE_INFORMATION	info;
			if (::GetFileInformationByHandle(hFile, &info) && (info.dwVolumeSerialNumber != 0 || info.nFileIndexLow != 0 || info.nFileIndexHigh != 0)) {
				m_vol_sn = info.dwVolumeSerialNumber;
				m_node_low = info.nFileIndexLow;
				m_node_high = info.nFileIndexHigh;
				m_links = info.nNumberOfLinks;
				return	true;
			}
		}
		return	false;
	}

	bool			operator==(const WinFileId &in) const {
		return	(m_vol_sn == in.m_vol_sn) && (m_node_low == in.m_node_low) && (m_node_high == in.m_node_high);
	}
	bool			operator!=(const WinFileId &in) const {
		return	!operator==(in);
	}
	bool			IsOK() {
		return	m_links;
	}

	DWORD			vol_sn() const {
		return	m_vol_sn;
	}
	DWORD			links() const {
		return	m_links;
	}
	DWORD			node_low() const {
		return	m_node_low;
	}
	DWORD			node_high() const {
		return	m_node_high;
	}
	uint64_t		node() const {
		ULARGE_INTEGER	Result;
		Result.LowPart = m_node_low;
		Result.HighPart = m_node_high;
		Result.QuadPart &= 0x0000FFFFFFFFFFFFULL;
		return	Result.QuadPart;
	}
};

///========================================================================================== WinDir
class		WinDir : public WinErrorCheck, private Uncopyable {
	WIN32_FIND_DATAW	m_find;
	HANDLE				m_handle;
	AutoUTF				m_path;
	AutoUTF				m_mask;

	void			Close() {
		if (m_handle && m_handle != INVALID_HANDLE_VALUE) {
			::FindClose(m_handle);
			m_handle = NULL;
		}
	}
public:
	~WinDir() {
		Close();
	}
	WinDir(const AutoUTF &path, const AutoUTF &mask = L"*"): m_handle(NULL), m_path(path), m_mask(mask) {
	}

	bool 			Next() {
		AutoUTF	tmp(SlashAdd(m_path));
		tmp += m_mask;
		if (m_handle == NULL) {
			m_handle = ::FindFirstFileW(tmp.c_str(), &m_find);
			ChkSucc(m_handle != INVALID_HANDLE_VALUE);
		} else {
			ChkSucc(::FindNextFileW(m_handle, &m_find) != 0);
		}
		if (IsOK() && !FileValidName(m_find.cFileName))
			return	Next();
		return	IsOK();
	}

	void			path(const AutoUTF &in) {
		Close();
		m_path = in;
	}
	void			mask(const AutoUTF &in) {
		Close();
		m_mask = in;
	}

	bool			IsDir() const {
		return	m_find.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
	}
	bool			IsJunc() const {
		return	m_find.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT;
	}
	DWORD			attr() const {
		return	m_find.dwFileAttributes;
	}
	FILETIME		time_cr() const {
		return	m_find.ftCreationTime;
	}
	FILETIME		time_ac() const {
		return	m_find.ftLastAccessTime;
	}
	FILETIME		time_wr() const {
		return	m_find.ftLastWriteTime;
	}
	PCWSTR			name() const {
		return	m_find.cFileName;
	}
	AutoUTF			name_full() const {
		AutoUTF	Result(SlashAdd(m_path));
		Result += m_find.cFileName;
		return	Result;
	}
	PCWSTR			name_dos() const {
		return	m_find.cAlternateFileName;
	}
	PCWSTR			path() const {
		return	m_path.c_str();
	}
	uint64_t		size() const {
		return	MyUI64(m_find.nFileSizeLow, m_find.nFileSizeHigh);
	}
};

///========================================================================================= FileMap
/// Отображение файла в память блоками
class		FileMap : private Uncopyable, public WinErrorCheck {
	HANDLE		m_hFile;
	HANDLE		m_hSect;
	PVOID		m_data;
	uintmax_t	m_sizefull;
	uint64_t	m_offset;
	size_t		m_binb;
	bool		m_write;
public:
	~FileMap() {
		Close();
	}
	FileMap(): m_hFile(NULL), m_hSect(NULL), m_data(NULL), m_sizefull(0), m_offset(0) {
		err(ERROR_FILE_NOT_FOUND);
	}
	FileMap(PCWSTR in, bool write = false): m_hFile(NULL), m_hSect(NULL), m_data(NULL), m_sizefull(0), m_offset(0) {
		Open(in, write);
	}

	bool			Close() {
		if (m_data) {
			::UnmapViewOfFile(m_data);
			m_data = NULL;
		}
		if (m_hSect) {
			::CloseHandle(m_hSect);
			m_hSect = NULL;
		}
		if (m_hFile) {
			::CloseHandle(m_hFile);
			m_hFile = NULL;
			return	true;
		}
		return	false;
	}
	bool			Open(PCWSTR in, bool write) {
		m_write	= write;
		ACCESS_MASK	amask = (m_write) ? GENERIC_READ | GENERIC_WRITE : GENERIC_READ;
		DWORD		share = (m_write) ? 0 : FILE_SHARE_DELETE | FILE_SHARE_READ;
		DWORD		creat = (m_write) ? OPEN_EXISTING : OPEN_EXISTING;
		DWORD		flags = (m_write) ? FILE_ATTRIBUTE_NORMAL : FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS;
		Close();
		Home();
		m_hFile = ::CreateFileW(in, amask, share, NULL, creat, flags, NULL);
		if (m_hFile && m_hFile != INVALID_HANDLE_VALUE) {
			m_sizefull = FileSize(m_hFile);
			amask = (m_write) ? PAGE_READWRITE : PAGE_READONLY;
			m_hSect = ::CreateFileMapping(m_hFile, NULL, amask, 0, 0, NULL);
		}
		return	ChkSucc(m_hSect != NULL);
	}
	bool			Create(PCWSTR in, LONGLONG size) {
		m_write	= true;
		ACCESS_MASK	amask = GENERIC_READ | GENERIC_WRITE;
		DWORD		share = 0;
		DWORD		creat = CREATE_ALWAYS;
		DWORD		flags = FILE_ATTRIBUTE_NORMAL;
		Close();
		Home();
		m_hFile = ::CreateFileW(in, amask, share, NULL, creat, flags, NULL);
		if (m_hFile && m_hFile != INVALID_HANDLE_VALUE) {
			m_sizefull = size;
			LONG dwHigh = (m_sizefull >> 32);
			::SetFilePointer(m_hFile, (LONG)(m_sizefull & 0xFFFFFFFF), &dwHigh, FILE_BEGIN);
			::SetEndOfFile(m_hFile);
			amask = (m_write) ? PAGE_READWRITE : PAGE_READONLY;
			m_hSect = ::CreateFileMapping(m_hFile, NULL, amask, 0, 0, NULL);
		}
		return	ChkSucc(m_hSect != NULL);
	}

	bool			Next() {
		if (m_data) {
			::UnmapViewOfFile(m_data);
			m_data = NULL;
		}
		if ((m_sizefull - m_offset) > 0) {
			if ((m_sizefull - m_offset) < (uint64_t)m_binb)
				m_binb = (size_t)(m_sizefull - m_offset);
			if (m_hSect) {
				ACCESS_MASK	amask = (m_write) ? FILE_MAP_WRITE : FILE_MAP_READ;
				m_data = ::MapViewOfFile(m_hSect, amask, (DWORD)(m_offset >> 32), (DWORD)(m_offset & 0xFFFFFFFF), m_binb);
				m_offset += m_binb;
				return	ChkSucc(m_data != NULL);
			}
		}
		return	false;
	}
	bool			Home() {
		SYSTEM_INFO	info;
		::GetSystemInfo(&info);
		m_binb = info.dwAllocationGranularity * 1024;
		m_offset = 0LL;
		return	true;
	}

	uintmax_t		sizefile() const {
		return	m_sizefull;
	}
	size_t			size() const {
		return	m_binb;
	}
	PVOID			data() const {
		return	m_data;
	}
	uintmax_t		Size() const {
		return	m_sizefull;
	}
};

///========================================================================================= WinGUID
namespace	WinGUID {
AutoUTF			Gen();
}

///========================================================================================= WinPriv
/// Функции работы с привилегиями
namespace	WinPriv {
bool 			IsExist(HANDLE hToken, LUID priv);
bool 			IsExist(HANDLE hToken, PCWSTR sPriv);
bool 			IsExist(HANDLE hToken, PCSTR sPriv);

bool			IsEnabled(HANDLE hToken, LUID priv);
bool 			IsEnabled(HANDLE hToken, PCWSTR sPriv);
bool 			IsEnabled(HANDLE hToken, PCSTR sPriv);

bool 			Modify(HANDLE hToken, LUID priv, bool bEnable);
bool 			Modify(HANDLE hToken, PCWSTR sPriv, bool bEnable);
bool 			Modify(HANDLE hToken, PCSTR	sPriv, bool bEnable);
}

///========================================================================================= WinProc
/// Обертка хэндла процесса
class		WinProcess: private Uncopyable, public WinErrorCheck {
	HANDLE	m_hndl;
public:
	~WinProcess() {
		::CloseHandle(m_hndl);
	}
	WinProcess() {
		m_hndl = ::GetCurrentProcess();
	}
	WinProcess(ACCESS_MASK mask, DWORD pid): m_hndl(NULL) {
		m_hndl = ::OpenProcess(mask, false, pid);
		ChkSucc(m_hndl != NULL);
	}
	operator		HANDLE() const {
		return	m_hndl;
	}
	DWORD			GetId() const {
		return	::GetProcessId(m_hndl);
	}

// static
	static	DWORD	Id() {
		return	::GetCurrentProcessId();
	}
	static	DWORD	Id(HANDLE hProc) {
		return	::GetProcessId(hProc);
	}
	static	AutoUTF	User();
	static	AutoUTF	FullPath();
	static	AutoUTF	CmdLine() {
		return	::GetCommandLineW();
	}
};

///======================================================================================== WinToken
/// Обертка токена
class		WinToken: private Uncopyable, public WinErrorCheck {
	HANDLE	m_handle;
public:
	~WinToken() {
		::CloseHandle(m_handle);
	}
	WinToken(ACCESS_MASK mask = TOKEN_ALL_ACCESS): m_handle(NULL) {
		ChkSucc(::OpenProcessToken(WinProcess(), mask, &m_handle) != 0);
	}
	WinToken(ACCESS_MASK mask, HANDLE hProcess): m_handle(NULL) {
		ChkSucc(::OpenProcessToken(hProcess, mask, &m_handle) != 0);
	}
	operator		HANDLE() const {
		return	m_handle;
	}
};

///======================================================================================== WinTimer
/// Оконный таймер
class		WinTimer {
	HANDLE			hTimer;
	LARGE_INTEGER	liUTC;
	long			lPeriod;

	void			Open();
	void			Close();
public:
	~WinTimer();
	WinTimer();
	WinTimer(LONGLONG time, long period = 0);
	void			Set(LONGLONG time, long period = 0);
	void			Start();
	void			Stop();
	void			StartTimer();
	operator		HANDLE() const {
		return	hTimer;
	}
};

///========================================================================================== WinReg
class		WinReg {
	HKEY	mutable	hKeyOpend;
	HKEY			hKeyReq;
	AutoUTF			m_path;

	void			CloseKey() const;
	bool			OpenKey(ACCESS_MASK acc) const {
		return	OpenKey(hKeyReq, m_path, acc);
	}
	bool			OpenKey(HKEY hkey, const AutoUTF &path, ACCESS_MASK acc) const;

	template <typename Type>
	void			SetRaw(const AutoUTF &name, const Type &value, DWORD type = REG_BINARY) const {
		if (OpenKey(KEY_WRITE)) {
			::RegSetValueExW(hKeyOpend, name.c_str(), NULL, type, (PBYTE)(&value), sizeof(value));
			CloseKey();
		}
	}
	template <typename Type>
	bool			GetRaw(const AutoUTF &name, Type &value, const Type &def) const {
		bool	Result = OpenKey(KEY_READ);
		value = def;
		if (Result) {
			DWORD	size = sizeof(value);
			Result = ::RegQueryValueExW(hKeyOpend, name.c_str(), NULL, NULL, (PBYTE)(&value), &size) == ERROR_SUCCESS;
			CloseKey();
		}
		return	Result;
	}
public:
	~WinReg() {
		CloseKey();
	}
	WinReg(): hKeyOpend(0), hKeyReq(HKEY_CURRENT_USER) {
	}
	WinReg(HKEY hkey, const AutoUTF &path): hKeyOpend(0), hKeyReq(hkey), m_path(path) {
	}
	WinReg(const AutoUTF &path);

	AutoUTF			path() const {
		return	m_path;
	}
	void			path(const AutoUTF &path) {
		m_path = path;
	}
	void			key(HKEY hkey) {
		hKeyReq = hkey;
	}

	bool			Add(const AutoUTF &name) const;
	bool			Del(const AutoUTF &name) const;

	void			Set(const AutoUTF &name, PCWSTR value) const;
	void			Set(const AutoUTF &name, int value) const;

	bool			Get(const AutoUTF &name, AutoUTF &value, const AutoUTF &def) const;
	bool			Get(const AutoUTF &name, int &value, int def) const;
};

///============================================================================================= Sid
/// Security Identifier (Идентификатор безопасности) -
/// структура данных переменной длины, которая идентифицирует учетную запись пользователя, группы,
/// домена или компьютера
#ifndef PSIDFromPACE
#define PSIDFromPACE(pACE)((PSID)(&((pACE)->SidStart)))
#endif

class		Sid : private Uncopyable {
	PSID	pSID;

	Sid();
	void				Copy(PSID in);
	void				Free(PSID &in);
public:
	~Sid() {
		Free(pSID);
	}
//	Sid(WELL_KNOWN_SID_TYPE	wns);
	Sid(PCWSTR sSID);
	Sid(PCWSTR name, PCWSTR dom);
	Sid(const AutoUTF &sSID);
	Sid(const AutoUTF &name, const AutoUTF &dom);
	explicit			Sid(PSID in): pSID(NULL) {
		Copy(in);
	}
	bool				IsOK() const {
		return		Valid(pSID);
	}

	operator			const PSID() {
		return	pSID;
	}
	PSID				Data() const {
		return	pSID;
	}

	size_t				Size() const {
		return	pSID ? Size(pSID) : 0;
	}
	AutoUTF				AsStr() const {
		return	AsStr(pSID);
	}
	AutoUTF				AsName() const {
		return	AsName(pSID);
	}
	AutoUTF				AsDom() const {
		return	AsDom(pSID);
	}

// static
	static bool			Valid(PSID in) {
		return	::IsValidSid(in);
	}
	static size_t		Size(PSID in) {
		return	::GetLengthSid(in);
	}
	static size_t		SizeCounter(size_t &cnt, PSID in) {
		return	cnt += Size(in);
	}
	static size_t		SizeCounter(size_t &cnt, const AutoUTF &name, const AutoUTF &dom = L"") {
		Sid		sid(name, dom);
		return	cnt += sid.Size();
	}

	// PSID to sid string
	static AutoUTF		AsStr(PSID	in);

	// name to sid string
	static AutoUTF		AsStr(const AutoUTF &name, const AutoUTF &dom = L"");

	// PSID to name
	static AutoUTF		AsName(PSID pSID);
	static AutoUTF		AsDom(PSID pSID);
	static DWORD		AsName(PSID pSID, AutoUTF &name, AutoUTF &dom);

	// Sid string to name
	static AutoUTF		AsName(const AutoUTF &sSID);
	static AutoUTF		AsDom(const AutoUTF &sSID);
	static DWORD		AsName(const AutoUTF &sSID, AutoUTF &name, AutoUTF &dom);

// WELL KNOWN SIDS
	static PCWSTR		SID_NOBODY;				// NULL SID
	static PCWSTR		SID_LOCAL;				// ЛОКАЛЬНЫЕ
	static PCWSTR		SID_EVERIONE;			// Все
	static PCWSTR		SID_CREATOR_OWNER;		// СОЗДАТЕЛЬ-ВЛАДЕЛЕЦ
	static PCWSTR		SID_CREATOR_GROUP;		// ГРУППА-СОЗДАТЕЛЬ
	static PCWSTR		SID_CREATOR_OWNER_S;	// СОЗДАТЕЛЬ-ВЛАДЕЛЕЦ СЕРВЕР
	static PCWSTR		SID_CREATOR_GROUP_S;	// ГРУППА-СОЗДАТЕЛЬ СЕРВЕР
	static PCWSTR		SID_DIALUP;				// УДАЛЕННЫЙ ДОСТУП
	static PCWSTR		SID_NETWORK;			// СЕТЬ
	static PCWSTR		SID_BATCH;				// ПАКЕТНЫЕ ФАЙЛЫ
	static PCWSTR		SID_SELF;				// SELF
	static PCWSTR		SID_AUTH_USERS;			// Прошедшие проверку
	static PCWSTR		SID_RESTRICTED;			// ОГРАНИЧЕННЫЕ
	static PCWSTR		SID_TS_USERS;			// ПОЛЬЗОВАТЕЛЬ СЕРВЕРА ТЕРМИНАЛОВ
	static PCWSTR		SID_RIL;				// REMOTE INTERACTIVE LOGON
	static PCWSTR		SID_LOCAL_SYSTEM;		// SYSTEM
	static PCWSTR		SID_LOCAL_SERVICE;		// LOCAL SERVICE
	static PCWSTR		SID_NETWORK_SERVICE;	// NETWORK SERVICE
	static PCWSTR		SID_ADMINS;				// Администраторы
	static PCWSTR		SID_USERS;				// Пользователи
	static PCWSTR		SID_GUESTS;				// Гости
	static PCWSTR		SID_POWER_USERS;		// Опытные пользователи
	static PCWSTR		SID_ACCOUNT_OPERATORS;
	static PCWSTR		SID_SERVER_OPERATORS;
	static PCWSTR		SID_PRINT_OPERATORS;
	static PCWSTR		SID_BACKUP_OPERATORS;	// Операторы архива
	static PCWSTR		SID_REPLICATORS;		// Репликатор
	static PCWSTR		SID_REMOTE_DESKTOP;		// Пользователи удаленного рабочего стола
	static PCWSTR		SID_NETWORK_OPERATOR;	// Операторы настройки сети
	static PCWSTR		SID_IIS;
	static PCWSTR		SID_INTERACTIVE;		// ИНТЕРАКТИВНЫЕ
	static PCWSTR		SID_SERVICE;			// СЛУЖБА
	static PCWSTR		SID_ANONYMOUS;			// АНОНИМНЫЙ ВХОД
	static PCWSTR		SID_PROXY;				// PROXY
};

///=========================================================================================== Win64
/// Функции работы с WOW64
namespace	Win64 {
bool		WowDisable(PVOID &oldValue);
bool		WowEnable(PVOID &oldValue);
bool		IsWOW64();
}

///====================================================================================== WinSysInfo
struct		WinSysInfo: public SYSTEM_INFO {
	WinSysInfo();
	size_t		Uptime(size_t del = 1000);
};

///========================================================================================= WinPerf
struct		WinPerf: public PERFORMANCE_INFORMATION {
	WinPerf();
};

///=========================================================================================== DEBUG
#ifdef DEBUG
#include <iostream>
#include <ostream>
using	std::cout;
using	std::ostream;
using	std::endl;

inline ostream		&operator<<(ostream &s, PCWSTR rhs) {
	return	s << oem(rhs).c_str();
}
inline ostream		&operator<<(ostream &s, const AutoUTF &rhs) {
	return	s << oem(rhs).c_str();
}
#endif

#endif // WIN_DEF_HPP
