#define _WIN32_WINNT 0x0600

#include <libext/policy.hpp>
#include <libext/exception.hpp>
#include <libext/sid.hpp>

#include <ntstatus.h>

namespace Ext {

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

	void WinPolicy::InitLsaString(LSA_UNICODE_STRING &lsaString, const ustring &in) {
		lsaString.Buffer = (PWSTR)in.c_str();
		lsaString.Length = in.size() * sizeof(wchar_t);
		lsaString.MaximumLength = (in.size() + 1) * sizeof(wchar_t);
	}

	LSA_HANDLE WinPolicy::GetPolicyHandle(const ustring &dom) {
		LSA_HANDLE hPolicy = nullptr;
		LSA_OBJECT_ATTRIBUTES oa = {0};
		LSA_UNICODE_STRING lsaName = {0};

		InitLsaString(lsaName, dom);

		NTSTATUS Result = ::LsaOpenPolicy(&lsaName, &oa, POLICY_ALL_ACCESS, &hPolicy);
		if (Result != STATUS_SUCCESS) {
			hPolicy = nullptr;
			::SetLastError(::LsaNtStatusToWinError(Result));
		}
		return hPolicy;
	}

	NTSTATUS WinPolicy::AccountRightAdd(const ustring &name, const ustring &right, const ustring &dom) {
		NTSTATUS Result = STATUS_SUCCESS;
		LSA_HANDLE hPolicy = GetPolicyHandle(dom.c_str());
		if (hPolicy) {
			LSA_UNICODE_STRING lsaRight;
			InitLsaString(lsaRight, right);

			Result = ::LsaAddAccountRights(hPolicy, Sid(name.c_str(), dom.c_str()), &lsaRight, 1);
			::LsaClose(hPolicy);
			::SetLastError(::LsaNtStatusToWinError(Result));
		}
		return Result;
	}

	NTSTATUS WinPolicy::AccountRightDel(const ustring &name, const ustring &right, const ustring &dom) {
		NTSTATUS Result = STATUS_SUCCESS;
		LSA_HANDLE hPolicy = GetPolicyHandle(dom.c_str());
		if (hPolicy) {
			LSA_UNICODE_STRING lsaRight;
			InitLsaString(lsaRight, right);

			Result = ::LsaRemoveAccountRights(hPolicy, Sid(name.c_str(), dom.c_str()), false,
			                                  &lsaRight, 1);
			::LsaClose(hPolicy);
			::SetLastError(::LsaNtStatusToWinError(Result));
		}
		return Result;
	}

}
