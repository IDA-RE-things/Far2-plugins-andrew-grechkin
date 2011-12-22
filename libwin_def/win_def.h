/**
	win_def
	Main windows application include, always include first
	@classes	(WinMem, WinFlag, WinBit, WinTimer, WinErrorCheck, Uncopyable)
	@author		© 2010 Andrew Grechkin
	@link		(shlwapi, )
	@link		(ole32) for WinCom
**/

#ifndef WIN_DEF_HPP
#define WIN_DEF_HPP

#include "std.h"
#include "shared_ptr.h"

#include <stdio.h>

extern "C" {
	INT WINAPI	SHCreateDirectoryExA(HWND, PCSTR, PSECURITY_ATTRIBUTES);
	INT WINAPI	SHCreateDirectoryExW(HWND, PCWSTR, PSECURITY_ATTRIBUTES);
//	WINBASEAPI ULONGLONG WINAPI	GetTickCount64();
}

///===================================================================================== definitions
astring		Hash2Str(const PBYTE buf, size_t size);
astring		Hash2StrNum(const PBYTE buf, size_t size);
bool		Str2Hash(const astring &str, PVOID &hash, ULONG &size);

UINT		CheckUnicode(const PVOID buf, size_t size);
UINT		IsUTF8(const PVOID buf, size_t size);
bool		GetCP(HANDLE hFile, UINT &cp, bool bUseHeuristics = false);

ustring		AsStr(const SYSTEMTIME &in, bool tolocal = true);
ustring		AsStr(const FILETIME &in);

ustring		CopyAfterLast(const ustring &in, const ustring &delim);

ustring&	Cut(ustring &inout, const ustring &in);
bool		Cut(ustring &inout, ssize_t &num, int base = 10);
ustring&	CutAfter(ustring &inout, const ustring &delim);
ustring&	CutBefore(ustring &inout, const ustring &delim);

ustring&	ToLower(ustring &inout);
ustring		ToLowerOut(const ustring &in);
ustring&	ToUpper(ustring &inout);
ustring		ToUpperOut(const ustring &in);

/////=================================================================================== WinErrorCheck
///// Базовый класс для проверки и хранения кода ошибки
//class		WinErrorCheck {
//	mutable DWORD	m_err;
//protected:
//	~WinErrorCheck() {
//	}
//	WinErrorCheck(): m_err(NO_ERROR) {
//	}
//public:
//	DWORD			err() const {
//		return m_err;
//	}
//	DWORD			err(DWORD err) const {
//		return (m_err = err);
//	}
//	bool			IsOK() const {
//		return m_err == NO_ERROR;
//	}
//	bool			ChkSucc(bool in) const {
//		if (!in) {
//			err(::GetLastError());
//		} else {
//			err(NO_ERROR);
//		}
//		return in;
//	}
//	template<typename Type>
//	void			SetIfFail(Type &in, const Type &value) {
//		if (m_err != NO_ERROR) {
//			in = value;
//		}
//	}
//};

///=========================================================================================== Types
template <typename Type>
struct		NamedValues {
	PCWSTR	name;
	Type	value;

	static	PCWSTR	GetName(NamedValues<Type> dim[], size_t size, const Type &in) {
		for (size_t i = 0; i < size; ++i) {
			if (dim[i].value == in) {
				return dim[i].name;
			}
		}
		return L"unknown";
	}
};

///========================================================================================= mstring
class		mstring {
public:
	mstring(PCWSTR in = L""): m_str(new impl(in)) {
	}

	size_t	size() const {
		return m_str->m_size;
	}

	size_t	capacity() const {
		return m_str->m_capa;
	}

	PCWSTR	c_str() const {
		return m_str->m_data;
	}

	PCWSTR	operator[](int index) const {
		PCWSTR	ptr = c_str();
		int		cnt = 0;
		while (*ptr && (cnt++ < index)) {
			ptr += (Len(ptr) + 1);
		}
		return ptr;
	}

private:
	class	impl {
	public:
		~impl() {
			WinMem::Free(m_data);
		}

