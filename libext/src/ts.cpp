#include <libbase/std.hpp>
#include <libbase/str.hpp>
#include <libext/dll.hpp>
#include <libext/ts.hpp>
#include <libext/exception.hpp>

#include <wtsapi32.h>

using namespace Base;

namespace Ext {

	namespace {
		///================================================================================ Wtsapi32_dll
		struct Wtsapi32_dll: private DynamicLibrary {
			typedef VOID (WINAPI *FWTSCloseServer)(HANDLE hServer);
			typedef VOID (WINAPI *FWTSFreeMemory)(PVOID pMemory);
			typedef HANDLE (WINAPI *FWTSOpenServerW)(LPWSTR pServerName);
			typedef WINBOOL (WINAPI *FWTSDisconnectSession)(HANDLE hServer,DWORD SessionId,WINBOOL bWait);
			typedef WINBOOL (WINAPI *FWTSEnumerateSessionsW)(HANDLE hServer,DWORD Reserved,DWORD Version,PWTS_SESSION_INFOW *ppSessionInfo,DWORD *pCount);
			typedef WINBOOL (WINAPI *FWTSLogoffSession)(HANDLE hServer,DWORD SessionId,WINBOOL bWait);
			typedef WINBOOL (WINAPI *FWTSQuerySessionInformationW)(HANDLE hServer,DWORD SessionId,WTS_INFO_CLASS WTSInfoClass,LPWSTR *ppBuffer,DWORD *pBytesReturned);
			typedef WINBOOL (WINAPI *FWTSSendMessageW)(HANDLE hServer,DWORD SessionId,LPWSTR pTitle,DWORD TitleLength,LPWSTR pMessage,DWORD MessageLength,DWORD Style,DWORD Timeout,DWORD *pResponse,WINBOOL bWait);
			typedef WINBOOL (WINAPI *FWTSShutdownSystem)(HANDLE hServer,DWORD ShutdownFlag);
			typedef BOOL (WINAPI *FWTSConnectSessionW)(ULONG, ULONG, PCWSTR, BOOL);
			typedef BOOL (WINAPI *FWTSStartRemoteControlSessionW)(PCWSTR, ULONG, BYTE, USHORT);

			DEFINE_FUNC(WTSCloseServer);
			DEFINE_FUNC(WTSConnectSessionW);
			DEFINE_FUNC(WTSDisconnectSession);
			DEFINE_FUNC(WTSEnumerateSessionsW);
			DEFINE_FUNC(WTSFreeMemory);
			DEFINE_FUNC(WTSLogoffSession);
			DEFINE_FUNC(WTSOpenServerW);
			DEFINE_FUNC(WTSQuerySessionInformationW);
			DEFINE_FUNC(WTSSendMessageW);
			DEFINE_FUNC(WTSShutdownSystem);
			DEFINE_FUNC(WTSStartRemoteControlSessionW);

			static Wtsapi32_dll & inst() {
				static Wtsapi32_dll ret;
				return ret;
			}

		private:
			Wtsapi32_dll():
				DynamicLibrary(L"Wtsapi32.dll") {
				GET_DLL_FUNC(WTSCloseServer);
				GET_DLL_FUNC(WTSDisconnectSession);
				GET_DLL_FUNC(WTSEnumerateSessionsW);
				GET_DLL_FUNC(WTSFreeMemory);
				GET_DLL_FUNC(WTSLogoffSession);
				GET_DLL_FUNC(WTSOpenServerW);
				GET_DLL_FUNC(WTSQuerySessionInformationW);
				GET_DLL_FUNC(WTSSendMessageW);
				GET_DLL_FUNC(WTSShutdownSystem);
				GET_DLL_FUNC_NT(WTSConnectSessionW);
				GET_DLL_FUNC_NT(WTSStartRemoteControlSessionW);
			}
		};
	}

	///===================================================================================== WinTSHandle
	WinTSHandle::~WinTSHandle() {
		if (m_ts && m_ts != WTS_CURRENT_SERVER_HANDLE)
			Wtsapi32_dll::inst().WTSCloseServer(m_ts);
	}

	WinTSHandle::WinTSHandle(): m_ts(WTS_CURRENT_SERVER_HANDLE) {
	}

