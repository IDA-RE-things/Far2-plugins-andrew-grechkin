/**
	win_sid
	SID utilites
	@classes	(Sid)
	@author		© ISPsystem 2009 (Andrew Grechkin)
	@link		()
**/
#include "win_def.h"

#ifndef SID_MAX_SUB_AUTHORITIES
#define SID_MAX_SUB_AUTHORITIES          (15)
#endif
#ifndef SECURITY_MAX_SID_SIZE
#define SECURITY_MAX_SID_SIZE  (sizeof(SID) - sizeof(DWORD) + (SID_MAX_SUB_AUTHORITIES * sizeof(DWORD)))
#endif

EXTERN_C {
	WINADVAPI BOOL WINAPI ConvertSidToStringSidW(
		IN  PSID     Sid,
		OUT LPWSTR  *StringSid
	);
	WINADVAPI BOOL WINAPI ConvertStringSidToSidW(
		IN LPCWSTR   StringSid,
		OUT PSID   *Sid
	);
//	BOOL WINAPI		CreateWellKnownSid(WELL_KNOWN_SID_TYPE WellKnownSidType, PSID DomainSid, PSID pSid, DWORD *cbSid);
}

/*
typedef		enum {
	WinNullSid                                  = 0,
	WinWorldSid                                 = 1,
	WinLocalSid                                 = 2,
	WinCreatorOwnerSid                          = 3,
	WinCreatorGroupSid                          = 4,
	WinCreatorOwnerServerSid                    = 5,
	WinCreatorGroupServerSid                    = 6,
	WinNtAuthoritySid                           = 7,
	WinDialupSid                                = 8,
	WinNetworkSid                               = 9,
	WinBatchSid                                 = 10,
	WinInteractiveSid                           = 11,
	WinServiceSid                               = 12,
	WinAnonymousSid                             = 13,
	WinProxySid                                 = 14,
	WinEnterpriseControllersSid                 = 15,
	WinSelfSid                                  = 16,
	WinAuthenticatedUserSid                     = 17,
	WinRestrictedCodeSid                        = 18,
	WinTerminalServerSid                        = 19,
	WinRemoteLogonIdSid                         = 20,
	WinLogonIdsSid                              = 21,
	WinLocalSystemSid                           = 22,
	WinLocalServiceSid                          = 23,
	WinNetworkServiceSid                        = 24,
	WinBuiltinDomainSid                         = 25,
	WinBuiltinAdministratorsSid                 = 26,
	WinBuiltinUsersSid                          = 27,
	WinBuiltinGuestsSid                         = 28,
	WinBuiltinPowerUsersSid                     = 29,
	WinBuiltinAccountOperatorsSid               = 30,
	WinBuiltinSystemOperatorsSid                = 31,
	WinBuiltinPrintOperatorsSid                 = 32,
	WinBuiltinBackupOperatorsSid                = 33,
	WinBuiltinReplicatorSid                     = 34,
	WinBuiltinPreWindows2000CompatibleAccessSid = 35,
	WinBuiltinRemoteDesktopUsersSid             = 36,
	WinBuiltinNetworkConfigurationOperatorsSid  = 37,
	WinAccountAdministratorSid                  = 38,
	WinAccountGuestSid                          = 39,
	WinAccountKrbtgtSid                         = 40,
	WinAccountDomainAdminsSid                   = 41,
	WinAccountDomainUsersSid                    = 42,
	WinAccountDomainGuestsSid                   = 43,
	WinAccountComputersSid                      = 44,
	WinAccountControllersSid                    = 45,
	WinAccountCertAdminsSid                     = 46,
	WinAccountSchemaAdminsSid                   = 47,
	WinAccountEnterpriseAdminsSid               = 48,
	WinAccountPolicyAdminsSid                   = 49,
	WinAccountRasAndIasServersSid               = 50,
	WinNTLMAuthenticationSid                    = 51,
	WinDigestAuthenticationSid                  = 52,
	WinSChannelAuthenticationSid                = 53,
	WinThisOrganizationSid                      = 54,
	WinOtherOrganizationSid                     = 55,
	WinBuiltinIncomingForestTrustBuildersSid    = 56,
	WinBuiltinPerfMonitoringUsersSid            = 57,
	WinBuiltinPerfLoggingUsersSid               = 58,
	WinBuiltinAuthorizationAccessSid            = 59,
	WinBuiltinTerminalServerLicenseServersSid   = 60,
	WinBuiltinDCOMUsersSid                      = 61,
	WinBuiltinIUsersSid                         = 62,
	WinIUserSid                                 = 63,
	WinBuiltinCryptoOperatorsSid                = 64,
	WinUntrustedLabelSid                        = 65,
	WinLowLabelSid                              = 66,
	WinMediumLabelSid                           = 67,
	WinHighLabelSid                             = 68,
	WinSystemLabelSid                           = 69,
	WinWriteRestrictedCodeSid                   = 70,
	WinCreatorOwnerRightsSid                    = 71,
	WinCacheablePrincipalsGroupSid              = 72,
	WinNonCacheablePrincipalsGroupSid           = 73,
	WinEnterpriseReadonlyControllersSid         = 74,
	WinAccountReadonlyControllersSid            = 75,
	WinBuiltinEventLogReadersGroup              = 76,
} WELL_KNOWN_SID_TYPE;
*/

