/**
	win_net

	@classes	()
	@author		© 2009 Andrew Grechkin
	@link		()
**/

#ifndef WIN_NET_HPP
#define WIN_NET_HPP

#include "win_def.h"

#include "win_c_map.h"

#include <sys/types.h>
#include <aclapi.h>
#include <lm.h>
#include <Wincrypt.h>
#include <wtsapi32.h>
#include <iphlpapi.h>

#ifndef S_IXUSR
#define S_IFDIR 0x4000
#define S_IRUSR 0x0100
#define S_IWUSR 0x0080
#define S_IXUSR 0x0040
#endif
#define S_IRGRP 0x0020
#define S_IWGRP 0x0010
#define S_IXGRP 0x0008
#define S_IROTH 0x0004
#define S_IWOTH 0x0002
#define S_IXOTH 0x0001

///▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒
///======================================================================================== WinError

class		RuntimeError {
	AutoUTF	m_what;
protected:
	AutoUTF		what(PCWSTR msg) {
		return	m_what = msg;
	}
	AutoUTF		what(const AutoUTF &msg) {
		return	m_what = msg;
	}
public:
	virtual ~RuntimeError() {
	}
	RuntimeError() {
	}
	RuntimeError(const AutoUTF &msg): m_what(msg) {
	}
	AutoUTF		what() const {
		return	m_what;
	}
//	virtual void		show() const = 0;
};
//void		RuntimeError::show() const {
//	mbox(m_what.c_str(), L"RuntimeError");
//}

class		WinError: public RuntimeError {
	size_t	m_code;
protected:
	size_t		code(size_t code) {
		return	m_code = code;
	}
public:
	WinError(size_t code): m_code(code) {
	}
	WinError(const AutoUTF &msg): RuntimeError(msg), m_code(0) {
	}
	WinError(size_t code, const AutoUTF &msg): RuntimeError(msg), m_code(code) {
	}

	size_t		code() const {
		return	m_code;
	}
	virtual AutoUTF		msg() const = 0;
};
class		ApiError: public WinError {
public:
	ApiError(): WinError(::GetLastError()) {
	}
	ApiError(size_t code): WinError(code) {
	}
	ApiError(size_t code, const AutoUTF &msg): WinError(code, msg) {
	}
	AutoUTF		msg() const {
		return	ErrAsStr(code());
	}
};
class		WSockError: public WinError {
public:
	WSockError(): WinError(::WSAGetLastError()) {
	}
	WSockError(size_t code): WinError(code) {
	}
	WSockError(const AutoUTF &msg): WinError(::WSAGetLastError(), msg) {
	}
	AutoUTF		msg() const {
		return	ErrAsStr(code());
	}
};

inline void			CheckAPI(bool r) {
	if (!r) {
		throw	ApiError();
	}
}
inline void			CheckNetApi(NET_API_STATUS r) {
	if (r != NERR_Success) {
		throw	ApiError(r);
	}
}
inline void			CheckError(DWORD err) {
	if (err != ERROR_SUCCESS) {
		throw	ApiError(err);
	}
}

///========================================================================================== WinNet
namespace	WinNet {
AutoUTF 			GetCompName(COMPUTER_NAME_FORMAT cnf = ComputerNameNetBIOS);
inline bool 		SetCompName(const AutoUTF &in, COMPUTER_NAME_FORMAT cnf) {
	return	::SetComputerNameExW(cnf, in.c_str()) != 0;
}
}

///▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒ net_rc
///================================================================================ RemoteConnection
class		RemoteConnection {
	AutoUTF	m_host;
	bool	m_conn;
public:
	~RemoteConnection();
	RemoteConnection(PCWSTR host = NULL);
	void		Open(PCWSTR host, PCWSTR user = NULL, PCWSTR pass = NULL);
	void		Close();

	bool		TestConn(PCWSTR host) const;
	AutoUTF		host() const {
		return	m_host;
	}
};

///▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒ net_svc
///========================================================================================== WinScm
class		WinScm {
	SC_HANDLE			m_hndl;
	ACCESS_MASK			m_mask;
	RemoteConnection	*m_conn;

public:
	~WinScm() {
		Close(m_hndl);
	}
//	WinScm(ACCESS_MASK acc = SC_MANAGER_CONNECT | SC_MANAGER_ENUMERATE_SERVICE, RemoteConnection *conn = NULL): m_hndl(NULL), m_mask(acc), m_conn(conn) {
	WinScm(ACCESS_MASK acc, RemoteConnection *conn = NULL): m_hndl(NULL), m_mask(acc), m_conn(conn) {
		Open(m_hndl, m_mask, m_conn);
	}

	void					Close() {
		Close(m_hndl);
	}
	void					Reopen(ACCESS_MASK acc = SC_MANAGER_CONNECT | SC_MANAGER_ENUMERATE_SERVICE, RemoteConnection *conn = NULL) {
		Close(m_hndl);
		m_mask = acc;
		m_conn = conn;
		Open(m_hndl, m_mask, m_conn);
	}

	operator				SC_HANDLE() const {
		return	m_hndl;
	}
	SC_HANDLE				Handle() const {
		return	m_hndl;
	}

	void					Create(PCWSTR name, PCWSTR path, DWORD StartType = SERVICE_DEMAND_START, PCWSTR disp = NULL) {
		SC_HANDLE	hSvc = ::CreateServiceW(
							 m_hndl, name,
							 (disp == NULL) ? name : disp,
							 SERVICE_ALL_ACCESS,		// desired access
							 SERVICE_WIN32_OWN_PROCESS,	// service type
							 StartType,					// start type
							 SERVICE_ERROR_NORMAL,		// WinError control type
							 path,             			// path to service's binary
							 NULL,						// no load ordering group
							 NULL,						// no tag identifier
							 NULL,						// no dependencies
							 NULL,						// LocalSystem account
							 NULL);						// no password
		CheckAPI(hSvc != NULL);
		::CloseServiceHandle(hSvc);
	}

	static void				Open(SC_HANDLE &hSC, ACCESS_MASK acc = SC_MANAGER_CONNECT, RemoteConnection *conn = NULL) {
		hSC = ::OpenSCManagerW((conn != NULL) ? conn->host().c_str() : NULL, NULL, acc);
		CheckAPI(hSC != NULL);
	}
	static void				Close(SC_HANDLE &in) {
		if (in && in != INVALID_HANDLE_VALUE) {
			::CloseServiceHandle(in);
			in = NULL;
		}
	}
};

///========================================================================================== WinSvc
class		WinSvc {
	SC_HANDLE			m_hndl;

	void					Close() {
		if (m_hndl) {
			::CloseServiceHandle(m_hndl);
			m_hndl = NULL;
		}
	}
public:
	~WinSvc() {
		Close();
	}
	WinSvc(PCWSTR name, ACCESS_MASK access, RemoteConnection *conn = NULL): m_hndl(NULL) {
		WinScm	scm(SC_MANAGER_CONNECT, conn);
		m_hndl = ::OpenServiceW(scm, name, access);
		CheckAPI(m_hndl != NULL);
	}
	WinSvc(PCWSTR name, ACCESS_MASK access, const WinScm &scm): m_hndl(NULL) {
		m_hndl = ::OpenServiceW(scm, name, access);
		CheckAPI(m_hndl != NULL);
	}

	void					QueryConfig(WinBuf<QUERY_SERVICE_CONFIGW> &buf) const;
	void					QueryConfig2(WinBuf<BYTE> &buf, DWORD level) const;

