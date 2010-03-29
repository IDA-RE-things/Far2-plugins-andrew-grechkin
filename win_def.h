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
#include <stdint.h>

class		CStrA;
class		CStrW;

#ifdef DEBUG
#include <iostream>
#include <ostream>
using	std::cout;
using	std::ostream;
using	std::endl;
ostream			&operator<<(ostream &s, PCWSTR rhs);
#endif

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

typedef const void			*PCVOID;

#ifdef NoStdNew
inline void*	operator new(size_t size) {
	return	::HeapAlloc(::GetProcessHeap(), HEAP_ZERO_MEMORY, size);
}
inline void*	operator new[](size_t size) {
	return ::operator new(size);
}
inline void		operator delete(void *in) {
	::HeapFree(::GetProcessHeap(), 0, (PVOID)in);
}
inline void 	operator delete[](void *ptr) {
	::operator	delete(ptr);
}
#endif

bool				consoleout(PCWSTR in, DWORD nStdHandle = STD_OUTPUT_HANDLE);
bool				consoleout(const CStrW &in, DWORD nStdHandle = STD_ERROR_HANDLE);

inline void			mbox(PCSTR	text, PCSTR capt = "") {
	::MessageBoxA(NULL, text, capt, MB_OK);
}
inline void			mbox(PCWSTR	text, PCWSTR capt = L"") {
	::MessageBoxW(NULL, text, capt, MB_OK);
}

///============================================================================================ path
CStrW				Expand(const CStrW &path);
CStrW				Canonicalize(const CStrW &path);
CStrW				Validate(const CStrW &path);

///============================================================================================ Exec
extern DWORD		EXEC_TIMEOUT;
extern DWORD		EXEC_TIMEOUT_DX;
bool				Exec(const CStrW &cmd);
int					Exec(const CStrW &cmd, CStrA &out);
int					Exec(const CStrW &cmd, CStrA &out, const CStrA &in);
int					ExecWait(const CStrW &cmd, DWORD wait = EXEC_TIMEOUT);

///=================================================================================================
inline void			XchgByte(WORD &inout) {
	inout = inout >> 8 | inout << 8;
}
inline void			XchgWord(DWORD &inout) {
	inout = inout >> 16 | inout << 16;
}

