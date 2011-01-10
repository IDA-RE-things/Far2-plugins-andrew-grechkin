#include "ts.h"

#include <wtsapi32.h>

///===================================================================================== WinTSHandle
WinTSHandle::~WinTSHandle() {
	if (m_ts && m_ts != WTS_CURRENT_SERVER_HANDLE)
		::WTSCloseServer(m_ts);
}

WinTSHandle::WinTSHandle(): m_ts(WTS_CURRENT_SERVER_HANDLE) {
}

WinTSHandle::WinTSHandle(PCWSTR host): m_ts(WTS_CURRENT_SERVER_HANDLE) {
	if (host && !Empty(host)) {
		m_ts = ::WTSOpenServerW((PWSTR)host);
		CheckApi(m_ts != nullptr && m_ts != INVALID_HANDLE_VALUE);
	}
}

///===================================================================================== WinTSession
#define LOGONID_CURRENT ((ULONG)-1)

#ifndef REMOTECONTROL_KBDSHIFT_HOTKEY
const DWORD REMOTECONTROL_KBDSHIFT_HOTKEY = 0x0001;
const DWORD REMOTECONTROL_KBDCTRL_HOTKEY = 0x0002;
const DWORD REMOTECONTROL_KBDALT_HOTKEY = 0x0004;
#endif

void	WinTSession::ConnectLocal(DWORD id, PCWSTR pass) {
	typedef BOOL (WINAPI *pf)(ULONG, ULONG, PCWSTR, BOOL);
	DynamicLibrary dll(L"Wtsapi32");
	pf func = (pf)dll.get_function_nothrow("WTSConnectSessionW");
	if (func) {
		CheckApi(func(id, LOGONID_CURRENT, pass, false));
	}
}

void	WinTSession::ConnectRemote(DWORD id, PCWSTR host) {
	typedef BOOL (WINAPI *pf)(PCWSTR, ULONG, BYTE, USHORT);
	DynamicLibrary dll(L"Wtsapi32");
	pf func = (pf)dll.get_function_nothrow("WTSStartRemoteControlSessionW");
	if (func) {
		CheckApi(func(host, id, VK_PAUSE, REMOTECONTROL_KBDCTRL_HOTKEY | REMOTECONTROL_KBDALT_HOTKEY));
	}
}

void	WinTSession::Disconnect(DWORD id, const WinTSHandle &host) {
	CheckApi(::WTSDisconnectSession(host, id, true));
}

void	WinTSession::LogOff(DWORD id, const WinTSHandle &host) {
	CheckApi(::WTSLogoffSession(host, id, true));
}

DWORD	WinTSession::Question(DWORD id, PCWSTR ttl, PCWSTR msg, DWORD time, const WinTSHandle &host) {
	DWORD	Result = 0;
	CheckApi(::WTSSendMessageW(host, id,
							   (PWSTR)ttl, Len(ttl)*sizeof(WCHAR),
							   (PWSTR)msg, Len(msg)*sizeof(WCHAR),
							   MB_OKCANCEL | MB_ICONQUESTION, time, &Result, true));
	return	Result;
}

DWORD	WinTSession::Message(DWORD id, PCWSTR ttl, PCWSTR msg, DWORD time, bool wait, const WinTSHandle &host) {
	DWORD	Result = 0;
	CheckApi(::WTSSendMessageW(host, id,
							   (PWSTR)ttl, Len(ttl)*sizeof(WCHAR),
							   (PWSTR)msg, Len(msg)*sizeof(WCHAR),
							   MB_OK | MB_ICONASTERISK, time, &Result, wait));
	return	Result;
}

void	WinTSession::Reboot(const WinTSHandle &host) {
	CheckApi(::WTSShutdownSystem(host, WTS_WSD_REBOOT));
}

void	WinTSession::Turnoff(const WinTSHandle &host) {
	CheckApi(::WTSShutdownSystem(host, WTS_WSD_POWEROFF));
}

///======================================================================================= WinTSInfo
PCWSTR	WinTSInfo::ParseState(int st) const {
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

PCWSTR	WinTSInfo::ParseStateFull(int st) const {
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

ustring	WinTSInfo::Info() const {
	ustring	ret(L"Id:           ");
	ret += Num2Str((size_t)m_impl->id);
	ret += L"\n\n";
	ret += L"User name:    ";
	ret += m_impl->user;
	ret += L"\n\n";
	ret += L"State:        ";
	ret += GetState();
	ret += L"\n\n";
	ret += L"Session:      ";
	ret += m_impl->sess;
	ret += L"\n\n";
	ret += L"WinStation:   ";
	ret += m_impl->winSta;
	ret += L"\n\n";
	ret += L"Client:       ";
	ret += m_impl->client;
	ret += L"\n\n";
	return	ret;
}

bool WinTSInfo::is_disconnected() const {
	return m_impl->state == WTSDisconnected;
}

///==================================================================================== WinTSessions
void WinTS::Cache(const WinTSHandle &host) {
	PWTS_SESSION_INFOW	all_info;
	DWORD				cnt = 0;
	CheckApi(::WTSEnumerateSessionsW(host, 0, 1, &all_info, &cnt));
	for (size_t i = 0; i < cnt; ++i) {
		PWSTR	buf = nullptr;
		DWORD	size;
		if (!::WTSQuerySessionInformationW(host, all_info[i].SessionId, WTSUserName, &buf, &size))
			continue;
		if (!buf || Empty(buf))
			continue;
		WinTSInfo info(all_info[i].SessionId, all_info[i].pWinStationName, buf, all_info[i].State);
		::WTSFreeMemory(buf);

		if (::WTSQuerySessionInformationW(host, all_info[i].SessionId, WTSWinStationName, &buf, &size) && buf) {
			info.winSta(buf);
			::WTSFreeMemory(buf);
		}
		if (::WTSQuerySessionInformationW(host, all_info[i].SessionId, WTSClientName, &buf, &size) && buf) {
			info.client(buf);
			::WTSFreeMemory(buf);
		}
		m_c.push_back(info);
	}
	::WTSFreeMemory(all_info);
	std::sort(begin(), end());
}

bool	WinTS::FindSess(PCWSTR /*in*/) const {
	return	false;
}

bool	WinTS::FindUser(PCWSTR /*in*/) const {
	return	false;
}