	template<typename Functor>
	void					WaitForState(DWORD state, DWORD dwTimeout, Functor &func, PVOID param = NULL) const {
		DWORD	dwStartTime = ::GetTickCount();
		DWORD	dwBytesNeeded;
		SERVICE_STATUS_PROCESS ssp = {0};
		while (true) {
			CheckAPI(::QueryServiceStatusEx(m_hndl, SC_STATUS_PROCESS_INFO, (PBYTE)&ssp, sizeof(ssp), &dwBytesNeeded));
			if (ssp.dwCurrentState == state)
				break;
			if (::GetTickCount() - dwStartTime > dwTimeout)
				throw	ApiError(WAIT_TIMEOUT);
			func(state, ::GetTickCount() - dwStartTime, param);
		}
	}
	void					WaitForState(DWORD state, DWORD dwTimeout) const {
		DWORD	dwStartTime = ::GetTickCount();
		SERVICE_STATUS_PROCESS ssp = {0};
		while (true) {
			GetStatus(ssp);
			if (ssp.dwCurrentState == state)
				break;
			if (::GetTickCount() - dwStartTime > dwTimeout)
				throw	ApiError(WAIT_TIMEOUT);
			::Sleep(200);
		};
	}

	bool					Start() {
		try {
			CheckAPI(::StartService(m_hndl, 0, NULL));
		} catch (WinError &e) {
			if (e.code() != ERROR_SERVICE_ALREADY_RUNNING)
				throw;
		}
		return	true;
	}
	bool					Stop() {
		SERVICE_STATUS	ss;
		try {
			CheckAPI(::ControlService(m_hndl, SERVICE_CONTROL_STOP, &ss));
		} catch (WinError &e) {
			if (e.code() != ERROR_SERVICE_NOT_ACTIVE)
				throw;
		}
		return	true;
	}
	void					Continue() {
		SERVICE_STATUS	ss;
		CheckAPI(::ControlService(m_hndl, SERVICE_CONTROL_CONTINUE, &ss));
	}
	void					Pause() {
		SERVICE_STATUS	ss;
		CheckAPI(::ControlService(m_hndl, SERVICE_CONTROL_PAUSE, &ss));
	}

	void					Del() {
		CheckAPI(::DeleteService(m_hndl));
		Close();
	}

	void					SetStartup(DWORD type) {
		CheckAPI(::ChangeServiceConfigW(
					 m_hndl,			// handle of service
					 SERVICE_NO_CHANGE,	// service type: no change
					 type,				// service start type
					 SERVICE_NO_CHANGE,	// error control: no change
					 NULL,				// binary path: no change
					 NULL,				// load order group: no change
					 NULL,				// tag ID: no change
					 NULL,				// dependencies: no change
					 NULL,				// account name: no change
					 NULL,				// password: no change
					 NULL));			// display name: no change
	}
	void					SetLogon(const AutoUTF &user, const AutoUTF &pass = L"", bool desk = false) {
		if (user.empty())
			return;
		DWORD	type = SERVICE_NO_CHANGE;
		DWORD	tmp = GetType();
//		PCWSTR	psw = pass.empty() ? L"" : pass.c_str();
		if (WinFlag::Check(tmp, (DWORD)SERVICE_WIN32_OWN_PROCESS) || WinFlag::Check(tmp, (DWORD)SERVICE_WIN32_SHARE_PROCESS)) {
			if (WinFlag::Check(tmp, (DWORD)SERVICE_INTERACTIVE_PROCESS) != desk) {
				WinFlag::Switch(tmp, (DWORD)SERVICE_INTERACTIVE_PROCESS, desk);
				type = tmp;
			}
		}
		CheckAPI(::ChangeServiceConfigW(
					 m_hndl,			// handle of service
					 type,				// service type: no change
					 SERVICE_NO_CHANGE,	// service start type
					 SERVICE_NO_CHANGE,	// error control: no change
					 NULL,				// binary path: no change
					 NULL,				// load order group: no change
					 NULL,				// tag ID: no change
					 NULL,				// dependencies: no change
					 user.c_str(),
					 pass.c_str(),
					 NULL));			// display name: no change
	}

	void					GetStatus(SERVICE_STATUS_PROCESS &info) const {
		DWORD	dwBytesNeeded;
		CheckAPI(::QueryServiceStatusEx(m_hndl, SC_STATUS_PROCESS_INFO, (PBYTE)&info, sizeof(info), &dwBytesNeeded));
	}
	DWORD					GetState() const {
		SERVICE_STATUS_PROCESS	ssp;
		GetStatus(ssp);
		return	ssp.dwCurrentState;
	}
	DWORD					GetType() const {
		WinBuf<QUERY_SERVICE_CONFIGW>	buf;
		QueryConfig(buf);
		return	buf->dwServiceType;
	}
	AutoUTF					GetUser() const {
		WinBuf<QUERY_SERVICE_CONFIGW>	buf;
		QueryConfig(buf);
		if (buf->lpServiceStartName)
			return	AutoUTF(buf->lpServiceStartName);
		return	AutoUTF();
	}

	operator				SC_HANDLE() const {
		return	m_hndl;
	}
};

///====================================================================================== WinService
namespace	WinService {
AutoUTF				ParseState(DWORD in);
AutoUTF				ParseState(const AutoUTF &name);
void				WaitForState(const AutoUTF &name, DWORD state, DWORD dwTimeout = 10000);
//DWORD				WaitForState(const WinSvcHnd &sch, DWORD state, DWORD dwTimeout = 10000);

void				Add(const AutoUTF &name, const AutoUTF &path, const AutoUTF &disp = L"");
void				Del(const AutoUTF &name);
void				Start(const AutoUTF &name);
void				Stop(const AutoUTF &name);

void				Auto(const AutoUTF &name);
void				Manual(const AutoUTF &name);
void				Disable(const AutoUTF &name);

bool				IsExist(const AutoUTF &name);
bool				IsRunning(const AutoUTF &name);
bool				IsStarting(const AutoUTF &name);
bool				IsStopping(const AutoUTF &name);
bool				IsStopped(const AutoUTF &name);
bool				IsAuto(const AutoUTF &name);
bool				IsManual(const AutoUTF &name);
bool				IsDisabled(const AutoUTF &name);

void				GetStatus(SC_HANDLE sch, SERVICE_STATUS_PROCESS &ssp);
void				GetStatus(const AutoUTF &name, SERVICE_STATUS_PROCESS &ssp);
DWORD				GetState(const AutoUTF &name);
AutoUTF				GetDesc(const AutoUTF &name);
AutoUTF				GetDName(const AutoUTF &name);
AutoUTF				GetPath(const AutoUTF &name);

//DWORD				SetConf(const AutoUTF &name, SvcConf &conf);
void				SetDesc(const AutoUTF &name, const AutoUTF &in);
void				SetDName(const AutoUTF &name, const AutoUTF &in);
void				SetPath(const AutoUTF &name, const AutoUTF &in);
};

///========================================================================================== struct
struct		s_ServiceInfo: public _SERVICE_STATUS {
	AutoUTF		name;				// AN C0
	AutoUTF		dname;				// N
	AutoUTF		path;				// C3
	AutoUTF		descr;				// Z
	AutoUTF		OrderGroup;			// C5
	AutoUTF		ServiceStartName;	// C6
	CStrMW		Dependencies;		// LN

	DWORD		StartType;			// C2
	DWORD		ErrorControl;
	DWORD		TagId;

	s_ServiceInfo() {
		WinMem::Zero(this, sizeof(_SERVICE_STATUS));
		StartType = ErrorControl = TagId = 0;
	}
	s_ServiceInfo(const AutoUTF &n, const SERVICE_STATUS &sp): _SERVICE_STATUS(sp), name(n) {
		StartType = ErrorControl = TagId = 0;
	}
};

///===================================================================================== WinServices
class		WinServices : public MapContainer<AutoUTF, s_ServiceInfo> {
	RemoteConnection	*m_conn;
	DWORD				m_type;
public:
	~WinServices() {
		Clear();
	}
	WinServices(RemoteConnection *conn = NULL, bool autocache = true): m_conn(conn) {
		m_type = SERVICE_WIN32 | SERVICE_INTERACTIVE_PROCESS;
		if (autocache)
			Cache();
	}
	bool				Cache();
	bool				CacheByName(const AutoUTF &in);
	bool				CacheByState(DWORD state = SERVICE_STATE_ALL);
	bool				CacheByType(DWORD state = SERVICE_STATE_ALL);