///============================================================================================= Sid
//private
void					Sid::Copy(PSID in) {
	if (Valid(in)) {
		DWORD size	= Size(in);
		if (WinMem::Alloc(pSID, size))
			::CopySid(size, pSID, in);
	}
}
void					Sid::Free(PSID &in) {
	if (in) {
		WinMem::Free(in);
		in = NULL;
	}
}

//public
//Sid::Sid(WELL_KNOWN_SID_TYPE	wns): pSID(NULL) {
//	DWORD	size = SECURITY_MAX_SID_SIZE;
//	if (WinMem::Alloc(pSID, size)) {
//			if (::CreateWellKnownSid(wns, NULL, pSID, &size))
//		WinMem::Realloc(pSID, size);
//	}
//}
Sid::Sid(PCWSTR sSID): pSID(NULL) {
	PSID	sid = NULL;
	if (::ConvertStringSidToSidW((PWSTR)sSID, &sid)) {
		Copy(sid);
		Free(sid);
	}
}
Sid::Sid(PCWSTR name, PCWSTR dom): pSID(NULL) {
	DWORD	dwSidSize	= 0, dwDomSize	= 0;
	PWSTR	pDom	= NULL;
	SID_NAME_USE	type;
	::LookupAccountNameW(dom, name, pSID, &dwSidSize, pDom, &dwDomSize, &type);

	if (::GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
		WinMem::Alloc(pSID, dwSidSize);
		WinMem::Alloc(pDom, dwDomSize * sizeof(WCHAR));

		::LookupAccountNameW(dom, name, pSID, &dwSidSize, pDom, &dwDomSize, &type);
		WinMem::Free(pDom);
	}
}
Sid::Sid(const AutoUTF &sSID): pSID(NULL) {
	PSID	sid = NULL;
	if (::ConvertStringSidToSidW((PWSTR)sSID.c_str(), &sid)) {
		Copy(sid);
		Free(sid);
	}
}
Sid::Sid(const AutoUTF &name, const AutoUTF &dom): pSID(NULL) {
	DWORD	dwSidSize	= 0, dwDomSize	= 0;
	PWSTR	pDom	= NULL;
	SID_NAME_USE	type;
	::LookupAccountNameW(dom.c_str(), name.c_str(), pSID, &dwSidSize, pDom, &dwDomSize, &type);

	if (::GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
		WinMem::Alloc(pSID, dwSidSize);
		WinMem::Alloc(pDom, dwDomSize * sizeof(WCHAR));

		::LookupAccountNameW(dom.c_str(), name.c_str(), pSID, &dwSidSize, pDom, &dwDomSize, &type);
		WinMem::Free(pDom);
	}
}

//static
AutoUTF					Sid::AsStr(PSID in) {
	AutoUTF	Result;
	if (Valid(in)) {
		PWSTR sSID = NULL;
		if (::ConvertSidToStringSidW(in, &sSID)) {
			Result = sSID;
			::LocalFree(sSID);
		}
	}
	return	Result;
}
AutoUTF					Sid::AsStr(const AutoUTF &name, const AutoUTF &dom) {
	Sid		sid(name, dom);
	return	sid.AsStr();
}

AutoUTF					Sid::AsName(PSID pSID) {
	AutoUTF	name, dom;
	AsName(pSID, name, dom);
	return	name;
}
AutoUTF					Sid::AsDom(PSID pSID) {
	AutoUTF	name, dom;
	AsName(pSID, name, dom);
	return	dom;
}
DWORD					Sid::AsName(PSID pSID, AutoUTF &name, AutoUTF &dom) {
	DWORD	err = ERROR_INVALID_SID;
	if (Valid(pSID)) {
		DWORD	dwNameSize	= 0;
		DWORD	dwDomSize	= 0;
		PWSTR	pName	= NULL;
		PWSTR	pDom	= NULL;
		SID_NAME_USE type;

		// determine size of name
		::LookupAccountSidW(dom.c_str(), pSID, pName, &dwNameSize, pDom, &dwDomSize, &type);
		err = ::GetLastError();

		if (err == ERROR_INSUFFICIENT_BUFFER) {
			// allocating memory
			WinMem::Alloc(pName, dwNameSize * sizeof(WCHAR));
			WinMem::Alloc(pDom, dwDomSize  * sizeof(WCHAR));

			// retrieve name
			if (::LookupAccountSidW(dom.c_str(), pSID, pName, &dwNameSize, pDom, &dwDomSize, &type)) {
				name = pName;
				dom  = pDom;
				err  = NO_ERROR;
			} else {
				err = ::GetLastError();
			}
			WinMem::Free(pName);
			WinMem::Free(pDom);
		}
	}
	return	err;
}

