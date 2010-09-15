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

#include <psapi.h>
#include <ntsecapi.h>

EXTERN_C {
	WINBASEAPI ULONGLONG WINAPI		GetTickCount64();
//	WINBASEAPI VOID WINAPI			GetNativeSystemInfo(LPSYSTEM_INFO);
//	WINBASEAPI DWORD WINAPI			GetProcessId(HANDLE Process);

	_CRTIMP int __cdecl				_snwprintf(wchar_t*, size_t, const wchar_t*, ...);
	long long __MINGW_NOTHROW		wcstoll(const wchar_t * __restrict__, wchar_t** __restrict__, int);
	unsigned long long __MINGW_NOTHROW wcstoull(const wchar_t * __restrict__, wchar_t ** __restrict__, int);
}

///===================================================================================== definitions
#define null_ptr				0
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

///=================================================================================================
inline void			XchgByte(WORD &inout) {
	inout = inout >> 8 | inout << 8;
}
inline void			XchgWord(DWORD &inout) {
	inout = inout >> 16 | inout << 16;
}

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

uintmax_t			Mega2Bytes(size_t in);
size_t				Bytes2Mega(uintmax_t in);

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
	Type	tmp(x);
	x = y;
	y = tmp;
}

///========================================================================================= WinFlag
/// Проверка и установка битовых флагов
namespace	WinFlag {
template<typename Type>
bool		Check(Type in, Type flag) {
	return	flag == (in & flag);
}
template<typename Type>
Type		&Set(Type &in, Type flag) {
	return	in |= flag;
}
template<typename Type>
Type		&UnSet(Type &in, Type flag) {
	return	in &= ~flag;
}
template<typename Type>
Type		&Switch(Type &in, Type flag, bool sw) {
	if (sw)
		return	in |= flag;
	else
		return	in &= ~flag;
}
};

///========================================================================================= WinFlag
/// Проверка и установка битов
namespace	WinBit {
template<typename Type>
size_t		BIT_LIMIT() {
	return	sizeof(Type) * 8;
}
template<typename Type>
bool		BadBit(size_t in) {
	return	 !(in < BIT_LIMIT<Type>());
}
template<typename Type>
size_t		Limit(size_t in) {
	return	(in == 0) ? BIT_LIMIT<Type>() : Min<int>(in, BIT_LIMIT<Type>());
}

template<typename Type>
bool		Check(Type in, size_t bit) {
	if (BadBit<Type>(bit))
		return	false;
	Type	tmp = 1;
	tmp <<= bit;
	return	(in & tmp);
}
template<typename Type>
Type		&Set(Type &in, size_t bit) {
	if (BadBit<Type>(bit))
		return	in;
	Type	tmp = 1;
	tmp <<= bit;
	return	(in |= tmp);
}
template<typename Type>
Type		&UnSet(Type &in, size_t bit) {
	if (BadBit<Type>(bit))
		return	in;
	Type	tmp = 1;
	tmp <<= bit;
	return	(in &= ~tmp);
}
template<typename Type>
Type		&Switch(Type &in, size_t bit, bool sw) {
	if (sw)
		return	Set(in, bit);
	else
		return	UnSet(in, bit);
}
}

///========================================================================================== WinCom
/// Класс инициализации COM (объекты создавать запрещено, нужно использовать фукцию IsOK)
class		WinCOM {
	bool		m_err;

