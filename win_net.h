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
struct		s_ServiceSmallInfo {
	SERVICE_STATUS	ssp;
	CStrW		dname;
	CStrW		descr;

	DWORD		StartType;

	s_ServiceSmallInfo() {
		WinMem::Zero(*this);
	}
	s_ServiceSmallInfo(PCWSTR dn, const SERVICE_STATUS &sp):
			ssp(sp), dname(dn), StartType(0) {
	}
};

///===================================================================================== WinServices
class		WinServices : public MapContainer<CStrW, s_ServiceSmallInfo> {
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
		return	Value().ssp.dwCurrentState;
	}

//	bool				Add(CONSTRW &name, CONSTRW &path);
//	bool				Del();

//	bool				Start();
//	bool				Stop();
//	bool				Restart();

//	bool				IsAuto() const;
//	bool				IsRunning() const;

//	wstring				GetName() const;
//	wstring				GetDName() const;
//	wstring				GetPath() const;

//	size_t				size() const {
//		return	m_list.size();
//}
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


#endif // WIN_NET_HPP
