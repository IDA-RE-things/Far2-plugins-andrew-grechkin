#include <libext/dacl.hpp>
#include <libext/exception.hpp>
#include <libext/sd.hpp>
#include <libbase/std.hpp>
#include <libbase/bit_str.hpp>

using namespace Base;

namespace Ext {

	///========================================================================================= WinDacl
	WinDacl::~WinDacl() {
		::LocalFree(m_dacl);
	}

	WinDacl::WinDacl(size_t size):
		m_dacl(WinDacl::create(size)) {
	}

	WinDacl::WinDacl(PACL acl):
		m_dacl(WinDacl::copy(acl)) {
		CheckApi(is_valid(m_dacl));
	}

	WinDacl::WinDacl(PSECURITY_DESCRIPTOR sd):
		m_dacl(WinDacl::copy(sd)) {
		CheckApi(is_valid(m_dacl));
	}

	WinDacl::WinDacl(const ustring & name, SE_OBJECT_TYPE type):
		m_dacl(WinDacl::copy(WinSDW(name, type))) {
		CheckApi(is_valid(m_dacl));
	}

	WinDacl::WinDacl(const this_class & rhs):
		m_dacl(WinDacl::copy(rhs.m_dacl)) {
		CheckApi(is_valid(m_dacl));
	}

	WinDacl::this_class & WinDacl::operator =(const this_class & rhs) {
		if (this != &rhs)
			this_class(rhs).swap(*this);
		return *this;
	}

	WinDacl::operator PACL() const {
		return m_dacl;
	}

	//PACL* WinDacl::operator&() {
	//	if (m_dacl) {
	//		::LocalFree(m_dacl);
	//		m_dacl = nullptr;
	//	}
	//	return &m_dacl;
	//}

	//void	WinDacl::AddA(const Sid &sid) {
	//	CheckApi(::AddAccessAllowedAce(m_dacl, ACL_REVISION, GENERIC_READ | GENERIC_WRITE, sid));
	//}
	//
	//void	WinDacl::AddD(const Sid &sid) {
	//	CheckApi(::AddAccessDeniedAce(m_dacl, ACL_REVISION, GENERIC_READ, sid));
	//}

	void WinDacl::set_entries(const ExpAccess & ea) {
		PACL new_acl(nullptr);
		CheckApiError(::SetEntriesInAclW(1, (PEXPLICIT_ACCESSW)&ea, m_dacl, &new_acl));
		attach(new_acl);
	}

	void WinDacl::set_access(PCWSTR name, ACCESS_MASK acc) {
		WinDacl::set_entries(ExpAccess(name, acc, SET_ACCESS));
	}

	void WinDacl::revoke_access(PCWSTR name) {
		WinDacl::set_entries(ExpAccess(name, GENERIC_ALL, REVOKE_ACCESS));
	}

	void WinDacl::grant_access(PCWSTR name, ACCESS_MASK acc) {
		WinDacl::set_entries(ExpAccess(name, acc, GRANT_ACCESS));
	}

	void WinDacl::deny_access(PCWSTR name, ACCESS_MASK acc) {
		WinDacl::set_entries(ExpAccess(name, acc, DENY_ACCESS));
	}

	void WinDacl::set_to(DWORD flag, const ustring & name, SE_OBJECT_TYPE type) const {
		WinDacl::set(name.c_str(), m_dacl, flag, type);
	}

	size_t WinDacl::count() const {
		return count(m_dacl);
	}

	size_t WinDacl::size() const {
		return size(m_dacl);
	}

	void WinDacl::attach(PACL & acl) {
		swap(acl);
		::LocalFree(acl);
		acl = nullptr;
	}

	void WinDacl::detach(PACL & acl) {
		acl = WinDacl::create(64);
		using std::swap;
		swap(m_dacl, acl);
	}

	void WinDacl::swap(PACL & acl) {
		CheckApi(is_valid(acl));
		using std::swap;
		swap(m_dacl, acl);
	}

	void WinDacl::swap(WinDacl & rhs) {
		using std::swap;
		swap(m_dacl, rhs.m_dacl);
	}

	void WinDacl::del_inherited_aces() {
		WinDacl::del_inherited_aces(m_dacl);
	}

	//size_t	WinDacl::count(PACL acl, size_t &sz, bool inh) {
	//	ACL_SIZE_INFORMATION	info;
	//	get_info(acl, info);
	//	sz = 0;
	//	size_t	Result = 0;
	//	for (DWORD i = 0; i < info.AceCount; ++i) {
	//		ACCESS_ALLOWED_ACE	*pACE = (ACCESS_ALLOWED_ACE*)get_ace(acl, i);
	//		if (inh || !(pACE->Header.AceFlags & INHERITED_ACE)) {
	//			PSID pSID = PSIDFromPACE(pACE);
	//			++Result;
	//			sz += Sid::size(pSID);
	//		}
	//	}
	//	return Result;
	//}

