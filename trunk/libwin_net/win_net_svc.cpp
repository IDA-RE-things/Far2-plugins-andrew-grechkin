#include "win_net.h"

///========================================================================================== WinScm
void			WinSvc::QueryConfig(WinBuf<QUERY_SERVICE_CONFIGW> &buf) const {
	DWORD	dwBytesNeeded = 0;
	if (!::QueryServiceConfigW(m_hndl, NULL, 0, &dwBytesNeeded)) {
		DWORD	err = ::GetLastError();
		CheckAPI(err == ERROR_INSUFFICIENT_BUFFER);
		buf.reserve(dwBytesNeeded, true);
		CheckAPI(::QueryServiceConfigW(m_hndl, buf, buf.size(), &dwBytesNeeded));
	}
}
void			WinSvc::QueryConfig2(WinBuf<BYTE> &buf, DWORD level) const {
	DWORD	dwBytesNeeded = 0;
	if (!::QueryServiceConfig2W(m_hndl, level, NULL, 0, &dwBytesNeeded)) {
		DWORD	err = ::GetLastError();
		CheckAPI(err == ERROR_INSUFFICIENT_BUFFER);
		buf.reserve(dwBytesNeeded, true);
		CheckAPI(::QueryServiceConfig2W(m_hndl, level, buf, buf.size(), &dwBytesNeeded));
	}
}

void			WinSvc::WaitForState(DWORD state, DWORD dwTimeout) {
	DWORD	dwStartTime = ::GetTickCount();
	DWORD	dwBytesNeeded;
	SERVICE_STATUS_PROCESS ssp = {0};
	while (true) {
		CheckAPI(::QueryServiceStatusEx(m_hndl, SC_STATUS_PROCESS_INFO, (PBYTE)&ssp, sizeof(ssp), &dwBytesNeeded));
		if (ssp.dwCurrentState == state)
			break;
		if (::GetTickCount() - dwStartTime > dwTimeout)
			throw	ActionError(WAIT_TIMEOUT);
		::Sleep(200);
	};
}

///===================================================================================== WinServices
bool			WinServices::Cache() {
	try {
		WinScm	scm(SC_MANAGER_CONNECT | SC_MANAGER_ENUMERATE_SERVICE, m_conn);
		DWORD	dwBufNeed = 0, dwNumberOfService = 0;
		::EnumServicesStatusW(scm, m_type, SERVICE_STATE_ALL, NULL,
							  0, &dwBufNeed, &dwNumberOfService, NULL);
		CheckAPI(::GetLastError() == ERROR_MORE_DATA);

		WinBuf<ENUM_SERVICE_STATUSW> buf(dwBufNeed, true);
		CheckAPI(::EnumServicesStatusW(scm, m_type, SERVICE_STATE_ALL, buf, buf.size(),
									   &dwBufNeed, &dwNumberOfService, NULL));

		Clear();
		WinBuf<QUERY_SERVICE_CONFIGW>	buf1;
		WinBuf<BYTE>	buf2;
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
				info.ServiceType = buf1->dwServiceType;
				info.StartType = buf1->dwStartType;
				info.ErrorControl = buf1->dwErrorControl;
				info.TagId = buf1->dwTagId;
				svc.QueryConfig2(buf2, SERVICE_CONFIG_DESCRIPTION);
				LPSERVICE_DESCRIPTIONW ff = (LPSERVICE_DESCRIPTIONW)buf2.data();
				info.descr = ff->lpDescription;
			} catch (WinError e) {
				//	e.show();
			}
			Insert(pInfo[i].lpServiceName, info);
		}
	} catch (WinError e) {
//		farebox(e.code());
	}
	return	true;
}
bool			WinServices::CacheByState(DWORD state) {
	try {
		/*
				WinScm	scm(SC_MANAGER_CONNECT | SC_MANAGER_ENUMERATE_SERVICE, m_conn);
				DWORD	dwBufNeed = 0, dwNumberOfService = 0;
				::EnumServicesStatusExW(scm, SC_ENUM_PROCESS_INFO, SERVICE_WIN32, state,
										NULL, 0, &dwBufNeed, &dwNumberOfService, NULL, NULL);
				CheckAPI(::GetLastError() == ERROR_MORE_DATA);

				WinBuf<ENUM_SERVICE_STATUS_PROCESSW> buf(dwBufNeed, true);

				CheckAPI(::EnumServicesStatusExW(scm, SC_ENUM_PROCESS_INFO, SERVICE_WIN32, state,
													(PBYTE)buf.data(), buf.size(), &dwBufNeed, &dwNumberOfService, NULL, NULL));
				LPENUM_SERVICE_STATUS_PROCESSW pInfo = (LPENUM_SERVICE_STATUS_PROCESSW)buf.data();
				Clear();
				WinBuf<QUERY_SERVICE_CONFIGW>	buf1;
				WinBuf<BYTE>	buf2;
				for (ULONG i = 0; i < dwNumberOfService; ++i) {
					s_ServiceSmallInfo	info(pInfo[i].lpDisplayName, pInfo[i].ServiceStatusProcess);
					try {
						WinSvc	svc(pInfo[i].lpServiceName, SERVICE_QUERY_CONFIG);
						svc.QueryConfig(buf1);
						info.StartType = buf1->dwStartType;
						svc.QueryConfig2(buf2, SERVICE_CONFIG_DESCRIPTION);
						LPSERVICE_DESCRIPTIONW ff = (LPSERVICE_DESCRIPTIONW)buf2.data();
						info.descr = ff->lpDescription;
					} catch (WinError e) {
						//					e.show();
					}
					Insert(pInfo[i].lpServiceName, info);
				}
		*/
	} catch (WinError e) {
//		farebox(e.code());
	}
	return	true;
}

void			InstallService(PCWSTR name, PCWSTR path, DWORD StartType, PCWSTR dispname) {
	WCHAR	fullpath[MAX_PATH_LENGTH];
	if (!path || Empty(path)) {
		CheckAPI(::GetModuleFileNameW(0, fullpath, sizeofa(fullpath)));
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
