/**
	net_sid
	SID utilites
	@classes (Sid)
	@author © 2012 Andrew Grechkin
	@link ()
**/

#include "sid.h"
#include "exception.h"

extern "C" {
	WINADVAPI BOOL WINAPI ConvertSidToStringSidW(PSID Sid, PWSTR * StringSid);
	WINADVAPI BOOL WINAPI ConvertStringSidToSidW(PCWSTR StringSid, PSID * Sid);
}

///============================================================================================= Sid
Sid::~Sid() {
	::LocalFree(m_sid);
}

Sid::Sid(WELL_KNOWN_SID_TYPE wns):
	m_sid(get_sid(wns)) {
}

Sid::Sid(value_type rhs):
	m_sid(this_type::copy(rhs)) {
}

Sid::Sid(PCWSTR name, PCWSTR srv):
	m_sid(this_type::get_sid(name, srv)) {
}

Sid::Sid(const ustring & name, PCWSTR srv):
	m_sid(this_type::get_sid(name.c_str(), srv)) {
}

Sid::Sid(const this_type & rhs):
	m_sid(this_type::copy(rhs.m_sid)) {
}

Sid & Sid::operator =(const this_type & rhs) {
	if (this != &rhs)
		this_type(rhs).swap(*this);
	return *this;
}

Sid & Sid::operator =(value_type rhs) {
	this_type(rhs).swap(*this);
	return *this;
}

bool Sid::operator ==(value_type rhs) const {
	return check(rhs), ::EqualSid(m_sid, rhs);
}

void Sid::copy_to(value_type out, size_t size) const {
	CheckApi(::CopySid(size, out, m_sid));
}

bool Sid::is_valid(value_type in) {
	return in && ::IsValidSid(in);
}

void Sid::check(value_type in) {
	CheckApiThrowError(is_valid(in), ERROR_INVALID_SID);
}

void Sid::detach(value_type & sid) {
	sid = get_sid(WinNullSid);
	using std::swap;
	swap(m_sid, sid);
}

void Sid::swap(this_type & rhs) {
	using std::swap;
	swap(m_sid, rhs.m_sid);
}

Sid::size_type Sid::size(value_type in) {
	return check(in), ::GetLengthSid(in);
}

Sid::size_type Sid::sub_authority_count(value_type in) {
	return check(in), *(::GetSidSubAuthorityCount(in));
}

Sid::size_type Sid::get_rid(value_type in) {
	size_t cnt = sub_authority_count(in);
	return *(::GetSidSubAuthority(in, cnt - 1));
}

// PSID to sid string
ustring Sid::as_str(value_type psid) {
	check(psid);
	auto_close<PWSTR> ret(nullptr, ::LocalFree);
	CheckApi(::ConvertSidToStringSidW(psid, &ret));
	return ustring(ret);
}

void Sid::get_name_dom(value_type sid, ustring & name, ustring & dom, PCWSTR srv) {
	check(sid);
	DWORD size_nam = 0;
	DWORD size_dom = 0;
	SID_NAME_USE type;

	// determine size of name
	::LookupAccountSidW(srv, sid, nullptr, &size_nam, nullptr, &size_dom, &type);
	CheckApi(::GetLastError() == ERROR_INSUFFICIENT_BUFFER);
	WCHAR pName[size_nam];
	WCHAR pDom[size_dom];

	CheckApi(::LookupAccountSidW(srv, sid, pName, &size_nam, pDom, &size_dom, &type));
	name = pName;
	dom = pDom;
}

ustring Sid::get_name(value_type sid, PCWSTR srv) {
	ustring nam, dom;
	get_name_dom(sid, nam, dom, srv);
	return nam;
}

ustring Sid::get_full_name(value_type sid, PCWSTR srv) {
	ustring nam, dom;
	get_name_dom(sid, nam, dom, srv);
	if (!dom.empty() && !nam.empty()) {
		dom.reserve(dom.size() + nam.size() + 1);
		dom += PATH_SEPARATOR;
		dom += nam;
		return dom;
	}
	return nam;
}

ustring Sid::get_domain(value_type sid, PCWSTR srv) {
	ustring nam, dom;
	get_name_dom(sid, nam, dom, srv);
	return dom;
}

PSID Sid::copy(value_type in) {
	DWORD size = this_type::size(in);
	PSID m_sid = (value_type)::LocalAlloc(LPTR, size);
	CheckApi(::CopySid(size, m_sid, in));
	return m_sid;
}

PSID Sid::get_sid(WELL_KNOWN_SID_TYPE wns) {
	DWORD size = SECURITY_MAX_SID_SIZE;
	PSID m_sid = (value_type)::LocalAlloc(LPTR, SECURITY_MAX_SID_SIZE);
	CheckApi(::CreateWellKnownSid(wns, nullptr, m_sid, &size));
	return m_sid;
}

PSID Sid::get_sid(PCWSTR name, PCWSTR srv) {
	DWORD size_sid = SECURITY_MAX_SID_SIZE;
	PSID m_sid = (value_type)::LocalAlloc(LPTR, size_sid);
	DWORD size_dom = MAX_PATH;
	WCHAR dom[size_dom];
	SID_NAME_USE type;
	CheckApi(::LookupAccountNameW(srv, name, m_sid, &size_sid, dom, &size_dom, &type));
	return m_sid;
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
		return Sid::get_name(buf->User.Sid);
	}
	return ustring();
}
