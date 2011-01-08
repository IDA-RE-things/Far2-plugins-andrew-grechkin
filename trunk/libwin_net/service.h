/**
	win_svc

	@classes	()
	@author		© 2009 Andrew Grechkin
	@link		()
**/

#ifndef WIN_SVC_HPP
#define WIN_SVC_HPP

#include "win_net.h"

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
//	WinScm(ACCESS_MASK acc = SC_MANAGER_CONNECT | SC_MANAGER_ENUMERATE_SERVICE, RemoteConnection *conn = nullptr): m_hndl(nullptr), m_mask(acc), m_conn(conn) {
	WinScm(ACCESS_MASK acc, RemoteConnection *conn = nullptr): m_hndl(nullptr), m_mask(acc), m_conn(conn) {
		Open(m_hndl, m_mask, m_conn);
	}

	void					Close() {
		Close(m_hndl);
	}

	void					Reopen(ACCESS_MASK acc = SC_MANAGER_CONNECT | SC_MANAGER_ENUMERATE_SERVICE, RemoteConnection *conn = nullptr) {
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

	void					Create(PCWSTR name, PCWSTR path, DWORD StartType = SERVICE_DEMAND_START, PCWSTR disp = nullptr) {
		SC_HANDLE	hSvc = ::CreateServiceW(
							 m_hndl, name,
							 (disp == nullptr) ? name : disp,
							 SERVICE_ALL_ACCESS,		// desired access
							 SERVICE_WIN32_OWN_PROCESS,	// service type
							 StartType,					// start type
							 SERVICE_ERROR_NORMAL,		// WinError control type
							 path,             			// path to service's binary
							 nullptr,						// no load ordering group
							 nullptr,						// no tag identifier
							 nullptr,						// no dependencies
							 nullptr,						// LocalSystem account
							 nullptr);						// no password
		CheckApi(hSvc != nullptr);
		::CloseServiceHandle(hSvc);
	}

	static void				Open(SC_HANDLE &hSC, ACCESS_MASK acc = SC_MANAGER_CONNECT, RemoteConnection *conn = nullptr) {
		hSC = ::OpenSCManagerW((conn != nullptr) ? conn->host().c_str() : nullptr, nullptr, acc);
		CheckApi(hSC != nullptr);
	}

	static void				Close(SC_HANDLE &in) {
		if (in && in != INVALID_HANDLE_VALUE) {
			::CloseServiceHandle(in);
			in = nullptr;
		}
	}
};

///========================================================================================== WinSvc
class		WinSvc {
	SC_HANDLE			m_hndl;

	void					Close() {
		if (m_hndl) {
			::CloseServiceHandle(m_hndl);
			m_hndl = nullptr;
		}
	}
public:
	~WinSvc() {
		Close();
	}
	WinSvc(PCWSTR name, ACCESS_MASK access, RemoteConnection *conn = nullptr): m_hndl(nullptr) {
		WinScm	scm(SC_MANAGER_CONNECT, conn);
		m_hndl = ::OpenServiceW(scm, name, access);
		CheckApi(m_hndl != nullptr);
	}
	WinSvc(PCWSTR name, ACCESS_MASK access, const WinScm &scm): m_hndl(nullptr) {
		m_hndl = ::OpenServiceW(scm, name, access);
		CheckApi(m_hndl != nullptr);
	}

	void					QueryConfig(WinBuf<QUERY_SERVICE_CONFIGW> &buf) const;
	void					QueryConfig2(WinBuf<BYTE> &buf, DWORD level) const;

	template<typename Functor>
	void					WaitForState(DWORD state, DWORD dwTimeout, Functor &func, PVOID param = nullptr) const {
		DWORD	dwStartTime = ::GetTickCount();
		DWORD	dwBytesNeeded;
		SERVICE_STATUS_PROCESS ssp = {0};
		while (true) {
			CheckApi(::QueryServiceStatusEx(m_hndl, SC_STATUS_PROCESS_INFO, (PBYTE)&ssp, sizeof(ssp), &dwBytesNeeded));
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
			CheckApi(::StartService(m_hndl, 0, nullptr));
		} catch (WinError &e) {
			if (e.code() != ERROR_SERVICE_ALREADY_RUNNING)
				throw;
		}
		return	true;
	}
	bool					Stop() {
		SERVICE_STATUS	ss;
		try {
			CheckApi(::ControlService(m_hndl, SERVICE_CONTROL_STOP, &ss));
		} catch (WinError &e) {
			if (e.code() != ERROR_SERVICE_NOT_ACTIVE)
				throw;
		}
		return	true;
	}
	void					Continue() {
		SERVICE_STATUS	ss;
		CheckApi(::ControlService(m_hndl, SERVICE_CONTROL_CONTINUE, &ss));
	}
	void					Pause() {
		SERVICE_STATUS	ss;
		CheckApi(::ControlService(m_hndl, SERVICE_CONTROL_PAUSE, &ss));
	}

	void					Del() {
		CheckApi(::DeleteService(m_hndl));
		Close();
	}

	void					SetStartup(DWORD type) {
		CheckApi(::ChangeServiceConfigW(
					 m_hndl,			// handle of service
					 SERVICE_NO_CHANGE,	// service type: no change
					 type,				// service start type
					 SERVICE_NO_CHANGE,	// error control: no change
					 nullptr,				// binary path: no change
					 nullptr,				// load order group: no change
					 nullptr,				// tag ID: no change
					 nullptr,				// dependencies: no change
					 nullptr,				// account name: no change
					 nullptr,				// password: no change
					 nullptr));			// display name: no change
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
		CheckApi(::ChangeServiceConfigW(
					 m_hndl,			// handle of service
					 type,				// service type: no change
					 SERVICE_NO_CHANGE,	// service start type
					 SERVICE_NO_CHANGE,	// error control: no change
					 nullptr,				// binary path: no change
					 nullptr,				// load order group: no change
					 nullptr,				// tag ID: no change
					 nullptr,				// dependencies: no change
					 user.c_str(),
					 pass.c_str(),
					 nullptr));			// display name: no change
	}

	void					GetStatus(SERVICE_STATUS_PROCESS &info) const {
		DWORD	dwBytesNeeded;
		CheckApi(::QueryServiceStatusEx(m_hndl, SC_STATUS_PROCESS_INFO, (PBYTE)&info, sizeof(info), &dwBytesNeeded));
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
	WinServices(RemoteConnection *conn = nullptr, bool autocache = true): m_conn(conn) {
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

void		InstallService(PCWSTR name, PCWSTR path, DWORD StartType = SERVICE_DEMAND_START, PCWSTR dispname = nullptr);
void		UninstallService(PCWSTR name);

#endif // WIN_SVC_HPP