	bool				services() const {
		return m_type == (SERVICE_WIN32 | SERVICE_INTERACTIVE_PROCESS);
	}
	bool				drivers() const {
		return m_type == (SERVICE_ADAPTER | SERVICE_DRIVER);
	}
	DWORD				type() const {
		return	m_type;
	}
	DWORD				state() const {
		return	Value().dwCurrentState;
	}
	void				services(bool st) {
		if (st)
			m_type = SERVICE_WIN32 | SERVICE_INTERACTIVE_PROCESS;
	}
	void				drivers(bool st) {
		if (st)
			m_type = SERVICE_ADAPTER | SERVICE_DRIVER;
	}

	bool				Add(const AutoUTF &name, const AutoUTF &path);
	bool				Del();

	bool				Start() const;
	bool				Stop() const;
	bool				Restart() const;

	bool				IsAuto() const;
	bool				IsRunning() const;

	AutoUTF				GetName() const;
	AutoUTF				GetDName() const;
	AutoUTF				GetPath() const;
};

void		InstallService(PCWSTR name, PCWSTR path, DWORD StartType = SERVICE_DEMAND_START, PCWSTR dispname = NULL);
void		UninstallService(PCWSTR name);

///▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒ net_ts
///===================================================================================== WinTSHandle
class		WinTSHandle {
	HANDLE		m_ts;
public:
	~WinTSHandle();
	WinTSHandle(PCWSTR host = NULL);
	WinTSHandle(RemoteConnection* conn);
	operator		HANDLE() const {
		return	m_ts;
	}
};

///===================================================================================== WinTSession
namespace	WinTSession {
void			Disconnect(DWORD id, PCWSTR host = L"");
void			LogOff(DWORD id, PCWSTR host = L"");
DWORD			Question(DWORD id, PCWSTR ttl, PCWSTR msg, DWORD time = 60, PCWSTR host = L"");
DWORD			Message(DWORD id, PCWSTR ttl, PCWSTR msg, DWORD time = 60, bool wait = true, PCWSTR host = L"");

void			Disconnect(DWORD id, RemoteConnection *host = NULL);
void			LogOff(DWORD id, RemoteConnection *host = NULL);
DWORD			Question(DWORD id, PCWSTR ttl, PCWSTR msg, DWORD time = 60, RemoteConnection *host = NULL);
DWORD			Message(DWORD id, PCWSTR ttl, PCWSTR msg, DWORD time = 60, bool wait = true, RemoteConnection *host = NULL);
};

///======================================================================================= WinTSInfo
struct		WinTSInfo {
	DWORD					id;
	AutoUTF					sess;
	AutoUTF					user;
	AutoUTF					winSta;
	AutoUTF					client;
	WTS_CONNECTSTATE_CLASS	state;

	WinTSInfo(DWORD i, const AutoUTF &s, const AutoUTF &u, WTS_CONNECTSTATE_CLASS st): id(i), sess(s), user(u), state(st) {
	}
	PCWSTR			GetState() const {
		return	ParseState(state);
	}
	PCWSTR			ParseState(WTS_CONNECTSTATE_CLASS st) const {
		switch	(st) {
			case WTSActive:
				return	L"Active";
			case WTSConnected:
				return	L"Connected";
			case WTSConnectQuery:
				return	L"Query";
			case WTSShadow:
				return	L"Shadow";
			case WTSDisconnected:
				return	L"Disconnected";
			case WTSIdle:
				return	L"Idle";
			case WTSListen:
				return	L"Listen";
			case WTSReset:
				return	L"Reset";
			case WTSDown:
				return	L"Down";
			case WTSInit:
				return	L"Initializing";
		}
		return	L"Unknown state";
	}
	PCWSTR			ParseStateFull(WTS_CONNECTSTATE_CLASS st) const {
		switch	(st) {
			case WTSActive:
				return	L"A user is logged on to the WinStation";
			case WTSConnected:
				return	L"The WinStation is connected to the client";
			case WTSConnectQuery:
				return	L"The WinStation is in the process of connecting to the client";
			case WTSShadow:
				return	L"The WinStation is shadowing another WinStation";
			case WTSDisconnected:
				return	L"The WinStation is active but the client is disconnected";
			case WTSIdle:
				return	L"The WinStation is waiting for a client to connect";
			case WTSListen:
				return	L"The WinStation is listening for a connection. A listener session waits for requests for new client connections. No user is logged on a listener session. A listener session cannot be reset, shadowed, or changed to a regular client session.";
			case WTSReset:
				return	L"The WinStation is being reset";
			case WTSDown:
				return	L"The WinStation is down due to an error";
			case WTSInit:
				return	L"The WinStation is initializing";
		}
		return	L"Unknown state";
	}
};

///==================================================================================== WinTSessions
class		WinTS : public MapContainer<DWORD, WinTSInfo> {
	RemoteConnection	*m_conn;
public:
	~WinTS() {
		Clear();
	}
	WinTS(RemoteConnection *conn = NULL, bool autocache = true): m_conn(conn) {
		if (autocache)
			Cache();
	}
	bool				Cache();

	AutoUTF				Info() const {
		AutoUTF	Result;
		Result += L"Id:           ";
		Result += Num2Str((size_t)Key());
		Result += L"\n\n";
		Result += L"User name:    ";
		Result += Value().user;
		Result += L"\n\n";
		Result += L"State:        ";
		Result += Value().GetState();
		Result += L"\n\n";
		Result += L"Session:      ";
		Result += Value().sess;
		Result += L"\n\n";
		Result += L"WinStation:   ";
		Result += Value().winSta;
		Result += L"\n\n";
		Result += L"Client:       ";
		Result += Value().client;
		Result += L"\n\n";
		return	Result;
	}
	bool				FindSess(PCWSTR in) const;
	bool				FindUser(PCWSTR in) const;
};

///▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒ net_log
///========================================================================================== WinLog
class		WinLog {
	HANDLE	m_hndl;

	WinLog();
public:
	~WinLog() {
		::DeregisterEventSource(m_hndl);
	}
	WinLog(PCWSTR name): m_hndl(::RegisterEventSourceW(NULL, name)) {
		CheckAPI(m_hndl != NULL);
	}
	void		Write(DWORD Event, WORD Count, LPCWSTR *Strings) {
		/*
		PSID user = NULL;
		HANDLE token;
		PTOKEN_USER token_user = NULL;
				if (OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, FALSE, &token)) {
					token_user = (PTOKEN_USER)DefaultTokenInformation(token, TokenUser);
					if (token_user)
						user = token_user->User.Sid;
					CloseHandle(token);
				}
				ReportEventW(m_hndl, EVENTLOG_ERROR_TYPE, 0, Event, user, Count, 0, Strings, NULL);
				free(token_user);
		*/
	}