AutoUTF					Sid::AsName(const AutoUTF &sSID) {
	Sid	sid(sSID);
	return	sid.AsName();
}
AutoUTF					Sid::AsDom(const AutoUTF &sSID) {
	Sid	sid(sSID);
	return	sid.AsDom();
}
DWORD					Sid::AsName(const AutoUTF &sSID, AutoUTF &name, AutoUTF &dom) {
	Sid	sid(sSID);
	return	AsName(sid, name, dom);
}

// WELL KNOWN SIDS
PCWSTR	Sid::SID_NOBODY				= L"S-1-0-0";			// NULL SID
PCWSTR	Sid::SID_LOCAL				= L"S-1-0-0";			// ЛОКАЛЬНЫЕ
PCWSTR	Sid::SID_EVERIONE			= L"S-1-1-0";			// Все
PCWSTR	Sid::SID_CREATOR_OWNER		= L"S-1-3-0";			// СОЗДАТЕЛЬ-ВЛАДЕЛЕЦ
PCWSTR	Sid::SID_CREATOR_GROUP		= L"S-1-3-1";			// ГРУППА-СОЗДАТЕЛЬ
PCWSTR	Sid::SID_CREATOR_OWNER_S	= L"S-1-3-2";			// СОЗДАТЕЛЬ-ВЛАДЕЛЕЦ СЕРВЕР
PCWSTR	Sid::SID_CREATOR_GROUP_S	= L"S-1-3-3";			// ГРУППА-СОЗДАТЕЛЬ СЕРВЕР
PCWSTR	Sid::SID_DIALUP				= L"S-1-5-1";			// УДАЛЕННЫЙ ДОСТУП
PCWSTR	Sid::SID_NETWORK			= L"S-1-5-2";			// СЕТЬ
PCWSTR	Sid::SID_BATCH				= L"S-1-5-3";			// ПАКЕТНЫЕ ФАЙЛЫ
PCWSTR	Sid::SID_SELF				= L"S-1-5-10";			// SELF
PCWSTR	Sid::SID_AUTH_USERS			= L"S-1-5-11";			// Прошедшие проверку
PCWSTR	Sid::SID_RESTRICTED			= L"S-1-5-12";			// ОГРАНИЧЕННЫЕ
PCWSTR	Sid::SID_TS_USERS			= L"S-1-5-13";			// ПОЛЬЗОВАТЕЛЬ СЕРВЕРА ТЕРМИНАЛОВ
PCWSTR	Sid::SID_RIL				= L"S-1-5-14";			// REMOTE INTERACTIVE LOGON
PCWSTR	Sid::SID_LOCAL_SYSTEM		= L"S-1-5-18";			// SYSTEM
PCWSTR	Sid::SID_LOCAL_SERVICE		= L"S-1-5-19";			// LOCAL SERVICE
PCWSTR	Sid::SID_NETWORK_SERVICE	= L"S-1-5-20";			// NETWORK SERVICE
PCWSTR	Sid::SID_ADMINS				= L"S-1-5-32-544";		// Администраторы
PCWSTR	Sid::SID_USERS				= L"S-1-5-32-545";		// Пользователи
PCWSTR	Sid::SID_GUESTS				= L"S-1-5-32-546";		// Гости
PCWSTR	Sid::SID_POWER_USERS		= L"S-1-5-32-547";		// Опытные пользователи
PCWSTR	Sid::SID_ACCOUNT_OPERATORS	= L"S-1-5-32-548";
PCWSTR	Sid::SID_SERVER_OPERATORS	= L"S-1-5-32-549";
PCWSTR	Sid::SID_PRINT_OPERATORS	= L"S-1-5-32-550";
PCWSTR	Sid::SID_BACKUP_OPERATORS	= L"S-1-5-32-551";		// Операторы архива
PCWSTR	Sid::SID_REPLICATORS		= L"S-1-5-32-552";		// Репликатор
PCWSTR	Sid::SID_REMOTE_DESKTOP		= L"S-1-5-32-555";		// Пользователи удаленного рабочего стола
PCWSTR	Sid::SID_NETWORK_OPERATOR	= L"S-1-5-32-556";		// Операторы настройки сети
PCWSTR	Sid::SID_IIS				= L"S-1-5-32-568";
PCWSTR	Sid::SID_INTERACTIVE		= L"S-1-5-4";			// ИНТЕРАКТИВНЫЕ
PCWSTR	Sid::SID_SERVICE			= L"S-1-5-6";			// СЛУЖБА
PCWSTR	Sid::SID_ANONYMOUS			= L"S-1-5-7";			// АНОНИМНЫЙ ВХОД
PCWSTR	Sid::SID_PROXY				= L"S-1-5-8";			// PROXY
