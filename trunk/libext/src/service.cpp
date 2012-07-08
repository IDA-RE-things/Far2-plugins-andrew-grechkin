#include <libext/service.hpp>
#include <libext/exception.hpp>
#include <libext/rc.hpp>
#include <libbase/bit.hpp>

using namespace Base;

namespace Ext {

	///========================================================================================== WinScm
	WinScm::~WinScm() {
		::CloseServiceHandle(m_hndl);
	}

	WinScm::WinScm(ACCESS_MASK acc, RemoteConnection * conn):
		m_hndl(CheckHandleErr(::OpenSCManagerW((conn) ? conn->get_host() : nullptr, nullptr, acc))) {
	}

	WinSvc WinScm::create_service(PCWSTR name, PCWSTR path, DWORD StartType, PCWSTR disp) {
		SC_HANDLE hndl = CheckHandleErr(::CreateServiceW(m_hndl, name,
		                                                 (disp == nullptr) ? name : disp,
		                                                	 SERVICE_ALL_ACCESS,		// desired access
		                                                	 SERVICE_WIN32_OWN_PROCESS,	// service type
		                                                	 StartType,					// start type
		                                                	 SERVICE_ERROR_NORMAL,		// WinError control type
		                                                	 path,             			// path to service's binary
		                                                	 nullptr,					// no load ordering group
		                                                	 nullptr,					// no tag identifier
		                                                	 nullptr,					// no dependencies
		                                                	 nullptr,					// LocalSystem account
		                                                	 nullptr));					// no password
		return WinSvc(hndl);
	}

	///========================================================================================== WinSvc
	WinSvc::~WinSvc() {
		::CloseServiceHandle(m_hndl);
	}

	WinSvc::WinSvc(PCWSTR name, ACCESS_MASK access, RemoteConnection * conn):
		m_hndl(Open(WinScm(SC_MANAGER_CONNECT, conn), name, access)) {
	}

	WinSvc::WinSvc(PCWSTR name, ACCESS_MASK access, const WinScm & scm):
		m_hndl(Open(scm, name, access)) {
	}

	Base::auto_buf<LPQUERY_SERVICE_CONFIGW> WinSvc::QueryConfig() const {
		DWORD bytesNeeded(0);
		CheckApi(!::QueryServiceConfigW(m_hndl, nullptr, 0, &bytesNeeded) && ::GetLastError() == ERROR_INSUFFICIENT_BUFFER);
		Base::auto_buf<LPQUERY_SERVICE_CONFIGW> buf(bytesNeeded);
		CheckApi(::QueryServiceConfigW(m_hndl, buf, buf.size(), &bytesNeeded));
		return buf;
	}

	Base::auto_buf<PBYTE> WinSvc::QueryConfig2(DWORD level) const {
		DWORD bytesNeeded(0);
		CheckApi(!::QueryServiceConfig2W(m_hndl, level, nullptr, 0, &bytesNeeded) && ::GetLastError() == ERROR_INSUFFICIENT_BUFFER);
		Base::auto_buf<PBYTE> buf(bytesNeeded);
		CheckApi(::QueryServiceConfig2W(m_hndl, level, buf, buf.size(), &bytesNeeded));
		return buf;
	}

	void WinSvc::WaitForState(DWORD state, DWORD dwTimeout) const {
		DWORD dwStartTime = ::GetTickCount();
		SERVICE_STATUS_PROCESS ssp = {0};
		while (true) {
			get_status(ssp);
			if (ssp.dwCurrentState == state)
				break;
			if (::GetTickCount() - dwStartTime > dwTimeout)
				CheckApiError(WAIT_TIMEOUT);
			::Sleep(200);
		};
	}

	//	template<typename Functor>
	//	void WaitForState(DWORD state, DWORD dwTimeout, Functor &func, PVOID param = nullptr) const {
	//		DWORD	dwStartTime = ::GetTickCount();
	//		DWORD	dwBytesNeeded;
	//		SERVICE_STATUS_PROCESS ssp = {0};
	//		while (true) {
	//			CheckApi(::QueryServiceStatusEx(m_hndl, SC_STATUS_PROCESS_INFO, (PBYTE)&ssp, sizeof(ssp), &dwBytesNeeded));
	//			if (ssp.dwCurrentState == state)
	//				break;
	//			if (::GetTickCount() - dwStartTime > dwTimeout)
	//				throw	ApiError(WAIT_TIMEOUT);
	//			func(state, ::GetTickCount() - dwStartTime, param);
	//		}
	//	}

	void WinSvc::Start() {
		CheckApi(::StartServiceW(m_hndl, 0, nullptr) || ::GetLastError() == ERROR_SERVICE_ALREADY_RUNNING);
	}

	void WinSvc::Stop() {
		SERVICE_STATUS ss;
		CheckApi(::ControlService(m_hndl, SERVICE_CONTROL_STOP, &ss) || ::GetLastError() == ERROR_SERVICE_NOT_ACTIVE);
	}

