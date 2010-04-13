#include "win_net.h"

///===================================================================================== WinTSHandle
WinTSHandle::~WinTSHandle() {
	if (m_ts && m_ts != WTS_CURRENT_SERVER_HANDLE)
		::WTSCloseServer(m_ts);
}
WinTSHandle::WinTSHandle(PCWSTR host): m_ts(WTS_CURRENT_SERVER_HANDLE) {
	if (host && !Empty(host)) {
		m_ts = ::WTSOpenServerW((PWSTR)host);
		CheckAPI(m_ts != NULL);
	}
}
WinTSHandle::WinTSHandle(RemoteConnection* conn): m_ts(WTS_CURRENT_SERVER_HANDLE) {
	if (conn && !conn->host().empty()) {
		m_ts = ::WTSOpenServerW(conn->host().buffer());
		CheckAPI(m_ts != NULL);
	}
}

///===================================================================================== WinTSession
void				WinTSession::Disconnect(DWORD id, PCWSTR host) {
	WinTSHandle	srv(host);
	CheckAPI(::WTSDisconnectSession(srv, id, false));
}
void				WinTSession::LogOff(DWORD id, PCWSTR host) {
	WinTSHandle	srv(host);
	CheckAPI(::WTSLogoffSession(srv, id, false));
}
DWORD				WinTSession::Question(DWORD id, PCWSTR ttl, PCWSTR msg, DWORD time, PCWSTR host) {
	DWORD	Result = 0;
	WinTSHandle	srv(host);
	CheckAPI(::WTSSendMessageW(srv, id,
							   (PWSTR)ttl, Len(ttl)*sizeof(WCHAR),
							   (PWSTR)msg, Len(msg)*sizeof(WCHAR),
							   MB_OKCANCEL | MB_ICONQUESTION, time, &Result, true));
	return	Result;
}
DWORD				WinTSession::Message(DWORD id, PCWSTR ttl, PCWSTR msg, DWORD time, bool wait, PCWSTR host) {
	WinTSHandle	srv(host);
	DWORD	Result = 0;
	CheckAPI(::WTSSendMessageW(srv, id,
							   (PWSTR)ttl, Len(ttl)*sizeof(WCHAR),
							   (PWSTR)msg, Len(msg)*sizeof(WCHAR),
							   MB_OK | MB_ICONASTERISK, time, &Result, wait));
	return	Result;
}

void				WinTSession::Disconnect(DWORD id, RemoteConnection *host) {
	WinTSHandle	srv(host);
	CheckAPI(::WTSDisconnectSession(srv, id, false));
}
void				WinTSession::LogOff(DWORD id, RemoteConnection *host) {
	WinTSHandle	srv(host);
	CheckAPI(::WTSLogoffSession(srv, id, false));
}
DWORD				WinTSession::Question(DWORD id, PCWSTR ttl, PCWSTR msg, DWORD time, RemoteConnection *host) {
	DWORD	Result = 0;
	WinTSHandle	srv(host);
	CheckAPI(::WTSSendMessageW(srv, id,
							   (PWSTR)ttl, Len(ttl)*sizeof(WCHAR),
							   (PWSTR)msg, Len(msg)*sizeof(WCHAR),
							   MB_OKCANCEL | MB_ICONQUESTION, time, &Result, true));
	return	Result;
}
DWORD				WinTSession::Message(DWORD id, PCWSTR ttl, PCWSTR msg, DWORD time, bool wait, RemoteConnection *host) {
	WinTSHandle	srv(host);
	DWORD	Result = 0;
	CheckAPI(::WTSSendMessageW(srv, id,
							   (PWSTR)ttl, Len(ttl)*sizeof(WCHAR),
							   (PWSTR)msg, Len(msg)*sizeof(WCHAR),
							   MB_OK | MB_ICONASTERISK, time, &Result, wait));
	return	Result;
}

///==================================================================================== WinTSessions
bool				WinTS::Cache() {
	WinTSHandle	srv(m_conn);
	PWTS_SESSION_INFOW	all_info;
	DWORD				cnt = 0;
	if (::WTSEnumerateSessionsW(srv, 0, 1, &all_info, &cnt)) {
		Clear();
		for (DWORD i = 0; i < cnt; ++i) {
			PWSTR	buf = NULL;
			DWORD	size;
			if (!::WTSQuerySessionInformationW(srv, all_info[i].SessionId, WTSUserName, &buf, &size))
				continue;
			if (!buf || ::Empty(buf))
				continue;
			WinTSInfo info(all_info[i].SessionId, all_info[i].pWinStationName, buf, all_info[i].State);
			::WTSFreeMemory(buf);
			if (::WTSQuerySessionInformationW(srv, all_info[i].SessionId, WTSWinStationName, &buf, &size)) {
				info.winSta = buf;
				::WTSFreeMemory(buf);
			}
			if (::WTSQuerySessionInformationW(srv, all_info[i].SessionId, WTSClientName, &buf, &size)) {
				info.client = buf;
				::WTSFreeMemory(buf);
			}
			Insert(all_info[i].SessionId, info);
		}
		::WTSFreeMemory(all_info);
	}
	return	true;
}

bool				WinTS::FindSess(PCWSTR in) const {
	return	false;
}
bool				WinTS::FindUser(PCWSTR in) const {
	return	false;
}
