#include "service.h"
#include "exception.h"

#include <libwin_def/console.h>

///===================================================================================== WinServices
ServiceInfo::ServiceInfo(const WinScm & scm, const ENUM_SERVICE_STATUSW & st):
	Name(st.lpServiceName),
	DName(st.lpDisplayName),
	Status(st.ServiceStatus) {

	try {
		WinSvc svc(Name.c_str(), SERVICE_QUERY_CONFIG, scm);
		auto_buf<LPQUERY_SERVICE_CONFIGW> conf(svc.QueryConfig());
		Path = conf->lpBinaryPathName;
		OrderGroup = conf->lpLoadOrderGroup;
		Dependencies = conf->lpDependencies;
		ServiceStartName = conf->lpServiceStartName;
		StartType = conf->dwStartType;
		ErrorControl = conf->dwErrorControl;
		TagId = conf->dwTagId;

		auto_buf<PBYTE> buf2(svc.QueryConfig2(SERVICE_CONFIG_DESCRIPTION));
		LPSERVICE_DESCRIPTIONW ff = (LPSERVICE_DESCRIPTIONW)buf2.data();
		if (ff->lpDescription)
			Descr = ff->lpDescription;
	} catch (...) {
		// skip query info
	}
}

bool ServiceInfo::operator < (const ServiceInfo & rhs) const {
	return Name < rhs.Name;
}

bool ServiceInfo::operator == (const ustring & nm) const {
	return this->Name == nm;
}

WinServices::WinServices(RemoteConnection * conn, bool autocache):
	m_conn(conn),
	m_type(type_svc) {
	if (autocache)
		cache();
}

bool WinServices::cache_by_name(const ustring & in) {
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
//			if (in.find(pInfo[i].lpServiceName) == ustring::npos)
//				continue;
//			s_ServiceInfo	info(pInfo[i].lpServiceName, pInfo[i].ServiceStatus);
//			info.dname = pInfo[i].lpDisplayName;
//			Insert(pInfo[i].lpServiceName, info);
//		}
//	} catch (WinError &e) {
//	}
	return true;
}

bool WinServices::cache_by_state(DWORD /*state*/) {
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
	} catch (WinError & e) {
//		farebox(e.code());
	}
	return true;
}

bool WinServices::cache_by_type(DWORD type) {
	WinScm scm(SC_MANAGER_CONNECT | SC_MANAGER_ENUMERATE_SERVICE, m_conn);
	DWORD dwBufNeed = 0, dwNumberOfService = 0;
	::EnumServicesStatusW(scm, type, SERVICE_STATE_ALL, nullptr, 0, &dwBufNeed, &dwNumberOfService, nullptr);
	CheckApi(::GetLastError() == ERROR_MORE_DATA);

	auto_buf<LPENUM_SERVICE_STATUSW> enum_svc(dwBufNeed);
	CheckApi(::EnumServicesStatusW(scm, type, SERVICE_STATE_ALL, enum_svc, enum_svc.size(),
	                               &dwBufNeed, &dwNumberOfService, nullptr));
	clear();
	for (ULONG i = 0; i < dwNumberOfService; ++i) {
		push_back(ServiceInfo(scm, enum_svc.data()[i]));
	}
	m_type = type;
	return true;
}

WinServices::iterator WinServices::find(const ustring & name) {
	return std::find(begin(), end(), name);
}

WinServices::const_iterator WinServices::find(const ustring & name) const {
	return std::find(begin(), end(), name);
}

void WinServices::add(const ustring &name, const ustring &path) {
	try {
		WinScm(SC_MANAGER_CREATE_SERVICE).create_service(name.c_str(), path.c_str(), SERVICE_DEMAND_START);
		push_back(ServiceInfo(name));
	} catch (WinError &e) {
		Rethrow(e, L"Unable to create service");
	}
}

void WinServices::del(const ustring &name) {
	iterator it = find(name);
	if (it != end())
		del(it);
}

void WinServices::del(iterator it) {
	try {
		WinSvc::Del(it->Name);
		erase(it);
	} catch (WinError &e) {
		Rethrow(e, L"Unable to delete service");
	}
}

///====================================================================================== WinService
//ustring			WinService::ParseState(DWORD in) {
//	switch (in) {
//		case SERVICE_CONTINUE_PENDING:
//			return L"The service continue is pending";
//		case SERVICE_PAUSE_PENDING:
//			return L"The service pause is pending";
//		case SERVICE_PAUSED:
//			return L"The service is paused";
//		case SERVICE_RUNNING:
//			return L"The service is running";
//		case SERVICE_START_PENDING:
//			return L"The service is starting";
//		case SERVICE_STOP_PENDING:
//			return L"The service is stopping";
//		case SERVICE_STOPPED:
//			return L"The service is not running";
//	}
//	return L"Unknown State";
//}
//
//ustring			WinService::ParseState(const ustring &name) {
//	DWORD state = get_state(name);
//	return ParseState(state);
//}

//void			WinService::WaitForState(const ustring &name, DWORD state, DWORD dwTimeout) {
//	WinSvc	sch(name.c_str(), SERVICE_QUERY_STATUS);
//	sch.WaitForState(state, dwTimeout);
//}

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
//	return (Result);
//}