	void WinSvc::Continue() {
		SERVICE_STATUS ss;
		CheckApi(::ControlService(m_hndl, SERVICE_CONTROL_CONTINUE, &ss));
	}

	void WinSvc::Pause() {
		SERVICE_STATUS ss;
		CheckApi(::ControlService(m_hndl, SERVICE_CONTROL_PAUSE, &ss));
	}

	void WinSvc::Del() {
		CheckApi(::DeleteService(m_hndl));
	}

	void WinSvc::set_startup(DWORD type) {
		CheckApi(::ChangeServiceConfigW(
			m_hndl,			// handle of service
			SERVICE_NO_CHANGE,	// service type: no change
			type,				// service start type
			SERVICE_NO_CHANGE,	// error control: no change
			nullptr,			// binary path: no change
			nullptr,			// load order group: no change
			nullptr,			// tag ID: no change
			nullptr,			// dependencies: no change
			nullptr,			// account name: no change
			nullptr,			// password: no change
			nullptr));			// display name: no change
	}

	void WinSvc::set_logon(const ustring &user, const ustring &pass, bool desk) {
		if (user.empty())
			return;
		DWORD type = SERVICE_NO_CHANGE;
		DWORD tmp = get_type();
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
			nullptr,			// binary path: no change
			nullptr,			// load order group: no change
			nullptr,			// tag ID: no change
			nullptr,			// dependencies: no change
			user.c_str(),
			pass.c_str(),
			nullptr));			// display name: no change
	}

	void WinSvc::get_status(SERVICE_STATUS_PROCESS &info) const {
		DWORD bytesNeeded;
		CheckApi(::QueryServiceStatusEx(m_hndl, SC_STATUS_PROCESS_INFO, (PBYTE)&info, sizeof(info), &bytesNeeded));
	}

	DWORD WinSvc::get_state() const {
		SERVICE_STATUS_PROCESS ssp;
		get_status(ssp);
		return ssp.dwCurrentState;
	}

	DWORD WinSvc::get_type() const {
		return QueryConfig()->dwServiceType;
	}

	ustring WinSvc::get_user() const {
		return ustring((QueryConfig()->lpServiceStartName) ?: EMPTY_STR);
	}

	SC_HANDLE WinSvc::Open(SC_HANDLE scm, PCWSTR name, ACCESS_MASK acc) {
		return CheckHandleErr(::OpenServiceW(scm, name, acc));
	}

	void WinSvc::Create(const ustring & name, const ustring & path, DWORD StartType, PCWSTR dispname) {
		WCHAR fullpath[MAX_PATH_LEN];
		if (path.empty()) {
			CheckApi(::GetModuleFileNameW(0, fullpath, sizeofa(fullpath)));
		} else {
			copy_str(fullpath, path.c_str(), sizeofa(fullpath));
		}
		WinScm(SC_MANAGER_CREATE_SERVICE).create_service(name.c_str(), fullpath, StartType, dispname);
	}

	void WinSvc::Del(const ustring & name, RemoteConnection * conn) {
		WinSvc svc(name.c_str(), SERVICE_STOP | DELETE, conn);
		svc.Stop();
		svc.Del();
	}

	void WinSvc::Start(const ustring & name, RemoteConnection * conn) {
		WinSvc(name.c_str(), SERVICE_START | SERVICE_QUERY_STATUS, conn).Start();
	}

	void WinSvc::Stop(const ustring & name, RemoteConnection * conn) {
		WinSvc(name.c_str(), SERVICE_STOP, conn).Stop();
	}

	void WinSvc::Restart(const ustring & name, RemoteConnection * conn) {
		Stop(name, conn);
		Start(name, conn);
	}

	void WinSvc::Continue(const ustring & name, RemoteConnection * conn) {
		WinSvc(name.c_str(), SERVICE_START,conn).Continue();
	}

	void WinSvc::Pause(const ustring & name, RemoteConnection * conn) {
		WinSvc(name.c_str(), SERVICE_STOP, conn).Pause();
	}

	bool WinSvc::is_exist(const ustring &name, RemoteConnection * conn) {
		try {
			WinSvc(name.c_str(), SERVICE_QUERY_STATUS, conn);
		} catch (WinError & e) {
			if (e.code() == ERROR_SERVICE_DOES_NOT_EXIST)
				return false;
			throw;
		}
		return true;
	}

	bool WinSvc::is_running(const ustring & name, RemoteConnection * conn) {
		return get_state(name, conn) == SERVICE_RUNNING;
	}

	bool WinSvc::is_starting(const ustring & name, RemoteConnection * conn) {
		return get_state(name, conn) == SERVICE_START_PENDING;
	}

	bool WinSvc::is_stopped(const ustring & name, RemoteConnection * conn) {
		return get_state(name, conn) == SERVICE_STOPPED;
	}

	bool WinSvc::is_stopping(const ustring & name, RemoteConnection * conn) {
		return get_state(name, conn) == SERVICE_STOP_PENDING;
	}

	bool WinSvc::is_auto(const ustring & name, RemoteConnection * conn) {
		return get_start_type(name, conn) == SERVICE_AUTO_START;
	}

	bool WinSvc::is_manual(const ustring & name, RemoteConnection * conn) {
		return get_start_type(name, conn) == SERVICE_DEMAND_START;
	}

	bool WinSvc::is_disabled(const ustring & name, RemoteConnection * conn) {
		return get_start_type(name, conn) == SERVICE_DISABLED;
	}

	DWORD WinSvc::get_start_type(const ustring & name, RemoteConnection * conn) {
		return WinSvc(name.c_str(), SERVICE_QUERY_CONFIG, conn).QueryConfig()->dwStartType;
	}

	void WinSvc::get_status(const ustring & name, SERVICE_STATUS_PROCESS & ssp, RemoteConnection * conn) {
		WinSvc(name.c_str(), SERVICE_QUERY_STATUS, conn).get_status(ssp);
	}

	DWORD WinSvc::get_state(const ustring & name, RemoteConnection * conn) {
		SERVICE_STATUS_PROCESS ssp;
		get_status(name, ssp, conn);
		return ssp.dwCurrentState;
	}

	ustring WinSvc::get_desc(const ustring & name, RemoteConnection * conn) {
		auto_buf<PBYTE> conf(WinSvc(name.c_str(), SERVICE_QUERY_CONFIG, conn).QueryConfig2(SERVICE_CONFIG_DESCRIPTION));
		LPSERVICE_DESCRIPTIONW lpsd = (LPSERVICE_DESCRIPTIONW)conf.data();
		if (lpsd->lpDescription)
			return ustring(lpsd->lpDescription);
		return ustring();
	}

	ustring WinSvc::get_dname(const ustring & name, RemoteConnection * conn) {
		return ustring(WinSvc(name.c_str(), SERVICE_QUERY_CONFIG, conn).QueryConfig()->lpDisplayName ?: EMPTY_STR);
	}

	ustring WinSvc::get_path(const ustring & name, RemoteConnection * conn) {
		return ustring(WinSvc(name.c_str(), SERVICE_QUERY_CONFIG, conn).QueryConfig()->lpBinaryPathName ?: EMPTY_STR);
	}

	void WinSvc::set_auto(const ustring & name, RemoteConnection * conn) {
		WinSvc(name.c_str(), SERVICE_CHANGE_CONFIG, conn).set_startup(SERVICE_AUTO_START);
	}

	void WinSvc::set_manual(const ustring & name, RemoteConnection * conn) {
		WinSvc(name.c_str(), SERVICE_CHANGE_CONFIG, conn).set_startup(SERVICE_DEMAND_START);
	}

	void WinSvc::set_disable(const ustring & name, RemoteConnection * conn) {
		WinSvc(name.c_str(), SERVICE_CHANGE_CONFIG, conn).set_startup(SERVICE_DISABLED);
	}

	void WinSvc::set_desc(const ustring & name, const ustring & in, RemoteConnection * conn) {
		WinSvc sch(name.c_str(), SERVICE_CHANGE_CONFIG, conn);
		SERVICE_DESCRIPTIONW sd = {0};
		sd.lpDescription = (PWSTR)in.c_str();
		CheckApi(::ChangeServiceConfig2W(
			sch,						// handle to service
			SERVICE_CONFIG_DESCRIPTION,	// change: description
			&sd));						// new description
	}

	void WinSvc::set_dname(const ustring & name, const ustring & in, RemoteConnection * conn) {
		WinSvc sch(name.c_str(), SERVICE_CHANGE_CONFIG, conn);
		CheckApi(::ChangeServiceConfigW(
			sch,				// handle of service
			SERVICE_NO_CHANGE,	// service type: no change
			SERVICE_NO_CHANGE,	// service start type
			SERVICE_NO_CHANGE,	// error control: no change
			nullptr,			// binary path: no change
			nullptr,			// load order group: no change
			nullptr,			// tag ID: no change
			nullptr,			// dependencies: no change
			nullptr,			// account name: no change
			nullptr,			// password: no change
			in.c_str()));		// display name: no change
	}

	void WinSvc::set_path(const ustring & name, const ustring & in, RemoteConnection * conn) {
		WinSvc sch(name.c_str(), SERVICE_CHANGE_CONFIG, conn);
		CheckApi(::ChangeServiceConfigW(
			sch,				// handle of service
			SERVICE_NO_CHANGE,	// service type: no change
			SERVICE_NO_CHANGE,	// service start type
			SERVICE_NO_CHANGE,	// error control: no change
			in.c_str(),		// binary path: no change
			nullptr,			// load order group: no change
			nullptr,			// tag ID: no change
			nullptr,			// dependencies: no change
			nullptr,			// account name: no change
			nullptr,			// password: no change
			nullptr));			// display name: no change
	}

}