		explicit	impl(PCWSTR in): m_size(0) {
			if (!in)
				in = EMPTY_STR;
			PCWSTR	ptr = in;
			while (*ptr) {
				ptr += (Len(ptr) + 1);
				++m_size;
			}
			m_capa = ptr - in + 1;
			WinMem::Alloc(m_data, sizeof(WCHAR) * m_capa);
			WinMem::Copy(m_data, in, m_capa * sizeof(WCHAR));
		}
	private:
		PWSTR	m_data;
		size_t	m_capa;
		size_t	m_size;

		friend class mstring;
	};

	winstd::shared_ptr<impl>	m_str;
};

///========================================================================================= BitMask
template<typename Type>
struct		BitMask {
	static Type		FromStr(const ustring &in, size_t lim = 0) {
		// count bits from 1
		Type	Result = 0;
		intmax_t	bit = 0;
		ustring	tmp(in);
		lim = WinBit::Limit<Type>(lim);
		while (tmp.Cut(bit)) {
			if (!WinBit::BadBit<Type>(--bit))
				WinBit::Set(Result, bit);
		}
		return Result;
	}
	static Type		FromStr0(const ustring &in, size_t lim = 0) {
		// count bits from zero
		Type	Result = 0;
		ssize_t	bit = 0;
		ustring	tmp(in);
		lim = WinBit::Limit<Type>(lim);
		while (tmp.Cut(bit)) {
			if (!WinBit::BadBit<Type>(bit))
				WinBit::Set(Result, bit);
		}
		return Result;
	}

	static ustring	AsStr(Type in, size_t lim = 0) {
		// count bits from 1
		ustring	Result;
		lim = WinBit::Limit<Type>(lim);
		for (size_t bit = 0; bit < lim; ++bit) {
			if (WinBit::Check(in, bit)) {
				Result.Add(Num2Str(bit + 1), L",");
			}
		}
		return Result;
	}
	static ustring	AsStr0(Type in, size_t lim = 0) {
		// count bits from zero
		ustring	Result;
		lim = WinBit::Limit<Type>(lim);
		for (size_t	bit = 0; bit < lim; ++bit) {
			if (WinBit::Check(in, bit)) {
				Result.Add(Num2Str(bit), L",");
			}
		}
		return Result;
	}
	static ustring	AsStrBin(Type in, size_t lim = 0) {
		ustring	Result;
		uintmax_t	flag = (uintmax_t)1 << (WinBit::Limit<Type>(lim) - 1);
		while (flag) {
			Result += WinFlag::Check(in, (Type)flag) ? L'1' : L'0';
			flag >>= 1;
		}
		return Result;
	}
	static ustring	AsStrNum(Type in, size_t lim = 0) {
		ustring	Result;
		uintmax_t	flag = (uintmax_t)1 << (WinBit::Limit<Type>(lim) - 1);
		while (flag) {
			if (WinFlag::Check(in, (Type)flag)) {
				Result.Add(Num2Str(flag), L",");
			}
			flag >>= 1;
		}
		return Result;
	}
};

///▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒ env
///========================================================================================== WinEnv
namespace	WinEnv {
	ustring	Get(PCWSTR name);
	bool	Set(PCWSTR name, PCWSTR val);
	bool	Add(PCWSTR name, PCWSTR val);
	bool	Del(PCWSTR name);
}

inline DWORD	UserLogon(HANDLE &hToken, PCWSTR name, PCWSTR pass, DWORD type, PCWSTR dom = L"") {
	DWORD	Result = NO_ERROR;
	if (!::LogonUserW((PWSTR)name, (PWSTR)dom, (PWSTR)pass, type, LOGON32_PROVIDER_DEFAULT, &hToken)) {
		Result = ::GetLastError();
	}
	return Result;
}

///▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒ file_path
///============================================================================================ path
ustring			Canonicalize(PCWSTR path);
inline ustring	Canonicalize(const ustring &path) {
	return Canonicalize(path.c_str());
}