	static void		Register(PCWSTR name, PCWSTR path = NULL) {
		WCHAR	fullpath[MAX_PATH_LENGTH];
		WCHAR	key[MAX_PATH_LENGTH];
		if (!path || Empty(path)) {
			CheckAPI(::GetModuleFileNameW(0, fullpath, sizeofa(fullpath)));
		} else {
			Copy(fullpath, path, sizeofa(fullpath));
		}
		HKEY	hKey = NULL;
		Copy(key, L"SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\", sizeofa(key));
		Cat(key, name, sizeofa(key));
		CheckAPI(::RegCreateKeyW(HKEY_LOCAL_MACHINE, key, &hKey) == ERROR_SUCCESS);
		// Add the Event ID message-file name to the subkey.
		::RegSetValueExW(hKey, L"EventMessageFile", 0, REG_EXPAND_SZ, (LPBYTE)fullpath, (DWORD)((Len(fullpath) + 1)*sizeof(WCHAR)));
		// Set the supported types flags.
		DWORD dwData = EVENTLOG_ERROR_TYPE | EVENTLOG_WARNING_TYPE | EVENTLOG_INFORMATION_TYPE;
		::RegSetValueExW(hKey, L"TypesSupported", 0, REG_DWORD, (LPBYTE)&dwData, sizeof(dwData));
		::RegCloseKey(hKey);
	}
};

///▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒ net_user
///========================================================================================= NetUser
namespace	NetUser {
bool				IsExist(const AutoUTF &name, const AutoUTF &dom = L"");
bool				IsDisabled(const AutoUTF &name, const AutoUTF &dom = L"");
bool				IsExpired(const AutoUTF &name, const AutoUTF &dom = L"");

AutoUTF				GetComm(const AutoUTF &name, const AutoUTF &dom = L"");
AutoUTF				GetDesc(const AutoUTF &name, const AutoUTF &dom = L"");
AutoUTF				GetFName(const AutoUTF &name, const AutoUTF &dom = L"");
AutoUTF				GetHome(const AutoUTF &name, const AutoUTF &dom = L"");
AutoUTF				GetParams(const AutoUTF &name, const AutoUTF &dom = L"");
AutoUTF				GetProfile(const AutoUTF &name, const AutoUTF &dom = L"");
AutoUTF				GetScript(const AutoUTF &name, const AutoUTF &dom = L"");
AutoUTF				GetWorkstations(const AutoUTF &name, const AutoUTF &dom = L"");
DWORD				GetFlags(const AutoUTF &name, const AutoUTF &dom = L"");
DWORD				GetUID(const AutoUTF &name, const AutoUTF &dom = L"");

void				Add(const AutoUTF &name, const AutoUTF &pass = L"", const AutoUTF &dom = L"");
void				Del(const AutoUTF &name, const AutoUTF &dom = L"");
void				Disable(const AutoUTF &name, const AutoUTF &dom = L"");
void				Enable(const AutoUTF &name, const AutoUTF &dom = L"");

void				SetExpire(const AutoUTF &name, bool in, const AutoUTF &dom = L"");
void				SetName(const AutoUTF &name, const AutoUTF &in, const AutoUTF &dom = L"");
void				SetPass(const AutoUTF &name, const AutoUTF &in, const AutoUTF &dom = L"");
void				SetDesc(const AutoUTF &name, const AutoUTF &in, const AutoUTF &dom = L"");
void				SetFName(const AutoUTF &name, const AutoUTF &in, const AutoUTF &dom = L"");
void				SetComm(const AutoUTF &name, const AutoUTF &in, const AutoUTF &dom = L"");
void				SetProfile(const AutoUTF &name, const AutoUTF &in, const AutoUTF &dom = L"");
void				SetHome(const AutoUTF &name, const AutoUTF &in, const AutoUTF &dom = L"");
void				SetScript(const AutoUTF &name, const AutoUTF &in, const AutoUTF &dom = L"");
void				SetFlags(const AutoUTF &name, DWORD in, bool value = true, const AutoUTF &dom = L"");
}

///======================================================================================== SysUsers
struct		UserInfo {
	AutoUTF	desc;
	AutoUTF	fname;
	AutoUTF	comm;
	AutoUTF	prof;
	AutoUTF	home;
	AutoUTF	script;
	DWORD	priv;	// priv USER_PRIV_GUEST = 0, USER_PRIV_USER = 1, USER_PRIV_ADMIN = 2
	DWORD	flags;

	UserInfo();
	UserInfo(const AutoUTF &name, const AutoUTF &dom = L"");
};

class		SysUsers : public MapContainer<AutoUTF, UserInfo> {
	AutoUTF	gr;
	AutoUTF dom;
public:
	SysUsers(bool autocache = true);
	bool					Cache(const AutoUTF &dom = L"");
	bool					CacheByPriv(DWORD priv, const AutoUTF &dom = L"");
	bool					CacheByGroup(const AutoUTF &name, const AutoUTF &dom = L"");
	bool					CacheByGid(const AutoUTF &gid, const AutoUTF &dom = L"");

	bool					IsAdmin() const;
	bool					IsDisabled() const;

	void					Add(const AutoUTF &name, const AutoUTF &pass = L"");
	void					Del();
	void					Del(const AutoUTF &name);

	void					Disable();
	void					Disable(const AutoUTF &name);
	void					Enable();
	void					Enable(const AutoUTF &name);

	void					SetName(const AutoUTF &in);
	void					SetPass(const AutoUTF &in);
	void					SetDesc(const AutoUTF &in);
	void					SetFName(const AutoUTF &in);
	void					SetComm(const AutoUTF &in);
	void					SetProfile(const AutoUTF &in);
	void					SetHome(const AutoUTF &in);
	void					SetScript(const AutoUTF &in);
	void					SetFlags(DWORD in, bool value);
	AutoUTF					GetName() const;
	AutoUTF					GetDesc() const;
	AutoUTF					GetFName() const;
	AutoUTF					GetComm() const;
	AutoUTF					GetProfile() const;
	AutoUTF					GetHome() const;
	AutoUTF					GetScript() const;
	DWORD					GetPriv() const;
	DWORD					GetFlags() const;
};

///▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒ net_group
///======================================================================================== NetGroup
namespace	NetGroup {
bool				IsExist(const AutoUTF &name, const AutoUTF &dom = L"");
bool				IsMember(const AutoUTF &name, const AutoUTF &in, const AutoUTF &dom = L"");

AutoUTF				GetComm(const AutoUTF &name, const AutoUTF &dom = L"");
//AutoUTF			GetName(const AutoUTF &name, const AutoUTF &dom = L"");
//DWORD				GetGID(const AutoUTF &name, const AutoUTF &dom = L"");

void				Add(const AutoUTF &name, const AutoUTF &dom = L"");
void				Del(const AutoUTF &name, const AutoUTF &dom = L"");

void				AddMember(const AutoUTF &name, const AutoUTF &user, const AutoUTF &dom = L"");
void				AddMemberGid(const AutoUTF &gid, const AutoUTF &user, const AutoUTF &dom = L"");
void				DelMember(const AutoUTF &name, const AutoUTF &in, const AutoUTF &dom = L"");
void				SetName(const AutoUTF &name, const AutoUTF &in, const AutoUTF &dom = L"");
void				SetComm(const AutoUTF &name, const AutoUTF &in, const AutoUTF &dom = L"");
}

///======================================================================================= SysGroups
struct		GroupInfo {
	AutoUTF	comm;
	GroupInfo() : comm(L"") {
	}
};

class		SysGroups : public MapContainer<AutoUTF, GroupInfo> {
	AutoUTF		dom;
public:
	SysGroups(bool autocache = true) {
		if (autocache)
			Cache();
	}
	bool				Cache(const AutoUTF &dom = L"");
	bool				CacheByUser(const AutoUTF &name, const AutoUTF &dom = L"");

	void				Add(const AutoUTF &name);
	void				Del();
	void				SetName(const AutoUTF &in);
	void				SetComm(const AutoUTF &in);

	AutoUTF				GetName() const;
	AutoUTF				GetComm() const;
};

///▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒ net_SD
AutoUTF				AsSddl(PSECURITY_DESCRIPTOR pSD, SECURITY_INFORMATION in);
AutoUTF				Mode2Sddl(const AutoUTF &name, const AutoUTF &group, mode_t mode, const AutoUTF dom = L"");
AutoUTF				MakeSDDL(const AutoUTF &name, const AutoUTF &group, mode_t mode, bool pr = false, const AutoUTF dom = L"");

inline void			SetOwner(HANDLE handle, PSID owner, SE_OBJECT_TYPE type = SE_FILE_OBJECT) {
	CheckError(::SetSecurityInfo(handle, type, OWNER_SECURITY_INFORMATION, owner, NULL, NULL, NULL));
}
inline void			SetGroup(HANDLE handle, PSID owner, SE_OBJECT_TYPE type = SE_FILE_OBJECT) {
	CheckError(::SetSecurityInfo(handle, type, GROUP_SECURITY_INFORMATION, NULL, owner, NULL, NULL));
}
inline void			SetDacl(HANDLE handle, PACL pACL, SE_OBJECT_TYPE type = SE_FILE_OBJECT) {
	CheckError(::SetSecurityInfo(handle, type, DACL_SECURITY_INFORMATION, NULL, NULL, pACL, NULL));
}
inline void			SetSacl(HANDLE handle, PACL pACL, SE_OBJECT_TYPE type = SE_FILE_OBJECT) {
	CheckError(::SetSecurityInfo(handle, type, SACL_SECURITY_INFORMATION, NULL, NULL, NULL, pACL));
}

///=========================================================================================== WinSD
/// Security descriptor (Дескриптор защиты)
/// Version		- версия SD (revision)
/// Flags		- флаги состояния
/// Owner SID	- sid владельца
/// Group SID	- sid группы (не используется вендой, лишь для совместимости с POSIX)
/// DACL		- список записей контроля доступа
/// SACL		- список записей аудита

class		WinSD {
protected:
	PSECURITY_DESCRIPTOR	m_PSD;
	SE_OBJECT_TYPE			m_type;

