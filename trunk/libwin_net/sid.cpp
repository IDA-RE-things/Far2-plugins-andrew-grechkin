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
void Sid::copy(value_type in) {
	DWORD size = m_psid.size();
	CheckApi(::CopySid(size, m_psid, in));
}

void Sid::init(PCWSTR name, PCWSTR srv) {
	DWORD size_sid = 0;
	DWORD size_dom = 0;
	SID_NAME_USE type;
	::LookupAccountNameW(srv, name, nullptr, &size_sid, nullptr, &size_dom, &type);
	CheckApi(::GetLastError() == ERROR_INSUFFICIENT_BUFFER);
	m_psid.reserve(size_sid);
	WCHAR pDom[size_dom];
	CheckApi(::LookupAccountNameW(srv, name, m_psid, &size_sid, pDom, &size_dom, &type));
}

Sid::Sid(WELL_KNOWN_SID_TYPE wns): m_psid(SECURITY_MAX_SID_SIZE) {
	DWORD size = m_psid.size();
	CheckApi(::CreateWellKnownSid(wns, nullptr, m_psid, &size));
}

Sid& Sid::operator=(value_type rhs) {
	Sid(rhs).swap(*this);
	return *this;
}

Sid& Sid::operator=(const class_type &rhs) {
	if (this != &rhs)
		Sid(rhs).swap(*this);
	return *this;
}

bool Sid::operator==(value_type rhs) const {
	return check(m_psid), check(rhs), ::EqualSid(m_psid, rhs);
}

void Sid::check(value_type in) {
	if (!is_valid(in))
		CheckApiError(ERROR_INVALID_SID);
}

Sid::size_type Sid::size(value_type in) {
	return check(in), ::GetLengthSid(in);
}

Sid::size_type Sid::sub_authority_count(value_type in) {
	return check(in), *(::GetSidSubAuthorityCount(in));
}

Sid::size_type Sid::rid(value_type in) {
	size_t cnt = sub_authority_count(in);
	return *(::GetSidSubAuthority(in, cnt - 1));
}

// PSID to sid string
AutoUTF Sid::str(value_type psid) {
	check(psid);
	auto_close<PWSTR> ret(nullptr, ::LocalFree);
	CheckApi(::ConvertSidToStringSidW(psid, &ret));
	return AutoUTF(ret);
}

// PSID to name
AutoUTF Sid::str(const AutoUTF &name, PCWSTR srv) {
	return class_type(name, srv).str();
}

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
		dom += PATH_SEPARATOR;
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

//void Sid::Init(DWORD in) { // DOMAIN_ALIAS_RID_ADMINS
//	SID_IDENTIFIER_AUTHORITY NtAuthority = {SECURITY_NT_AUTHORITY};
//	PSID psid = nullptr;
//	CheckApi(::AllocateAndInitializeSid(&NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID,
//			in, 0, 0, 0, 0, 0, 0, &psid));
//	Copy(psid);
//	FreeSid(psid);
//}

void SidString::init(PCWSTR str) {
	value_type psid = nullptr;
	CheckApi(::ConvertStringSidToSidW((PWSTR)str, &psid));
	Sid(psid).swap(*this);
	::LocalFree(psid);
}

bool IsUserAdmin() {
	return	WinToken::CheckMembership(Sid(WinBuiltinAdministratorsSid), nullptr);
}

AutoUTF GetUser(HANDLE hToken) {
	AutoUTF Result;
	DWORD dwInfoBufferSize = 0;
	if (!::GetTokenInformation(hToken, TokenUser, nullptr, 0, &dwInfoBufferSize)) {
		auto_buf<PTOKEN_USER> buf(dwInfoBufferSize);
		if (::GetTokenInformation(hToken, TokenUser, buf, buf.size(), &dwInfoBufferSize)) {
			return Sid::name(buf->User.Sid);
		}
	}
	return AutoUTF();
}
