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
WinTSInfo::WinTSInfo(DWORD i, const AutoUTF &s, const AutoUTF &u, int st):
	m_id(i),
	m_sess(s),
	m_user(u),
	m_state(st) {
}

WinTSInfo::WinTSInfo(const WinTSHandle &host, DWORD id, const AutoUTF &ws, int st):
	m_id(id),
	m_winSta(ws),
	m_state(st) {
	PWSTR	buf = nullptr;
	DWORD	size = 0;
	if (::WTSQuerySessionInformationW(host, m_id, WTSUserName, &buf, &size) && buf) {
		m_user = buf;
		::WTSFreeMemory(buf);
	}
//	if (::WTSQuerySessionInformationW(host, m_id, WTSWinStationName, &buf, &size) && buf) {
//		m_winSta = buf;
//		::WTSFreeMemory(buf);
//	}
	if (::WTSQuerySessionInformationW(host, m_id, WTSClientName, &buf, &size) && buf) {
		m_client = buf;
		::WTSFreeMemory(buf);
	}

}

bool WinTSInfo::is_disconnected() const {
	return m_state == WTSDisconnected;
}

///==================================================================================== WinTSessions
void WinTS::Cache(const WinTSHandle &host) {
	PWTS_SESSION_INFOW	all_info;
	DWORD				cnt = 0;
	CheckApi(::WTSEnumerateSessionsW(host, 0, 1, &all_info, &cnt));
	clear();
	for (size_t i = 0; i < cnt; ++i) {
		WinTSInfo info(host, all_info[i].SessionId, all_info[i].pWinStationName, all_info[i].State);
		if (!info.user().empty())
			push_back(info);
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
