#include "ts.h"

///===================================================================================== WinTSHandle
WinTSHandle::~WinTSHandle() {
	if (m_ts && m_ts != WTS_CURRENT_SERVER_HANDLE)
		::WTSCloseServer(m_ts);
}

WinTSHandle::WinTSHandle(PCWSTR host): m_ts(WTS_CURRENT_SERVER_HANDLE) {
	if (host && !Empty(host)) {
		m_ts = ::WTSOpenServerW((PWSTR)host);
		CheckApi(m_ts != nullptr);
	}
}

WinTSHandle::WinTSHandle(shared_ptr<RemoteConnection> conn): m_ts(WTS_CURRENT_SERVER_HANDLE) {
	if (!conn->host().empty()) {
		m_ts = ::WTSOpenServerW((PWSTR)conn->host().c_str());
		CheckApi(m_ts != nullptr && m_ts != INVALID_HANDLE_VALUE);
	}
}

///===================================================================================== WinTSession
//#ifndef LOGONID_CURRENT
//#define LOGONID_CURRENT     ((ULONG)-1)
//#endif

#ifndef LOGONID_CURRENT
const ULONG LOGONID_CURRENT = -1;
#endif

#ifndef REMOTECONTROL_KBDSHIFT_HOTKEY
const DWORD REMOTECONTROL_KBDSHIFT_HOTKEY = 0x0001;
const DWORD REMOTECONTROL_KBDCTRL_HOTKEY = 0x0002;
const DWORD REMOTECONTROL_KBDALT_HOTKEY = 0x0004;
#endif

void	WinTSession::ConnectLocal(DWORD id, PCWSTR pass) {
	typedef BOOL (WINAPI *FWTSConnectSession)(ULONG, ULONG, PCWSTR, BOOL);
	bool ret = true;
	HINSTANCE lib = ::LoadLibraryW(L"Wtsapi32");
	if (lib != nullptr) {
		FWTSConnectSession func = (FWTSConnectSession)GetProcAddress(lib, "WTSConnectSessionW");
		if (func != nullptr) {
			ret = func(id, LOGONID_CURRENT, pass, false);
		}
		::FreeLibrary(lib);
		CheckApi(ret);
	}
}

void	WinTSession::ConnectRemote(DWORD id, shared_ptr<RemoteConnection> host) {
	typedef BOOL (WINAPI *pf)(PCWSTR, ULONG, BYTE, USHORT);
	bool ret = true;
	HINSTANCE lib = ::LoadLibraryW(L"Wtsapi32");
	if (lib != nullptr) {
		pf func = (pf)::GetProcAddress(lib, "WTSStartRemoteControlSessionW");
#define REMOTECONTROL_KBDSHIFT_HOTKEY   0x0001
#define REMOTECONTROL_KBDCTRL_HOTKEY    0x0002
#define REMOTECONTROL_KBDALT_HOTKEY     0x0004
		if (func != nullptr) {
			ret = func(host->host().c_str(), id, VK_PAUSE, REMOTECONTROL_KBDCTRL_HOTKEY | REMOTECONTROL_KBDALT_HOTKEY);
		}
		::FreeLibrary(lib);
		CheckApi(ret);
	}
}

void	WinTSession::Disconnect(DWORD id, PCWSTR host) {
	WinTSHandle	srv(host);
	CheckApi(::WTSDisconnectSession(srv, id, true));
}

void	WinTSession::Disconnect(DWORD id, shared_ptr<RemoteConnection> host) {
	WinTSHandle	srv(host);
	CheckApi(::WTSDisconnectSession(srv, id, true));
}

void	WinTSession::LogOff(DWORD id, PCWSTR host) {
	WinTSHandle	srv(host);
	CheckApi(::WTSLogoffSession(srv, id, true));
}

void	WinTSession::LogOff(DWORD id, shared_ptr<RemoteConnection> host) {
	WinTSHandle	srv(host);
	CheckApi(::WTSLogoffSession(srv, id, true));
}