template <typename Type>
inline	const Type	&Min(const Type &a, const Type &b) {
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
	Shared_ptr():data(new Pointee<Type>(NULL)) {
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

///========================================================================================== WinStr
/// Функции работы символьными строками
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

inline bool			IsSpace(WCHAR in) {
	return	in == L' ' || in == L'\t';
}
inline bool			IsEol(WCHAR in) {
	return	in == L'\r' || in == L'\n';
}
inline bool			IsUpper(WCHAR in) {
	return	::IsCharUpper(in);
}
inline bool			IsLower(WCHAR in) {
	return	::IsCharLower(in);
}
inline bool			IsAlpha(char in) {
	return	::IsCharAlphaA(in);
}
inline bool			IsAlpha(WCHAR in) {
	return	::IsCharAlphaW(in);
}
inline bool			IsAlphaNum(WCHAR in) {
	return	::IsCharAlphaNumeric(in);
}

inline int 			StrCmpNNI(PCWSTR s1, int n1, PCWSTR s2, int n2) {
	return	::CompareStringW(0, NORM_IGNORECASE | NORM_STOP_ON_NULL | SORT_STRINGSORT, s1, n1, s2, n2) - 2;
}
inline int 			StrCmpNI(PCWSTR s1, PCWSTR s2, int n) {
	return	::StrCmpNNI(s1, n, s2, n);
}
inline int 			StrCmpI(PCWSTR s1, PCWSTR s2) {
	return	::CompareStringW(0, NORM_IGNORECASE | SORT_STRINGSORT, s1, -1, s2, -1) - 2;
}
inline int 			StrCmpNN(PCWSTR s1, int n1, PCWSTR s2, int n2) {
	return	::CompareStringW(0, NORM_STOP_ON_NULL | SORT_STRINGSORT, s1, n1, s2, n2) - 2;
}
inline int 			StrCmpN(PCWSTR s1, PCWSTR s2, int n) {
	return	::StrCmpNN(s1, n, s2, n);
}
inline int 			StrCmp(PCWSTR s1, PCWSTR s2) {
	return	::CompareStringW(0, SORT_STRINGSORT, s1, -1, s2, -1) - 2;
}

inline int			Cmp(PCSTR in1, PCSTR in2) {
	return	::strcmp(in1, in2);
}
inline int			Cmp(PCSTR in1, PCSTR in2, size_t n) {
	return	::strncmp(in1, in2, n);
}
inline int			Cmp(PCWSTR in1, PCWSTR in2) {
	return	::wcscmp(in1, in2);
}
inline int			Cmp(PCWSTR in1, PCWSTR in2, size_t n) {
	return	::wcsncmp(in1, in2, n);
}
inline int			Cmpi(PCSTR in1, PCSTR in2) {
	return	::_stricmp(in1, in2);
}
inline int			Cmpi(PCWSTR in1, PCWSTR in2) {
	return	::_wcsicmp(in1, in2);
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

PWSTR				CharFirstOf(PCWSTR in, PCWSTR mask);
PWSTR				CharFirstNotOf(PCWSTR in, PCWSTR mask);
PWSTR				CharLastOf(PCWSTR in, PCWSTR mask);
PWSTR				CharLastNotOf(PCWSTR in, PCWSTR mask);

inline LONGLONG		AsLongLong(PCSTR in) {
	return	_atoi64(in);
}
inline ULONG		AsULong(PCSTR in) {
	return	(unsigned long)AsLongLong(in);
}
inline long			AsLong(PCSTR in) {
	return	(long)AsLongLong(in);
}
inline UINT			AsUInt(PCSTR in) {
	return	(unsigned int)AsLongLong(in);
}
inline int			AsInt(PCSTR in) {
	return	(int)AsLongLong(in);
}

inline LONGLONG		AsLongLong(PCWSTR in) {
	return	_wtoi64(in);
}
inline ULONG		AsULong(PCWSTR in) {
	return	(unsigned long)AsLongLong(in);
}
inline long			AsLong(PCWSTR in) {
	return	(long)AsLongLong(in);
}
inline UINT			AsUInt(PCWSTR in) {
	return	(unsigned int)AsLongLong(in);
}
inline int			AsInt(PCWSTR in) {
	return	(int)AsLongLong(in);
}

inline WCHAR		ToUpper(WCHAR in) {
	::CharUpperBuffW(&in, 1);
	return	in;
}
inline WCHAR 		ToLower(WCHAR in) {
	::CharLowerBuffW(&in, 1);
	return	in;
}

inline void			ToUpper(PWSTR Buf, int Length) {
	::CharUpperBuffW(Buf, Length);
}
inline void			ToLower(PWSTR Buf, int Length) {
	::CharLowerBuffW(Buf, Length);
}
inline void			ToUpper(PWSTR s1) {
	ToUpper(s1, Len(s1));
}
inline void			ToLower(PWSTR s1) {
	ToLower(s1, Len(s1));
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

///========================================================================================= WinTime
struct		WinTime: public FILETIME {
private:
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
	uint64_t			operator-(const WinTime &in) {
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
	WinBuf(size_t size, bool byte = false): m_buf(NULL), m_size((byte) ? size : size * sizeof(Type)) {
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
	bool			reserve(size_t size, bool byte = false) {
		if (!byte) {
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

/// Обертка буфера с подсчетом ссылок
template<typename Type>
class		WinBufferCtr: private Uncopyable {
	Type		*m_buf;
	size_t		m_ctr;
	size_t		m_capacity;

public:
	~WinBufferCtr() {
		Dec();
	}
	WinBufferCtr(): m_buf(NULL),  m_ctr(0), m_capacity(0) {
	}
	WinBufferCtr(size_t capa): m_buf(NULL),  m_ctr(0) {
		Init(capa);
	}
	bool			Init(size_t capa) {
		m_capacity = capa;
		if (WinMem::Realloc(m_buf, (m_capacity + 1) * sizeof(Type))) {
			m_ctr = 1;
			return	true;
		}
		return	false;
	}
	bool			Init(const WinBufferCtr<Type> &src) {
		if (WinMem::Realloc(m_buf, (src.m_capacity + 1) * sizeof(Type))) {
			m_ctr = 1;
			m_capacity = src.m_capacity;
			WinMem::Copy(m_buf, src.m_buf, size());
			return	true;
		}
		return	false;
	}

	Type			*Buf() const {
		return	m_buf;
	}

	size_t			size() const {
		return	capacity() * sizeof(Type);
	}
	size_t			capacity() const {
		return	m_capacity;
	}
	bool			reserve(size_t capa) {
		if (capacity() < capa) {
			m_capacity = capa;
			return	WinMem::Realloc(m_buf, (m_capacity + 1) * sizeof(Type));
		}
		return	false;
	}
	void			zero() {
//		m_buf[0] = 0;
		WinMem::Zero(m_buf, size());
	}

	size_t			links() const {
		return	m_ctr;
	}
	bool			IsSingle() const {
		return	m_ctr <= 1;
	}

	void			Inc() {
		++m_ctr;
	}
	bool			Dec() {
		if (--m_ctr == 0) {
			WinMem::Free(m_buf);
			return	true;
		}
		return	false;
	}

	WinBufferCtr<Type> *split() {
		if (!IsSingle()) {
			WinBufferCtr<Type>	*tmp;
			WinMem::Alloc(tmp, sizeof(*tmp));
			tmp->Init(*this);
			Dec();
			return	tmp;
		}
		return	this;
	}
};

///============================================================================================ CStr
/// Строки с счетчиком ссылок, передача по значению обходится очень дешево
class		CStrA {
	WinBufferCtr<CHAR>	*m_data;

	void				Alloc(WinBufferCtr<CHAR>* &data, size_t size) {
		WinMem::Alloc(data, sizeof(*data));
		data->Init(size);
	}
	void				Assign(WinBufferCtr<CHAR>* &data, PCSTR in, size_t size) {
		WinMem::Alloc(data, sizeof(*data));
		data->Init(size);
		Copy(data->Buf(), in, data->capacity());
	}
	void				Release(WinBufferCtr<CHAR>* &data) {
		if (data && data->Dec()) {
			WinMem::Free(data);
			data = NULL;
		}
	}

public:
	~CStrA() {
		Release(m_data);
	}
	CStrA(): m_data(NULL) {
		Alloc(m_data, 0);
	}
	CStrA(const CStrA &in) {
		in.m_data->Inc();
		m_data = in.m_data;
	}
	CStrA(PCSTR in, size_t num = 0) {
		if (num == 0)
			num = Len(in);
		Assign(m_data, in, num);
	}
	CStrA(PCWSTR in, UINT cp) {
		Alloc(m_data, ::WideCharToMultiByte(cp, 0, in, -1, NULL, 0, NULL, NULL));
		::WideCharToMultiByte(cp, 0, in, -1, m_data->Buf(), (int)m_data->capacity(), NULL, NULL);
	}
	CStrA(size_t in) {
		Alloc(m_data, in);
	}

	const CStrA			&operator=(const CStrA & in) {
		if (m_data != in.m_data) {
			Release(m_data);
			m_data = in.m_data;
			m_data->Inc();
		}
		return	*this;
	}
	const CStrA			&operator=(PCSTR in) {
		WinBufferCtr<CHAR>	*tmp;
		Assign(tmp, in, Len(in));
		Swp(m_data, tmp);
		Release(tmp);
		return	*this;
	}
	const CStrA			&operator=(CHAR in) {
		CHAR tmp[] = {in, '\0'};
		operator+=(tmp);
		return	*this;
	}
	CStrA				&operator+=(const CStrA & in) {
		if (m_data != in.m_data) {
			operator+=(in.m_data->Buf());
		}
		return	*this;
	}
	CStrA				&operator+=(PCSTR in) {
		m_data = m_data->split();
		m_data->reserve(m_data->capacity() + Len(in));
		Cat(m_data->Buf(), in);
		return	*this;
	}
	CStrA				&operator+=(CHAR in) {
		CHAR tmp[] = {in, '\0'};
		operator+=(tmp);
		return	*this;
	}

	CStrA				operator+(const CStrA &in) {
		CStrA	tmp(*this);
		tmp += in;
		return	tmp;
	}
	CStrA				operator+(PCSTR in) {
		CStrA	tmp(*this);
		tmp += in;
		return	tmp;
	}
	CStrA				operator+(CHAR in) {
		CStrA	tmp(*this);
		tmp += in;
		return	tmp;
	}

	bool				operator==(const CStrA &in) const {
		return	Eq(m_data->Buf(), in.m_data->Buf());
	}
	bool				operator==(PCSTR in) const {
		return	Eq(m_data->Buf(), in);
	}

	bool				operator!=(const CStrA &in) const {
		return	!operator==(in);
	}
	bool				operator!=(PCSTR in) const {
		return	!operator==(in);
	}

	bool				operator<(const CStrA &in) const {
		return	Cmp(m_data->Buf(), in.m_data->Buf()) < 0;
	}
	bool				operator<(PCSTR in) const {
		return	Cmp(m_data->Buf(), in) < 0;
	}

	bool				operator>(const CStrA &in) const {
		return	Cmp(m_data->Buf(), in.m_data->Buf()) > 0;
	}
	bool				operator>(PCSTR in) const {
		return	Cmp(m_data->Buf(), in) > 0;
	}

	bool				operator<=(const CStrA &in) const {
		return	operator==(in) || operator<(in);
	}
	bool				operator<=(PCSTR in) const {
		return	operator==(in) || operator<(in);
	}

	bool				operator>=(const CStrA &in) const {
		return	operator==(in) || operator>(in);
	}
	bool				operator>=(PCSTR in) const {
		return	operator==(in) || operator>(in);
	}

	CHAR				&operator[](int in) {
		m_data = m_data->split();
		return	m_data->Buf()[in];
	}
	const CHAR			&operator[](int in) const {
		return	m_data->Buf()[in];
	}

	size_t				links() const {
		return	m_data->links();
	}
	size_t				capacity() const {
		return	m_data->capacity();
	}
	size_t				size() const {
		return	Len(m_data->Buf());
	}
	bool				empty() const {
		return	(m_data && m_data->Buf() && (m_data->Buf()[0] == '\0'));
	}
	void				reserve(size_t size) {
		m_data = m_data->split();
		m_data->reserve(size);
	}
	void				clear() {
		m_data = m_data->split();
		m_data->zero();
	}

	operator			PCSTR() const {
		return	m_data->Buf();
	}
	PCSTR				c_str() const {
		return	m_data->Buf();
	}
	PSTR				buffer() {
		m_data = m_data->split();
		return	(PSTR)m_data->Buf();
	}

	CStrA				&cp(PCWSTR in, UINT cp) {
		m_data = m_data->split();
		m_data->reserve(::WideCharToMultiByte(cp, 0, in, -1, NULL, 0, NULL, NULL));
		::WideCharToMultiByte(cp, 0, in, -1, m_data->Buf(), (int)m_data->capacity(), NULL, NULL);
		return	*this;
	}
};
class		CStrW {
	WinBufferCtr<WCHAR>	*m_data;

	void				Alloc(WinBufferCtr<WCHAR>* &data, size_t size) {
		WinMem::Alloc(data, sizeof(*data));
		data->Init(size);
	}
	void				Assign(WinBufferCtr<WCHAR>* &data, PCWSTR in, size_t size) {
		if (in == NULL) {
			Alloc(m_data, 0);
			return;
		}
		WinMem::Alloc(data, sizeof(*data));
		data->Init(size);
		Copy(data->Buf(), in, data->capacity());
	}
	void				Release(WinBufferCtr<WCHAR>* &data) {
		if (data && data->Dec()) {
			WinMem::Free(data);
			data = NULL;
		}
	}

public:
	~CStrW() {
		Release(m_data);
	}
	CStrW(): m_data(NULL) {
		Alloc(m_data, 0);
	}
	CStrW(const CStrW &in) {
		in.m_data->Inc();
		m_data = in.m_data;
	}
	CStrW(PCWSTR in, size_t num = 0) {
		if (in && num == 0)
			num = Len(in);
		Assign(m_data, in, num);
	}
	CStrW(PCSTR in, UINT cp) {
		Alloc(m_data, ::MultiByteToWideChar(cp, 0, in, -1, NULL, 0));
		::MultiByteToWideChar(cp, 0, in, -1, m_data->Buf(), (int)m_data->capacity());
	}
	CStrW(size_t in) {
		Alloc(m_data, in);
	}

	const CStrW			&operator=(const CStrW & in) {
		if (m_data != in.m_data) {
			Release(m_data);
			m_data = in.m_data;
			m_data->Inc();
		}
		return	*this;
	}
	const CStrW			&operator=(WCHAR in) {
		WCHAR tmp[] = {in, L'\0'};
		operator+=(tmp);
		return	*this;
	}
	CStrW				&operator+=(const CStrW & in) {
		if (m_data != in.m_data) {
			operator+=(in.m_data->Buf());
		}
		return	*this;
	}
	CStrW				&operator+=(PCWSTR in) {
		m_data = m_data->split();
		m_data->reserve(m_data->capacity() + Len(in));
		Cat(m_data->Buf(), in);
		return	*this;
	}
	CStrW				&operator+=(WCHAR in) {
		WCHAR tmp[] = {in, L'\0'};
		operator+=(tmp);
		return	*this;
	}

	CStrW				operator+(const CStrW &in) {
		CStrW	tmp(*this);
		tmp += in;
		return	tmp;
	}
	CStrW				operator+(PCWSTR in) {
		CStrW	tmp(*this);
		tmp += in;
		return	tmp;
	}
	CStrW				operator+(WCHAR in) {
		CStrW	tmp(*this);
		tmp += in;
		return	tmp;
	}

	bool				operator==(const CStrW &in) const {
		return	Eq(m_data->Buf(), in.m_data->Buf());
	}
	bool				operator==(PCWSTR in) const {
		return	Eq(m_data->Buf(), in);
	}

	bool				operator!=(const CStrW &in) const {
		return	!operator==(in);
	}
	bool				operator!=(PCWSTR in) const {
		return	!operator==(in);
	}

	bool				operator<(const CStrW &in) const {
		return	Cmp(m_data->Buf(), in.m_data->Buf()) < 0;
	}
	bool				operator<(PCWSTR in) const {
		return	Cmp(m_data->Buf(), in) < 0;
	}

	bool				operator>(const CStrW &in) const {
		return	Cmp(m_data->Buf(), in.m_data->Buf()) > 0;
	}
	bool				operator>(PCWSTR in) const {
		return	Cmp(m_data->Buf(), in) > 0;
	}

	bool				operator<=(const CStrW &in) const {
		return	operator==(in) || operator<(in);
	}
	bool				operator<=(PCWSTR in) const {
		return	operator==(in) || operator<(in);
	}

	bool				operator>=(const CStrW &in) const {
		return	operator==(in) || operator>(in);
	}
	bool				operator>=(PCWSTR in) const {
		return	operator==(in) || operator>(in);
	}

	WCHAR				&operator[](int in) {
		m_data = m_data->split();
		return	m_data->Buf()[in];
	}
	const WCHAR			&operator[](int in) const {
		return	m_data->Buf()[in];
	}

	size_t				links() const {
		return	m_data->links();
	}
	size_t				capacity() const {
		return	m_data->capacity();
	}
	size_t				size() const {
		return	Len(m_data->Buf());
	}
	bool				empty() const {
		return	(m_data && m_data->Buf() && (m_data->Buf()[0] == '\0'));
	}
	void				reserve(size_t size) {
		m_data = m_data->split();
		m_data->reserve(size);
	}
	void				clear() {
		m_data = m_data->split();
		m_data->zero();
	}

	size_t				data_length() const {
		return	Len(m_data->Buf())*sizeof(WCHAR);
	}
	operator			PCWSTR() const {
		return	m_data->Buf();
	}
	PCWSTR				c_str() const {
		return	m_data->Buf();
	}
	PWSTR				buffer() {
		m_data = m_data->split();
		return	(PWSTR)m_data->Buf();
	}

	CStrW				&cp(PCSTR in, UINT cp) {
		m_data = m_data->split();
		m_data->reserve(::MultiByteToWideChar(cp, 0, in, -1, NULL, 0));
		::MultiByteToWideChar(cp, 0, in, -1, m_data->Buf(), (int)m_data->capacity());
		return	*this;
	}
	CStrA				utf8() const {
		CStrA	tmp(c_str(), CP_UTF8);
		return	tmp;
	}

	bool				cout() const {
		return	consoleout(*this, STD_OUTPUT_HANDLE);
	}
	bool				cerr() const {
		return	consoleout(*this, STD_ERROR_HANDLE);
	}

	CStrW&				AddNum(intmax_t in) {
		CStrW	tmp(32);
		::_i64tow(in, (PWSTR)tmp.c_str(), 10);
		operator+=(tmp);
		return	*this;
	}

// static
	static CStrW		time(const SYSTEMTIME &in, bool tolocal = true) {
		CStrW		Result(MAX_PATH);
		SYSTEMTIME	stTime;
		if (tolocal) {
			::SystemTimeToTzSpecificLocalTime(NULL, (SYSTEMTIME*)&in, &stTime);
		} else {
			stTime = in;
		}
		_snwprintf((PWSTR)Result.c_str(), Result.capacity(), L"%04d-%02d-%02d %02d:%02d:%02d",
				   stTime.wYear, stTime.wMonth, stTime.wDay,
				   stTime.wHour, stTime.wMinute, stTime.wSecond);
		return	Result;
	}
	static CStrW		time(const FILETIME &in) {
		SYSTEMTIME	stUTC;
		::FileTimeToSystemTime(&in, &stUTC);
		return	time(stUTC);
	}
	static CStrW		interval(uint64_t in) {
		CStrW	Result(128);
		intmax_t	tmp = in / WinTime::SecPerDay();
		if (tmp != 0) {
			Result += tmp;
			Result += L" days ";
			in = in % WinTime::SecPerDay();
		}
		tmp = in / WinTime::SecPerHour();
		Result += tmp;
		Result += L":";
		in = in % WinTime::SecPerHour();
		tmp = in / 60;
		Result += tmp;
		Result += L":";
		in = in % 60;
		Result += (intmax_t)in;

		return	Result;
	}
	static bool			cout(const CStrW &in) {
		return	consoleout(in, STD_OUTPUT_HANDLE);
	}
	bool				cerr(const CStrW &in) {
		return	consoleout(in, STD_ERROR_HANDLE);
	}
};
class		CStrMW {
	class	MzsData {
		PWSTR		m_data;
		size_t		m_capa;
		size_t		m_size;
	public:
		~MzsData() {
			delete[] m_data;
		}
		explicit MzsData(PCWSTR in):m_capa(0), m_size(0) {
			PCWSTR	ptr = in;
			while (*ptr) {
				ptr = ptr + Len(ptr) + 1;
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
	CStrMW(PCWSTR in = L""):m_str(new MzsData(in)) {
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
			ptr = ptr + Len(ptr) + 1;
		}
		return	ptr;
	}
};

inline void			Num2Str(PWSTR str, int in, int base = 10) {
	::_itow(in, str, base);
}
inline CStrW		Num2Str(int in, int base = 10) {
	CStrW	buf(12);
	::_itow(in, (PWSTR)buf.c_str(), base);
	return	buf.c_str();
}

inline CStrW		Err(HRESULT err = ::GetLastError(), PCWSTR lib = NULL) {
	CStrW	Result;
	HMODULE	mod = NULL;
	if (err != 0 && lib) {
		mod = ::LoadLibraryW(lib);
	}
	PWSTR	buf = NULL;
	::FormatMessageW(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | ((mod) ?  FORMAT_MESSAGE_FROM_HMODULE : FORMAT_MESSAGE_FROM_SYSTEM),
		mod,
		err,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(PWSTR)&buf, 0, NULL);
	Result = (buf) ? buf : L"Unknown error\r\n";
	::LocalFree(buf);
	Result[(int)Result.size()-2] = L'\0';
	if (mod)
		::FreeLibrary(mod);
	return	Result;
}
inline CStrW		ErrWmi(HRESULT err) {
	return	Err(err, L"wmiutils.dll");
}

inline void			mbox(HRESULT err, PCWSTR lib = NULL) {
	::MessageBoxW(NULL, Err(err, lib), L"Error", MB_OK);
}

#ifdef DEBUG
inline ostream		&operator<<(ostream &s, PCWSTR rhs) {
	CStrA	oem(rhs, CP_OEMCP);
	return	(s << oem.c_str());
}
inline ostream		&operator<<(ostream &s, const CStrW &rhs) {
	CStrA	oem(rhs, CP_OEMCP);
	return	(s << oem.c_str());
}
#endif

/*
class		BOM {
public:
	static	PCSTR	utf8() {
		static	const char bom_utf8[] = {'\xEF', '\xBB', '\xBF', '\xAA'};
		return	bom_utf8;
	}
	static	PCSTR	utf16le() {
		static	const char bom_utf16le[] = {'\xFF', '\xFE', '\xAA'};
		return	bom_utf16le;
	}
	static	PCSTR	utf16be() {
		static	const char bom_utf16be[] = {'\xFE', '\xFF', '\xAA'};
		return	bom_utf16be;
	}
	static	PCSTR	utf32le() {
		static	const char bom_utf32le[] = {'\xFF', '\xFE', '\0', '\0', '\xAA'};
		return	bom_utf32le;
	}
	static	PCSTR	utf32be() {
		static	const char bom_utf32be[] = {'\0', '\0', '\xFE', '\xFF', '\xAA'};
		return	bom_utf32be;
	}
//	00 00 FE FF	UTF-32, big-endian
//	FF FE 00 00	UTF-32, little-endian
//	FE FF	UTF-16, big-endian
//	FF FE	UTF-16, little-endian
//	EF BB BF	UTF-8
};
*/
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

///========================================================================================== WinEnv
namespace	WinEnv {
CStrW		Get(PCWSTR name);
bool		Set(PCWSTR name, PCWSTR val);
bool		Add(PCWSTR name, PCWSTR val);
bool		Del(PCWSTR name);
}

///=========================================================================================== WinFS
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

inline CStrW		TempDir() {
	CStrW	buf(::GetTempPath(0, NULL));
	::GetTempPathW(buf.capacity(), buf.buffer());
	return	buf.c_str();
}
inline CStrW		TempFile(PCWSTR s) {
	CStrW	buf(MAX_PATH);
	::GetTempFileNameW(TempDir().c_str(), s, 0, buf.buffer());
	return	buf.c_str();
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
inline bool			IsDir(PCWSTR path) {
	return	(::GetFileAttributesW(path) & FILE_ATTRIBUTE_DIRECTORY) != 0;
}
inline bool			IsJunc(PCWSTR path) {
	return	(::GetFileAttributesW(path) & FILE_ATTRIBUTE_REPARSE_POINT) != 0;
}

namespace	WinFS {
inline	CStrW	ExtractPath(PCWSTR path) {
	size_t	len = Len(path);
	PWSTR	ch = CharLast((PWSTR)path, PATH_SEPARATOR_C);
	if (ch && ch < (path + len)) {
		return	CStrW(path, ch - path);
	}
	return	L"";
}
inline	CStrW	ExtractFile(PCWSTR path) {
	size_t	len = Len(path);
	PWSTR	ch = CharLast((PWSTR)path, PATH_SEPARATOR_C);
	if (ch && ++ch < (path + len)) {
		return	CStrW(ch);
	}
	return	L"";
}

inline	CStrW&	AddSlash(CStrW &path) {
	size_t	size = path.size();
	if (size != 0) {
		if (path[size-1] != PATH_SEPARATOR_C) {
			path.reserve(path.size() + 2);
			path[size] = PATH_SEPARATOR_C;
			path[size + 1] = L'\0';
		}
	}
	return	path;
}
inline	CStrW	Expand(PCWSTR path) {
	CStrW	tmp(::ExpandEnvironmentStringsW(path, NULL, 0));
	::ExpandEnvironmentStringsW(path, (PWSTR)tmp.c_str(), (DWORD)tmp.capacity());
	return	tmp;
}

inline bool		Copy(PCWSTR path, PCWSTR dest) {
	return	::CopyFileW(path, dest, true) != 0;
}
inline bool		Move(PCWSTR path, PCWSTR dest) {
	return	::MoveFileW(path, dest) != 0;
}
inline bool		Move(PCWSTR path, PCWSTR dest, bool copy) {
	return	::MoveFileExW(path, dest, (copy) ? MOVEFILE_COPY_ALLOWED : 0) != 0;
}
inline bool		DelDir(PCWSTR	path) {
	::SetFileAttributesW(path, FILE_ATTRIBUTE_NORMAL);
	return	::RemoveDirectoryW(path) != 0;
}
inline bool		DelFile(PCWSTR	path) {
	return	(::DeleteFileW(path) != 0) ?
		   true :
		   (::SetFileAttributesW(path, FILE_ATTRIBUTE_NORMAL) ||
			::DeleteFileW(path));
}

inline bool		FileRead(HANDLE hFile, PBYTE buf, DWORD &size) {
	return	::ReadFile(hFile, buf, size, &size, NULL) != 0;
}
inline bool		FileRead(PCWSTR	path, CStrA &buf) {
	bool	Result = false;
	HANDLE	hFile = ::CreateFileW(path, GENERIC_READ, 0, NULL, OPEN_EXISTING,
								 FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS, NULL);
	if (hFile != INVALID_HANDLE_VALUE) {
		DWORD	size = (DWORD)FileSize(hFile);
		buf.reserve(size);
		Result = ::ReadFile(hFile, buf.buffer(), buf.size(), &size, NULL) != 0;
		::CloseHandle(hFile);
	}
	return	Result;
}
/*
inline bool		FileWrite(PCWSTR path, const CStrA &buf)
{
	bool	Result = false;
	HANDLE	hFile = ::CreateFileW(path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
	                             FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS, NULL);
	if (hFile != INVALID_HANDLE_VALUE) {
		DWORD	written;
		Result = ::WriteFile(hFile, buf.c_str(), (DWORD)buf.size(), &written, NULL) != 0;
		::CloseHandle(hFile);
	}
	return	Result;
}
*/
inline bool		FileWrite(PCWSTR path, const PCVOID buf, size_t size, bool rewrite = false) {
	bool	Result = false;
	DWORD	dwCreationDisposition = rewrite ? CREATE_ALWAYS : CREATE_NEW;
	HANDLE	hFile = ::CreateFileW(path, GENERIC_WRITE, 0, NULL, dwCreationDisposition, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile != INVALID_HANDLE_VALUE) {
		DWORD	cbWritten = 0;
		Result = ::WriteFile(hFile, buf, size, &cbWritten, NULL) != 0;
		::CloseHandle(hFile);
	}
	return	Result;
}
}

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
	CStrW				m_path;
	CStrW				m_mask;

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
	WinDir(const CStrW &path, const CStrW &mask = L"*"): m_handle(NULL), m_path(path), m_mask(mask) {
	}

	bool 			Next() {
		CStrW	tmp(m_path);
		WinFS::AddSlash(tmp);
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

	void			path(const CStrW &in) {
		Close();
		m_path = in;
	}
	void			mask(const CStrW &in) {
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
	CStrW			name_full() const {
		CStrW	Result(m_path);
		WinFS::AddSlash(Result);
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

///========================================================================================= WinGUID
namespace	WinGUID {
CStrW			Gen();
}

#endif // WIN_DEF_HPP
