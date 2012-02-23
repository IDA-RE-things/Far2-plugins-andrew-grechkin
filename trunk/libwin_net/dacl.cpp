#include "dacl.h"
#include "exception.h"
#include "sd.h"
#include <libwin_def/win_def.h>

NamedValues<BYTE> aceTypes[] = {
	{L"ACCESS_ALLOWED_ACE_TYPE", ACCESS_ALLOWED_ACE_TYPE},
	{L"ACCESS_DENIED_ACE_TYPE", ACCESS_DENIED_ACE_TYPE},
	{L"SYSTEM_AUDIT_ACE_TYPE", SYSTEM_AUDIT_ACE_TYPE},
	{L"ACCESS_ALLOWED_OBJECT_ACE_TYPE", ACCESS_ALLOWED_OBJECT_ACE_TYPE},
	{L"ACCESS_DENIED_OBJECT_ACE_TYPE", ACCESS_DENIED_OBJECT_ACE_TYPE},
	{L"SYSTEM_AUDIT_OBJECT_ACE_TYPE", SYSTEM_AUDIT_OBJECT_ACE_TYPE},
};

NamedValues<ULONG> aceFlags[] = {
	{L"INHERITED_ACE", INHERITED_ACE},
	{L"CONTAINER_INHERIT_ACE", CONTAINER_INHERIT_ACE},
	{L"OBJECT_INHERIT_ACE", OBJECT_INHERIT_ACE},
	{L"INHERIT_ONLY_ACE", INHERIT_ONLY_ACE},
	{L"NO_PROPAGATE_INHERIT_ACE", NO_PROPAGATE_INHERIT_ACE},
	{L"FAILED_ACCESS_ACE_FLAG", FAILED_ACCESS_ACE_FLAG},
	{L"SUCCESSFUL_ACCESS_ACE_FLAG", SUCCESSFUL_ACCESS_ACE_FLAG}
};

///========================================================================================= Trustee
trustee_t::trustee_t(PCWSTR name) {
	::BuildTrusteeWithNameW(this, (PWSTR)name);
}

trustee_t::trustee_t(PSID sid) {
	::BuildTrusteeWithSidW(this, sid);
}

///======================================================================================= ExpAccess
ExpAccess::ExpAccess(PCWSTR name, ACCESS_MASK acc, ACCESS_MODE mode, DWORD inh) {
	::BuildExplicitAccessWithNameW(this, (PWSTR)name, acc, mode, inh);
}

ustring ExpAccess::get_name() const {
	TRUSTEE_FORM tf = ::GetTrusteeFormW((PTRUSTEEW)&Trustee);
	if (tf == TRUSTEE_IS_NAME)
		return ustring(::GetTrusteeNameW((PTRUSTEEW)&Trustee));
	else if (tf != TRUSTEE_IS_SID)
		CheckApiError(ERROR_INVALID_PARAMETER);
	return Sid((PSID)Trustee.ptstrName).get_name();
}

Sid ExpAccess::get_sid() const {
	if (::GetTrusteeFormW((PTRUSTEEW)&Trustee) != TRUSTEE_IS_SID)
		CheckApiError(ERROR_INVALID_PARAMETER);
	return Sid((PSID)Trustee.ptstrName);
}

ExpAccessArray::~ExpAccessArray() {
	::LocalFree(m_eacc);
}

ExpAccessArray::ExpAccessArray(PACL acl) {
	CheckApiError(::GetExplicitEntriesFromAclW(acl, &m_size, (PEXPLICIT_ACCESSW*)&m_eacc));
}

ExpAccess & ExpAccessArray::operator [](size_t index) const {
	return m_eacc[index];
}

size_t ExpAccessArray::size() const {
	return m_size;
}

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
		set(path.c_str(), sd.Dacl(), UNPROTECTED_DACL_SECURITY_INFORMATION, type);
}

void WinDacl::protect(const ustring &path, SE_OBJECT_TYPE type) {
	WinSDW sd(path);
	if (!sd.is_protected()) {
		WinDacl::del_inherited_aces(sd.Dacl());
		set(path.c_str(), sd.Dacl(), PROTECTED_DACL_SECURITY_INFORMATION, type);
	}
}

void WinDacl::protect_copy(const ustring &path, SE_OBJECT_TYPE type) {
	WinSDW sd(path);
	if (!sd.is_protected())
		set(path.c_str(), sd.Dacl(), PROTECTED_DACL_SECURITY_INFORMATION, type);
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
	WinMem::Copy(m_dacl, acl, size);
	return m_dacl;
}

PACL WinDacl::copy(PSECURITY_DESCRIPTOR sd) {
	BOOL bDaclPresent = false;
	BOOL bDaclDefaulted = false;
	PACL acl = nullptr;
	CheckApi(::GetSecurityDescriptorDacl(sd, &bDaclPresent, &acl, &bDaclDefaulted));
	return copy(acl);
}

#ifndef NDEBUG
ustring as_str(PACL acl) {
	ustring Result = L"DACL:";
	if (!acl) {
		Result += L"\tNULL\nAll access allowed\n";
		return Result;
	}
	ACL_SIZE_INFORMATION aclSize;
	CheckApi(::GetAclInformation(acl, &aclSize, sizeof(aclSize), AclSizeInformation));
	if (aclSize.AceCount == 0)
		Result += L" empty";

	Result += ustring(L"\tACE count: ") + as_str(aclSize.AceCount) +
			  L"\tUse: " + as_str(aclSize.AclBytesInUse) + L" bytes"
			  L"\tFree: " + as_str(aclSize.AclBytesFree) + L" bytes" + L"\n";
	for (ULONG lIndex = 0; lIndex < aclSize.AceCount; ++lIndex) {
		ACCESS_ALLOWED_ACE *pACE;
		if (!::GetAce(acl, lIndex, (PVOID*)&pACE))
			return Result;

		Result += ustring(L"ACE [") + as_str(lIndex) + L"]\n";

		PSID pSID = PSIDFromPACE(pACE);
		Result = Result + L"\tACE Name: " + Sid::get_name(pSID).c_str() + L" (" + Sid::as_str(pSID) + L")";

		ULONG lIndex2 = 6;
		PCWSTR pszString = L"Unknown ACE Type";
		while (lIndex2--) {
			if (pACE->Header.AceType == aceTypes[lIndex2].value) {
				pszString = aceTypes[lIndex2].name;
				break;
			}
		}
		Result += ustring(L"\tAceType: ") + pszString;
		Result += L"\n\tACE Flags:";
		lIndex2 = 7;
		while (lIndex2--) {
			if ((pACE->Header.AceFlags & aceFlags[lIndex2].value) != 0) {
				Result += ustring(L" ") + aceFlags[lIndex2].name;
			}
		}
		Result += L"\n\tACE Mask: ";
		Result += BitMask<DWORD>::as_str_bin(pACE->Mask);
		Result += L"\n";
	}
	return Result;
}
#endif
