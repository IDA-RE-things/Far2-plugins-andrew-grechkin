#include "service.h"

///========================================================================================== WinScm
SC_HANDLE WinScm::Open(ACCESS_MASK acc, RemoteConnection *conn) {
	return CheckHandleErr(::OpenSCManagerW((conn != nullptr) ? conn->host() : nullptr, nullptr, acc));
}

void WinScm::Close(SC_HANDLE &hndl) {
	if (hndl && hndl != INVALID_HANDLE_VALUE) {
		::CloseServiceHandle(hndl);
		hndl = nullptr;
	}
}

void WinScm::Reopen(ACCESS_MASK acc, RemoteConnection *conn) {
	SC_HANDLE hndl = Open(acc, conn);
	using std::swap;
	swap(m_hndl, hndl);
	Close(hndl);
}

void WinScm::Create(PCWSTR name, PCWSTR path, DWORD StartType, PCWSTR disp) {
	SC_HANDLE hndl = CheckHandleErr(::CreateServiceW(
						 m_hndl, name,
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
	::CloseServiceHandle(hndl);
}

///========================================================================================== WinSvc
WinSvc::WinSvc(PCWSTR name, ACCESS_MASK access, RemoteConnection *conn):
	m_hndl(Open(WinScm(SC_MANAGER_CONNECT, conn), name, access)) {
}

WinSvc::WinSvc(PCWSTR name, ACCESS_MASK access, const WinScm &scm):
	m_hndl(Open(scm, name, access)) {
}

void WinSvc::QueryConfig(auto_buf<LPQUERY_SERVICE_CONFIGW> &buf) const {
	DWORD	dwBytesNeeded = 0;
	if (!::QueryServiceConfigW(m_hndl, nullptr, 0, &dwBytesNeeded)) {
		DWORD	err = ::GetLastError();
		CheckApi(err == ERROR_INSUFFICIENT_BUFFER);
		buf.reserve(dwBytesNeeded);
		CheckApi(::QueryServiceConfigW(m_hndl, buf, buf.size(), &dwBytesNeeded));
	}
}

void WinSvc::QueryConfig2(auto_buf<PBYTE> &buf, DWORD level) const {
	DWORD	dwBytesNeeded = 0;
	if (!::QueryServiceConfig2W(m_hndl, level, nullptr, 0, &dwBytesNeeded)) {
		DWORD	err = ::GetLastError();
		CheckApi(err == ERROR_INSUFFICIENT_BUFFER);
		buf.reserve(dwBytesNeeded);
		CheckApi(::QueryServiceConfig2W(m_hndl, level, buf, buf.size(), &dwBytesNeeded));
	}
}

void WinSvc::WaitForState(DWORD state, DWORD dwTimeout) const {
	DWORD	dwStartTime = ::GetTickCount();
	SERVICE_STATUS_PROCESS ssp = {0};
	while (true) {
		GetStatus(ssp);
		if (ssp.dwCurrentState == state)
			break;
		if (::GetTickCount() - dwStartTime > dwTimeout)
			throw WinError(WAIT_TIMEOUT);
		::Sleep(200);
	};
}

bool WinSvc::Start() {
	try {
		CheckApi(::StartService(m_hndl, 0, nullptr));
	} catch (WinError &e) {
		if (e.code() != ERROR_SERVICE_ALREADY_RUNNING)
			throw;
	}
	return	true;
}

bool WinSvc::Stop() {
	SERVICE_STATUS	ss;
	if (!::ControlService(m_hndl, SERVICE_CONTROL_STOP, &ss)) {
		DWORD err = ::GetLastError();
		if (err != ERROR_SERVICE_NOT_ACTIVE)
			CheckApiError(err);
		return	false;
	}
	return	true;
}

void WinSvc::Continue() {
	SERVICE_STATUS	ss;
	CheckApi(::ControlService(m_hndl, SERVICE_CONTROL_CONTINUE, &ss));
}

void WinSvc::Pause() {
	SERVICE_STATUS	ss;
	CheckApi(::ControlService(m_hndl, SERVICE_CONTROL_PAUSE, &ss));
}

void WinSvc::Del() {
	CheckApi(::DeleteService(m_hndl));
	Close(m_hndl);
}

void WinSvc::SetStartup(DWORD type) {
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

void WinSvc::SetLogon(const AutoUTF &user, const AutoUTF &pass, bool desk) {
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

void WinSvc::GetStatus(SERVICE_STATUS_PROCESS &info) const {
	DWORD	dwBytesNeeded;
	CheckApi(::QueryServiceStatusEx(m_hndl, SC_STATUS_PROCESS_INFO, (PBYTE)&info, sizeof(info), &dwBytesNeeded));
}

DWORD WinSvc::GetState() const {
	SERVICE_STATUS_PROCESS	ssp;
	GetStatus(ssp);
	return	ssp.dwCurrentState;
}

DWORD WinSvc::GetType() const {
	auto_buf<LPQUERY_SERVICE_CONFIGW> buf;
	QueryConfig(buf);
	return	buf->dwServiceType;
}

AutoUTF WinSvc::GetUser() const {
	auto_buf<LPQUERY_SERVICE_CONFIGW> buf;
	QueryConfig(buf);
	if (buf->lpServiceStartName)
		return	AutoUTF(buf->lpServiceStartName);
	return	AutoUTF();
}

SC_HANDLE WinSvc::Open(SC_HANDLE scm, PCWSTR name, ACCESS_MASK acc) {
	return CheckHandleErr(::OpenServiceW(scm, name, acc));
}

void WinSvc::Close(SC_HANDLE &hndl) {
	if (hndl && hndl != INVALID_HANDLE_VALUE) {
		::CloseServiceHandle(hndl);
		hndl = nullptr;
	}
}

///====================================================================================== WinService
AutoUTF			WinService::ParseState(DWORD in) {
	switch (in) {
		case SERVICE_CONTINUE_PENDING:
			return	L"The service continue is pending";
		case SERVICE_PAUSE_PENDING:
			return	L"The service pause is pending";
		case SERVICE_PAUSED:
			return	L"The service is paused";
		case SERVICE_RUNNING:
			return	L"The service is running";
		case SERVICE_START_PENDING:
			return	L"The service is starting";
		case SERVICE_STOP_PENDING:
			return	L"The service is stopping";
		case SERVICE_STOPPED:
			return	L"The service is not running";
	}
	return	L"Unknown State";
}

AutoUTF			WinService::ParseState(const AutoUTF &name) {
	DWORD state = GetState(name);
	return	ParseState(state);
}

void			WinService::WaitForState(const AutoUTF &name, DWORD state, DWORD dwTimeout) {
	WinSvc	sch(name.c_str(), SERVICE_QUERY_STATUS);
	sch.WaitForState(state, dwTimeout);
}
//DWORD				WinService::WaitForState(const WinSvcHnd &sch, DWORD state, DWORD dwTimeout) {
//	DWORD	Result = NO_ERROR;
//	DWORD	dwStartTime = ::GetTickCount();
//	DWORD	dwBytesNeeded;
//	SERVICE_STATUS_PROCESS ssp = {0};
//	while (true) {
//		if (::QueryServiceStatusEx(sch, SC_STATUS_PROCESS_INFO, (PBYTE)&ssp,
//								   sizeof(SERVICE_STATUS_PROCESS), &dwBytesNeeded)) {
//			if (ssp.dwCurrentState == state) {
//				Result = NO_ERROR;
//				break;
//			}
//		} else {
//			Result = ::GetLastError();
//			break;
//		}
//		if (::GetTickCount() - dwStartTime > dwTimeout) {
//			Result = WAIT_TIMEOUT;
//			break;
//		}
//		::Sleep(200);
//	};
//	return	(Result);
//}

void	WinService::Del(const AutoUTF &name) {
	Stop(name);
	WinSvc(name.c_str(), SERVICE_STOP | DELETE).Del();
}

void	WinService::Start(const AutoUTF &name) {
	WinSvc(name.c_str(), SERVICE_START | SERVICE_QUERY_STATUS).Start();
}

void	WinService::Stop(const AutoUTF &name) {
	WinSvc(name.c_str(), SERVICE_STOP).Stop();
}

void	WinService::Auto(const AutoUTF &name) {
	WinSvc(name.c_str(), SERVICE_CHANGE_CONFIG).SetStartup(SERVICE_AUTO_START);
}

void	WinService::Manual(const AutoUTF &name) {
	WinSvc(name.c_str(), SERVICE_CHANGE_CONFIG).SetStartup(SERVICE_DEMAND_START);
}

void	WinService::Disable(const AutoUTF &name) {
	WinSvc(name.c_str(), SERVICE_CHANGE_CONFIG).SetStartup(SERVICE_DISABLED);
}

bool	WinService::IsExist(const AutoUTF &name) {
	try {
		WinSvc	sch(name.c_str(), SERVICE_QUERY_STATUS);
	} catch (WinError &e) {
		if (e.code() == ERROR_SERVICE_DOES_NOT_EXIST)
			return	false;
		throw;
	}
	return	true;
}

bool	WinService::IsRunning(const AutoUTF &name) {
	return	GetState(name) == SERVICE_RUNNING;
}

bool	WinService::IsStarting(const AutoUTF &name) {
	return	GetState(name) == SERVICE_START_PENDING;
}

bool	WinService::IsStopped(const AutoUTF &name) {
	return	GetState(name) == SERVICE_STOPPED;
}

bool	WinService::IsStopping(const AutoUTF &name) {
	return	GetState(name) == SERVICE_STOP_PENDING;
}

DWORD	WinService::GetStartType(const AutoUTF &name) {
	auto_buf<LPQUERY_SERVICE_CONFIGW> conf;
	WinSvc(name.c_str(), SERVICE_QUERY_CONFIG).QueryConfig(conf);
	return	conf->dwStartType;
}

bool	WinService::IsAuto(const AutoUTF &name) {
	return	GetStartType(name) == SERVICE_AUTO_START;
}

bool	WinService::IsManual(const AutoUTF &name) {
	return	GetStartType(name) == SERVICE_DEMAND_START;
}

bool	WinService::IsDisabled(const AutoUTF &name) {
	return	GetStartType(name) == SERVICE_DISABLED;
}

void	WinService::GetStatus(const AutoUTF &name, SERVICE_STATUS_PROCESS &ssp) {
	WinSvc(name.c_str(), SERVICE_QUERY_STATUS).GetStatus(ssp);
}

DWORD	WinService::GetState(const AutoUTF &name) {
	SERVICE_STATUS_PROCESS	ssp;
	GetStatus(name, ssp);
	return	ssp.dwCurrentState;
}

AutoUTF	WinService::GetDesc(const AutoUTF &name) {
	auto_buf<PBYTE>	conf;
	WinSvc(name.c_str(), SERVICE_QUERY_CONFIG).QueryConfig2(conf, SERVICE_CONFIG_DESCRIPTION);
	LPSERVICE_DESCRIPTIONW lpsd = (LPSERVICE_DESCRIPTIONW)conf.data();
	if (lpsd->lpDescription)
		return	AutoUTF(lpsd->lpDescription);
	return	AutoUTF();
}

AutoUTF	WinService::GetDName(const AutoUTF &name) {
	auto_buf<LPQUERY_SERVICE_CONFIGW>	conf;
	WinSvc(name.c_str(), SERVICE_QUERY_CONFIG).QueryConfig(conf);
	if (conf->lpDisplayName)
		return	AutoUTF(conf->lpDisplayName);
	return	AutoUTF();
}

AutoUTF	WinService::GetPath(const AutoUTF &name) {
	auto_buf<LPQUERY_SERVICE_CONFIGW> conf;
	WinSvc(name.c_str(), SERVICE_QUERY_CONFIG).QueryConfig(conf);
	if (conf->lpBinaryPathName)
		return	AutoUTF(conf->lpBinaryPathName);
	return	AutoUTF();
}

void	WinService::SetDesc(const AutoUTF &name, const AutoUTF &in) {
	WinSvc	sch(name.c_str(), SERVICE_CHANGE_CONFIG);
	SERVICE_DESCRIPTIONW sd = {0};
	sd.lpDescription = (PWSTR)in.c_str();
	CheckApi(::ChangeServiceConfig2W(
			sch,               		  // handle to service
			SERVICE_CONFIG_DESCRIPTION, // change: description
			&sd));                     // new description
}
void	WinService::SetDName(const AutoUTF &name, const AutoUTF &in) {
	WinSvc	sch(name.c_str(), SERVICE_CHANGE_CONFIG);
	CheckApi(::ChangeServiceConfigW(
				 sch,				// handle of service
				 SERVICE_NO_CHANGE,	// service type: no change
				 SERVICE_NO_CHANGE,	// service start type
				 SERVICE_NO_CHANGE,	// error control: no change
				 nullptr,				// binary path: no change
				 nullptr,				// load order group: no change
				 nullptr,				// tag ID: no change
				 nullptr,				// dependencies: no change
				 nullptr,				// account name: no change
				 nullptr,				// password: no change
				 in.c_str()));		// display name: no change
}
void	WinService::SetPath(const AutoUTF &name, const AutoUTF &in) {
	WinSvc	sch(name.c_str(), SERVICE_CHANGE_CONFIG);
	CheckApi(::ChangeServiceConfigW(
				 sch,				// handle of service
				 SERVICE_NO_CHANGE,	// service type: no change
				 SERVICE_NO_CHANGE,	// service start type
				 SERVICE_NO_CHANGE,	// error control: no change
				 in.c_str(),			// binary path: no change
				 nullptr,				// load order group: no change
				 nullptr,				// tag ID: no change
				 nullptr,				// dependencies: no change
				 nullptr,				// account name: no change
				 nullptr,				// password: no change
				 nullptr));				// display name: no change
}

///===================================================================================== WinServices
bool			WinServices::Cache() {
	try {
		WinScm	scm(SC_MANAGER_CONNECT | SC_MANAGER_ENUMERATE_SERVICE, m_conn);
		DWORD	dwBufNeed = 0, dwNumberOfService = 0;
		::EnumServicesStatusW(scm, m_type, SERVICE_STATE_ALL, nullptr,
							  0, &dwBufNeed, &dwNumberOfService, nullptr);
		CheckApi(::GetLastError() == ERROR_MORE_DATA);

		auto_buf<LPENUM_SERVICE_STATUSW> buf(dwBufNeed);
		CheckApi(::EnumServicesStatusW(scm, m_type, SERVICE_STATE_ALL, buf, buf.size(),
									   &dwBufNeed, &dwNumberOfService, nullptr));
		this->Clear();
		auto_buf<LPQUERY_SERVICE_CONFIGW> buf1;
		auto_buf<PBYTE>	buf2;
		LPENUM_SERVICE_STATUSW pInfo = (LPENUM_SERVICE_STATUSW)buf.data();
		for (ULONG i = 0; i < dwNumberOfService; ++i) {
			s_ServiceInfo	info(pInfo[i].lpServiceName, pInfo[i].ServiceStatus);
			info.dname = pInfo[i].lpDisplayName;
			try {
				WinSvc	svc(pInfo[i].lpServiceName, SERVICE_QUERY_CONFIG, scm);
				svc.QueryConfig(buf1);
				info.path = buf1->lpBinaryPathName;
				info.OrderGroup = buf1->lpLoadOrderGroup;
				info.Dependencies = buf1->lpDependencies;
				info.ServiceStartName = buf1->lpServiceStartName;
				info.StartType = buf1->dwStartType;
				info.ErrorControl = buf1->dwErrorControl;
				info.TagId = buf1->dwTagId;
				svc.QueryConfig2(buf2, SERVICE_CONFIG_DESCRIPTION);
				LPSERVICE_DESCRIPTIONW ff = (LPSERVICE_DESCRIPTIONW)buf2.data();
				if (ff->lpDescription)
					info.descr = ff->lpDescription;
			} catch (WinError &e) {
				//	e.show();
			}
			Insert(pInfo[i].lpServiceName, info);
		}
	} catch (WinError &e) {
	}
	return	true;
}
bool			WinServices::CacheByName(const AutoUTF &in) {
	try {
		WinScm		scm(SC_MANAGER_CONNECT | SC_MANAGER_ENUMERATE_SERVICE);
		DWORD	dwBufNeed = 0, dwNumberOfService = 0;
		::EnumServicesStatusW(scm, m_type, SERVICE_STATE_ALL, nullptr,
							  0, &dwBufNeed, &dwNumberOfService, nullptr);
		CheckApi(::GetLastError() == ERROR_MORE_DATA);

		auto_buf<LPENUM_SERVICE_STATUSW> buf(dwBufNeed);
		CheckApi(::EnumServicesStatusW(scm, m_type, SERVICE_STATE_ALL, buf, buf.size(),
									   &dwBufNeed, &dwNumberOfService, nullptr));
		Clear();
		LPENUM_SERVICE_STATUSW pInfo = (LPENUM_SERVICE_STATUSW)buf.data();
		for (ULONG i = 0; i < dwNumberOfService; ++i) {
			if (in.find(pInfo[i].lpServiceName) == AutoUTF::npos)
				continue;
			s_ServiceInfo	info(pInfo[i].lpServiceName, pInfo[i].ServiceStatus);
			info.dname = pInfo[i].lpDisplayName;
			Insert(pInfo[i].lpServiceName, info);
		}
	} catch (WinError &e) {
	}
	return	true;
}
bool			WinServices::CacheByState(DWORD /*state*/) {
	try {
//		WinScm	scm(SC_MANAGER_CONNECT | SC_MANAGER_ENUMERATE_SERVICE, m_conn);
//		DWORD	dwBufNeed = 0, dwNumberOfService = 0;
//		::EnumServicesStatusExW(scm, SC_ENUM_PROCESS_INFO, SERVICE_WIN32, state,
//								nullptr, 0, &dwBufNeed, &dwNumberOfService, nullptr, nullptr);
//		CheckApi(::GetLastError() == ERROR_MORE_DATA);
//
//		WinBuf<ENUM_SERVICE_STATUS_PROCESSW> buf(dwBufNeed, true);
//
//		CheckApi(::EnumServicesStatusExW(scm, SC_ENUM_PROCESS_INFO, SERVICE_WIN32, state,
//										 (PBYTE)buf.data(), buf.size(), &dwBufNeed, &dwNumberOfService, nullptr, nullptr));
//		LPENUM_SERVICE_STATUS_PROCESSW pInfo = (LPENUM_SERVICE_STATUS_PROCESSW)buf.data();
//		Clear();
//		WinBuf<QUERY_SERVICE_CONFIGW>	buf1;
//		WinBuf<BYTE>	buf2;
//		for (ULONG i = 0; i < dwNumberOfService; ++i) {
//			s_ServiceSmallInfo	info(pInfo[i].lpDisplayName, pInfo[i].ServiceStatusProcess);
//			try {
//				WinSvc	svc(pInfo[i].lpServiceName, SERVICE_QUERY_CONFIG);
//				svc.QueryConfig(buf1);
//				info.StartType = buf1->dwStartType;
//				svc.QueryConfig2(buf2, SERVICE_CONFIG_DESCRIPTION);
//				LPSERVICE_DESCRIPTIONW ff = (LPSERVICE_DESCRIPTIONW)buf2.data();
//				info.descr = ff->lpDescription;
//			} catch (WinError e) {
//				//					e.show();
//			}
//			Insert(pInfo[i].lpServiceName, info);
//		}
	} catch (WinError &e) {
//		farebox(e.code());
	}
	return	true;
}

bool			WinServices::Add(const AutoUTF &name, const AutoUTF &path) {
	WinScm(SC_MANAGER_CREATE_SERVICE).Create(name.c_str(), path.c_str(), SERVICE_DEMAND_START);
	s_ServiceInfo	info;
	Insert(name, info);
	return	true;
}
bool			WinServices::Del() {
	WinService::Del(Key());
	Erase();
	return	true;
}

bool			WinServices::Start() const {
	if (ValidPtr()) {
		WinSvc	svc(Key().c_str(), GENERIC_EXECUTE | SERVICE_QUERY_STATUS);
		return	svc.Start();
	}
	return	false;
}
bool			WinServices::Stop() const {
	if (ValidPtr()) {
		WinSvc	svc(Key().c_str(), GENERIC_EXECUTE | SERVICE_QUERY_STATUS);
		return	svc.Stop();
	}
	return	false;
}
bool			WinServices::Restart() const {
	if (ValidPtr()) {
		WinSvc	svc(Key().c_str(), GENERIC_EXECUTE | SERVICE_QUERY_STATUS);
		svc.Stop();
		svc.WaitForState(SERVICE_STOPPED, 30000);
		svc.Start();
		svc.WaitForState(SERVICE_RUNNING, 30000);
		return	true;
	}
	return	false;
}

bool			WinServices::IsAuto() const {
	return	(ValidPtr()) ? Value().StartType == SERVICE_AUTO_START : false;
}
bool			WinServices::IsRunning() const {
	return	(ValidPtr()) ? WinService::IsRunning(Key()) : false;
}
AutoUTF			WinServices::GetName() const {
	return	(ValidPtr()) ? Key() : L"";
}
AutoUTF			WinServices::GetDName() const {
	return	(ValidPtr()) ? Value().dname : L"";
}
AutoUTF			WinServices::GetPath() const {
	return	(ValidPtr()) ? Value().path : L"";
}

void			InstallService(PCWSTR name, PCWSTR path, DWORD StartType, PCWSTR dispname) {
	WCHAR	fullpath[MAX_PATH_LEN];
	if (!path || Empty(path)) {
		CheckApi(::GetModuleFileNameW(0, fullpath, sizeofa(fullpath)));
	} else {
		Copy(fullpath, path, sizeofa(fullpath));
	}
	WinScm	hSCM(SC_MANAGER_CREATE_SERVICE);
	hSCM.Create(name, fullpath, StartType, dispname);
}
void			UninstallService(PCWSTR name) {
	WinScm	hSCM(SC_MANAGER_CONNECT);
	WinSvc	hSvc(name, SERVICE_STOP | DELETE, hSCM);
	hSvc.Stop();
	hSvc.Del();
}

bool			IsServiceInstalled(PCWSTR name) {
	WinScm	hSCM(SC_MANAGER_CONNECT);
	try {
		WinSvc	hSvc(name, GENERIC_READ, hSCM);
	} catch (...) {
		return	false;
	}
	return	true;
}

///==================================================================================== WinServices1
ServiceInfo::ServiceInfo(const WinScm &scm, const ENUM_SERVICE_STATUSW &st):
	Name(st.lpServiceName),
	DName(st.lpDisplayName),
	Status(st.ServiceStatus) {
	WinSvc svc(Name.c_str(), SERVICE_QUERY_CONFIG, scm);
    auto_buf<LPQUERY_SERVICE_CONFIGW> conf;
	svc.QueryConfig(conf);
	Path = conf->lpBinaryPathName;
	OrderGroup = conf->lpLoadOrderGroup;
	Dependencies = conf->lpDependencies;
	ServiceStartName = conf->lpServiceStartName;
	StartType = conf->dwStartType;
	ErrorControl = conf->dwErrorControl;
	TagId = conf->dwTagId;

	auto_buf<PBYTE>	buf2;
	svc.QueryConfig2(buf2, SERVICE_CONFIG_DESCRIPTION);
	LPSERVICE_DESCRIPTIONW ff = (LPSERVICE_DESCRIPTIONW)buf2.data();
	if (ff->lpDescription)
		Descr = ff->lpDescription;
}

bool ServiceInfo::operator<(const ServiceInfo &rhs) const {
	return Name < rhs.Name;
}

bool ServiceInfo::operator==(const AutoUTF &nm) const {
	return this->Name == nm;
}

bool WinServices1::cache_by_name(const AutoUTF &in) {
//	try {
//		WinScm		scm(SC_MANAGER_CONNECT | SC_MANAGER_ENUMERATE_SERVICE);
//		DWORD	dwBufNeed = 0, dwNumberOfService = 0;
//		::EnumServicesStatusW(scm, m_type, SERVICE_STATE_ALL, nullptr,
//							  0, &dwBufNeed, &dwNumberOfService, nullptr);
//		CheckApi(::GetLastError() == ERROR_MORE_DATA);
//
//		auto_buf<LPENUM_SERVICE_STATUSW> buf(dwBufNeed);
//		CheckApi(::EnumServicesStatusW(scm, m_type, SERVICE_STATE_ALL, buf, buf.size(),
//									   &dwBufNeed, &dwNumberOfService, nullptr));
//		Clear();
//		LPENUM_SERVICE_STATUSW pInfo = (LPENUM_SERVICE_STATUSW)buf.data();
//		for (ULONG i = 0; i < dwNumberOfService; ++i) {
//			if (in.find(pInfo[i].lpServiceName) == AutoUTF::npos)
//				continue;
//			s_ServiceInfo	info(pInfo[i].lpServiceName, pInfo[i].ServiceStatus);
//			info.dname = pInfo[i].lpDisplayName;
//			Insert(pInfo[i].lpServiceName, info);
//		}
//	} catch (WinError &e) {
//	}
	return	true;
}

bool WinServices1::cache_by_state(DWORD /*state*/) {
	try {
//		WinScm	scm(SC_MANAGER_CONNECT | SC_MANAGER_ENUMERATE_SERVICE, m_conn);
//		DWORD	dwBufNeed = 0, dwNumberOfService = 0;
//		::EnumServicesStatusExW(scm, SC_ENUM_PROCESS_INFO, SERVICE_WIN32, state,
//								nullptr, 0, &dwBufNeed, &dwNumberOfService, nullptr, nullptr);
//		CheckApi(::GetLastError() == ERROR_MORE_DATA);
//
//		WinBuf<ENUM_SERVICE_STATUS_PROCESSW> buf(dwBufNeed, true);
//
//		CheckApi(::EnumServicesStatusExW(scm, SC_ENUM_PROCESS_INFO, SERVICE_WIN32, state,
//										 (PBYTE)buf.data(), buf.size(), &dwBufNeed, &dwNumberOfService, nullptr, nullptr));
//		LPENUM_SERVICE_STATUS_PROCESSW pInfo = (LPENUM_SERVICE_STATUS_PROCESSW)buf.data();
//		Clear();
//		WinBuf<QUERY_SERVICE_CONFIGW>	buf1;
//		WinBuf<BYTE>	buf2;
//		for (ULONG i = 0; i < dwNumberOfService; ++i) {
//			s_ServiceSmallInfo	info(pInfo[i].lpDisplayName, pInfo[i].ServiceStatusProcess);
//			try {
//				WinSvc	svc(pInfo[i].lpServiceName, SERVICE_QUERY_CONFIG);
//				svc.QueryConfig(buf1);
//				info.StartType = buf1->dwStartType;
//				svc.QueryConfig2(buf2, SERVICE_CONFIG_DESCRIPTION);
//				LPSERVICE_DESCRIPTIONW ff = (LPSERVICE_DESCRIPTIONW)buf2.data();
//				info.descr = ff->lpDescription;
//			} catch (WinError e) {
//				//					e.show();
//			}
//			Insert(pInfo[i].lpServiceName, info);
//		}
	} catch (WinError &e) {
//		farebox(e.code());
	}
	return	true;
}

bool WinServices1::cache_by_type(DWORD type) {
	WinScm	scm(SC_MANAGER_CONNECT | SC_MANAGER_ENUMERATE_SERVICE, m_conn);
	DWORD	dwBufNeed = 0, dwNumberOfService = 0;
	::EnumServicesStatusW(scm, type, SERVICE_STATE_ALL, nullptr, 0, &dwBufNeed, &dwNumberOfService, nullptr);
	CheckApi(::GetLastError() == ERROR_MORE_DATA);

	auto_buf<LPENUM_SERVICE_STATUSW> enum_svc(dwBufNeed);
	CheckApi(::EnumServicesStatusW(scm, type, SERVICE_STATE_ALL, enum_svc, enum_svc.size(),
	                               &dwBufNeed, &dwNumberOfService, nullptr));
	m_type = type;
	clear();
	for (ULONG i = 0; i < dwNumberOfService; ++i) {
		push_back(ServiceInfo(scm, enum_svc.data()[i]));
	}
	return	true;
}

WinServices1::iterator WinServices1::find(const AutoUTF &name) {
	return std::find(begin(), end(), name);
}

void WinServices1::add(const AutoUTF &name, const AutoUTF &path) {
	WinScm(SC_MANAGER_CREATE_SERVICE).Create(name.c_str(), path.c_str(), SERVICE_DEMAND_START);
	push_back(ServiceInfo(name));
}

void WinServices1::del(const AutoUTF &name) {
	iterator it = find(name);
	if (it != end())
		del(it);
}

void WinServices1::del(iterator it) {
	WinService::Del(it->Name);
	erase(it);
}