DWORD	WinTSession::Question(DWORD id, PCWSTR ttl, PCWSTR msg, DWORD time, PCWSTR host) {
	DWORD	Result = 0;
	WinTSHandle	srv(host);
	CheckApi(::WTSSendMessageW(srv, id,
							   (PWSTR)ttl, Len(ttl)*sizeof(WCHAR),
							   (PWSTR)msg, Len(msg)*sizeof(WCHAR),
							   MB_OKCANCEL | MB_ICONQUESTION, time, &Result, true));
	return	Result;
}

DWORD	WinTSession::Question(DWORD id, shared_ptr<RemoteConnection> host, PCWSTR ttl, PCWSTR msg, DWORD time) {
	DWORD	Result = 0;
	WinTSHandle	srv(host);
	CheckApi(::WTSSendMessageW(srv, id,
							   (PWSTR)ttl, Len(ttl)*sizeof(WCHAR),
							   (PWSTR)msg, Len(msg)*sizeof(WCHAR),
							   MB_OKCANCEL | MB_ICONQUESTION, time, &Result, true));
	return	Result;
}

DWORD	WinTSession::Message(DWORD id, PCWSTR ttl, PCWSTR msg, DWORD time, bool wait, PCWSTR host) {
	WinTSHandle	srv(host);
	DWORD	Result = 0;
	CheckApi(::WTSSendMessageW(srv, id,
							   (PWSTR)ttl, Len(ttl)*sizeof(WCHAR),
							   (PWSTR)msg, Len(msg)*sizeof(WCHAR),
							   MB_OK | MB_ICONASTERISK, time, &Result, wait));
	return	Result;
}

DWORD	WinTSession::Message(DWORD id, shared_ptr<RemoteConnection> host, PCWSTR ttl, PCWSTR msg, DWORD time, bool wait) {
	WinTSHandle	srv(host);
	DWORD	Result = 0;
	CheckApi(::WTSSendMessageW(srv, id,
							   (PWSTR)ttl, Len(ttl)*sizeof(WCHAR),
							   (PWSTR)msg, Len(msg)*sizeof(WCHAR),
							   MB_OK | MB_ICONASTERISK, time, &Result, wait));
	return	Result;
}

void	WinTSession::Reboot(PCWSTR host) {
	WinTSHandle	srv(host);
	CheckApi(::WTSShutdownSystem(srv, WTS_WSD_REBOOT));
}

void	WinTSession::Reboot(shared_ptr<RemoteConnection> host) {
	WinTSHandle	srv(host);
	CheckApi(::WTSShutdownSystem(srv, WTS_WSD_REBOOT));
}

void	WinTSession::Turnoff(PCWSTR host) {
	WinTSHandle	srv(host);
	CheckApi(::WTSShutdownSystem(srv, WTS_WSD_POWEROFF));
}

void	WinTSession::Turnoff(shared_ptr<RemoteConnection> host) {
	WinTSHandle	srv(host);
	CheckApi(::WTSShutdownSystem(srv, WTS_WSD_POWEROFF));
}

///======================================================================================= WinTSInfo
	PCWSTR			WinTSInfo::ParseState(WTS_CONNECTSTATE_CLASS st) const {
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

	PCWSTR			WinTSInfo::ParseStateFull(WTS_CONNECTSTATE_CLASS st) const {
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

///==================================================================================== WinTSessions
void	WinTS::Cache(shared_ptr<RemoteConnection> conn) {
	WinTSHandle	srv(conn);
	PWTS_SESSION_INFOW	all_info;
	DWORD				cnt = 0;
	CheckApi(::WTSEnumerateSessionsW(srv, 0, 1, &all_info, &cnt));
	Clear();
	for (DWORD i = 0; i < cnt; ++i) {
		PWSTR	buf = nullptr;
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

bool	WinTS::FindSess(PCWSTR /*in*/) const {
	return	false;
}

bool	WinTS::FindUser(PCWSTR /*in*/) const {
	return	false;
}
