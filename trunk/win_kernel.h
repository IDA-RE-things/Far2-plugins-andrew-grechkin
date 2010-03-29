#ifndef WIN_KERNEL_HPP
#define WIN_KERNEL_HPP

#include "win_def.h"

///========================================================================================== WinReg
class		WinReg {
	HKEY	mutable	hKeyOpend;
	HKEY			hKeyReq;
	CStrW			m_path;

	void			CloseKey() const {
		if (hKeyOpend) {
			::RegCloseKey(hKeyOpend);
			hKeyOpend = NULL;
		}
	}
	bool			OpenKey(ACCESS_MASK acc) const {
		return	OpenKey(hKeyReq, m_path, acc);
	}
	bool			OpenKey(HKEY hkey, const CStrW &path, ACCESS_MASK acc) const {
		CloseKey();
		bool	Result = false;
		if (acc == KEY_READ)
			Result = ::RegOpenKeyExW(hkey, path.c_str(), 0, acc, &hKeyOpend) == ERROR_SUCCESS;
		else
			Result = ::RegCreateKeyExW(hkey, path.c_str(), 0, NULL, 0, acc, 0, &hKeyOpend, 0) == ERROR_SUCCESS;
		return	Result;
	}

	template <typename Type>
	void			SetRaw(const CStrW &name, const Type &value, DWORD type = REG_BINARY) const {
		if (OpenKey(KEY_WRITE)) {
			::RegSetValueExW(hKeyOpend, name.c_str(), NULL, type, (PBYTE)(&value), sizeof(value));
			CloseKey();
		}
	}
	template <typename Type>
	bool			GetRaw(const CStrW &name, Type &value, const Type &def) const {
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
	WinReg(const CStrW &path): hKeyOpend(0), hKeyReq(0), m_path(path) {
		hKeyReq = HKEY_CURRENT_USER;
		CStrW	tmp = L"HKEY_CURRENT_USER\\";
		if (Find(m_path, tmp)) {
			m_path = m_path.c_str() + tmp.size();
			return;
		}
		tmp = L"HKCU\\";
		if (Find(m_path, tmp)) {
			m_path = m_path.c_str() + tmp.size();
			return;
		}
		tmp = L"HKEY_LOCAL_MACHINE\\";
		if (Find(m_path, tmp)) {
			m_path = m_path.c_str() + tmp.size();
			hKeyReq = HKEY_LOCAL_MACHINE;
			return;
		}
		tmp = L"HKLM\\";
		if (Find(m_path, tmp)) {
			m_path = m_path.c_str() + tmp.size();
			hKeyReq = HKEY_LOCAL_MACHINE;
			return;
		}
		tmp = L"HKEY_USERS\\";
		if (Find(m_path, tmp)) {
			m_path = m_path.c_str() + tmp.size();
			hKeyReq = HKEY_USERS;
			return;
		}
		tmp = L"HKU\\";
		if (Find(m_path, tmp)) {
			m_path = m_path.c_str() + tmp.size();
			hKeyReq = HKEY_USERS;
			return;
		}
		tmp = L"HKEY_CLASSES_ROOT\\";
		if (Find(m_path, tmp)) {
			m_path = m_path.c_str() + tmp.size();
			hKeyReq = HKEY_CLASSES_ROOT;
			return;
		}
		tmp = L"HKCR\\";
		if (Find(m_path, tmp)) {
			m_path = m_path.c_str() + tmp.size();
			hKeyReq = HKEY_CLASSES_ROOT;
			return;
		}
	}
	WinReg(HKEY hkey, const CStrW &path): hKeyOpend(0), hKeyReq(hkey), m_path(path) {
	}

	CStrW			path() const {
		return	m_path;
	}
	void			path(const CStrW &path) {
		m_path = path;
	}
	void			key(HKEY hkey) {
		hKeyReq = hkey;
	}

	bool			Add(const CStrW &name) const {
		bool	Result = OpenKey(KEY_WRITE);
		if (Result) {
			HKEY tmp = NULL;
			Result = (::RegCreateKeyW(hKeyOpend, name.c_str(), &tmp) == ERROR_SUCCESS);
			if (Result) {
				::RegCloseKey(tmp);
			}
			CloseKey();
		}
		return	Result;
	}
	bool			Del(const CStrW &name) const {
		bool	Result = OpenKey(KEY_WRITE);
		if (Result) {
			Result = (::RegDeleteValueW(hKeyOpend, name.c_str()) == ERROR_SUCCESS);
			CloseKey();
		}
		return	Result;
	}

	void			Set(const CStrW &name, PCWSTR value) const {
		if (OpenKey(KEY_WRITE)) {
			::RegSetValueExW(hKeyOpend, name.c_str(), NULL, REG_SZ, (PBYTE)value, (Len(value)+1) * sizeof(TCHAR));
			CloseKey();
		}
	}
	void			Set(const CStrW &name, int value) const {
		SetRaw(name, value, REG_DWORD);
	}

	bool			Get(const CStrW &name, CStrW &value, const CStrW &def) const {
		bool	Result = OpenKey(KEY_READ);
		value = def;
		if (Result) {
			DWORD	size = 0;
			if (::RegQueryValueExW(hKeyOpend, name.c_str(), NULL, NULL, NULL, &size) == ERROR_MORE_DATA) {
				CStrW	data(size);
				if (::RegQueryValueExW(hKeyOpend, name.c_str(), NULL, NULL, (PBYTE)data.buffer(), &size) == ERROR_SUCCESS) {
					value = data;
					Result = true;
				}
			}
			CloseKey();
		}
		return	Result;
	}
	bool			Get(const CStrW &name, int &value, int def) const {
		return	GetRaw(name, value, def);
	}
};

///======================================================================================== WinTimer
/// Оконный таймер
class		WinTimer {
	HANDLE			hTimer;
	LARGE_INTEGER	liUTC;
	long			lPeriod;

