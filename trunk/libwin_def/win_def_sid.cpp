/**
	win_sid
	SID utilites
	@classes	(Sid)
	@author		© 2009 Andrew Grechkin
	@link		()
**/
#include "win_def.h"

EXTERN_C {
	WINADVAPI BOOL WINAPI ConvertSidToStringSidW(
		IN  PSID     Sid,
		OUT LPWSTR  *StringSid
	);
	WINADVAPI BOOL WINAPI ConvertStringSidToSidW(
		IN LPCWSTR   StringSid,
		OUT PSID   *Sid
	);
}

///============================================================================================= Sid
//private
void					Sid::Copy(PSID in) {
	Free(pSID);
	if (Valid(in)) {
		DWORD	size	= Size(in);
		if (WinMem::Alloc(pSID, size))
			::CopySid(size, pSID, in);
	}
}
bool					Sid::Init(PCWSTR name, PCWSTR srv) {
	PWSTR	pDom = null_ptr;
	DWORD	dwSidSize = 0;
	DWORD	dwDomSize = 0;
	SID_NAME_USE	type;
	::LookupAccountNameW(srv, name, pSID, &dwSidSize, pDom, &dwDomSize, &type);

	if (::GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
		WinMem::Alloc(pSID, dwSidSize);
		WinBuf<WCHAR>	pDom(dwDomSize);
		return	::LookupAccountNameW(srv, name, pSID, &dwSidSize, pDom, &dwDomSize, &type);
	}
	return	false;
}

//public
Sid::Sid(PCWSTR sSID): pSID(null_ptr) {
	PSID	sid = null_ptr;
	if (::ConvertStringSidToSidW((PWSTR)sSID, &sid)) {
		Copy(sid);
		Free(sid);
	}
}
Sid::Sid(PCWSTR name, PCWSTR srv): pSID(null_ptr), m_srv(srv) {
	Init(name, srv);
}
Sid::Sid(const AutoUTF &sSID): pSID(null_ptr) {
	PSID	sid = null_ptr;
	if (::ConvertStringSidToSidW((PWSTR)sSID.c_str(), &sid)) {
		Copy(sid);
		Free(sid);
	}
}
Sid::Sid(const AutoUTF &name, const AutoUTF &srv): pSID(null_ptr), m_srv(srv) {
	Init(name.c_str(), srv.c_str());
}

//static
// PSID to sid string
AutoUTF					Sid::AsStr(PSID in) {
	AutoUTF	Result;
	if (Valid(in)) {
		PWSTR sSID = null_ptr;
		if (::ConvertSidToStringSidW(in, &sSID)) {
			Result = sSID;
			::LocalFree(sSID);
		}
	}
	return	Result;
}

// name to sid string
AutoUTF					Sid::AsStr(const AutoUTF &name, const AutoUTF &dom) {
	Sid		sid(name, dom);
	return	sid.AsStr();
}

// PSID to name
DWORD					Sid::AsName(PSID pSID, AutoUTF &name, AutoUTF &dom, const AutoUTF &srv) {
	DWORD	err = ERROR_INVALID_SID;
	if (Valid(pSID)) {
		PWSTR	pName = null_ptr;
		PWSTR	pDom = null_ptr;
		DWORD	dwNameSize = 0;
		DWORD	dwDomSize = 0;
		SID_NAME_USE type;

		// determine size of name
		::LookupAccountSidW(srv.c_str(), pSID, pName, &dwNameSize, pDom, &dwDomSize, &type);
		err = ::GetLastError();

		if (err == ERROR_INSUFFICIENT_BUFFER) {
			WCHAR	pName[dwNameSize];
			WCHAR	pDom[dwDomSize];

			// retrieve name
			if (::LookupAccountSidW(srv.c_str(), pSID, pName, &dwNameSize, pDom, &dwDomSize, &type)) {
				name = pName;
				dom  = pDom;
				err  = NO_ERROR;
			} else {
				err = ::GetLastError();
			}
		}
	}
	return	err;
}
AutoUTF					Sid::AsName(PSID pSID, const AutoUTF &srv) {
	AutoUTF	name, dom;
	AsName(pSID, name, dom, srv);
	return	name;
}
AutoUTF					Sid::AsFullName(PSID pSID, const AutoUTF &srv) {
	AutoUTF	name, dom;
	AsName(pSID, name, dom, srv);
	if (!dom.empty() && !name.empty()) {
		dom += L"\\";
		dom += name;
		return	dom;
	}
	return	name;
}
AutoUTF					Sid::AsDom(PSID pSID, const AutoUTF &srv) {
	AutoUTF	name, dom;
	AsName(pSID, name, dom, srv);
	return	dom;
}

/*
// WELL KNOWN SIDS
PCWSTR	Sid::SID_NOBODY				= L"S-1-0-0";			// NULL_SID
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
*/
