/**
	win_net

	@classes	()
	@author		© 2009 Andrew Grechkin
	@link		()
	Source code: <http://code.google.com/p/andrew-grechkin>
**/

#ifndef WIN_NET_HPP
#define WIN_NET_HPP

#include "win_def.h"

#include "win_c_map.h"

#include <wtsapi32.h>

///*************************************************************************************************
///======================================================================================== WinError
class		WinError {
	CStrW	m_msg;
	DWORD	m_code;
public:
	WinError(const CStrW &msg, DWORD code = 0): m_msg(msg), m_code(code) {
	}
	WinError(DWORD code, const CStrW &msg = L""): m_msg(msg), m_code(code) {
	}

	CStrW		msg(PCWSTR msg) {
		return	(m_msg = msg);
	}
	CStrW		msg() const {
		return	m_msg;
	}
	DWORD		code() const {
		return	m_code;
	}
	void		show() const {
		mbox(m_msg, L"WinError");
	}
};
class		RuntimeError : public WinError {
public:
	RuntimeError(const CStrW &msg, DWORD code = 0): WinError(msg, code) {
	}
	RuntimeError(DWORD code, const CStrW &msg = L""): WinError(msg, code) {
	}
};
class		ApiError: public RuntimeError {
public:
	ApiError(const CStrW &msg, DWORD code = 0): RuntimeError(msg, code) {
	}
	ApiError(DWORD code): RuntimeError(Err(code), code) {
	}
};
class		NetError: public ApiError {
public:
	NetError(const CStrW &msg): ApiError(msg) {
	}
	NetError(DWORD code): ApiError(code) {
	}
};
class		ActionError: public ApiError {
public:
	ActionError(const CStrW &msg): ApiError(msg) {
	}
	ActionError(DWORD code): ApiError(code) {
	}
	ActionError(): ApiError(::GetLastError()) {
	}
};

inline void	CheckAPI(bool r) {
	if (!r) {
		throw	ActionError();
	}
}

///========================================================================================== WinNet
namespace	WinNet {
bool 			GetCompName(CStrW &buf, COMPUTER_NAME_FORMAT cnf);
inline bool 	SetCompName(const CStrW &in, COMPUTER_NAME_FORMAT cnf) {
	return	::SetComputerNameExW(cnf, in.c_str()) != 0;
}
}

///*************************************************************************************************
///================================================================================ RemoteConnection
class		RemoteConnection {
	CStrW	m_host;
	bool	m_conn;
public:
	~RemoteConnection();
	RemoteConnection(PCWSTR host = NULL);
	void		Open(PCWSTR host, PCWSTR user = NULL, PCWSTR pass = NULL);
	void		Close();
	bool		Test(PCWSTR host);
	CStrW		host() const {
		return	m_host;
	}
};

///*************************************************************************************************
///========================================================================================== WinScm
class		WinScm {
	SC_HANDLE			m_hndl;
	ACCESS_MASK			m_mask;
	RemoteConnection	*m_conn;

public:
	~WinScm() {
		CloseMGR(m_hndl);
	}
//	WinScm(ACCESS_MASK acc = SC_MANAGER_CONNECT | SC_MANAGER_ENUMERATE_SERVICE, RemoteConnection *conn = NULL): m_hndl(NULL), m_mask(acc), m_conn(conn) {
	WinScm(ACCESS_MASK acc, RemoteConnection *conn = NULL): m_hndl(NULL), m_mask(acc), m_conn(conn) {
		OpenMGR(m_hndl, m_mask, m_conn);
	}

