/**
	net_sid
	SID utilites
	@classes (Sid)
	@author © 2012 Andrew Grechkin
	@link ()
 **/

#include <libbase/std.hpp>
#include <libbase/memory.hpp>
//#include <libbase/str.hpp>
#include <libext/sid.hpp>
#include <libext/exception.hpp>

extern "C" {
	WINADVAPI BOOL WINAPI ConvertStringSidToSidW(PCWSTR StringSid, PSID * Sid);
}

using namespace Base;

namespace Ext {

	///============================================================================================= Sid
	Sid::~Sid() {
		::LocalFree(m_sid);
	}

	Sid::Sid(WELL_KNOWN_SID_TYPE wns):
		m_sid(get_sid(wns)) {
	}

	Sid::Sid(value_type rhs):
		m_sid(this_type::clone(rhs)) {
	}

	Sid::Sid(PCWSTR name, PCWSTR srv):
		m_sid(this_type::get_sid(name, srv)) {
	}

	Sid::Sid(const ustring & name, PCWSTR srv):
		m_sid(this_type::get_sid(name.c_str(), srv)) {
	}

	Sid::Sid(const this_type & rhs):
		m_sid(this_type::clone(rhs.m_sid)) {
	}

	Sid::Sid(this_type && rhs):
		m_sid(nullptr)
	{
		using std::swap;
		swap(m_sid, rhs.m_sid);
	}

	Sid & Sid::operator = (const this_type & rhs) {
		if (this != &rhs)
			this_type(rhs).swap(*this);
		return *this;
	}

	Sid & Sid::operator = (this_type && rhs) {
		if (this != &rhs)
			this_type(std::move(rhs)).swap(*this);
		return *this;
	}

	Sid & Sid::operator = (value_type rhs) {
		this_type(rhs).swap(*this);
		return *this;
	}

	bool Sid::operator == (value_type rhs) const {
		return check(rhs), ::EqualSid(m_sid, rhs);
	}

	void Sid::copy_to(value_type out, size_t size) const {
		CheckApi(::CopySid(size, out, m_sid));
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

	Sid::size_type Sid::sub_authority_count(value_type in) {
		return check(in), *(::GetSidSubAuthorityCount(in));
	}

	Sid::size_type Sid::get_rid(value_type in) {
		size_t cnt = sub_authority_count(in);
		return *(::GetSidSubAuthority(in, cnt - 1));
	}

	PSID Sid::clone(value_type in) {
		DWORD size = this_type::size(in);
		PSID sid = (value_type)::LocalAlloc(LPTR, size);
		CheckApi(::CopySid(size, sid, in));
		return sid;
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

}