ustring			Expand(PCWSTR path);
inline ustring	Expand(const ustring &path) {
	return Expand(path.c_str());
}

ustring			UnExpand(PCWSTR path);
inline ustring	UnExpand(const ustring &path) {
	return UnExpand(path.c_str());
}

ustring 		MakeGoodPath(PCWSTR path);
inline ustring MakeGoodPath(const ustring path) {
	return MakeGoodPath(path.c_str());
}

ustring			get_fullpath(PCWSTR path);
inline ustring	get_fullpath(const ustring &path) {
	return get_fullpath(path.c_str());
}

ustring			PathNice(PCWSTR path);
inline ustring	PathNice(const ustring &path) {
	return Canonicalize(Expand(path.c_str()));
}

ustring			path_compact(PCWSTR path, size_t size);
inline ustring	path_compact(const ustring &path, size_t size) {
	return path_compact(path.c_str(), size);
}

ustring&		ensure_end_path_separator(ustring &path, WCHAR sep = PATH_SEPARATOR_C);

ustring&		ensure_no_end_path_separator(ustring &path);

ustring			Secure(PCWSTR path);
inline ustring	Secure(const ustring &path) {
	return Secure(path.c_str());
}

ustring			Validate(PCWSTR path);
inline ustring	Validate(const ustring &path) {
	return Validate(path.c_str());
}

ustring			SlashAdd(const ustring &path, WCHAR sep = PATH_SEPARATOR_C);
ustring			SlashDel(const ustring &path);

bool			IsPathUnix(PCWSTR path);
inline bool		IsPathUnix(const ustring &path) {
	return IsPathUnix(path.c_str());
}

ustring	ExtractFile(const ustring &path);

ustring	ExtractPath(const ustring &path);

ustring			GetSpecialPath(int csidl, bool create = true);

bool			MaskMatch(PCWSTR path, PCWSTR mask, DWORD flags = 0);

ustring			MakePath(PCWSTR path, PCWSTR name);
inline ustring	MakePath(const ustring &path, const ustring &name) {
	return MakePath(path.c_str(), name.c_str());
}

ustring			PathUnix(PCWSTR path);
inline ustring	PathUnix(const ustring &path) {
	return PathUnix(path.c_str());
}

ustring			PathWin(PCWSTR path);
inline ustring	PathWin(const ustring &path) {
	return PathWin(path.c_str());
}

ustring			GetWorkDirectory();

bool			SetWorkDirectory(PCWSTR path);
inline bool		SetWorkDirectory(const ustring &path) {
	return SetWorkDirectory(path.c_str());
}

ustring			get_root(PCWSTR path);
inline ustring	get_root(const ustring &path) {
	return get_root(path.c_str());
}

bool			is_path_mask(PCWSTR path);
inline bool		is_path_mask(const ustring &path) {
	return is_path_mask(path.c_str());
}

bool			is_valid_filename(PCWSTR name);
inline bool		is_valid_filename(const ustring &name) {
	return is_valid_filename(name.c_str());
}

ustring			remove_path_prefix(const ustring &path, PCWSTR pref = PATH_PREFIX_NT);

ustring			ensure_path_prefix(const ustring &path, PCWSTR pref = PATH_PREFIX_NT);

ustring			get_path_from_mask(const ustring &mask);

ustring			TempDir();

ustring			TempFile(PCWSTR path);
inline ustring	TempFile(const ustring &path) {
	return TempFile(path.c_str());
}

bool substr_match(const ustring& str, size_t pos, PCWSTR mstr);

void locate_path_root(const ustring& path, size_t& path_root_len, bool& is_unc_path);

ustring extract_file_name(const ustring& path);

///========================================================================================= SysPath
namespace	SysPath {
	ustring	Winnt();
	ustring	Sys32();
	ustring	SysNative();
	ustring	InetSrv();
	ustring	Dns();
	ustring	Temp();

	ustring	Users();
}

///========================================================================================== SysApp
namespace	SysApp {
	ustring	appcmd();
//	ustring	dnscmd();
}