	void					Close() {
		CloseMGR(m_hndl);
	}
	void					Reopen(ACCESS_MASK acc = SC_MANAGER_CONNECT | SC_MANAGER_ENUMERATE_SERVICE, RemoteConnection *conn = NULL) {
		CloseMGR(m_hndl);
		m_mask = acc;
		m_conn = conn;
		OpenMGR(m_hndl, m_mask, m_conn);
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
							 SERVICE_ALL_ACCESS,			// desired access
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

	static void				OpenMGR(SC_HANDLE &hSC, DWORD acc = SC_MANAGER_CONNECT, RemoteConnection *conn = NULL) {
		hSC = ::OpenSCManagerW((conn != NULL) ? conn->host().c_str() : NULL, NULL, acc);
		CheckAPI(hSC != NULL);
	}
	static void				CloseMGR(SC_HANDLE &in) {
		if (in && in != INVALID_HANDLE_VALUE) {
			::CloseServiceHandle(in);
			in = NULL;
		}
	}
};

///========================================================================================== WinSvc
class		WinSvc {
	SC_HANDLE			m_hndl;
public:
	~WinSvc() {
		if (m_hndl) {
			::CloseServiceHandle(m_hndl);
		}
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
	void					WaitForState(DWORD state, DWORD dwTimeout, Functor &func, PVOID param = NULL) {
		DWORD	dwStartTime = ::GetTickCount();
		DWORD	dwBytesNeeded;
		SERVICE_STATUS_PROCESS ssp = {0};
		while (true) {
			CheckAPI(::QueryServiceStatusEx(m_hndl, SC_STATUS_PROCESS_INFO, (PBYTE)&ssp, sizeof(ssp), &dwBytesNeeded));
			if (ssp.dwCurrentState == state)
				break;
			if (::GetTickCount() - dwStartTime > dwTimeout)
				throw	ActionError(WAIT_TIMEOUT);
			func(state, ::GetTickCount() - dwStartTime, param);
		}
	}
	void					WaitForState(DWORD state, DWORD dwTimeout);

	void					Start() {
		try {
		CheckAPI(::StartService(m_hndl, 0, NULL));
		} catch (ApiError e) {
			if (e.code() != 1056)
				throw;
		}
	}
	void					Stop() {
		SERVICE_STATUS	ss;
		try {
			CheckAPI(::ControlService(m_hndl, SERVICE_CONTROL_STOP, &ss));
		} catch (ApiError e) {
			if (e.code() != 1062)
				throw;
		}
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
	}

	operator				SC_HANDLE() const {
		return	m_hndl;
	}
};

///========================================================================================== struct
struct		s_ServiceInfo: public _SERVICE_STATUS {
	CStrW		name;				// AN C0
	CStrW		dname;				// N
	CStrW		path;				// C3
	CStrW		descr;				// Z
	CStrMW		Dependencies;		// LN
	CStrW		OrderGroup;			// C5
	CStrW		ServiceStartName;	// C6

	DWORD		StartType;			// C2
	DWORD		ServiceType;
	DWORD		ErrorControl;
	DWORD		TagId;

	s_ServiceInfo() {
		WinMem::Zero(*this);
	}
	s_ServiceInfo(const CStrW &n, const SERVICE_STATUS &sp): _SERVICE_STATUS(sp), name(n) {
		ServiceType = StartType = ErrorControl = TagId = 0;
	}
};

///===================================================================================== WinServices
class		WinServices : public MapContainer<CStrW, s_ServiceInfo> {
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
//	bool				CacheByName(CONSTRW &in);
	bool				CacheByState(DWORD state = SERVICE_STATE_ALL);
	bool				CacheByType(DWORD state = SERVICE_STATE_ALL);

	bool				services() {
		return m_type == (SERVICE_WIN32 | SERVICE_INTERACTIVE_PROCESS);
	}
	bool				drivers() {
		return m_type == (SERVICE_ADAPTER | SERVICE_DRIVER);
	}
	void				services(bool st) {
		if (st)
			m_type = SERVICE_WIN32 | SERVICE_INTERACTIVE_PROCESS;
	}
	void				drivers(bool st) {
		if (st)
			m_type = SERVICE_ADAPTER | SERVICE_DRIVER;
	}
	DWORD				state() const {
		return	Value().dwCurrentState;
	}
};

void		InstallService(PCWSTR name, PCWSTR path, DWORD StartType = SERVICE_DEMAND_START, PCWSTR dispname = NULL);
void		UninstallService(PCWSTR name);

///*************************************************************************************************
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
	CStrW					sess;
	CStrW					user;
	CStrW					winSta;
	CStrW					client;
	WTS_CONNECTSTATE_CLASS	state;

	WinTSInfo(DWORD i, const CStrW &s, const CStrW &u, WTS_CONNECTSTATE_CLASS st): id(i), sess(s), user(u), state(st) {
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

	CStrW				Info() const {
		CStrW	Result;
		Result += L"Id:           ";
		Result.AddNum(Key());
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

///*************************************************************************************************
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

#endif // WIN_NET_HPP
