#include "win_def.h"

///========================================================================================= WinPriv
/// Функции работы с привилегиями
namespace WinPriv {
	bool IsExist(HANDLE hToken, LUID priv) {
		DWORD dwSize = 0;
		// определяем размер буфера, необходимый для получения всех привилегий
		if (!::GetTokenInformation(hToken, TokenPrivileges, nullptr, 0, &dwSize)
		    && ::GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
			// выделяем память для выходного буфера
			WinBuf<TOKEN_PRIVILEGES> ptp(dwSize, true);
			if (::GetTokenInformation(hToken, TokenPrivileges, ptp, ptp.size(), &dwSize)) {
				// проходим по списку привилегий и проверяем, есть ли в нем указанная привилегия
				for (DWORD i = 0; i < ptp->PrivilegeCount; ++i) {
					if (ptp->Privileges[i].Luid.LowPart == priv.LowPart
					    && ptp->Privileges[i].Luid.HighPart == priv.HighPart) {
						return	true;
						break;
					}
				}
			}
		}
		return false;
	}

	bool IsExist(HANDLE hToken, PCWSTR sPriv) {
		LUID luid;
		if (::LookupPrivilegeValueW(nullptr, sPriv, &luid))
			return IsExist(hToken, luid);
		return false;
	}

	bool IsExist(LUID priv) {
		WinToken hToken(TOKEN_QUERY);
		if (hToken.IsOK()) {
			return IsExist(hToken, priv);
		}
		return false;
	}

	bool IsExist(PCWSTR sPriv) {
		WinToken hToken(TOKEN_QUERY);
		if (hToken.IsOK()) {
			return IsExist(hToken, sPriv);
		}
		return false;
	}

	bool IsEnabled(HANDLE hToken, LUID priv) {
		BOOL Result = false;
		PRIVILEGE_SET ps = {0};
		ps.PrivilegeCount = 1;
		ps.Privilege[0].Luid = priv;

		::PrivilegeCheck(hToken, &ps, &Result);
		return Result;
	}

	bool IsEnabled(HANDLE hToken, PCWSTR sPriv) {
		LUID luid;
		if (::LookupPrivilegeValueW(nullptr, sPriv, &luid))
			return IsEnabled(hToken, luid);
		return false;
	}

	bool IsEnabled(LUID priv) {
		WinToken hToken(TOKEN_QUERY);
		if (hToken.IsOK()) {
			return IsEnabled(hToken, priv);
		}
		return false;
	}

	bool IsEnabled(PCWSTR sPriv) {
		WinToken hToken(TOKEN_QUERY);
		if (hToken.IsOK()) {
			return IsEnabled(hToken, sPriv);
		}
		return false;
	}

	bool Modify(HANDLE hToken, LUID priv, bool bEnable) {
		TOKEN_PRIVILEGES tp = {0};
//		WinMem::Zero(tp);
		tp.PrivilegeCount = 1;
		tp.Privileges[0].Luid = priv;
		tp.Privileges[0].Attributes = (bEnable) ? SE_PRIVILEGE_ENABLED : 0;

		return	::AdjustTokenPrivileges(hToken, false, &tp, sizeof(tp), nullptr, nullptr);
	}

	bool Modify(HANDLE hToken, PCWSTR sPriv, bool bEnable) {
		LUID luid;
		if (::LookupPrivilegeValueW(nullptr, sPriv, &luid))
			return Modify(hToken, luid, bEnable);
		return false;
	}

	bool Modify(LUID priv, bool bEnable) {
		WinToken hToken(TOKEN_ADJUST_PRIVILEGES);
		if (hToken.IsOK()) {
			return Modify(hToken, priv, bEnable);
		}
		return false;
	}

	bool Modify(PCWSTR sPriv, bool bEnable) {
		WinToken hToken(TOKEN_ADJUST_PRIVILEGES);
		if (hToken.IsOK()) {
			return Modify(hToken, sPriv, bEnable);
		}
		return false;
	}

	AutoUTF GetName(PCWSTR sPriv) {
		DWORD dwSize = 0, dwLang = 0;
		::LookupPrivilegeDisplayNameW(nullptr, sPriv, nullptr, &dwSize, &dwLang);
		if (::GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
			WCHAR sName[dwSize];
			if (::LookupPrivilegeDisplayNameW(nullptr, sPriv, sName, &dwSize, &dwLang))
				return sName;
		}
		return AutoUTF();
	}
}
///========================================================================================= WinProc
/// Обертка хэндла процесса
AutoUTF WinProcess::User() {
	DWORD size = MAX_PATH;
	WCHAR buf[size];
	::GetUserNameW(buf, &size);
	return buf;
}

AutoUTF WinProcess::FullPath() {
	WCHAR tmp[MAX_PATH];
	size_t sz = ::GetModuleFileNameW(nullptr, tmp, sizeofa(tmp));
	if (sz > sizeofa(tmp)) {
		WCHAR Result[sz];
		::GetModuleFileNameW(nullptr, Result, sizeofa(Result));
		return Result;
	}
	return tmp;
}
