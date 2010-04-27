#include "win_def.h"

///========================================================================================= WinPriv
/// Функции работы с привилегиями
namespace	WinPriv {
bool 			IsExist(HANDLE hToken, LUID priv) {
	bool	Result = false;

	DWORD	dwSize = 0;
	// определяем размер буфера, необходимый для получения всех привилегий
	if (!::GetTokenInformation(hToken, TokenPrivileges, NULL, 0, &dwSize) && ::GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
		// выделяем память для выходного буфера
		WinBuf<TOKEN_PRIVILEGES>	ptp(dwSize);
		if (::GetTokenInformation(hToken, TokenPrivileges, ptp, (DWORD)ptp.capacity(), &dwSize)) {
			// проходим по списку привилегий и проверяем, есть ли в нем указанная привилегия
			for (DWORD i = 0; i < ptp->PrivilegeCount; ++i) {
				if (ptp->Privileges[i].Luid.LowPart == priv.LowPart && ptp->Privileges[i].Luid.HighPart == priv.HighPart) {
					Result = true;
					break;
				}
			}
		}
	}
	return	Result;
}
bool 			IsExist(HANDLE hToken, PCWSTR sPriv) {
	LUID	luid;
	if (::LookupPrivilegeValueW(NULL, sPriv, &luid))
		return	IsExist(hToken, luid);
	return	false;
}
bool 			IsExist(HANDLE hToken, PCSTR sPriv) {
	LUID	luid;
	if (::LookupPrivilegeValueA(NULL, sPriv, &luid))
		return	IsExist(hToken, luid);
	return	false;
}

bool			IsEnabled(HANDLE hToken, LUID priv) {
	BOOL	Result = false;

	PRIVILEGE_SET		ps;
	WinMem::Zero(ps);
	ps.PrivilegeCount = 1;
	ps.Privilege[0].Luid = priv;
	::PrivilegeCheck(hToken, &ps, &Result);
	return	Result != 0;
}
bool 			IsEnabled(HANDLE hToken, PCWSTR sPriv) {
	LUID	luid;
	// получаем идентификатор привилегии
	if (::LookupPrivilegeValueW(NULL, sPriv, &luid))
		return	IsEnabled(hToken, luid);
	return	false;
}
bool 			IsEnabled(HANDLE hToken, PCSTR sPriv) {
	LUID	luid;
	// получаем идентификатор привилегии
	if (::LookupPrivilegeValueA(NULL, sPriv, &luid))
		return	IsEnabled(hToken, luid);
	return	false;
}

bool 			Modify(HANDLE hToken, LUID priv, bool bEnable) {
	bool	Result = false;
	TOKEN_PRIVILEGES	tp;
	WinMem::Zero(tp);
	tp.PrivilegeCount = 1;
	tp.Privileges[0].Luid = priv;
	tp.Privileges[0].Attributes = (bEnable) ? SE_PRIVILEGE_ENABLED : 0;

	if (::AdjustTokenPrivileges(hToken, false, &tp, sizeof(tp), NULL, NULL))
		Result = true;
	return	Result;
}
bool 			Modify(HANDLE hToken, PCWSTR sPriv, bool bEnable) {
	LUID	luid;
	if (::LookupPrivilegeValueW(NULL, sPriv, &luid))
		return	Modify(hToken, luid, bEnable);
	return	false;
}
bool 			Modify(HANDLE hToken, PCSTR	sPriv, bool bEnable) {
	LUID	luid;
	if (::LookupPrivilegeValueA(NULL, sPriv, &luid))
		return	Modify(hToken, luid, bEnable);
	return	false;
}
}

///========================================================================================= WinProc
/// Обертка хэндла процесса
AutoUTF			WinProcess::User() {
	DWORD	size = MAX_PATH;
	WCHAR	buf[size];
	::GetUserNameW(buf, &size);
	return	buf;
}
AutoUTF			WinProcess::FullPath() {
	WCHAR	tmp[MAX_PATH];
	size_t	sz = ::GetModuleFileNameW(NULL, tmp, sizeofa(tmp));
	if (sz > sizeofa(tmp)) {
		WCHAR	Result[sz];
		::GetModuleFileNameW(NULL, Result, sizeofa(Result));
		return	Result;
	}
	return	tmp;
}