	void			Free(PSECURITY_DESCRIPTOR &in) {
		if (in) {
			::LocalFree(in);
			in = NULL;
		}
	}
public:
	~WinSD() {
		Free(m_PSD);
	}
	WinSD(): m_PSD(NULL), m_type(SE_UNKNOWN_OBJECT_TYPE) {
		m_PSD = (PSECURITY_DESCRIPTOR)::LocalAlloc(LMEM_ZEROINIT, sizeof(SECURITY_DESCRIPTOR));
		CheckAPI(::InitializeSecurityDescriptor(m_PSD, SECURITY_DESCRIPTOR_REVISION));
	}
	WinSD(SE_OBJECT_TYPE type): m_PSD(NULL), m_type(type) {
	}
	WinSD(const AutoUTF &in);

	operator		PSECURITY_DESCRIPTOR() const {
		return	m_PSD;
	}

	bool			Valid() const {
		return	Valid(m_PSD);
	}
	bool			IsProtected() const {
		WORD	control = Control();
		return	WinFlag::Check(control, (WORD)SE_DACL_PROTECTED);
	}
	bool			IsSelfRelative() const {
		WORD	control = Control();
		return	WinFlag::Check(control, (WORD)SE_SELF_RELATIVE);
	}
	DWORD			Size() const {
		return	Size(m_PSD);
	}

	WORD			Control() const {
		return	Control(m_PSD);
	}
	void			Control(WORD flag, bool s) {
		Control(m_PSD, flag, s);
	}
	AutoUTF			Owner() const {
		return	Sid::AsName(GetOwner(m_PSD));
	}
	void			Owner(PSID pSid, bool deflt = false) {
		CheckAPI(::SetSecurityDescriptorOwner(m_PSD, pSid, deflt));
	}
	AutoUTF			Group() const {
		return	Sid::AsName(GetGroup(m_PSD));
	}
	void			Group(PSID pSid, bool deflt = false) {
		CheckAPI(::SetSecurityDescriptorGroup(m_PSD, pSid, deflt));
	}
	PACL			DACL() const {
		return	DACL(m_PSD);
	}
	void			DACL(PACL dacl) {
		CheckAPI(::SetSecurityDescriptorDacl(m_PSD, true, dacl, true));
	}
	void			MakeSelfRelative();
	void			Protect(bool pr) {
		Control(SE_DACL_PROTECTED, pr);
	}

	AutoUTF			AsSddl(SECURITY_INFORMATION in) const {
		return	::AsSddl(m_PSD, in);
	}
	AutoUTF			AsSddlOwner() const {
		return	AsSddl(OWNER_SECURITY_INFORMATION);
	}
	AutoUTF			AsSddlDACL() const {
		return	AsSddl(DACL_SECURITY_INFORMATION);
	}

	AutoUTF			Parse() const {
		return	Parse(m_PSD);
	}

	static bool		Valid(PSECURITY_DESCRIPTOR pSD) {
		return	pSD && ::IsValidSecurityDescriptor(pSD);
	}
	static DWORD	Size(PSECURITY_DESCRIPTOR pSD) {
		CheckAPI(Valid(pSD));
		return	::GetSecurityDescriptorLength(pSD);
	}
	static WORD		Control(PSECURITY_DESCRIPTOR pSD) {
		WORD	Result = 0;
		DWORD	Revision;
		CheckAPI(::GetSecurityDescriptorControl(pSD, &Result, &Revision));
		return	Result;
	}
	static void		Control(PSECURITY_DESCRIPTOR pSD, WORD flag, bool s) {
		CheckAPI(::SetSecurityDescriptorControl(pSD, flag, s ? flag : 0));
	}
	static PSID		GetOwner(PSECURITY_DESCRIPTOR pSD) {
		PSID	psid;
		BOOL	bTmp;
		CheckAPI(::GetSecurityDescriptorOwner(pSD, &psid, &bTmp));
		return	psid;
	}
	static PSID		GetGroup(PSECURITY_DESCRIPTOR pSD) {
		PSID	psid;
		BOOL	bTmp;
		CheckAPI(::GetSecurityDescriptorGroup(pSD, &psid, &bTmp));
		return	psid;
	}
	static PACL		DACL(PSECURITY_DESCRIPTOR pSD) {
		BOOL	bDaclPresent   = false;
		BOOL	bDaclDefaulted = false;
		PACL	Result = NULL;
		CheckAPI(::GetSecurityDescriptorDacl(pSD, &bDaclPresent, &Result, &bDaclDefaulted));
		CheckAPI(bDaclPresent);
		return	Result;
	}
	static AutoUTF	Parse(PSECURITY_DESCRIPTOR pSD);
};

void				SetOwner(const AutoUTF &path, PSID owner, bool setpriv, SE_OBJECT_TYPE type = SE_FILE_OBJECT);
inline void			SetOwner(const AutoUTF &path, PSID owner, SE_OBJECT_TYPE type = SE_FILE_OBJECT) {
	SetOwner(path, owner, false, type);
}
void				SetOwnerSD(const AutoUTF &name, PSECURITY_DESCRIPTOR pSD, SE_OBJECT_TYPE type = SE_FILE_OBJECT);

inline void			SetGroup(const AutoUTF &path, PSID owner, SE_OBJECT_TYPE type = SE_FILE_OBJECT) {
	CheckError(::SetNamedSecurityInfoW((PWSTR)path.c_str(), type, GROUP_SECURITY_INFORMATION, NULL, owner, NULL, NULL));
}
void				SetGroupSD(const AutoUTF &name, PSECURITY_DESCRIPTOR pSD, SE_OBJECT_TYPE type = SE_FILE_OBJECT);

inline void			SetDacl(const AutoUTF &path, PACL pacl, SE_OBJECT_TYPE type = SE_FILE_OBJECT) {
	CheckError(::SetNamedSecurityInfoW((PWSTR)path.c_str(), type, DACL_SECURITY_INFORMATION, NULL, NULL, pacl, NULL));
}
void				SetDacl(const AutoUTF &name, PSECURITY_DESCRIPTOR pSD, SE_OBJECT_TYPE type = SE_FILE_OBJECT);

inline void			SetSacl(const AutoUTF &path, PACL pacl, SE_OBJECT_TYPE type = SE_FILE_OBJECT) {
	CheckError(::SetNamedSecurityInfoW((PWSTR)path.c_str(), type, SACL_SECURITY_INFORMATION, NULL, NULL, NULL, pacl));
}

void				SetSecurity(const AutoUTF &path, PSECURITY_DESCRIPTOR pSD, SE_OBJECT_TYPE type = SE_FILE_OBJECT);
inline void			SetSecurity(const AutoUTF &path, const AutoUTF &sddl, SE_OBJECT_TYPE type = SE_FILE_OBJECT) {
	WinSD	sd(sddl);
	SetSecurity(path, sd, type);
}
void				SetSecurity(HANDLE hnd, PSECURITY_DESCRIPTOR pSD, SE_OBJECT_TYPE type = SE_FILE_OBJECT);

AutoUTF				GetOwner(HANDLE hnd, SE_OBJECT_TYPE type = SE_FILE_OBJECT);
AutoUTF				GetOwner(const AutoUTF &path, SE_OBJECT_TYPE type = SE_FILE_OBJECT);
AutoUTF				GetGroup(HANDLE hnd, SE_OBJECT_TYPE type = SE_FILE_OBJECT);
AutoUTF				GetGroup(const AutoUTF &path, SE_OBJECT_TYPE type = SE_FILE_OBJECT);

/// Security descriptor by handle
class		WinSDH: public WinSD {
	HANDLE		m_hnd;
public:
	WinSDH(HANDLE handle, SE_OBJECT_TYPE type = SE_FILE_OBJECT): WinSD(type), m_hnd(handle) {
		Get();
	}
	HANDLE			hnd() const {
		return	m_hnd;
	}
	void			Get() {
		Free(m_PSD);
		CheckError(::GetSecurityInfo(m_hnd, m_type,
									 OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION |
									 DACL_SECURITY_INFORMATION,
									 NULL, NULL, NULL, NULL, &m_PSD));
	}
	void			Set() const {
		SetSecurity(m_hnd, m_PSD, m_type);
	}
};

/// Security descriptor by name
class		WinSDW: public WinSD {
	AutoUTF		m_name;
public:
	WinSDW(const AutoUTF &name, SE_OBJECT_TYPE type = SE_FILE_OBJECT): WinSD(type), m_name(name) {
		Get();
	}
	AutoUTF			name() const {
		return	m_name;
	}
	void			Get() {
		Free(m_PSD);
		CheckError(::GetNamedSecurityInfoW((PWSTR)m_name.c_str(), m_type,
										   OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION |
										   DACL_SECURITY_INFORMATION,
										   NULL, NULL, NULL, NULL, &m_PSD));
	}
	void			Set() const {
		SetSecurity(m_name, m_PSD, m_type);
	}
};

///▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒ net_dacl
///========================================================================================= WinDacl
class		WinDacl {
	PACL		m_PACL;
	bool		needDelete;

