#include "win_def.h"

#ifndef NT_SUCCESS
#define NT_SUCCESS(x) ((x)>=0)
#define STATUS_SUCCESS ((NTSTATUS)0)
#endif

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
bool 			IsExist(LUID priv) {
	WinToken	hToken(TOKEN_QUERY);
	if (hToken.IsOK()) {
		return	IsExist(hToken, priv);
	}
	return	false;
}
bool 			IsExist(PCWSTR sPriv) {
	WinToken	hToken(TOKEN_QUERY);
	if (hToken.IsOK()) {
		return	IsExist(hToken, sPriv);
	}
	return	false;
}

bool			IsEnabled(HANDLE hToken, LUID priv) {
	BOOL	Result = false;
	PRIVILEGE_SET		ps = {0};
	ps.PrivilegeCount = 1;
	ps.Privilege[0].Luid = priv;

	::PrivilegeCheck(hToken, &ps, &Result);
	return	Result;
}
bool 			IsEnabled(HANDLE hToken, PCWSTR sPriv) {
	LUID	luid;
	// получаем идентификатор привилегии
	if (::LookupPrivilegeValueW(NULL, sPriv, &luid))
		return	IsEnabled(hToken, luid);
	return	false;
}
bool			IsEnabled(LUID priv) {
	WinToken	hToken(TOKEN_QUERY);
	if (hToken.IsOK()) {
		return	IsEnabled(hToken, priv);
	}
	return	false;
}
bool 			IsEnabled(PCWSTR sPriv) {
	WinToken	hToken(TOKEN_QUERY);
	if (hToken.IsOK()) {
		return	IsEnabled(hToken, sPriv);
	}
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
bool 			Modify(LUID priv, bool bEnable) {
	WinToken	hToken(TOKEN_ADJUST_PRIVILEGES);
	if (hToken.IsOK()) {
		return	Modify(hToken, priv, bEnable);
	}
	return	false;
}
bool 			Modify(PCWSTR sPriv, bool bEnable) {
	WinToken	hToken(TOKEN_ADJUST_PRIVILEGES);
	if (hToken.IsOK()) {
		return	Modify(hToken, sPriv, bEnable);
	}
	return	false;
}

AutoUTF			GetName(PCWSTR sPriv) {
	DWORD	dwSize = 0, dwLang = 0;
	::LookupPrivilegeDisplayNameW(NULL, sPriv, NULL, &dwSize, &dwLang);
	if (::GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
		WCHAR	sName[dwSize];
		if (::LookupPrivilegeDisplayNameW(NULL, sPriv, sName, &dwSize, &dwLang))
			return	sName;
	}
	return	AutoUTF();
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

///======================================================================================== WinToken
AutoUTF			WinToken::GetUser(HANDLE hToken) {
	AutoUTF	Result;
	DWORD	dwInfoBufferSize = 0;
	if (!::GetTokenInformation(hToken, TokenUser, NULL, 0, &dwInfoBufferSize)) {
		WinBuf<TOKEN_USER>	buf(dwInfoBufferSize, true);
		if (::GetTokenInformation(hToken, TokenUser, buf, buf.capacity(), &dwInfoBufferSize)) {
			return Sid::AsName(buf->User.Sid);
		}
	}
	return	AutoUTF();
}

///======================================================================================= WinPolicy
/*
PCSTR	WinPolicy::PrivNames[] = {
	"SeAssignPrimaryTokenPrivilege",
	"SeAuditPrivilege",
	"SeBackupPrivilege",
	"SeChangeNotifyPrivilege",
	"SeCreateGlobalPrivilege",
	"SeCreatePagefilePrivilege",
	"SeCreatePermanentPrivilege",
	"SeCreateTokenPrivilege",
	"SeDebugPrivilege",
	"SeEnableDelegationPrivilege",
	"SeImpersonatePrivilege",
	"SeIncreaseBasePriorityPrivilege",
	"SeIncreaseQuotaPrivilege",
	"SeLoadDriverPrivilege",
	"SeLockMemoryPrivilege",
	"SeMachineAccountPrivilege",
	"SeManageVolumePrivilege",
	"SeProfileSingleProcessPrivilege",
	"SeRemoteShutdownPrivilege",
	"SeRestorePrivilege",
	"SeSecurityPrivilege",
	"SeShutdownPrivilege",
	"SeSyncAgentPrivilege",
	"SeSystemEnvironmentPrivilege",
	"SeSystemProfilePrivilege",
	"SeSystemTimePrivilege",
	"SeTakeOwnershipPrivilege",
	"SeTcbPrivilege",
	"SeUndockPrivilege",
	"",
};
PCSTR	WinPolicy::PrivNamesEn[] = {
	"Replace a process level token",
	"Generate security audits",
	"Back up files and directories",
	"Bypass traverse checking",
	"Create global objects",
	"Create a pagefile",
	"Create permanent shared objects",
	"Create a token object",
	"Debug programs",
	"Enable computer and user accounts to be trusted for delegation",
	"Impersonate a client after authentication",
	"Increase scheduling priority",
	"Adjust memory quotas for a process",
	"Load and unload device drivers",
	"Lock pages in memory",
	"Add workstations to domain",
	"Perform volume maintenance tasks",
	"Profile single process",
	"Force shutdown from a remote system",
	"Restore files and directories",
	"Manage auditing and security log",
	"Shut down the system",
	"Synchronize directory service data",
	"Modify firmware environment values",
	"Profile system performance",
	"Change the system time",
	"Take ownership of files or other objects",
	"Act as part of the operating system",
	"Remove computer from docking station",
	"",
};
PCSTR	WinPolicy::PrivNamesRu[] = {
	"Замена маркера уровня процесса",
	"Создание журналов безопасности",
	"Архивирование файлов и каталогов",
	"Обход перекрестной проверки",
	"Создание глобальных объектов",
	"Создание страничного файла",
	"Создание постоянных объектов совместного использования",
	"Создание маркерного объекта",
	"Отладка программ",
	"Разрешение доверия к учетным записям при делегировании",
	"Олицетворение клиента после проверки подлинности",
	"Увеличение приоритета диспетчирования",
	"Настройка квот памяти для процесса",
	"Загрузка и выгрузка драйверов устройств",
	"Закрепление страниц в памяти",
	"Добавление рабочих станций к домену",
	"Запуск операций по обслуживанию тома",
	"Профилирование одного процесса",
	"Принудительное удаленное завершение",
	"Восстановление файлов и каталогов",
	"Управление аудитом и журналом безопасности",
	"Завершение работы системы",
	"Синхронизация данных службы каталогов",
	"Изменение параметров среды оборудования",
	"Профилирование загруженности системы",
	"Изменение системного времени",
	"Овладение файлами или иными объектами",
	"Работа в режиме операционной системы",
	"Извлечение компьютера из стыковочного узла",
	"",
};
PCSTR	WinPolicy::RightsNames[] = {
	"SeBatchLogonRight",
	"SeInteractiveLogonRight",
	"SeNetworkLogonRight",
	"SeRemoteInteractiveLogonRight",
	"SeServiceLogonRight",
	"SeDenyBatchLogonRight",
	"SeDenyInteractiveLogonRight",
	"SeDenyNetworkLogonRight",
	"SeDenyRemoteInteractiveLogonRight",
	"SeDenyServiceLogonRight",
	"",
};
PCSTR	WinPolicy::RightsNamesEn[] = {
	"Log on as a batch job",
	"Allow log on locally",
	"Access this computer from the network",
	"Allow log on through Terminal Services",
	"Log on as a service",
	"Deny log on as a batch job",
	"Deny log on locally",
	"Deny access to this computer from the network",
	"Deny log on through Terminal Services",
	"Deny log on as a service",
	"",
};
PCSTR	WinPolicy::RightsNamesRu[] = {
	"Вход в качестве пакетного задания",
	"Локальный вход в систему",
	"Доступ к компьютеру из сети",
	"Разрешать вход в систему через службу терминалов",
	"Вход в качестве службы",
	"Отказывать во входе в качестве пакетного задания",
	"Отклонять локальный вход",
	"Отказывать в доступе к компьютеру из сети",
	"Запретить вход в систему через службу терминалов",
	"Отказывать во входе в качестве службы",
	"",
};
*/

HANDLE					WinPolicy::Handle(const AutoUTF &path, bool bWrite) {
	// Obtain backup/restore privilege in case we don't have it
	bWrite ? WinPriv::Enable(SE_RESTORE_NAME) : WinPriv::Enable(SE_BACKUP_NAME);
	DWORD	dwAccess = (bWrite) ? (GENERIC_READ | GENERIC_WRITE) : GENERIC_READ;
	return	::CreateFileW(path.c_str(), dwAccess, 0, NULL, OPEN_EXISTING,
						 FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS, NULL);
}

void					WinPolicy::InitLsaString(LSA_UNICODE_STRING &lsaString, const AutoUTF &in) {
	lsaString.Buffer = (PWSTR)in.c_str();
	lsaString.Length = in.size() * sizeof(WCHAR);
	lsaString.MaximumLength = (in.size() + 1) * sizeof(WCHAR);
}
LSA_HANDLE				WinPolicy::GetPolicyHandle(const AutoUTF &dom) {
	LSA_HANDLE			hPolicy = NULL;
	LSA_OBJECT_ATTRIBUTES	oa = {0};
	LSA_UNICODE_STRING	lsaName = {0};

	InitLsaString(lsaName, dom);

	NTSTATUS	Result = ::LsaOpenPolicy(&lsaName, &oa, POLICY_ALL_ACCESS, &hPolicy);
	if (Result != STATUS_SUCCESS) {
		hPolicy = NULL;
		::SetLastError(::LsaNtStatusToWinError(Result));
	}
	return	hPolicy;
}

NTSTATUS				WinPolicy::AccountRightAdd(const AutoUTF &name, const AutoUTF &right, const AutoUTF &dom) {
	NTSTATUS	Result = STATUS_SUCCESS;
	LSA_HANDLE hPolicy = GetPolicyHandle(dom.c_str());
	if (hPolicy) {
		LSA_UNICODE_STRING lsaRight;
		InitLsaString(lsaRight, right);

		Result = ::LsaAddAccountRights(hPolicy, Sid(name.c_str(), dom.c_str()), &lsaRight, 1);
		::LsaClose(hPolicy);
		::SetLastError(::LsaNtStatusToWinError(Result));
	}
	return	Result;
}
NTSTATUS				WinPolicy::AccountRightDel(const AutoUTF &name, const AutoUTF &right, const AutoUTF &dom) {
	NTSTATUS Result = STATUS_SUCCESS;
	LSA_HANDLE hPolicy = GetPolicyHandle(dom.c_str());
	if (hPolicy) {
		LSA_UNICODE_STRING lsaRight;
		InitLsaString(lsaRight, right);

		Result = ::LsaRemoveAccountRights(hPolicy, Sid(name.c_str(), dom.c_str()), false, &lsaRight, 1);
		::LsaClose(hPolicy);
		::SetLastError(::LsaNtStatusToWinError(Result));
	}
	return	Result;
}
