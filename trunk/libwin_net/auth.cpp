/**
	win_net_auth
	Auth utilites
	@classes	()
	@author		© 2010 Andrew Grechkin
	@link		(advapi32)
**/

#define _WIN32_WINNT 0x0600

#include "win_net.h"

#include <WinCred.h>
#include <ntstatus.h>

//typedef enum _CRED_PROTECTION_TYPE {
//	CredUnprotected         = 0,
//	CredUserProtection      = 1,
//	CredTrustedProtection   = 2
//} CRED_PROTECTION_TYPE, *PCRED_PROTECTION_TYPE;
//
//extern "C" {
//	BOOL WINAPI CredProtectW(BOOL fAsSelf, PCWSTR pszCredentials, DWORD cchCredentials, PWSTR pszProtectedCredentials, DWORD *pcchMaxChars, CRED_PROTECTION_TYPE *ProtectionType);
//	BOOL WINAPI CredUnprotectW(BOOL fAsSelf, PCWSTR pszProtectedCredentials, DWORD cchCredentials, PWSTR pszCredentials, DWORD *pcchMaxChars);
//}

///========================================================================================== Base64
Credential::~Credential() {
	::CredFree(m_cred);
}

Credential::Credential(PCWSTR name, DWORD type) {
	CheckApi(::CredReadW(name, type, 0, &m_cred));
}

const _CREDENTIALW* Credential::operator->() const {
	return m_cred;
}

void Credential::add(PCWSTR name, PCWSTR pass, PCWSTR target) {
	CREDENTIALW cred = {0};
	cred.Type = CRED_TYPE_GENERIC;
	cred.Persist = CRED_PERSIST_LOCAL_MACHINE;
	cred.TargetName = target ? (PWSTR)target : (PWSTR)name;
	cred.UserName = (PWSTR)name;
	cred.CredentialBlob = (PBYTE)pass;
	cred.CredentialBlobSize = sizeof(*pass) * (Len(pass) + 1);
	CheckApi(::CredWriteW(&cred, 0));
}

void Credential::del(PCWSTR name, DWORD type) {
	CheckApi(::CredDeleteW(name, type, 0));
}

Credentials::~Credentials() {
	::CredFree(m_creds);
}

Credentials::Credentials() {
	CheckApi(::CredEnumerateW(nullptr, 0, &m_size, &m_creds));
}

void Credentials::Update() {
	::CredFree(m_creds);
	CheckApi(::CredEnumerateW(nullptr, 0, &m_size, &m_creds));
}

size_t Credentials::size() const {
	return m_size;
}

const _CREDENTIALW* Credentials::operator[](size_t ind) const {
	return m_creds[ind];
}

void	PassProtect(PCWSTR pass, PWSTR prot, DWORD size) {
	CRED_PROTECTION_TYPE type;
	CheckApi(CredProtectW(true, (PWSTR)pass, Len(pass) + 1, prot, &size, &type));
}

void	PassUnProtect(PCWSTR prot, DWORD ps, PWSTR pass, DWORD size) {
	CheckApi(::CredUnprotectW(true, (PWSTR)prot, ps, pass, &size));
}

void	PassSave(PCWSTR name, PCWSTR pass) {
//	WCHAR	prot[512];
//	PassProtect(pass, prot, sizeofa(prot));
	CREDENTIALW cred = {0};
	cred.Type = CRED_TYPE_GENERIC;
	cred.Persist = CRED_PERSIST_LOCAL_MACHINE;
	cred.TargetName = (PWSTR)name;
	cred.UserName = (PWSTR)name;
	cred.CredentialBlob = (PBYTE)pass;
	cred.CredentialBlobSize = sizeof(*pass) * (Len(pass) + 1);
	CheckApi(CredWriteW(&cred, 0));
}

void	PassDel(PCWSTR name) {
	CheckApi(CredDeleteW(name, CRED_TYPE_GENERIC, 0));
}

AutoUTF	PassRead(PCWSTR name) {
	PCREDENTIALW cred = nullptr;
	CheckApi(CredReadW(name, CRED_TYPE_GENERIC, 0, &cred));
//	WCHAR	pass[512];
//	PassUnProtect((PCWSTR)cred->CredentialBlob, cred->CredentialBlobSize, pass, sizeofa(pass));
//	return	AutoUTF(pass);
	return	AutoUTF((PCWSTR)cred->CredentialBlob);
}

void	PassList() {
//	DWORD cnt = 0;
//	PCREDENTIAL* cre;
//	CredEnumerate(nullptr, 0, &cnt, &cre);
//	CredFree(cre);
//
//	printf(L"size: %d\n", cnt);
//
//	for (size_t i = 0; i < cnt; ++i) {
//		printf(L"cre [%s] flags: ", i , cre[i]->Flags);
//		printf(L" Name: %s", cre[i]->TargetName);
//		printf(L" Blob: %s", (PCWSTR)cre[i]->CredentialBlob);
//	}
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

void WinPolicy::InitLsaString(LSA_UNICODE_STRING &lsaString, const AutoUTF &in) {
	lsaString.Buffer = (PWSTR)in.c_str();
	lsaString.Length = in.size() * sizeof(WCHAR);
	lsaString.MaximumLength = (in.size() + 1) * sizeof(WCHAR);
}

LSA_HANDLE WinPolicy::GetPolicyHandle(const AutoUTF &dom) {
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

NTSTATUS WinPolicy::AccountRightAdd(const AutoUTF &name, const AutoUTF &right, const AutoUTF &dom) {
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

NTSTATUS WinPolicy::AccountRightDel(const AutoUTF &name, const AutoUTF &right, const AutoUTF &dom) {
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
