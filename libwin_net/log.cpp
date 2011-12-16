#include "win_net.h"
#include "exception.h"

///========================================================================================== WinLog
WinLog::~WinLog() {
	::DeregisterEventSource(m_hndl);
}

WinLog::WinLog(PCWSTR name): m_hndl(::RegisterEventSourceW(nullptr, name)) {
	CheckApi(m_hndl != nullptr);
}

//	void		Write(DWORD Event, WORD Count, LPCWSTR *Strings) {
//		PSID user = nullptr;
//		HANDLE token;
//		PTOKEN_USER token_user = nullptr;
//		if (OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, FALSE, &token)) {
//			token_user = (PTOKEN_USER)DefaultTokenInformation(token, TokenUser);
//			if (token_user)
//				user = token_user->User.Sid;
//			CloseHandle(token);
//		}
//		ReportEventW(m_hndl, EVENTLOG_ERROR_TYPE, 0, Event, user, Count, 0, Strings, nullptr);
//		free(token_user);
//	}

void WinLog::Register(PCWSTR name, PCWSTR path) {
	WCHAR	fullpath[MAX_PATH_LEN];
	WCHAR	key[MAX_PATH_LEN];
	if (!path || Empty(path)) {
		CheckApi(::GetModuleFileNameW(0, fullpath, sizeofa(fullpath)));
	} else {
		Copy(fullpath, path, sizeofa(fullpath));
	}
	HKEY	hKey = nullptr;
	Copy(key, L"SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\", sizeofa(key));
	Cat(key, name, sizeofa(key));
	CheckApi(::RegCreateKeyW(HKEY_LOCAL_MACHINE, key, &hKey) == ERROR_SUCCESS);
	// Add the Event ID message-file name to the subkey.
	::RegSetValueExW(hKey, L"EventMessageFile", 0, REG_EXPAND_SZ, (LPBYTE)fullpath, (DWORD)((Len(fullpath) + 1)*sizeof(WCHAR)));
	// Set the supported types flags.
	DWORD dwData = EVENTLOG_ERROR_TYPE | EVENTLOG_WARNING_TYPE | EVENTLOG_INFORMATION_TYPE;
	::RegSetValueExW(hKey, L"TypesSupported", 0, REG_DWORD, (LPBYTE)&dwData, sizeof(dwData));
	::RegCloseKey(hKey);
}
