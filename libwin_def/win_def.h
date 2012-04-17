/**
	@author © 2010 Andrew Grechkin
**/

#ifndef WIN_DEF_HPP
#define WIN_DEF_HPP

#include "std.h"
#include "bit.h"
#include "memory.h"
#include "str.h"

///▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒ env
///========================================================================================== WinEnv
namespace WinEnv {
	ustring get(PCWSTR name);
	bool set(PCWSTR name, PCWSTR val);
	bool add(PCWSTR name, PCWSTR val);
	bool del(PCWSTR name);
}

//inline DWORD UserLogon(HANDLE & hToken, PCWSTR name, PCWSTR pass, DWORD type, PCWSTR dom = EMPTY_STR) {
//	DWORD Result = NO_ERROR;
//	if (!::LogonUserW((PWSTR)name, (PWSTR)dom, (PWSTR)pass, type, LOGON32_PROVIDER_DEFAULT, &hToken)) {
//		Result = ::GetLastError();
//	}
//	return Result;
//}

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
namespace SysPath {
	ustring Winnt();
	ustring Sys32();
	ustring SysNative();
	ustring InetSrv();
	ustring Dns();
	ustring Temp();

	ustring Users();
}

///========================================================================================== SysApp
namespace SysApp {
	ustring appcmd();
//	ustring dnscmd();
}

///========================================================================================= WinTime
struct WinFileTime: public FILETIME {
	WinFileTime() {
		now();
	}
	WinFileTime(const uint64_t & in) {
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

private:
	void Init(const ULARGE_INTEGER &in) {
		dwLowDateTime	= in.LowPart;
		dwHighDateTime	= in.HighPart;
	}
};

///========================================================================================= WinTime
struct WinSysTime: public SYSTEMTIME {
	WinSysTime() {
		WinMem::Zero(*this);
		now();
	}

	void now(bool isLocal = false) {
		if (isLocal)
			::GetLocalTime(this);
		else
			::GetSystemTime(this);
	}

	void add_year(size_t in) {
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
bool is_WOW64();
bool disable_WOW64(PVOID & oldValue);
bool enable_WOW64(PVOID & oldValue);

///====================================================================================== WinSysInfo
struct WinSysInfo: public SYSTEM_INFO {
	WinSysInfo();
	size_t Uptime(size_t del = 1000);
};

///========================================================================================= WinProc
struct WinProcess {
	WinProcess():
		m_hndl(::GetCurrentProcess()) {
	}

	WinProcess(ACCESS_MASK mask, DWORD pid):
		m_hndl(::OpenProcess(mask, false, pid)) {
	}

	operator HANDLE() const {
		return m_hndl;
	}

	DWORD get_id() const {
		return get_id(m_hndl);
	}

	static DWORD id() {
		return ::GetCurrentProcessId();
	}

	static DWORD get_id(HANDLE hProc) {
		return ::GetProcessId(hProc);
	}

	static ustring get_cmdLine() {
		return ustring(::GetCommandLineW());
	}

	static ustring get_owner();

	static ustring get_path();

private:
	auto_close<HANDLE> m_hndl;
};

#endif
