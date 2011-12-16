/**
	net_sid
	SID utilites
	@classes	(Sid)
	@author		2010 Andrew Grechkin
	@link		()
**/

#include "win_net.h"
#include "exception.h"

extern "C" {
	WINADVAPI BOOL WINAPI ConvertSidToStringSidW(IN PSID Sid, OUT LPWSTR *StringSid);
	WINADVAPI BOOL WINAPI ConvertStringSidToSidW(IN LPCWSTR StringSid, OUT PSID *Sid);
}

///============================================================================================= Sid
void Sid::init(value_type in) {
	DWORD size = class_type::size(in);
	m_sid = (value_type)::LocalAlloc(LPTR, size);
	CheckApi(::CopySid(size, m_sid, in));
}

void Sid::init(PCWSTR name, PCWSTR srv) {
	DWORD size_sid = SECURITY_MAX_SID_SIZE;
	m_sid = (value_type)::LocalAlloc(LPTR, size_sid);
	DWORD size_dom = 64;
	WCHAR pDom[size_dom];
	SID_NAME_USE type;
	CheckApi(::LookupAccountNameW(srv, name, m_sid, &size_sid, pDom, &size_dom, &type));
}

Sid::~Sid() {
	if (m_sid)
		::LocalFree(m_sid);
}

Sid::Sid(WELL_KNOWN_SID_TYPE wns): m_sid((value_type)::LocalAlloc(LPTR, SECURITY_MAX_SID_SIZE)) {
	DWORD size = SECURITY_MAX_SID_SIZE;
	CheckApi(::CreateWellKnownSid(wns, nullptr, m_sid, &size));
}

Sid::Sid(value_type rhs):
	m_sid(nullptr) {
	init(rhs);
}

Sid::Sid(const class_type &rhs):
	m_sid(nullptr) {
	init(rhs);
}

Sid::Sid(PCWSTR name, PCWSTR srv):
	m_sid(nullptr) {
	init(name, srv);
}

Sid::Sid(const ustring &name, PCWSTR srv):
	m_sid(nullptr) {
	init(name.c_str(), srv);
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
	return check(m_sid), check(rhs), ::EqualSid(m_sid, rhs);
}

void Sid::copy_to(value_type out, size_t size) const {
	CheckApi(::CopySid(size, out, m_sid));
}

void Sid::check(value_type in) {
	if (!is_valid(in))
		CheckApiError(ERROR_INVALID_SID);
}

void Sid::detach(value_type &sid) {
	sid = m_sid;
	m_sid = nullptr;
}

void Sid::swap(class_type &rhs) {
	using std::swap;
	swap(m_sid, rhs.m_sid);
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
ustring Sid::str(value_type psid) {
	check(psid);
	auto_close<PWSTR> ret(nullptr, ::LocalFree);
	CheckApi(::ConvertSidToStringSidW(psid, &ret));
	return ustring(ret);
}

// PSID to name
ustring Sid::str(const ustring &name, PCWSTR srv) {
	return class_type(name, srv).str();
}

void Sid::name(value_type pSID, ustring &name, ustring &dom, PCWSTR srv) {
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

ustring Sid::name(value_type pSID, PCWSTR srv) {
	ustring nam, dom;
	name(pSID, nam, dom, srv);
	return nam;
}

ustring Sid::full_name(value_type pSID, PCWSTR srv) {
	ustring nam, dom;
	name(pSID, nam, dom, srv);
	if (!dom.empty() && !nam.empty()) {
		dom.reserve(dom.size() + nam.size() + 1);
		dom += PATH_SEPARATOR;
		dom += nam;
		return dom;
	}
	return nam;
}

ustring Sid::domain(value_type pSID, PCWSTR srv) {
	ustring nam, dom;
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
	CheckApi(::ConvertStringSidToSidW((PWSTR)str, &m_sid));
}

//bool is_admin() {
//	return WinToken::CheckMembership(Sid(WinBuiltinAdministratorsSid), nullptr);
//}

ustring get_token_user(HANDLE hToken) {
	DWORD size = 0;
	if (!::GetTokenInformation(hToken, TokenUser, nullptr, 0, &size) && size) {
		auto_buf<PTOKEN_USER> buf(size);
		CheckApi(::GetTokenInformation(hToken, TokenUser, buf, buf.size(), &size));
		return Sid::name(buf->User.Sid);
	}
	return ustring();
}