	void		Init(PACL pACL);
	void		Init(PSECURITY_DESCRIPTOR pSD);
public:
	~WinDacl() {
		if (needDelete)
			WinMem::Free(m_PACL);
	}
	WinDacl(size_t size);
	WinDacl(PACL pACL);
	WinDacl(PSECURITY_DESCRIPTOR pSD);
	WinDacl(const AutoUTF &name, SE_OBJECT_TYPE type = SE_FILE_OBJECT);

	operator				const PACL() const {
		return	m_PACL;
	}

	bool					Valid() const {
		return	Valid(m_PACL);
	}
	ACL_SIZE_INFORMATION	GetAclInfo() const {
		ACL_SIZE_INFORMATION	info;
		GetAclInfo(m_PACL, info);
		return	info;
	}
	size_t					CountAces() const {
		return	CountAces(m_PACL);
	}
	size_t					GetUsed() const {
		return	GetUsed(m_PACL);
	}
	size_t					GetFree() const {
		return	GetFree(m_PACL);
	}
	size_t					GetSize() const {
		return	GetSize(m_PACL);
	}
	AutoUTF					Parse() const {
		return	Parse(m_PACL);
	}

	void					AddA(const Sid &sid);
	void					AddD(const Sid &sid);

	void					DelInheritedAces() {
		DelInheritedAces(m_PACL);
	}

	static bool				Valid(PACL in) {
		return	::IsValidAcl(in);
	}
	static void				GetAclInfo(PACL pACL, ACL_SIZE_INFORMATION &out);
	static void				DelInheritedAces(PACL pACL);
	static size_t			CountAces(PACL pACL);
	static size_t			CountAces(PACL pACL, size_t &sz, bool inh);
	static size_t			GetUsed(PACL pACL);
	static size_t			GetFree(PACL pACL);
	static size_t			GetSize(PACL pACL);
	static PVOID			GetAce(PACL pACL, size_t index);

	void					SetTo(DWORD flag, const AutoUTF &name, SE_OBJECT_TYPE type) const {
		SetTo(m_PACL, flag, name, type);
	}

	static AutoUTF			Parse(PACL pACL);

	static void				SetTo(PACL pACL, DWORD flag, const AutoUTF &path, SE_OBJECT_TYPE type = SE_FILE_OBJECT) {
		CheckError(::SetNamedSecurityInfoW((PWSTR)path.c_str(), type,
										   DACL_SECURITY_INFORMATION | flag, NULL, NULL, pACL, NULL));
	}
	static void				Inherit(const AutoUTF &path, SE_OBJECT_TYPE type = SE_FILE_OBJECT);
	static void				Protect(const AutoUTF &path, bool copy, SE_OBJECT_TYPE type = SE_FILE_OBJECT);
};

///======================================================================================= WinAccess
struct		AccessInfo {
	AutoUTF	type;
	AutoUTF unix;
	DWORD	mask;
};

class		WinAccess : public MultiMapContainer<AutoUTF, AccessInfo> {
	PACL pACL;
public:
	WinAccess(const WinSD &sd, bool autocache = true): pACL(NULL) {
		pACL = sd.DACL();
		if (autocache)
			Cache();
	}
	bool					Cache();
};

///▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒ net_crypt
///========================================================================================== Base64
namespace	Base64 {
bool			Decode(PCSTR in, WinBuf<BYTE> &buf, DWORD flags = CRYPT_STRING_BASE64_ANY);
bool			Decode(PCWSTR in, WinBuf<BYTE> &buf, DWORD flags = CRYPT_STRING_BASE64_ANY);
CStrA			EncodeA(PVOID buf, DWORD size, DWORD flags = CRYPT_STRING_BASE64);
AutoUTF			Encode(PVOID buf, DWORD size, DWORD flags = CRYPT_STRING_BASE64);
}

///==================================================================================== CertDataBlob
class		CertDataBlob : public CRYPT_DATA_BLOB {
	bool			Create(size_t size) {
		Destroy();
		cbData = size;
		WinMem::Alloc(pbData, cbData);
		return	true;
	}
	bool			Destroy() {
		if (pbData) {
			WinMem::Free(pbData);
			cbData = 0;
			pbData = NULL;
			return	true;
		}
		return	false;
	}
public:
	~CertDataBlob() {
		Destroy();
	}
	CertDataBlob(size_t size = 0) {
		WinMem::Zero(*this);
		Create(size);
	}
	CertDataBlob(const AutoUTF &in) {
		cbData = (in.size() + 1) * sizeof(wchar_t);
		WinMem::Alloc(pbData, cbData);
		Copy((PWSTR)pbData, in.c_str());
	}
	bool			MakeCertNameBlob(const AutoUTF &in, DWORD enc = X509_ASN_ENCODING | PKCS_7_ASN_ENCODING) {
		DWORD	dwStrType = CERT_X500_NAME_STR;
		if (in.Find(L'\"'))
			WinFlag::Set(dwStrType, (DWORD)CERT_NAME_STR_NO_QUOTING_FLAG);

		DWORD	size = 0;
		if (::CertStrToNameW(enc, in.c_str(), dwStrType, NULL, NULL, &size, NULL)) {
			Create(size);
			if (::CertStrToNameW(enc, in.c_str(), dwStrType, NULL, pbData, &cbData, NULL))
				return	true;
		}
		return	false;
	}
	void			reserve() {
		WinMem::Realloc(pbData, cbData);
	}
	bool			reserve(size_t size) {
		if (size > cbData) {
			cbData = size;
			WinMem::Realloc(pbData, size);
			return	true;
		}
		return	false;
	}
};

///==================================================================================== WinCryptProv
class		WinCryptProv: public WinErrorCheck {
	HCRYPTPROV	m_hnd;
	PCWSTR		m_prov;
	DWORD		m_type;
public:
	~WinCryptProv() {
		Close();
	}
	// type = (PROV_RSA_FULL, PROV_RSA_AES)
	WinCryptProv(PCWSTR prov = NULL, DWORD type = PROV_RSA_FULL): m_hnd(NULL), m_prov(prov), m_type(type) {
	}
	bool			Open(PCWSTR name = NULL, DWORD flags = CRYPT_MACHINE_KEYSET) {
		Close();
		if (!::CryptAcquireContextW(&m_hnd, name, m_prov, m_type, flags)) {
			return	ChkSucc(::CryptAcquireContextW(&m_hnd, name, m_prov, m_type, flags | CRYPT_NEWKEYSET));
		}
		return	true;
	}
	void			Close() {
		if (m_hnd) {
			::CryptReleaseContext(m_hnd, 0);
			m_hnd = NULL;
		}
	}
	bool			KeyExist(DWORD type) const {
		HCRYPTKEY	key = NULL;
		bool	Result = ::CryptGetUserKey(m_hnd, type, &key);
		if (Result) {
			::CryptDestroyKey(key);
			return	true;
		}
		return	false;
	}
	bool			KeyCheck(DWORD type) const {
		HCRYPTKEY	key = NULL;
//		if (!ChkSucc(::CryptGetUserKey(m_hnd, type, &key))) {
		if (!ChkSucc(::CryptGenKey(m_hnd, type, CRYPT_EXPORTABLE, &key))) {
			return	false;
		}
//		}
		return	::CryptDestroyKey(key);
	}
	HCRYPTKEY		KeyOpen(DWORD type, DWORD flags = CRYPT_EXPORTABLE) const {
		HCRYPTKEY	key = NULL;
		if (!::CryptGetUserKey(m_hnd, type, &key))
			return	KeyGenerate(type, flags);
		return	key;
	}
	HCRYPTKEY		KeyGenerate(DWORD type, DWORD flags = CRYPT_EXPORTABLE) const {
		HCRYPTKEY	key = NULL;
		::CryptGenKey(m_hnd, type, flags, &key);
		return	key;
	}
	bool			KeyClose(HCRYPTKEY key) const {
		return	::CryptDestroyKey(key);
	}
	operator		HCRYPTPROV() const {
		return	m_hnd;
	}
};

///==================================================================================== WinCryptHash
class		WinCryptHash: public WinErrorCheck {
	HCRYPTHASH		m_handle;