	bool WinDacl::is_valid(PACL in) {
		return ::IsValidAcl(in);
	}

	void WinDacl::get_info(PACL acl, ACL_SIZE_INFORMATION & out) {
		CheckApi(is_valid(acl));
		CheckApi(::GetAclInformation(acl, &out, sizeof(out), AclSizeInformation));
	}

	size_t WinDacl::count(PACL acl) {
		ACL_SIZE_INFORMATION info;
		get_info(acl, info);
		return info.AceCount;
	}

	size_t WinDacl::used_bytes(PACL acl) {
		ACL_SIZE_INFORMATION	info;
		get_info(acl, info);
		return info.AclBytesInUse;
	}

	size_t WinDacl::free_bytes(PACL acl) {
		ACL_SIZE_INFORMATION	info;
		get_info(acl, info);
		return info.AclBytesFree;
	}

	size_t WinDacl::size(PACL acl) {
		CheckApi(is_valid(acl));
		ACL_SIZE_INFORMATION	info;
		get_info(acl, info);
		return info.AclBytesFree + info.AclBytesInUse;
	}

	PACCESS_ALLOWED_ACE WinDacl::get_ace(PACL acl, size_t index) {
		PVOID Result = nullptr;
		CheckApi(::GetAce(acl, index, &Result));
		return (PACCESS_ALLOWED_ACE)Result;
	}

	void WinDacl::del_inherited_aces(PACL acl) {
		ACL_SIZE_INFORMATION info;
		get_info(acl, info);
		for (ssize_t i = (ssize_t)info.AceCount - 1; i >= 0; --i) {
			PACCESS_ALLOWED_ACE ace = get_ace(acl, i);
			if (ace->Header.AceFlags & INHERITED_ACE)
				CheckApi(::DeleteAce(acl, i));
		}
	}

	void WinDacl::set(PCWSTR path, PACL dacl, DWORD flag, SE_OBJECT_TYPE type) {
		CheckApiError(::SetNamedSecurityInfoW((PWSTR)path, type,
		                                      DACL_SECURITY_INFORMATION | flag,
		                                      nullptr, nullptr, dacl, nullptr));
	}

	void WinDacl::set_inherit(PCWSTR path, PACL dacl, SE_OBJECT_TYPE type) {
		set(path, dacl, UNPROTECTED_DACL_SECURITY_INFORMATION, type);
	}

	void WinDacl::set_protect(PCWSTR path, PACL dacl, SE_OBJECT_TYPE type) {
		set(path, dacl, PROTECTED_DACL_SECURITY_INFORMATION, type);
	}

	void WinDacl::set_protect_copy(PCWSTR path, PACL dacl, SE_OBJECT_TYPE type) {
		set(path, dacl, PROTECTED_DACL_SECURITY_INFORMATION, type);
	}

	void WinDacl::inherit(const ustring & path, SE_OBJECT_TYPE type) {
		WinSDW sd(path);
		if (sd.is_protected())
			set(path.c_str(), sd.get_dacl(), UNPROTECTED_DACL_SECURITY_INFORMATION, type);
	}

	void WinDacl::protect(const ustring &path, SE_OBJECT_TYPE type) {
		WinSDW sd(path);
		if (!sd.is_protected()) {
			WinDacl::del_inherited_aces(sd.get_dacl());
			set(path.c_str(), sd.get_dacl(), PROTECTED_DACL_SECURITY_INFORMATION, type);
		}
	}

	void WinDacl::protect_copy(const ustring & path, SE_OBJECT_TYPE type) {
		WinSDW sd(path);
		if (!sd.is_protected())
			set(path.c_str(), sd.get_dacl(), PROTECTED_DACL_SECURITY_INFORMATION, type);
	}

	PACL WinDacl::create(size_t size) {
		PACL acl = (PACL)CheckPointer(::LocalAlloc(LPTR, size));
		CheckApi(::InitializeAcl(acl, size, ACL_REVISION));
		CheckApi(is_valid(acl));
		return acl;
	}

	PACL WinDacl::copy(PACL acl) {
		size_t	size = WinDacl::size(acl);
		PACL m_dacl = (PACL)CheckPointer(::LocalAlloc(LPTR, size));
		Memory::copy(m_dacl, acl, size);
		return m_dacl;
	}

	PACL WinDacl::copy(PSECURITY_DESCRIPTOR sd) {
		BOOL bDaclPresent = false;
		BOOL bDaclDefaulted = false;
		PACL acl = nullptr;
		CheckApi(::GetSecurityDescriptorDacl(sd, &bDaclPresent, &acl, &bDaclDefaulted));
		return copy(acl);
	}

}
