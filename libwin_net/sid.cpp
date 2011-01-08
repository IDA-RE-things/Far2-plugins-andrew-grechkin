/**
	net_sid
	SID utilites
	@classes	(Sid)
	@author		2010 Andrew Grechkin
	@link		()
**/

#include "win_net.h"

extern "C" {
	WINADVAPI BOOL WINAPI ConvertSidToStringSidW(IN PSID Sid, OUT LPWSTR *StringSid);
	WINADVAPI BOOL WINAPI ConvertStringSidToSidW(IN LPCWSTR StringSid, OUT PSID *Sid);
}

///============================================================================================= Sid
Sid::Sid(WELL_KNOWN_SID_TYPE wns) {
	DWORD size = SECURITY_MAX_SID_SIZE;
	m_psid.reserve(size);
	CheckApi(::CreateWellKnownSid(wns, nullptr, m_psid, &size));
}

Sid::class_type& Sid::operator=(const class_type &rhs) {
	if (this != &rhs)
		Copy(rhs.m_psid);
	return *this;
}

// PSID to sid string
AutoUTF Sid::str(value_type psid) {
	check(psid);
	auto_close<PWSTR> ret(nullptr, ::LocalFree);
	CheckApi(::ConvertSidToStringSidW(psid, &ret));
	return AutoUTF(ret);
}

// PSID to name
void Sid::name(value_type pSID, AutoUTF &name, AutoUTF &dom, PCWSTR srv) {
	check(pSID);
	DWORD size_nam = 0;
	DWORD size_dom = 0;
	SID_NAME_USE type;

	// determine size of name
	::LookupAccountSidW(srv, pSID, nullptr, &size_nam, nullptr, &size_dom, &type);
	CheckApi(::GetLastError() == ERROR_INSUFFICIENT_BUFFER);
	WCHAR pName[size_nam];
	WCHAR pDom[size_dom];

	CheckApi(::LookupAccountSidW(srv, pSID, pName, &size_nam, pDom, &size_dom, &type));
	name = pName;
	dom = pDom;
}

AutoUTF Sid::name(value_type pSID, PCWSTR srv) {
	AutoUTF nam, dom;
	name(pSID, nam, dom, srv);
	return nam;
}

AutoUTF Sid::full_name(value_type pSID, PCWSTR srv) {
	AutoUTF nam, dom;
	name(pSID, nam, dom, srv);
	if (!dom.empty() && !nam.empty()) {
		dom.reserve(dom.size() + nam.size() + 1);
		dom += L"\\";
		dom += nam;
		return dom;
	}
	return nam;
}

AutoUTF Sid::domain(value_type pSID, PCWSTR srv) {
	AutoUTF nam, dom;
	name(pSID, nam, dom, srv);
	return dom;
}

void Sid::Copy(value_type in) {
	DWORD size = class_type::size(in);
	auto_buf<value_type> tmp(size);
	CheckApi(::CopySid(size, tmp, in));
	::swap(tmp, m_psid);
}

void Sid::Init(PCWSTR name, PCWSTR srv) {
	DWORD size_sid = 0;
	DWORD size_dom = 0;
	SID_NAME_USE type;
	::LookupAccountNameW(srv, name, nullptr, &size_sid, nullptr, &size_dom, &type);
	CheckApi(::GetLastError() == ERROR_INSUFFICIENT_BUFFER);
	m_psid.reserve(size_sid);
	WCHAR pDom[size_dom];
	CheckApi(::LookupAccountNameW(srv, name, m_psid, &size_sid, pDom, &size_dom, &type));
}

void Sid::Init(DWORD in) { // DOMAIN_ALIAS_RID_ADMINS
	SID_IDENTIFIER_AUTHORITY NtAuthority = {SECURITY_NT_AUTHORITY};
	PSID psid = nullptr;
	CheckApi(::AllocateAndInitializeSid(&NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID,
			in, 0, 0, 0, 0, 0, 0, &psid));
	Copy(psid);
	FreeSid(psid);
}


SidString::SidString(PCWSTR str) {
	value_type psid = nullptr;
	CheckApi(::ConvertStringSidToSidW((PWSTR)str, &psid));
	Copy(psid);
	::LocalFree(psid);
}
SidString::SidString(const AutoUTF &str) {
	value_type psid = nullptr;
	CheckApi(::ConvertStringSidToSidW((PWSTR)str.c_str(), &psid));
	Copy(psid);
	::LocalFree(psid);
}

AutoUTF GetUser(HANDLE hToken) {
	AutoUTF Result;
	DWORD dwInfoBufferSize = 0;
	if (!::GetTokenInformation(hToken, TokenUser, nullptr, 0, &dwInfoBufferSize)) {
		WinBuf<TOKEN_USER> buf(dwInfoBufferSize, true);
		if (::GetTokenInformation(hToken, TokenUser, buf, buf.capacity(), &dwInfoBufferSize)) {
			return Sid::name(buf->User.Sid);
		}
	}
	return AutoUTF();
}
