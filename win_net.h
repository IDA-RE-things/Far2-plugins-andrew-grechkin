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

///======================================================================================== WinError
class		WinError {
	CStrW	m_msg;
	DWORD	m_code;
public:
	WinError(const CStrW &msg, DWORD code = 0): m_msg(msg), m_code(code) {
	}
	WinError(DWORD code, const CStrW &msg = L""): m_msg(msg), m_code(code) {
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
	ApiError(DWORD code): RuntimeError(CStrW::err(code), code) {
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

inline void	CheckAction(bool r) {
	if (!r) {
		throw	ActionError();
	}
}

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
	WinScm(ACCESS_MASK acc, RemoteConnection *conn): m_hndl(NULL), m_mask(acc), m_conn(conn) {
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

	void					Create(PCWSTR name, PCWSTR path, PCWSTR disp = NULL) {
		SC_HANDLE	hSvc = ::CreateServiceW(
							 m_hndl, name,
							 (disp == NULL) ? name : disp,
							 SERVICE_ALL_ACCESS,			// desired access
							 SERVICE_WIN32_OWN_PROCESS,	// service type
							 SERVICE_DEMAND_START,		// start type
							 SERVICE_ERROR_NORMAL,		// WinError control type
							 path,             			// path to service's binary
							 NULL,						// no load ordering group
							 NULL,						// no tag identifier
							 NULL,						// no dependencies
							 NULL,						// LocalSystem account
							 NULL);						// no password
		CheckAction(hSvc != NULL);
		::CloseServiceHandle(hSvc);
	}

	static void				OpenMGR(SC_HANDLE &hSC, DWORD acc = SC_MANAGER_CONNECT, RemoteConnection *conn = NULL) {
		hSC = ::OpenSCManager((conn != NULL) ? conn->host().c_str() : NULL, NULL, acc);
		CheckAction(hSC != NULL);
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
//	WinSvc(PCWSTR name, ACCESS_MASK access = GENERIC_READ, RemoteConnection *conn = NULL): m_hndl(NULL) {
	WinSvc(PCWSTR name, ACCESS_MASK access, RemoteConnection *conn): m_hndl(NULL) {
		WinScm	scm(SC_MANAGER_CONNECT, conn);
		m_hndl = ::OpenServiceW(scm, name, access);
		CheckAction(m_hndl != NULL);
	}
	WinSvc(PCWSTR name, ACCESS_MASK access, const WinScm &scm): m_hndl(NULL) {
		m_hndl = ::OpenServiceW(scm, name, access);
		CheckAction(m_hndl != NULL);
	}

	void					QueryConfig(WinBuf<QUERY_SERVICE_CONFIGW> &buf) const;
	void					QueryConfig2(WinBuf<BYTE> &buf, DWORD level) const;

	void					WaitForState(DWORD state, DWORD dwTimeout);

	void					Start() {
		CheckAction(::StartService(m_hndl, 0, NULL));
	}
	void					Stop() {
		SERVICE_STATUS	ss;
		CheckAction(::ControlService(m_hndl, SERVICE_CONTROL_STOP, &ss));
	}
	void					Continue() {
		SERVICE_STATUS	ss;
		CheckAction(::ControlService(m_hndl, SERVICE_CONTROL_CONTINUE, &ss));
	}
	void					Pause() {
		SERVICE_STATUS	ss;
		CheckAction(::ControlService(m_hndl, SERVICE_CONTROL_PAUSE, &ss));
	}

	void					Del() {
		CheckAction(::DeleteService(m_hndl));
	}

	operator				SC_HANDLE() const {
		return	m_hndl;
	}
};

///========================================================================================== struct
struct		s_ServiceInfo: public _SERVICE_STATUS {
//	SERVICE_STATUS	ssp;
	CStrW		name;				// N
	CStrW		dname;				// C0
	CStrW		path;				// C3
	CStrW		descr;				// Z
	CStrW		Dependencies;		// C4
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

public:
	~WinServices() {
		Clear();
	}
	WinServices(RemoteConnection *conn = NULL, bool autocache = true): m_conn(conn) {
		if (autocache)
			Cache();
	}
	bool				Cache();
//	bool				CacheByName(CONSTRW &in);
	bool				CacheByState(DWORD state = SERVICE_STATE_ALL);
	bool				CacheByType(DWORD state = SERVICE_STATE_ALL);

	CStrW				Info() const {
		CStrW	Result;
		Result += L"Service name: ";
		Result += Key();
		Result += L"\n\n";
		Result += L"Display name: ";
		Result += Value().dname;
		Result += L"\n\n";
		Result += L"Description:  ";
		Result += Value().descr;
		Result += L"\n\n";
		return	Result;
	}
	DWORD				state() const {
		return	Value().dwCurrentState;
	}
};

///========================================================================================== SvcMgr
class		SvcMgr : private Uncopyable {
	RemoteConnection	m_conn;
	WinServices			m_sm;
public:
	SvcMgr(): m_sm(&m_conn, false) {
	}
	RemoteConnection*	conn() {
		return	&m_conn;
	}
	WinServices*		sm() {
		return	&m_sm;
	}

	CStrW				host() const {
		return	m_conn.host();
	}
	void				Connect(PCWSTR host, PCWSTR user = NULL, PCWSTR pass = NULL) {
		m_conn.Open(host, user, pass);
	}
	CStrW				name() const {
		return	m_sm.Key();
	}
	DWORD				state() const {
		return	m_sm.state();
	}
};

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
	CStrW					id;
	CStrW					sess;
	CStrW					user;
	WTS_CONNECTSTATE_CLASS	state;

	WinTSInfo(DWORD i, const CStrW &s, const CStrW &u, WTS_CONNECTSTATE_CLASS st): id(Num2Str(i)), sess(s), user(u), state(st) {
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

	bool				FindSess(PCWSTR in) const;
	bool				FindUser(PCWSTR in) const;
};

///========================================================================================== SvcMgr
class		TsMgr : private Uncopyable {
	RemoteConnection	m_conn;
	WinTS				m_ts;
public:
	TsMgr(): m_ts(&m_conn, false) {
	}
	RemoteConnection*	conn() {
		return	&m_conn;
	}
	WinTS*		ts() {
		return	&m_ts;
	}

	CStrW				host() const {
		return	m_conn.host();
	}
	void				Connect(PCWSTR host, PCWSTR user = NULL, PCWSTR pass = NULL) {
		m_conn.Open(host, user, pass);
	}

	DWORD				id() const {
		return	m_ts.Key();
	}
	CStrW				user() const {
		return	m_ts.Value().user;
	}
};

#endif // WIN_NET_HPP
