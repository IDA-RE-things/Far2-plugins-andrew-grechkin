#include "win_net.h"

///========================================================================================== WinScm
void			WinSvc::QueryConfig(WinBuf<QUERY_SERVICE_CONFIGW> &buf) const {
	DWORD	dwBytesNeeded = 0;
	if (!::QueryServiceConfigW(m_hndl, NULL, 0, &dwBytesNeeded)) {
		DWORD	err = ::GetLastError();
		CheckAction(err == ERROR_INSUFFICIENT_BUFFER);
		buf.reserve(dwBytesNeeded, true);
		CheckAction(::QueryServiceConfigW(m_hndl, buf, buf.size(), &dwBytesNeeded));
	}
}
void			WinSvc::QueryConfig2(WinBuf<BYTE> &buf, DWORD level) const {
	DWORD	dwBytesNeeded = 0;
	if (!::QueryServiceConfig2W(m_hndl, level, NULL, 0, &dwBytesNeeded)) {
		DWORD	err = ::GetLastError();
		CheckAction(err == ERROR_INSUFFICIENT_BUFFER);
		buf.reserve(dwBytesNeeded, true);
		CheckAction(::QueryServiceConfig2W(m_hndl, level, buf, buf.size(), &dwBytesNeeded));
	}
}

void			WinSvc::WaitForState(DWORD state, DWORD dwTimeout) {
	DWORD	dwStartTime = ::GetTickCount();
	DWORD	dwBytesNeeded;
	SERVICE_STATUS_PROCESS ssp = {0};
	while (true) {
		CheckAction(::QueryServiceStatusEx(m_hndl, SC_STATUS_PROCESS_INFO, (PBYTE)&ssp, sizeof(ssp), &dwBytesNeeded));
		if (ssp.dwCurrentState == state)
			break;
		if (::GetTickCount() - dwStartTime > dwTimeout)
			throw	ActionError(WAIT_TIMEOUT);
		::Sleep(200);
	};
}
/*
	template<typename Functor>
	void					WinSvc::WaitForState(DWORD state, DWORD dwTimeout, Functor &Func, PVOID param) {
		DWORD	dwStartTime = ::GetTickCount();
		DWORD	dwBytesNeeded;
		SERVICE_STATUS_PROCESS ssp = {0};
		while (true) {
			CheckAction(::QueryServiceStatusEx(m_hndl, SC_STATUS_PROCESS_INFO, (PBYTE)&ssp, sizeof(ssp), &dwBytesNeeded));
			if (ssp.dwCurrentState == state)
				break;
			if (::GetTickCount() - dwStartTime > dwTimeout)
				throw	ActionError(WAIT_TIMEOUT);
			Func(state, ::GetTickCount() - dwStartTime, param);
			::Sleep(200);
		}
	}
*/

///===================================================================================== WinServices
bool			WinServices::Cache() {
//	return	CacheByState();
	try {
		WinScm	scm(SC_MANAGER_CONNECT | SC_MANAGER_ENUMERATE_SERVICE, m_conn);
		DWORD	dwBufNeed = 0, dwNumberOfService = 0;
		::EnumServicesStatusW(scm, SERVICE_WIN32, SERVICE_STATE_ALL, NULL,
							  0, &dwBufNeed, &dwNumberOfService, NULL);
		CheckAction(::GetLastError() == ERROR_MORE_DATA);

		WinBuf<ENUM_SERVICE_STATUSW> buf(dwBufNeed, true);
		CheckAction(::EnumServicesStatusW(scm, SERVICE_WIN32, SERVICE_STATE_ALL, buf, buf.size(),
										  &dwBufNeed, &dwNumberOfService, NULL));

		Clear();
		WinBuf<QUERY_SERVICE_CONFIGW>	buf1;
		WinBuf<BYTE>	buf2;
		LPENUM_SERVICE_STATUSW pInfo = (LPENUM_SERVICE_STATUSW)buf.data();
		for (ULONG i = 0; i < dwNumberOfService; ++i) {
			s_ServiceSmallInfo	info(pInfo[i].lpDisplayName, pInfo[i].ServiceStatus);
			try {
				WinSvc	svc(pInfo[i].lpServiceName, SERVICE_QUERY_CONFIG, scm);
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
	} catch (WinError e) {
//		farebox(e.code());
	}
	/*
			if (ChkSucc(::EnumServicesStatusExW(m_scm, SC_ENUM_PROCESS_INFO, SERVICE_WIN32, state,
												pBuf, dwBufSize, &dwBufNeed, &dwNumberOfService, NULL, NULL))) {
				LPENUM_SERVICE_STATUS_PROCESSW pInfo = (LPENUM_SERVICE_STATUS_PROCESSW)pBuf.data();
				Clear();
				WinService	svc(m_scm);
				WinBuf<QUERY_SERVICE_CONFIGW>	buf1;
				WinBuf<BYTE>	buf2;
				for (ULONG i = 0; i < dwNumberOfService; ++i) {
					s_ServiceSmallInfo	info(pInfo[i].lpDisplayName, pInfo[i].ServiceStatusProcess);
					try {
						svc.Open(pInfo[i].lpServiceName, SERVICE_QUERY_CONFIG);
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
			}
		}
	*/
	return	true;
}
bool			WinServices::CacheByState(DWORD state) {
	try {
		/*
				WinScm	scm(SC_MANAGER_CONNECT | SC_MANAGER_ENUMERATE_SERVICE, m_conn);
				DWORD	dwBufNeed = 0, dwNumberOfService = 0;
				::EnumServicesStatusExW(scm, SC_ENUM_PROCESS_INFO, SERVICE_WIN32, state,
										NULL, 0, &dwBufNeed, &dwNumberOfService, NULL, NULL);
				CheckAction(::GetLastError() == ERROR_MORE_DATA);

				WinBuf<ENUM_SERVICE_STATUS_PROCESSW> buf(dwBufNeed, true);

				CheckAction(::EnumServicesStatusExW(scm, SC_ENUM_PROCESS_INFO, SERVICE_WIN32, state,
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