///========================================================================================= WinTime
struct		WinFileTime: public FILETIME {
	void			Init(const ULARGE_INTEGER &in) {
		dwLowDateTime	= in.LowPart;
		dwHighDateTime	= in.HighPart;
	}
public:
	WinFileTime() {
		now();
	}
	WinFileTime(const uint64_t &in) {
		ULARGE_INTEGER tmp;
		tmp.QuadPart = in;
		Init(tmp);
	}
	WinFileTime(const ULARGE_INTEGER &in) {
		Init(in);
	}
	operator	FILETIME() const {
		return *this;
	}
	operator	ULARGE_INTEGER() const {
		ULARGE_INTEGER	Result;
		Result.LowPart = this->dwLowDateTime;
		Result.HighPart = this->dwHighDateTime;
		return Result;
	}
	operator	uint64_t() const {
		ULARGE_INTEGER	Result;
		Result.LowPart = this->dwLowDateTime;
		Result.HighPart = this->dwHighDateTime;
		return Result.QuadPart;
	}
	void			now() {
		::GetSystemTimeAsFileTime(this);
	}

	const WinFileTime&	operator=(const ULARGE_INTEGER & in) {
		Init(in);
		return *this;
	}
	WinFileTime&		operator+=(const uint64_t & in) {
		ULARGE_INTEGER tmp = *this;
		tmp.QuadPart += in * Second();
		Init(tmp);
		return *this;
	}
	WinFileTime&		operator-=(const uint64_t & in) {
		ULARGE_INTEGER tmp = *this;
		tmp.QuadPart -= in * Second();
		Init(tmp);
		return *this;
	}
	WinFileTime			operator+(const uint64_t &in) {
		ULARGE_INTEGER tmp = *this;
		tmp.QuadPart += in * Second();
		return WinFileTime(tmp);
	}
	uint64_t		operator-(const WinFileTime &in) {
		ULARGE_INTEGER tmp = *this;
		tmp.QuadPart -= ((ULARGE_INTEGER)in).QuadPart;
		return tmp.QuadPart / Second();
	}

	static uint64_t	MiliSecond() {
		return 10000ULL;
	}
	static uint64_t	Second() {
		return 10000000ULL;
	}
	static uint64_t	Minute() {
		return 600000000ULL;
	}
	static uint64_t	Hour() {
		return 36000000000ULL;
	}
	static uint64_t	Day() {
		return 864000000000ULL;
	}
	static uint64_t	Week() {
		return 6048000000000ULL;
	}
	static uint64_t	SecPerDay() {
		return 60ULL * 60 * 24;
	}
	static uint64_t	SecPerHour() {
		return 60ULL * 60;
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
		return hTimer;
	}
};

///=========================================================================================== Win64
/// Функции работы с WOW64
bool	WowDisable(PVOID &oldValue);
bool	WowEnable(PVOID &oldValue);
bool	IsWOW64();

///====================================================================================== WinSysInfo
struct		WinSysInfo: public SYSTEM_INFO {
	WinSysInfo();
	size_t			Uptime(size_t del = 1000);
};

///========================================================================================= WinProc
/// Обертка хэндла процесса
class WinProcess {
public:
	WinProcess():
		m_hndl(::GetCurrentProcess()) {
	}

	WinProcess(ACCESS_MASK mask, DWORD pid):
		m_hndl(::OpenProcess(mask, false, pid)) {
	}

	operator HANDLE() const {
		return m_hndl;
	}

	DWORD GetId() const {
		return ::GetProcessId(m_hndl);
	}

	static DWORD id() {
		return ::GetCurrentProcessId();
	}
	static DWORD id(HANDLE hProc) {
		return ::GetProcessId(hProc);
	}
	static ustring User();
	static ustring FullPath();
	static ustring CmdLine() {
		return ::GetCommandLineW();
	}
private:
	auto_close<HANDLE>	m_hndl;
};

#endif // WIN_DEF_HPP