	WinTSHandle::WinTSHandle(PCWSTR host): m_ts(WTS_CURRENT_SERVER_HANDLE) {
		if (host && !Base::is_str_empty(host)) {
			m_ts = Wtsapi32_dll::inst().WTSOpenServerW((PWSTR)host);
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

	void WinTSession::ConnectLocal(DWORD id, PCWSTR pass) {
		if (Wtsapi32_dll::inst().WTSConnectSessionW)
			CheckApi(Wtsapi32_dll::inst().WTSConnectSessionW(id, LOGONID_CURRENT, pass, false));
	}

	void WinTSession::ConnectRemote(DWORD id, PCWSTR host) {
		if (Wtsapi32_dll::inst().WTSStartRemoteControlSessionW) {
			CheckApi(Wtsapi32_dll::inst().WTSStartRemoteControlSessionW(host, id, VK_PAUSE, REMOTECONTROL_KBDCTRL_HOTKEY | REMOTECONTROL_KBDALT_HOTKEY));
		}
	}

	void WinTSession::Disconnect(DWORD id, const WinTSHandle &host) {
		CheckApi(Wtsapi32_dll::inst().WTSDisconnectSession(host, id, true));
	}

	void WinTSession::LogOff(DWORD id, const WinTSHandle &host) {
		CheckApi(Wtsapi32_dll::inst().WTSLogoffSession(host, id, true));
	}

	DWORD WinTSession::Question(DWORD id, PCWSTR ttl, PCWSTR msg, DWORD time, const WinTSHandle &host) {
		DWORD Result = 0;
		CheckApi(Wtsapi32_dll::inst().WTSSendMessageW(host, id,
		                                              (PWSTR)ttl, get_str_len(ttl)*sizeof(WCHAR),
		                                              (PWSTR)msg, get_str_len(msg)*sizeof(WCHAR),
		                                              MB_OKCANCEL | MB_ICONQUESTION, time, &Result, true));
		return Result;
	}

	DWORD WinTSession::Message(DWORD id, PCWSTR ttl, PCWSTR msg, DWORD time, bool wait, const WinTSHandle &host) {
		DWORD	Result = 0;
		CheckApi(Wtsapi32_dll::inst().WTSSendMessageW(host, id,
		                                              (PWSTR)ttl, get_str_len(ttl)*sizeof(WCHAR),
		                                              (PWSTR)msg, get_str_len(msg)*sizeof(WCHAR),
		                                              MB_OK | MB_ICONASTERISK, time, &Result, wait));
		return Result;
	}

	void WinTSession::Reboot(const WinTSHandle &host) {
		CheckApi(Wtsapi32_dll::inst().WTSShutdownSystem(host, WTS_WSD_REBOOT));
	}

	void WinTSession::Turnoff(const WinTSHandle &host) {
		CheckApi(Wtsapi32_dll::inst().WTSShutdownSystem(host, WTS_WSD_POWEROFF));
	}

	///======================================================================================= WinTSInfo
	WinTSInfo::WinTSInfo(DWORD i, const ustring & s, const ustring & u, int st):
		m_id(i),
		m_sess(s),
		m_user(u),
		m_state(st) {
	}

	WinTSInfo::WinTSInfo(const WinTSHandle & host, DWORD id, const ustring & ws, int st):
		m_id(id),
		m_winSta(ws),
		m_state(st) {
		PWSTR buf = nullptr;
		DWORD size = 0;
		if (Wtsapi32_dll::inst().WTSQuerySessionInformationW(host, m_id, WTSUserName, &buf, &size) && buf) {
			m_user = buf;
			Wtsapi32_dll::inst().WTSFreeMemory(buf);
		}
		//	if (Wtsapi32_dll::inst().WTSQuerySessionInformationW(host, m_id, WTSWinStationName, &buf, &size) && buf) {
		//		m_winSta = buf;
		//		Wtsapi32_dll::inst().WTSFreeMemory(buf);
		//	}
		if (Wtsapi32_dll::inst().WTSQuerySessionInformationW(host, m_id, WTSClientName, &buf, &size) && buf) {
			m_client = buf;
			Wtsapi32_dll::inst().WTSFreeMemory(buf);
		}

	}

	bool WinTSInfo::is_disconnected() const {
		return m_state == WTSDisconnected;
	}

	///==================================================================================== WinTSessions
	void WinTS::Cache(const WinTSHandle &host) {
		PWTS_SESSION_INFOW all_info;
		DWORD cnt = 0;
		CheckApi(Wtsapi32_dll::inst().WTSEnumerateSessionsW(host, 0, 1, &all_info, &cnt));
		clear();
		for (size_t i = 0; i < cnt; ++i) {
			WinTSInfo info(host, all_info[i].SessionId, all_info[i].pWinStationName, all_info[i].State);
			if (!info.user().empty())
				push_back(info);
		}
		Wtsapi32_dll::inst().WTSFreeMemory(all_info);
		std::sort(begin(), end());
	}

	bool WinTS::FindSess(PCWSTR /*in*/) const {
		return false;
	}

	bool WinTS::FindUser(PCWSTR /*in*/) const {
		return false;
	}

}