	void			Open() {
		Close();
		hTimer = ::CreateWaitableTimer(NULL, false, NULL);
	}
	void			Close() {
		if (hTimer) {
			::CloseHandle(hTimer);
			hTimer = NULL;
		}
	}
public:
	~WinTimer() {
		Stop();
		Close();
	}
	WinTimer(): hTimer(NULL) {
		Open();
		liUTC.QuadPart = 0LL;
		lPeriod = 0L;
	}
	WinTimer(LONGLONG time, long period = 0): hTimer(NULL) {
		Open();
		Set(time, period);
	}
	void			Set(LONGLONG time, long period = 0) {
		if (time == 0LL) {
			liUTC.QuadPart = -period * 10000LL;
		} else {
			liUTC.QuadPart = time;
		}
		lPeriod = period;
	}
	void			Start() {
		if (hTimer)
			::SetWaitableTimer(hTimer, &liUTC, lPeriod, NULL, NULL, false);
	}
	void			Stop() {
		if (hTimer)
			::CancelWaitableTimer(hTimer);
	}
	operator		HANDLE() const {
		return	hTimer;
	}
	void			StartTimer() {
		// объявляем свои локальные переменные
		FILETIME ftLocal, ftUTC;
		LARGE_INTEGER liUTC;

// таймер должен сработать в первый раз 1 января 2002 года в 1:00 PM но местному времени
		SYSTEMTIME st;
		::GetSystemTime(&st);
//		st.wOayOfWeek = 0;			// игнорируется
//		st.wHour = 0;				// 0 PM
//		st.wMinute = 0;				// 0 минут
		st.wSecond = 0;				// 0 секунд
		st.wMilliseconds = 0;		// 0 миллисекунд
		::SystemTimeToFileTime(&st, &ftLocal);
//		::LocalFileTimeToFilelime(&ttLocal, &ftUTC);

// преобразуем FILETIME в LARGE_INTEGER из-за различий в выравнивании данных
		liUTC.LowPart = ftUTC.dwLowDateTime;
		liUTC.HighPart = ftUTC.dwHighDateTime;
	}
};

///========================================================================================= WinPriv
/// Функции работы с привилегиями
namespace	WinPriv {
bool 			IsExist(HANDLE hToken, LUID priv);
bool inline		IsExist(HANDLE hToken, PCWSTR sPriv) {
	LUID	luid;
	if (::LookupPrivilegeValueW(NULL, sPriv, &luid))
		return	IsExist(hToken, luid);
	return	false;
}
bool inline		IsExist(HANDLE hToken, PCSTR sPriv) {
	LUID	luid;
	if (::LookupPrivilegeValueA(NULL, sPriv, &luid))
		return	IsExist(hToken, luid);
	return	false;
}

bool			IsEnabled(HANDLE hToken, LUID priv);
bool inline		IsEnabled(HANDLE hToken, PCWSTR sPriv) {
	LUID	luid;
	// получаем идентификатор привилегии
	if (::LookupPrivilegeValueW(NULL, sPriv, &luid))
		return	IsEnabled(hToken, luid);
	return	false;
}
bool inline		IsEnabled(HANDLE hToken, PCSTR sPriv) {
	LUID	luid;
	// получаем идентификатор привилегии
	if (::LookupPrivilegeValueA(NULL, sPriv, &luid))
		return	IsEnabled(hToken, luid);
	return	false;
}

bool 			Modify(HANDLE hToken, LUID priv, bool bEnable);
bool inline		Modify(HANDLE hToken, PCWSTR sPriv, bool bEnable) {
	LUID	luid;
	if (::LookupPrivilegeValueW(NULL, sPriv, &luid))
		return	Modify(hToken, luid, bEnable);
	return	false;
}
bool inline		Modify(HANDLE hToken, PCSTR	sPriv, bool bEnable) {
	LUID	luid;
	if (::LookupPrivilegeValueA(NULL, sPriv, &luid))
		return	Modify(hToken, luid, bEnable);
	return	false;
}
}

///========================================================================================= WinProc
/// Обертка хэндла процесса
class		WinProc: private Uncopyable, public WinErrorCheck {
	HANDLE	m_handle;
public:
	~WinProc() {
		::CloseHandle(m_handle);
	}
	WinProc() {
		m_handle = ::GetCurrentProcess();
	}
	WinProc(ACCESS_MASK mask, DWORD pid): m_handle(NULL) {
		m_handle = ::OpenProcess(mask, false, pid);
		ChkSucc(m_handle != NULL);
	}
	operator		HANDLE() const {
		return	m_handle;
	}
	DWORD			GetId() const {
		return	::GetProcessId(m_handle);
	}

// static
	static	DWORD	Id() {
		return	::GetCurrentProcessId();
	}
	static	DWORD	Id(HANDLE hProc) {
		return	::GetProcessId(hProc);
	}
	static	CStrW	User() {
		DWORD	size = 0;
		::GetUserNameW(NULL, &size);
		CStrW	buf(size);
		::GetUserNameW((PWSTR)buf.c_str(), &size);
		return	buf;
	}
	static	CStrW	Path() {
		CStrW	Result(MAX_PATH);
		DWORD	size = ::GetModuleFileNameW(NULL, (PWSTR)Result.c_str(), (DWORD)Result.capacity());
		if (size > Result.capacity()) {
			Result.reserve(size);
			::GetModuleFileNameW(NULL, (PWSTR)Result.c_str(), (DWORD)Result.capacity());
		}
		return	Result;
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
		ChkSucc(::OpenProcessToken(WinProc(), mask, &m_handle) != 0);
	}
	WinToken(ACCESS_MASK mask, HANDLE hProcess): m_handle(NULL) {
		ChkSucc(::OpenProcessToken(hProcess, mask, &m_handle) != 0);
	}
	operator		HANDLE() const {
		return	m_handle;
	}
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
	WinSysInfo() {
		if (Win64::IsWOW64())
			::GetNativeSystemInfo((LPSYSTEM_INFO)this);
		else
			::GetSystemInfo((LPSYSTEM_INFO)this);
	}
	size_t		Uptime(size_t del = 1000) {
		return	0;//::GetTickCount64() / del;
	}
};

///========================================================================================= WinPerf
struct		WinPerf: public PERFORMANCE_INFORMATION {
	WinPerf() {
		WinMem::Zero(*this);
		typedef	BOOL (WINAPI * fn_GetPerformanceInfo)(PPERFORMANCE_INFORMATION pPerformanceInformation, DWORD cb);
		fn_GetPerformanceInfo ProcAddr = (fn_GetPerformanceInfo) ::GetProcAddress(::LoadLibraryW(L"psapi.dll"), "GetPerformanceInfo");
		if (ProcAddr)
			ProcAddr(this, sizeof(*this));
	}
};


#endif //WIN_KERNEL_HPP