	WinCOM(const WinCOM&);
	WinCOM(): m_err(true) {
		if (m_err) {
			m_err = !SUCCEEDED(::CoInitializeEx(null_ptr, COINIT_MULTITHREADED));
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
	Shared_ptr(): data(new Pointee<Type>(null_ptr)) {
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
	Type*			get() const {
		return	data->m_ptr;
	}
	void			release() {
		data->delRef();
	}
	void			reset(Type* ptr) {
		release();
		data = new Pointee<Type>(ptr);
	}
	void			swap(Shared_ptr<Type> &rhs) {
		Pointee<Type>	*tmp = data;
		data = rhs.data;
		rhs.data = tmp;
	}

	operator		bool() const {
		return	data->m_ptr;
	}
	operator		Type*() const {
		return	data->m_ptr;
	}
	Type*			operator->() const {
		return data->m_ptr;
	}
	Type&			operator*() {
		return *(data->m_ptr);
	}
};

///========================================================================================== WinMem
/// Функции работы с кучей
namespace	WinMem {
template <typename Type>
inline bool			Alloc(Type &in, size_t size) {
	in = static_cast<Type>(::HeapAlloc(::GetProcessHeap(), HEAP_ZERO_MEMORY, size));
	return	 in != null_ptr;
}
inline PVOID		Alloc(size_t size) {
	return	::HeapAlloc(::GetProcessHeap(), HEAP_ZERO_MEMORY, size);
}

template <typename Type>
inline bool			Realloc(Type &in, size_t size) {
	if (in != null_ptr)
		in = (Type)::HeapReAlloc(::GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS | HEAP_ZERO_MEMORY, (PVOID)in, size);
	else
		in = (Type)::HeapAlloc(::GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS | HEAP_ZERO_MEMORY, size);
	return	 in != null_ptr;
}

template <typename Type>
inline void			Free(Type &in) {
	if (in) {
		::HeapFree(::GetProcessHeap(), 0, (PVOID)in);
		in = null_ptr;
	}
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

///====================================================================== Функции работы с символами
inline int			GetType(WCHAR in) {
	WORD	Result[2] = {0};
	::GetStringTypeW(CT_CTYPE1, &in, 1, Result);
	return	Result[0];
}
inline bool			IsEol(WCHAR in) {
	return	in == L'\r' || in == L'\n';
}
inline bool			IsSpace(WCHAR in) {
//	return	in == L' ' || in == L'\t';
	return	WinFlag::Check(GetType(in), C1_SPACE);
}
inline bool			IsPrint(WCHAR in) {
	return	!WinFlag::Check(GetType(in), C1_CNTRL);
}
inline bool			IsCntrl(WCHAR in) {
//	return	in == L' ' || in == L'\t';
	return	WinFlag::Check(GetType(in), C1_CNTRL);
}
inline bool			IsUpper(WCHAR in) {
//	return	::IsCharUpperW(in);
	return	WinFlag::Check(GetType(in), C1_UPPER);
}
inline bool			IsLower(WCHAR in) {
//	return	::IsCharLowerW(in);
	return	WinFlag::Check(GetType(in), C1_LOWER);
}
inline bool			IsAlpha(WCHAR in) {
//	return	::IsCharAlphaW(in);
	return	WinFlag::Check(GetType(in), C1_ALPHA);
}
inline bool			IsAlNum(WCHAR in) {
//	return	::IsCharAlphaW(in);
	int	Result = GetType(in);
	return	WinFlag::Check(Result, C1_ALPHA) || WinFlag::Check(Result, C1_DIGIT);
}
inline bool			IsDigit(WCHAR in) {
//	return	::IsCharAlphaNumeric(in);
	return	WinFlag::Check(GetType(in), C1_DIGIT);
}
inline bool			IsXDigit(WCHAR in) {
//	return	::IsCharAlphaNumeric(in);
	return	WinFlag::Check(GetType(in), C1_XDIGIT);
}
inline bool			IsPunct(WCHAR in) {
//	return	::IsCharAlphaNumeric(in);
	return	WinFlag::Check(GetType(in), C1_PUNCT);
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

#ifndef NORM_STOP_ON_NULL
#define NORM_STOP_ON_NULL 0x10000000
#endif
inline int			Cmp(PCSTR in1, PCSTR in2) {
	return	::strcmp(in1, in2);
//	return	::CompareStringA(0, SORT_STRINGSORT, in1, -1, in2, -1) - CSTR_EQUAL;
}
inline int			Cmp(PCSTR in1, PCSTR in2, size_t n) {
	return	::strncmp(in1, in2, n);
//	return	::CompareStringA(0, NORM_STOP_ON_NULL | SORT_STRINGSORT, in1, n, in2, n) - CSTR_EQUAL;
}
inline int			Cmp(PCWSTR in1, PCWSTR in2) {
	return	::wcscmp(in1, in2);
//	return	::wcscoll(in1, in2);
//	return	::CompareStringW(0, SORT_STRINGSORT, in1, -1, in2, -1) - CSTR_EQUAL;
//	return	::CompareStringW(LOCALE_USER_DEFAULT, SORT_STRINGSORT, in1, -1, in2, -1) - CSTR_EQUAL;
}
inline int			Cmp(PCWSTR in1, PCWSTR in2, size_t n) {
	return	::wcsncmp(in1, in2, n);
//	return	::CompareStringW(0 , NORM_STOP_ON_NULL | SORT_STRINGSORT, in1, n, in2, n) - CSTR_EQUAL;
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
//	return	::_wcsicoll(lhs.first.c_str(), rhs.first.c_str()) < 0;
//	return	fsf.LStricmp(lhs.first.c_str(), rhs.first.c_str()) < 0;
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
inline PWSTR		Find(PCWSTR where, WCHAR what) {
	return	::wcschr(where, what);
}
inline PWSTR		RFind(PCWSTR where, PCWSTR what) {
	return	::wcsstr(where, what);
}
inline PWSTR		RFind(PCWSTR where, WCHAR what) {
	return	::wcschr(where, what);
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
	return	null_ptr;
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
	return	null_ptr;
}

inline int64_t		AsInt64(PCSTR in, int base = 10) {
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

inline int64_t		AsInt64(PCWSTR in, int base = 10) {
//	return	_wtoi64(in);
	PWSTR	end_ptr;
	return	::wcstoll(in, &end_ptr, base);
}
inline uint64_t		AsUInt64(PCWSTR in, int base = 10) {
//	return	_wtoi64(in);
	PWSTR	end_ptr;
	return	::wcstoull(in, &end_ptr, base);
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

inline PWSTR		AssignStr(PCWSTR src) {
	size_t	len = Len(src) + 1;
	PWSTR	dest;
	WinMem::Alloc(dest, len * sizeof(WCHAR));
	Copy(dest, src, len);
	return	dest;
}

///======================================================================================= WinBuffer
/// Обертка буфера
template<typename Type>
class		WinBuf {
	Type*		m_buf;
	size_t		m_size;

	void			Free() {
		WinMem::Free<Type*>(m_buf);
	}
public:
	~WinBuf() {
		Free();
	}
	WinBuf(): m_buf(null_ptr), m_size(0) {
	}
	WinBuf(size_t size, bool bytes = false): m_buf(null_ptr), m_size((bytes) ? size : size * sizeof(Type)) {
		WinMem::Alloc(m_buf, m_size);
	}
	WinBuf(const WinBuf &in): m_buf(null_ptr), m_size(in.m_size) {
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
inline size_t		Convert(PCSTR from, UINT cp, PWSTR to = null_ptr, size_t size = 0) {
	return	::MultiByteToWideChar(cp, 0, from, -1, to, (int)size);
}
inline size_t		Convert(PCWSTR from, UINT cp, PSTR to = null_ptr, size_t size = 0) {
	return	::WideCharToMultiByte(cp, 0, from, -1, to, (int)size, null_ptr, null_ptr);
}

class		CStrMW {
	class	MzsData {
		PWSTR		m_data;
		size_t		m_capa;
		size_t		m_size;
	public:
		~MzsData() {
			delete[]	m_data;
		}
		explicit	MzsData(PCWSTR in) {
			PCWSTR	ptr = in;
			while (*ptr) {
				ptr += (Len(ptr) + 1);
				++m_size;
			}
			m_capa = ptr - in + 1;
			m_data = new WCHAR[m_capa];
			WinMem::Copy(m_data, in, m_capa * sizeof(WCHAR));
		}
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
	PCWSTR			operator[](int index) const {
		PCWSTR	ptr = c_str();
		int		cnt = 0;
		while (*ptr && (cnt++ < index)) {
			ptr += (Len(ptr) + 1);
		}
		return	ptr;
	}
};

#ifdef NoStlString
#include "win_autostr.h"
#else
#include "win_autoutf.h"
#endif

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
	return	AutoUTF(in.c_str(), CP_UTF8);
}

inline PCWSTR		Num2Str(PWSTR str, intmax_t in, int base = 10) {
	return	::_i64tow(in, str, base);
}
inline AutoUTF		Num2Str(intmax_t in, int base = 10) {
	WCHAR	buf[64];
	Num2Str(buf, in, base);
	return	buf;
}

inline AutoUTF		ErrAsStr(HRESULT err = ::GetLastError(), PCWSTR lib = null_ptr) {
	HMODULE	mod = null_ptr;
	if (err && lib) {
		mod = ::LoadLibraryExW(lib, null_ptr, DONT_RESOLVE_DLL_REFERENCES); //LOAD_LIBRARY_AS_DATAFILE
	}
	PWSTR	buf = null_ptr;
	::FormatMessageW(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | ((mod) ?  FORMAT_MESSAGE_FROM_HMODULE : FORMAT_MESSAGE_FROM_SYSTEM),
		mod, err,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), //GetSystemDefaultLangID(),
		(PWSTR)&buf, 0, null_ptr);
	AutoUTF	Result((buf) ? buf : L"Unknown error\r\n");
	::LocalFree(buf);
	Result[Result.size() - 2] = L'\0';
	if (mod)
		::FreeLibrary(mod);
	return	Result;
}
inline AutoUTF		ErrWmiAsStr(HRESULT err) {
	return	ErrAsStr(err, L"wmiutils.dll");
}

inline void			mbox(PCSTR text, PCSTR capt = "") {
	::MessageBoxA(null_ptr, text, capt, MB_OK);
}
inline void			mbox(PCWSTR text, PCWSTR capt = L"") {
	::MessageBoxW(null_ptr, text, capt, MB_OK);
}
inline void			mbox(HRESULT err, PCWSTR lib = null_ptr) {
	::MessageBoxW(null_ptr, ErrAsStr(err, lib).c_str(), L"Error", MB_OK);
}

CStrA				Hash2Str(const PBYTE buf, size_t size);
CStrA				Hash2StrNum(const PBYTE buf, size_t size);
bool				Str2Hash(const CStrA &str, PVOID &hash, ULONG &size);

inline AutoUTF&		ReplaceAll(AutoUTF& str, const AutoUTF &from, const AutoUTF &to) {
//	PCWSTR	pos = null_ptr;
//	while ((pos = Find(str.c_str(), from.c_str()))) {
//		str.replace(pos - str.c_str(), from.size(), to);
//	}
//	return	str;

	size_t pos;
	while ((pos = str.find(from)) != AutoUTF::npos) {
		str.replace(pos, from.size(), to);
	}
	return	str;
}
inline AutoUTF		ReplaceAllOut(const AutoUTF& str, const AutoUTF &from, const AutoUTF &to) {
	AutoUTF	Result(str);
	return	ReplaceAll(Result, from, to);
}

UINT				CheckUnicode(const PVOID buf, size_t size);
UINT				IsUTF8(const PVOID buf, size_t size);
bool				GetCP(HANDLE hFile, UINT &cp, bool bUseHeuristics = false);

///========================================================================================= BitMask
template<typename Type>
class		BitMask {
public:
	static Type		FromStr(const AutoUTF &in, size_t lim = 0) {
		// count bits from 1
		Type	Result = 0;
		ssize_t	bit = 0;
		AutoUTF	tmp(in);
		lim = WinBit::Limit<Type>(lim);
		while (tmp.Cut(bit)) {
			if (!WinBit::BadBit<Type>(--bit))
				WinBit::Set(Result, bit);
		}
		return	Result;
	}
	static Type		FromStr0(const AutoUTF &in, size_t lim = 0) {
		// count bits from zero
		Type	Result = 0;
		ssize_t	bit = 0;
		AutoUTF	tmp(in);
		lim = WinBit::Limit<Type>(lim);
		while (tmp.Cut(bit)) {
			if (!WinBit::BadBit<Type>(bit))
				WinBit::Set(Result, bit);
		}
		return	Result;
	}

	static AutoUTF	AsStr(Type in, size_t lim = 0) {
		// count bits from 1
		AutoUTF	Result;
		lim = WinBit::Limit<Type>(lim);
		for (size_t bit = 0; bit < lim; ++bit) {
			if (WinBit::Check(in, bit)) {
				Result.Add(Num2Str(bit + 1), L",");
			}
		}
		return	Result;
	}
	static AutoUTF	AsStr0(Type in, size_t lim = 0) {
		// count bits from zero
		AutoUTF	Result;
		lim = WinBit::Limit<Type>(lim);
		for (size_t	bit = 0; bit < lim; ++bit) {
			if (WinBit::Check(in, bit)) {
				Result.Add(Num2Str(bit), L",");
			}
		}
		return	Result;
	}
	static AutoUTF	AsStrBin(Type in, size_t lim = 0) {
		AutoUTF	Result;
		uintmax_t	flag = (uintmax_t)1 << (WinBit::Limit<Type>(lim) - 1);
		while (flag) {
			Result += WinFlag::Check(in, (Type)flag) ? L'1' : L'0';
			flag >>= 1;
		}
		return	Result;
	}
	static AutoUTF	AsStrNum(Type in, size_t lim = 0) {
		AutoUTF	Result;
		uintmax_t	flag = (uintmax_t)1 << (WinBit::Limit<Type>(lim) - 1);
		while (flag) {
			if (WinFlag::Check(in, (Type)flag)) {
				Result.Add(Num2Str(flag), L",");
			}
			flag >>= 1;
		}
		return	Result;
	}
};

///==================================================================================== ConsoleColor
class		ConsoleColor {
	WORD	m_color;

	bool			ColorSave() {
		WinBuf<CONSOLE_SCREEN_BUFFER_INFO> tmp(1);
		if (::GetConsoleScreenBufferInfo(::GetStdHandle(STD_OUTPUT_HANDLE), tmp)) {
			m_color = tmp->wAttributes;
		}
		return	m_color;
	}
	void			ColorRestore() {
		if (m_color)
			::SetConsoleTextAttribute(::GetStdHandle(STD_OUTPUT_HANDLE), m_color);
	}
public:
	~ConsoleColor() {
		ColorRestore();
	}
	ConsoleColor(WORD color): m_color(0) {
		if (color && ColorSave())
			::SetConsoleTextAttribute(::GetStdHandle(STD_OUTPUT_HANDLE), color);
	}
};

///===================================================================================== Console out
int					consoleout(WCHAR in, DWORD nStdHandle = STD_OUTPUT_HANDLE);
int					consoleout(PCWSTR in, DWORD nStdHandle = STD_OUTPUT_HANDLE);
int					consoleout(PCSTR in, DWORD nStdHandle = STD_OUTPUT_HANDLE);
int					consoleout(const AutoUTF &in, DWORD nStdHandle = STD_OUTPUT_HANDLE/*STD_ERROR_HANDLE*/);

int					stdvprintf(DWORD nStdHandle, PCWSTR format, va_list vl);
int					stdprintf(DWORD nStdHandle, PCWSTR format, ...);
int					vsnprintf(PWSTR buff, size_t len, PCWSTR format, va_list vl);
int					printf(PCWSTR format, ...);
inline int			vprintf(PCWSTR format, va_list vl) {
	return	stdvprintf(STD_OUTPUT_HANDLE, format, vl);
}
int					snprintf(PWSTR buff, size_t len, PCWSTR format, ...);
void				errx(int eval, PCSTR format, ...);

enum		WinLogLevel {
	LOG_TRACE =	-3,
	LOG_DEBUG,
	LOG_VERBOSE,
	LOG_INFO,
	LOG_ERROR,
};

extern int			logLevel;
void				setLogLevel(WinLogLevel lvl);
void				logError(PCWSTR format, ...);
void				logError(DWORD errNumber, PCWSTR format, ...);
void				logDebug(PCWSTR format, ...);
void				logVerbose(PCWSTR format, ...);
void				logCounter(PCWSTR format, ...);
void				logInfo(PCWSTR format, ...);
void				logFile(WIN32_FIND_DATA FileData);

///========================================================================================== WinEnv
namespace	WinEnv {
AutoUTF				Get(PCWSTR name);
bool				Set(PCWSTR name, PCWSTR val);
bool				Add(PCWSTR name, PCWSTR val);
bool				Del(PCWSTR name);
}

///============================================================================================ Exec
extern DWORD		EXEC_TIMEOUT;
extern DWORD		EXEC_TIMEOUT_DX;
bool				Exec(const AutoUTF &cmd);
int					Exec(const AutoUTF &cmd, CStrA &out);
int					Exec(const AutoUTF &cmd, CStrA &out, const CStrA &in);
int					ExecWait(const AutoUTF &cmd, DWORD wait = EXEC_TIMEOUT);

inline DWORD		UserLogon(HANDLE &hToken, PCWSTR name, PCWSTR pass, DWORD type, PCWSTR dom = L"") {
	DWORD	Result = NO_ERROR;
	if (!::LogonUserW((PWSTR)name, (PWSTR)dom, (PWSTR)pass, type, LOGON32_PROVIDER_DEFAULT, &hToken)) {
		Result = ::GetLastError();
	}
	return	Result;
}
inline DWORD		CanLogon(PCWSTR name, PCWSTR pass, DWORD type = LOGON32_LOGON_BATCH, PCWSTR dom = L"") {
	HANDLE	hToken = null_ptr;
	DWORD	Result = UserLogon(hToken, name, pass, type, dom);
	if (Result == NO_ERROR)
		::CloseHandle(hToken);
	return	Result;
}

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
	DWORD	size = ::ExpandEnvironmentStringsW(path, null_ptr, 0);
	if (size) {
		WCHAR	Result[::ExpandEnvironmentStringsW(path, null_ptr, 0)];
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
inline AutoUTF		PathCompact(PCWSTR path, size_t size) {
	WCHAR	Result[MAX_PATH_LENGTH];
	::PathCompactPathExW(Result, path, size, 0);
	return	Result;
}
inline AutoUTF		PathCompact(const AutoUTF &path, size_t size) {
	return	PathCompact(path.c_str(), size);
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
	AutoUTF	Result(PathNice(SlashAdd(path)));
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
inline bool			SetWorkDirectory(const AutoUTF &path) {
	return	SetWorkDirectory(path.c_str());
}

///========================================================================================= SysPath
namespace	SysPath {
AutoUTF				Winnt();
AutoUTF				Sys32();
AutoUTF				SysNative();
AutoUTF				InetSrv();
AutoUTF				Dns();
AutoUTF				Temp();

AutoUTF				Users();
}

///========================================================================================== SysApp
namespace	SysApp {
AutoUTF				appcmd();
AutoUTF				dnscmd();
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
	hFile = ::CreateFileW(path, FILE_READ_DATA, FILE_SHARE_DELETE | FILE_SHARE_READ, null_ptr, OPEN_EXISTING,
						  FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS, null_ptr);
	return	hFile && hFile != INVALID_HANDLE_VALUE;
}
inline bool			FileOpenAttr(PCWSTR	path, HANDLE &hFile) {
	hFile = ::CreateFileW(path, FILE_READ_ATTRIBUTES, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, null_ptr, OPEN_EXISTING,
						  FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS, null_ptr);
	return	hFile && hFile != INVALID_HANDLE_VALUE;
}
inline bool			FileClose(HANDLE hFile) {
	return	::CloseHandle(hFile) != 0;
}

inline AutoUTF		TempDir() {
	WCHAR	buf[::GetTempPathW(0, null_ptr)];
	::GetTempPathW(sizeofa(buf), buf);
	return	buf;
}
inline AutoUTF		TempFile(PCWSTR path) {
	WCHAR	buf[MAX_PATH];
	WCHAR	pid[32];
	Num2Str(pid, ::GetCurrentProcessId());
	::GetTempFileNameW(path, pid, 0, buf);
	return	buf;
}
inline AutoUTF		TempFile(const AutoUTF &path) {
	return	TempFile(path.c_str());
}
inline AutoUTF		FullPath(PCWSTR path) {
	size_t	len = ::GetFullPathNameW(path, 0, null_ptr, null_ptr);
	if (len) {
		WCHAR	buf[len];
		::GetFullPathNameW(path, sizeofa(buf), buf, null_ptr);
		return	buf;
	}
	return	AutoUTF();
}

inline bool			HardLink(PCWSTR path, PCWSTR newfile) {
	return	::CreateHardLinkW(newfile, path, null_ptr) != 0;
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
	return	::SetFilePointerEx(hFile, pos, null_ptr, m) != 0;
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
	DWORD	JUNC_ATTR	= FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_REPARSE_POINT;
	DWORD	Attr = Attributes(path);
	return	(Attr != INVALID_FILE_ATTRIBUTES) && ((Attr & JUNC_ATTR) == JUNC_ATTR);
}
inline bool			IsJunc(const AutoUTF &path) {
	return	IsJunc(path.c_str());
}

bool				DirCreate(PCWSTR path);
inline bool			DirCreate(const AutoUTF &path) {
	return	DirCreate(path.c_str());
}
bool				FileCreate(PCWSTR path, PCWSTR name, PCSTR content);
inline bool			FileCreate(const AutoUTF &path, const AutoUTF &name, PCSTR content) {
	return	FileCreate(path.c_str(), name.c_str(), content);
}
bool				DirDel(PCWSTR path);
inline bool			DirDel(const AutoUTF &path) {
	return	DirDel(path.c_str());
}
inline bool			FileDel(PCWSTR path) {
	DWORD	attr = Attributes(path);
	if (::SetFileAttributesW(path, FILE_ATTRIBUTE_NORMAL)) {
		if (::DeleteFileW(path)) {
			return	true;
		}
		::SetFileAttributesW(path, attr);
	}
	return	false;
}
inline bool			FileDel(const AutoUTF &path) {
	return	FileDel(path.c_str());
}
inline bool			FileDelReboot(PCWSTR path) {
	return	::MoveFileExW(path, null_ptr, MOVEFILE_DELAY_UNTIL_REBOOT);
}
inline bool			FileDelReboot(const AutoUTF &path) {
	return	FileDelReboot(path.c_str());
}
bool				Del2(PCWSTR path);
bool				Recycle(PCWSTR path);
inline bool			Recycle(const AutoUTF &path) {
	return	Recycle(path.c_str());
}

inline bool			FileCopy(PCWSTR path, PCWSTR dest) {
	return	::CopyFileW(path, dest, true) != 0;
}
inline bool			FileCopy(const AutoUTF &path, const AutoUTF &dest) {
	return	FileCopy(path.c_str(), dest.c_str());
}
bool				FileCopySecurity(PCWSTR path, PCWSTR dest);

inline bool			FileMove(PCWSTR path, PCWSTR dest, DWORD flag = 0) {
	return	::MoveFileExW(path, dest, flag);
}
inline bool			FileMove(const AutoUTF &path, const AutoUTF &dest, DWORD flag = 0) {
	return	::MoveFileExW(path.c_str(), dest.c_str(), flag);
}

inline bool			FileRead(HANDLE hFile, PBYTE buf, DWORD &size) {
	return	::ReadFile(hFile, buf, size, &size, null_ptr) != 0;
}
bool				FileRead(PCWSTR	path, CStrA &buf);
bool				FileWrite(PCWSTR path, PCVOID buf, size_t size, bool rewrite = false);
inline bool			FileWrite(PCWSTR path, PCWSTR data, size_t size, bool rewrite = false) {
	return	FileWrite(path, data, size * sizeof(WCHAR), rewrite);
}
inline bool			FileWrite(const AutoUTF &path, const AutoUTF &data, bool rewrite = false) {
	return	FileWrite(path.c_str(), (PCVOID)data.c_str(), data.size() * sizeof(WCHAR), rewrite);
}
inline size_t		FileWrite(HANDLE file, const PVOID &in, size_t size) {
	DWORD	Result = 0;
	::WriteFile(file, in, size, &Result, null_ptr);
	return	Result;
}
inline size_t		FileWrite(HANDLE file, const AutoUTF &in) {
	return	FileWrite(file, (const PVOID)in.c_str(), in.size() * sizeof(WCHAR));
}
bool				FileWipe(PCWSTR path);

AutoUTF				GetDrives();

///========================================================================================= WinFile
class		WinFile: private Uncopyable, public WinErrorCheck {
	HANDLE	m_hndl;
public:
	~WinFile() {
		Close();
	}
	WinFile(): m_hndl(INVALID_HANDLE_VALUE) {
	}
	WinFile(PCWSTR path, bool write = false): m_hndl(INVALID_HANDLE_VALUE) {
		Open(path, write);
	}
	operator		HANDLE() const {
		return	m_hndl;
	}

	bool			Open(PCWSTR path, ACCESS_MASK access, DWORD share, PSECURITY_ATTRIBUTES sa, DWORD creat, DWORD flags) {
		Close();
		m_hndl = ::CreateFileW(path, access, share, sa, creat, flags, null_ptr);
		return	m_hndl && m_hndl != INVALID_HANDLE_VALUE;
	}
	bool			Open(PCWSTR path, bool write = false) {
		ACCESS_MASK	amask = (write) ? GENERIC_READ | GENERIC_WRITE : GENERIC_READ;
		DWORD		share = (write) ? 0 : FILE_SHARE_DELETE | FILE_SHARE_READ;
		DWORD		creat = (write) ? OPEN_EXISTING : OPEN_EXISTING;
		DWORD		flags = (write) ? FILE_ATTRIBUTE_NORMAL : FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS;
		return	Open(path, amask, share, null_ptr, creat, flags);
	}
	void			Close() {
		::CloseHandle(m_hndl);
		m_hndl = INVALID_HANDLE_VALUE;
	}
	bool			Attr(DWORD attr) {
		if (m_hndl && m_hndl != INVALID_HANDLE_VALUE) {
			WCHAR	path[MAX_PATH_LENGTH];
			Path(path, sizeofa(path));
			return	::SetFileAttributesW(path, attr);
		}
		return	false;
	}
	DWORD			Attr() const {
		if (m_hndl && m_hndl != INVALID_HANDLE_VALUE) {
			WCHAR	path[MAX_PATH_LENGTH];
			Path(path, sizeofa(path));
			return	::GetFileAttributesW(path);
		}
		return	0;
	}
	bool			Size(uint64_t &size) {
		if (m_hndl && m_hndl != INVALID_HANDLE_VALUE) {
			LARGE_INTEGER	tmp;
			if (::GetFileSizeEx(m_hndl, &tmp)) {
				size = tmp.QuadPart;
				return	true;
			}
		}
		return	false;
	}
	bool			Path(PWSTR path, size_t len) const;

	bool			Write(PVOID buf, size_t size, DWORD &written) {
		if (m_hndl && m_hndl != INVALID_HANDLE_VALUE) {
			return	::WriteFile(m_hndl, (PCSTR)buf, size, &written, null_ptr);
		}
		return	false;
	}
	bool			Pointer(uint64_t dist, DWORD dwMoveMethod) {
		if (m_hndl && m_hndl != INVALID_HANDLE_VALUE) {
			LARGE_INTEGER	tmp;
			tmp.QuadPart = dist;
			return	::SetFilePointerEx(m_hndl, tmp, null_ptr, dwMoveMethod);
		}
		return	false;
	}
	bool			SetEnd() {
		if (m_hndl && m_hndl != INVALID_HANDLE_VALUE) {
			return	::SetEndOfFile(m_hndl);
		}
		return	false;
	}
};

///======================================================================================= WinFileId
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
		HANDLE	hFile = ::CreateFileW(path, 0, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, null_ptr, OPEN_EXISTING,
									 FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS, null_ptr);
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

///========================================================================================== WinVol
class		WinVol : private Uncopyable, public WinErrorCheck {
	HANDLE		m_hnd;
	AutoUTF		name;

	void					Close();
public:
	~WinVol() {
		Close();
	}
	WinVol(): m_hnd(INVALID_HANDLE_VALUE) {}
	bool 					Next();

	AutoUTF			GetName() const {
		return	name;
	}
	AutoUTF			GetPath() const;
	AutoUTF			GetDevice() const;

	long long		GetSize() const {
//		long long tmp = f_.nFileSizeHigh;
//		tmp = tmp << (sizeof(f_.nFileSizeHigh) * 8);
//		tmp |= f_.nFileSizeLow;
		return	0;
	}

	DWORD			GetFlag() const {
		DWORD	Result = 0;
//		::GetVolumeInformation(path.c_str(), null_ptr, 0, null_ptr, null_ptr, &Result, null_ptr, 0);
		return	Result;
	}
	UINT			GetType() const {
		return	::GetDriveTypeW(name.c_str());
	}

	bool			IsSuppCompress() const {
		return	WinFlag::Check(GetFlag(), (DWORD)FILE_FILE_COMPRESSION);
	}
	bool			IsSuppEncrypt() const {
		return	WinFlag::Check(GetFlag(), (DWORD)FILE_SUPPORTS_ENCRYPTION);
	}
	bool			IsSuppStreams() const {
		return	WinFlag::Check(GetFlag(), (DWORD)FILE_NAMED_STREAMS);
	}
	bool			IsSuppACL() const {
		return	WinFlag::Check(GetFlag(), (DWORD)FILE_PERSISTENT_ACLS);
	}
	bool			IsReadOnly() const {
		return	WinFlag::Check(GetFlag(), (DWORD)FILE_READ_ONLY_VOLUME);
	}

	bool			IsRemovable() const {
		return	WinFlag::Check(GetType(), (UINT)DRIVE_REMOVABLE);
	}
	bool			IsFixed() const {
		return	WinFlag::Check(GetType(), (UINT)DRIVE_FIXED);
	}
	bool			IsRemote() const {
		return	WinFlag::Check(GetType(), (UINT)DRIVE_REMOTE);
	}
	bool			IsCdRom() const {
		return	WinFlag::Check(GetType(), (UINT)DRIVE_CDROM);
	}
	bool			IsRamdisk() const {
		return	WinFlag::Check(GetType(), (UINT)DRIVE_RAMDISK);
	}

	bool			GetSize(uint64_t &uiUserFree, uint64_t &uiTotalSize, uint64_t &uiTotalFree) const;
	uint64_t		GetSizeTotal() const {
		uint64_t uf = 0, ts = 0, tf = 0;
		GetSize(uf, ts, tf);
		return	ts;
	}
};

///========================================================================================== WinDir
class		WinDir : private Uncopyable, public WinErrorCheck {
	WIN32_FIND_DATAW	m_find;
	HANDLE				m_handle;
	AutoUTF				m_path;
	AutoUTF				m_mask;

	void			Close() {
		if (m_handle && m_handle != INVALID_HANDLE_VALUE) {
			::FindClose(m_handle);
			m_handle = null_ptr;
		}
	}
public:
	~WinDir() {
		Close();
	}
	WinDir(const AutoUTF &path, const AutoUTF &mask = L"*"): m_handle(null_ptr), m_path(path), m_mask(mask) {
	}

	bool 			Next() {
		AutoUTF	tmp(SlashAdd(m_path));
		tmp += m_mask;
		if (m_handle == null_ptr) {
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

///========================================================================================= WinJunc
namespace	WinJunc {
bool				IsJunc(const AutoUTF &path);
bool				Add(const AutoUTF &path, const AutoUTF &dest);
bool				Del(const AutoUTF &path);
AutoUTF				GetDest(const AutoUTF &path);
}

///========================================================================================= FileMap
/// Отображение файла в память блоками
class		FileMap : private Uncopyable, public WinErrorCheck {
	HANDLE		m_hSect;
	PVOID		m_data;
	size_t		m_framesize;
	uint64_t	m_mapsize;
	uint64_t	m_offset;
	bool		m_write;
public:
	~FileMap() {
		Close();
	}
	FileMap(const WinFile &wf, uint64_t size = (uint64_t) - 1, bool write = false): m_hSect(null_ptr), m_data(null_ptr), m_mapsize(0), m_offset(0) {
		Open(wf, size, write);
	}
	FileMap(PCWSTR path, uint64_t size = (uint64_t) - 1, bool write = false): m_hSect(null_ptr), m_data(null_ptr), m_mapsize(0), m_offset(0) {
		Open(path, size, write);
	}

	bool			Close() {
		if (m_data) {
			::UnmapViewOfFile(m_data);
			m_data = null_ptr;
		}
		if (m_hSect) {
			::CloseHandle(m_hSect);
			m_hSect = null_ptr;
		}
		return	true;
	}
	bool			Open(const WinFile &wf, uint64_t size = (uint64_t) - 1, bool write = false) {
		Close();
		Home();
		HANDLE	hFile = wf;
		m_write = write;
		if (hFile && hFile != INVALID_HANDLE_VALUE) {
			DWORD	protect = (m_write) ? PAGE_READWRITE : PAGE_READONLY;
			m_mapsize = Min(FileSize(hFile), size);
			m_hSect = ::CreateFileMapping(hFile, null_ptr, protect, (DWORD)(m_mapsize >> 32), (DWORD)(m_mapsize & 0xFFFFFFFF), null_ptr);
		}
		return	ChkSucc(m_hSect != null_ptr);
	}
	bool			Open(PCWSTR path, uint64_t size = (uint64_t) - 1, bool write = false) {
		WinFile	wf(path, write);
		return	Open(wf, size, write);
	}

	bool			Next() {
		if (m_data) {
			::UnmapViewOfFile(m_data);
			m_data = null_ptr;
		}
		if ((m_mapsize - m_offset) > 0) {
			if ((m_mapsize - m_offset) < (uint64_t)m_framesize)
				m_framesize = (size_t)(m_mapsize - m_offset);
			if (m_hSect) {
				ACCESS_MASK	amask = (m_write) ? FILE_MAP_WRITE : FILE_MAP_READ;
				m_data = ::MapViewOfFile(m_hSect, amask, (DWORD)(m_offset >> 32), (DWORD)(m_offset & 0xFFFFFFFF), m_framesize);
				m_offset += m_framesize;
				return	ChkSucc(m_data != null_ptr);
			}
		}
		return	false;
	}
	bool			Home() {
		SYSTEM_INFO	info;
		::GetSystemInfo(&info);
		m_framesize = info.dwAllocationGranularity * 128;
		m_offset = 0LL;
		return	true;
	}

	PVOID			data() const {
		return	m_data;
	}
	size_t			size() const {
		return	m_framesize;
	}
	uint64_t		offset() const {
		return	m_offset;
	}
	uint64_t		sizeOfMap() const {
		return	m_mapsize;
	}
};

///========================================================================================= WinGUID
namespace	WinGUID {
AutoUTF				Gen();
}

///============================================================================================= Sid
/// Security Identifier (Идентификатор безопасности) -
/// структура данных переменной длины, которая идентифицирует учетную запись пользователя, группы,
/// домена или компьютера
#ifndef PSIDFromPACE
#define PSIDFromPACE(pACE)((PSID)(&((pACE)->SidStart)))
#endif

class		Sid {
	PSID	pSID;
	AutoUTF	m_srv;

	Sid();
	void			Copy(PSID in);
	void			Free(PSID &in) {
		WinMem::Free<PSID>(in);
	}
	bool			Init(PCWSTR name, PCWSTR srv);
public:
	~Sid() {
		Free(pSID);
	}
	Sid(WELL_KNOWN_SID_TYPE wns): pSID(null_ptr) {
		DWORD	size = SECURITY_MAX_SID_SIZE;
		if (WinMem::Alloc(pSID, size)) {
			if (::CreateWellKnownSid(wns, null_ptr, pSID, &size)) {
				WinMem::Realloc(pSID, size);
			}
		}
	}
	Sid(PCWSTR sSID);
	Sid(const AutoUTF &sSID);
	Sid(PCWSTR name, PCWSTR srv);	// if domain name == account name use "dom\\acc"
	Sid(const AutoUTF &name, const AutoUTF &srv);
	Sid(const Sid &rhs): pSID(null_ptr), m_srv(rhs.m_srv) {
		Copy(rhs.pSID);
	}
	explicit Sid(PSID in): pSID(null_ptr) {
		Copy(in);
	}

	Sid&			operator=(const Sid &rhs) {
		Copy(rhs.pSID);
		m_srv = rhs.m_srv;
		return	*this;
	}

	bool			operator==(const Sid &rhs) const {
		return	(Valid() && rhs.Valid()) ? ::EqualSid(pSID, rhs.Data()) : false;
	}
	bool			operator!=(const Sid &rhs) const {
		return	!operator==(rhs);
	}
	operator		const PSID() const {
		return	pSID;
	}
	PSID			Data() const {
		return	pSID;
	}
	AutoUTF			Srv() const  {
		return	m_srv;
	}

	bool			Valid() const {
		return	Valid(pSID);
	}
	size_t			Size() const {
		return	pSID ? Size(pSID) : 0;
	}
	DWORD			Rid() const {
		return	Rid(pSID);
	}

	AutoUTF			AsStr() const {
		return	AsStr(pSID);
	}
	AutoUTF			AsName() const {
		return	AsName(pSID, m_srv);
	}
	AutoUTF			AsFullName() const {
		return	AsFullName(pSID, m_srv);
	}
	AutoUTF			AsDom() const {
		return	AsDom(pSID, m_srv);
	}

// static
	static bool		Valid(PSID in) {
		return	in && ::IsValidSid(in);
	}
	static size_t	CountSubAuthority(PSID in) {
		return	Valid(in) ? *(::GetSidSubAuthorityCount(in)) : 0;
	}
	static DWORD	Rid(PSID in) {
		size_t	cnt = CountSubAuthority(in);
		return	cnt ? *(::GetSidSubAuthority(in, cnt - 1)) : 0;
	}
	static size_t	Size(PSID in) {
		return	Valid(in) ? ::GetLengthSid(in) : 0;
	}
	static size_t	SizeCounter(size_t &cnt, PSID in) {
		return	cnt += Size(in);
	}
	static size_t	SizeCounter(size_t &cnt, const AutoUTF &name, const AutoUTF &dom = L"") {
		Sid		sid(name, dom);
		return	cnt += sid.Size();
	}

	// PSID to sid string
	static AutoUTF	AsStr(PSID in);

	// name to sid string
	static AutoUTF	AsStr(const AutoUTF &name, const AutoUTF &dom = L"");

	// PSID to name
	static DWORD	AsName(PSID pSID, AutoUTF &name, AutoUTF &dom, const AutoUTF &srv = L"");
	static AutoUTF	AsName(PSID pSID, const AutoUTF &srv = L"");
	static AutoUTF	AsFullName(PSID pSID, const AutoUTF &srv = L"");
	static AutoUTF	AsDom(PSID pSID, const AutoUTF &srv = L"");
};
/*
inline PSID			GetSid() {
	SID_IDENTIFIER_AUTHORITY NtAuthority = {SECURITY_NT_AUTHORITY};
	PSID	AdministratorsGroup = null_ptr;

	::AllocateAndInitializeSid(&NtAuthority, 2,
							   SECURITY_BUILTIN_DOMAIN_RID,
							   DOMAIN_ALIAS_RID_ADMINS,
							   0, 0, 0, 0, 0, 0,
							   &AdministratorsGroup);
	return	AdministratorsGroup;
}
*/

///========================================================================================= WinPriv
/// Функции работы с привилегиями
namespace	WinPriv {
bool 				IsExist(HANDLE hToken, LUID priv);
bool 				IsExist(HANDLE hToken, PCWSTR sPriv);
bool 				IsExist(LUID priv);
bool 				IsExist(PCWSTR sPriv);

bool				IsEnabled(HANDLE hToken, LUID priv);
bool 				IsEnabled(HANDLE hToken, PCWSTR sPriv);
bool				IsEnabled(LUID priv);
bool 				IsEnabled(PCWSTR sPriv);

bool 				Modify(HANDLE hToken, LUID priv, bool bEnable);
bool 				Modify(HANDLE hToken, PCWSTR sPriv, bool bEnable);
bool 				Modify(LUID priv, bool bEnable);
bool 				Modify(PCWSTR sPriv, bool bEnable);

inline bool			Disable(LUID in) {
	return	Modify(in, false);
}
inline bool			Disable(PCWSTR in) {
	return	Modify(in, false);
}

inline bool			Enable(LUID in) {
	return	Modify(in, true);
}
inline bool			Enable(PCWSTR in) {
	return	Modify(in, true);
}

AutoUTF				GetName(PCWSTR sPriv);
}

///======================================================================================= WinPolicy
namespace	WinPolicy {
/*
extern PCSTR			PrivNames[];
extern PCSTR			PrivNamesEn[];
extern PCSTR			PrivNamesRu[];
extern PCSTR			RightsNames[];
extern PCSTR			RightsNamesEn[];
extern PCSTR			RightsNamesRu[];
*/

HANDLE				Handle(const AutoUTF &path, bool bWrite = false);
void				InitLsaString(LSA_UNICODE_STRING &lsaString, const AutoUTF &in);
LSA_HANDLE			GetPolicyHandle(const AutoUTF &dom = L"");
NTSTATUS			AccountRightAdd(const AutoUTF &name, const AutoUTF &right, const AutoUTF &dom = L"");
NTSTATUS			AccountRightDel(const AutoUTF &name, const AutoUTF &right, const AutoUTF &dom = L"");

bool				GetTokenUser(HANDLE	hToken, AutoUTF &name);
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
	WinProcess(ACCESS_MASK mask, DWORD pid): m_hndl(null_ptr) {
		m_hndl = ::OpenProcess(mask, false, pid);
		ChkSucc(m_hndl != null_ptr);
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
	WinToken(ACCESS_MASK mask = TOKEN_ALL_ACCESS): m_handle(null_ptr) {
		ChkSucc(::OpenProcessToken(WinProcess(), mask, &m_handle) != 0);
	}
	WinToken(ACCESS_MASK mask, HANDLE hProcess): m_handle(null_ptr) {
		ChkSucc(::OpenProcessToken(hProcess, mask, &m_handle) != 0);
	}
	operator		HANDLE() const {
		return	m_handle;
	}

	static AutoUTF	GetUser(HANDLE hToken);
	static bool		CheckMembership(PSID sid, HANDLE hToken = null_ptr) {
		BOOL	Result;
		::CheckTokenMembership(hToken, sid, &Result);
		return	Result;
	}
};

inline bool			IsUserAdmin() {
	return	WinToken::CheckMembership(Sid(WinBuiltinAdministratorsSid), null_ptr);
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
class		WinReg: private Uncopyable {
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
			::RegSetValueExW(hKeyOpend, name.c_str(), 0, type, (PBYTE)(&value), sizeof(value));
			CloseKey();
		}
	}
	template <typename Type>
	bool			GetRaw(const AutoUTF &name, Type &value, const Type &def) const {
		bool	Result = OpenKey(KEY_READ);
		value = def;
		if (Result) {
			DWORD	size = sizeof(value);
			Result = ::RegQueryValueExW(hKeyOpend, name.c_str(), null_ptr, null_ptr, (PBYTE)(&value), &size) == ERROR_SUCCESS;
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
	void			Set(const AutoUTF &name, const AutoUTF &value) const {
		Set(name, value.c_str());
	}
	void			Set(const AutoUTF &name, int value) const;

	bool			Get(const AutoUTF &name, AutoUTF &value, const AutoUTF &def) const;
	bool			Get(const AutoUTF &name, int &value, int def) const;
};

///=========================================================================================== Win64
/// Функции работы с WOW64
namespace	Win64 {
bool				WowDisable(PVOID &oldValue);
bool				WowEnable(PVOID &oldValue);
bool				IsWOW64();
}

///====================================================================================== WinSysInfo
struct		WinSysInfo: public SYSTEM_INFO {
	WinSysInfo();
	size_t			Uptime(size_t del = 1000);
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