	bool			Close() {
		if (m_handle) {
			::CryptDestroyHash(m_handle);
			return	true;
		}
		return	false;
	}
public:
	~WinCryptHash() {
		Close();
	}
	// alg = (CALG_MD5, CALG_SHA1, CALG_SHA_512)
	WinCryptHash(HCRYPTPROV prov, ALG_ID alg): m_handle(NULL) {
		ChkSucc(::CryptCreateHash(prov, alg, 0, 0, &m_handle));
	}
	WinCryptHash(HCRYPTHASH hash): m_handle(NULL) {
		ChkSucc(::CryptDuplicateHash(hash, NULL, 0, &m_handle));
	}
	WinCryptHash(const WinCryptHash &in): m_handle(NULL) {
		ChkSucc(::CryptDuplicateHash(in, NULL, 0, &m_handle));
	}

	operator		HCRYPTHASH() const {
		return	m_handle;
	}
	const WinCryptHash&	operator=(const WinCryptHash &in) {
		if (this != &in) {
			Close();
			ChkSucc(::CryptDuplicateHash(in.m_handle, NULL, 0, &m_handle));
		}
		return	*this;
	}

	bool			Hash(const PBYTE buf, size_t size) {
		return	ChkSucc(::CryptHashData(m_handle, buf, size, 0));
	}
	bool			Hash(PCWSTR path, uint64_t size = (uint64_t) - 1) {
		FileMap	file(path, size);
		if (file.IsOK())
			return	Hash(file);
		err(file.err());
		return	false;
	}
	bool			Hash(FileMap &file) {
		bool	Result;
		file.Home();
		while (file.Next())
			Result = Hash((PBYTE)file.data(), file.size());
		return	Result;
	}
	size_t			GetHashSize() const {
		DWORD	Result = 0;
		::CryptGetHashParam(m_handle, HP_HASHVAL, NULL, &Result, 0);
		return	Result;
	}
	ALG_ID			GetHashAlg() const {
		DWORD	Result = 0;
		::CryptGetHashParam(m_handle, HP_ALGID, NULL, &Result, 0);
		return	Result;
	}
	bool			GetHash(PBYTE buf, DWORD size) const {
		return	ChkSucc(::CryptGetHashParam(m_handle, HP_HASHVAL, buf, &size, 0));
	}
};

///========================================================================================= WinCert
class		WinCert : public WinErrorCheck {
	PCCERT_CONTEXT  	m_cert;

	void 				CertClose() {
		if (m_cert != NULL) {
			::CertFreeCertificateContext(m_cert);
			m_cert = NULL;
		}
	}
public:
	~WinCert() {
		CertClose();
	}
	WinCert(): m_cert(NULL) {
	}
	explicit WinCert(PCCERT_CONTEXT in);
	explicit WinCert(const WinCert &in);
	bool				Gen(const AutoUTF &in, const AutoUTF &guid, PSYSTEMTIME until = NULL);
	bool				Del();

	bool				ToFile(const AutoUTF &path) const;

	bool				AddKey(const AutoUTF &in);
	bool				Store(HANDLE in);
	AutoUTF				GetAttr(DWORD in) const;
	AutoUTF				GetProp(DWORD in) const;
	AutoUTF				name() const {
		return	GetAttr(CERT_NAME_SIMPLE_DISPLAY_TYPE);
	}
	AutoUTF				GetDNS() const {
		return	GetAttr(CERT_NAME_DNS_TYPE);
	}
	AutoUTF				GetURL() const {
		return	GetAttr(CERT_NAME_URL_TYPE);
	}
	AutoUTF				GetUPN() const {
		return	GetAttr(CERT_NAME_UPN_TYPE);
	}
	AutoUTF				GetMail() const {
		return	GetAttr(CERT_NAME_EMAIL_TYPE);
	}
	AutoUTF				GetRdn() const {
		return	GetAttr(CERT_NAME_RDN_TYPE);
	}
	FILETIME			GetStart() const {
		return	m_cert->pCertInfo->NotBefore;
	}
	FILETIME			GetEnd() const {
		return	m_cert->pCertInfo->NotBefore;
	}

	CStrA				GetHashString() const;
	size_t				GetHashSize() const;
	bool				GetHash(PVOID hash, DWORD size) const;
	bool				GetHash(WinBuf<BYTE> &hash) const;

	AutoUTF				FriendlyName() const {
//		return	GetAttr(CERT_NAME_FRIENDLY_DISPLAY_TYPE);
		return	GetProp(CERT_FRIENDLY_NAME_PROP_ID);
	}
	bool				FriendlyName(const AutoUTF &in) const {
		return	FriendlyName(m_cert, in);
	}

	operator 			PCCERT_CONTEXT() {
		return	m_cert;
	}

	static AutoUTF		GetProp(PCCERT_CONTEXT pctx, DWORD in);
	static AutoUTF		FriendlyName(PCCERT_CONTEXT pctx) {
		return	GetProp(pctx, CERT_FRIENDLY_NAME_PROP_ID);
	}
	static bool			FriendlyName(PCCERT_CONTEXT pctx, const AutoUTF &in);
	static CStrA		HashString(PCCERT_CONTEXT pctx);
};

///======================================================================================== WinStore
class		WinStore : private Uncopyable, public WinErrorCheck {
	HCERTSTORE	m_hnd;
	AutoUTF		m_name;

	bool				StoreClose() {
		if (m_hnd && m_hnd != INVALID_HANDLE_VALUE) {
//			::CertCloseStore(m_hnd, CERT_CLOSE_STORE_FORCE_FLAG);
			::CertCloseStore(m_hnd, CERT_CLOSE_STORE_CHECK_FLAG);
			m_hnd = NULL;
			return	true;
		}
		return	false;
	}
public:
	~WinStore() {
		StoreClose();
	}
	explicit			WinStore(const AutoUTF &in): m_hnd(NULL), m_name(in) {
	}

	bool				OpenMachineStore(DWORD flags = 0) {
		WinFlag::Set(flags, (DWORD)CERT_SYSTEM_STORE_LOCAL_MACHINE);
		StoreClose();
		m_hnd = ::CertOpenStore(CERT_STORE_PROV_SYSTEM, 0, (HCRYPTPROV)NULL, flags, m_name.c_str());
		return	ChkSucc(m_hnd);
	}
	bool				OpenUserStore(DWORD flags = 0) {
		WinFlag::Set(flags, (DWORD)CERT_SYSTEM_STORE_CURRENT_USER);
		StoreClose();
		m_hnd = ::CertOpenStore(CERT_STORE_PROV_SYSTEM, 0, (HCRYPTPROV)NULL, flags, m_name.c_str());
		return	ChkSucc(m_hnd);
	}
	bool				OpenMemoryStore(DWORD flags = 0) {
		WinFlag::Set(flags, (DWORD)CERT_STORE_CREATE_NEW_FLAG);
		StoreClose();
		m_hnd = ::CertOpenStore(sz_CERT_STORE_PROV_MEMORY, 0, NULL, flags, NULL);
		return	ChkSucc(m_hnd);
	}

	operator 			HCERTSTORE() const {
		return	m_hnd;
	}
	AutoUTF				name() const {
		return	m_name;
	}

	CStrA				FromFile(const AutoUTF &path, const AutoUTF &pass, const AutoUTF &add) const;
};

///================================================================================= WinCertificates
class		WinCertificates : public MapContainer<CStrA, WinCert> {
public:
	~WinCertificates() {
	}
	WinCertificates() {
	}
	bool				CacheByStore(const WinStore &in) {
		if (in.IsOK()) {
			HRESULT	err = 0;
			PCCERT_CONTEXT  pCert = NULL;
			while ((pCert = ::CertEnumCertificatesInStore(in, pCert))) {
				WinCert	info(pCert);
				Insert(info.GetHashString(), info);
			}
			err = ::GetLastError();
			return	err == CRYPT_E_NOT_FOUND;
		}
		return	false;
	}
	bool				Del();
	bool				Del(const CStrA &hash) {
		if (Find(hash)) {
			return	Del();
		}
		return	false;
	}
	bool				FindByName(const AutoUTF &in);
	bool				FindByFriendlyName(const AutoUTF &in);
};

///==================================================================================== WinSysTimers
struct		WinSysTimers {
	LARGE_INTEGER liKeBootTime;
	LARGE_INTEGER liKeSystemTime;
	LARGE_INTEGER liExpTimeZoneBias;
	ULONG uCurrentTimeZoneId;
	DWORD dwReserved;

	WinSysTimers() {
		WinMem::Zero(*this);
		typedef LONG(WINAPI * PROCNTQSI)(UINT, PVOID, ULONG, PULONG);

		PROCNTQSI NtQuerySystemInformation;

		NtQuerySystemInformation = (PROCNTQSI)::GetProcAddress(::GetModuleHandleW(L"ntdll"), "NtQuerySystemInformation");

		if (!NtQuerySystemInformation)
			return;

		NtQuerySystemInformation(3, this, sizeof(*this), 0);
	}
	size_t		Uptime(size_t del = 1) {
		ULONGLONG	start = liKeBootTime.QuadPart;
		ULONGLONG	now = liKeSystemTime.QuadPart;
		ULONGLONG	Result = (now - start) / 10000000LL;
		return	Result / del;
	}
	AutoUTF		UptimeAsText();
};

///▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒ net_if
///=========================================================================================== WinIf
inline AutoUTF		MacAsStr(const PBYTE mac, size_t size) {
	WCHAR	buf[(size + 1) * 4];
	WinMem::Zero(buf, sizeof(buf));
	PWSTR	tmp = buf;
	for (size_t i = 0; i < size; ++i) {
		if (i == (size - 1))
			tmp += snprintf(tmp, sizeofa(buf) - i * 2, L"%.2X", mac[i]);
		else
			tmp += snprintf(tmp, sizeofa(buf) - i * 2, L"%.2X-", mac[i]);
	}
	return	AutoUTF(buf);
}
inline AutoUTF		IpAsStr(LPSOCKADDR addr, size_t	len) {
	WCHAR	buf[64];
	WinMem::Zero(buf, sizeof(buf));
	DWORD	size = sizeofa(buf);
	::WSAAddressToStringW(addr, len, NULL, buf, &size);
	return	AutoUTF(buf);
}
inline AutoUTF		IpAsStr(SOCKET_ADDRESS pAddr) {
	return	IpAsStr(pAddr.lpSockaddr, pAddr.iSockaddrLength);
}

class		WinIp: public MapContainer<AutoUTF, SOCKET_ADDRESS> {
public:
	WinIp(bool autocache = true) {
		if (autocache)
			Cache();
	}
	bool		Cache() {
		ULONG	size = 0;
		if (::GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, NULL, NULL, &size) == ERROR_BUFFER_OVERFLOW) {
			WinBuf<IP_ADAPTER_ADDRESSES> ipbuf(size, true);
			if (::GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, NULL, ipbuf.data(), &size) == ERROR_SUCCESS) {
				Clear();
				PIP_ADAPTER_ADDRESSES pCurrAddresses = ipbuf.data();
				while (pCurrAddresses) {
					size_t	i = 0;
					PIP_ADAPTER_UNICAST_ADDRESS pUnicast = pCurrAddresses->FirstUnicastAddress;
					for (i = 0; pUnicast != NULL; ++i) {
						Insert(IpAsStr(pUnicast->Address), pUnicast->Address);
						pUnicast = pUnicast->Next;
					}
					pCurrAddresses = pCurrAddresses->Next;
				}
				return	true;
			}
		}
		return	false;
	}
};


///▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒ net_sock
///======================================================================================== WSockLib
class		WSockLib {
	WSADATA	wsaData;

	WSockLib(const WSockLib&);
	WSockLib() {
		int		err = ::WSAStartup(MAKEWORD(1, 2), &wsaData);
		if (err)
			throw	WSockError(err);
	}
public:
	~WSockLib() {
		::WSACleanup();
	}

	static WSockLib	&Init() {
		static WSockLib init;
		return	init;
	}
};

///=========================================================================================== WSock
class		WSock {
	SOCKET	m_sock;
	int		m_fam;
public:
	~WSock() {
		::closesocket(m_sock);
	}
	WSock(int fam = AF_INET): m_sock(INVALID_SOCKET), m_fam(fam) {
		WSockLib::Init();
		m_sock = ::socket(m_fam, SOCK_STREAM, 0);
		if (m_sock == INVALID_SOCKET)
			throw	WSockError(L"WinSock allocate socket error: ");
	}

	void		Connect(const AutoUTF &ip, DWORD port) {
		INT		size = 128;
		WinBuf<SOCKADDR>	addr(size, true);
		INT		err = ::WSAStringToAddressW((PWSTR)ip.c_str(), m_fam, NULL, addr.data(), &size);
		if (err && err != WSAEFAULT)
			throw	WSockError(L"WinSock determine address error: ");
		if (err == WSAEFAULT) {
			addr.reserve(size);
			err = ::WSAStringToAddressW((PWSTR)ip.c_str(), m_fam, NULL, addr.data(), &size);
			if (err)
				throw	WSockError(L"WinSock determine address error: ");
		}
		if (m_fam == AF_INET || m_fam == AF_INET6) {
			sockaddr_in* tmp = (sockaddr_in*)addr.data();
			tmp->sin_port = htons(port);
		}
		if (::connect(m_sock, addr.data(), size))
			throw	WSockError(L"WinSock connect error: ");

	}
	void	Send(void* buf, size_t len) {
		::send(m_sock, (const char*)buf, len, 0);
	}
	operator	SOCKET() {
		return	m_sock;
	}
};

#endif // WIN_NET_HPP
