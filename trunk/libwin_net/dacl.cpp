#include "win_net.h"
#include "exception.h"

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
Trustee::Trustee(PCWSTR name) {
	::BuildTrusteeWithNameW(this, (PWSTR)name);
}

Trustee::Trustee(PSID sid) {
	::BuildTrusteeWithSidW(this, sid);
}

///======================================================================================= ExpAccess
ExpAccess::ExpAccess(PCWSTR name, ACCESS_MASK acc, ACCESS_MODE mode, DWORD inh) {
	::BuildExplicitAccessWithNameW(this, (PWSTR)name, acc, mode, inh);
}

ustring ExpAccess::get_name() const {
	if (::GetTrusteeFormW((PTRUSTEEW)&Trustee) == TRUSTEE_IS_NAME)
		return ustring(::GetTrusteeNameW((PTRUSTEEW)&Trustee));
	return Sid((PSID)Trustee.ptstrName).name();
}

ustring ExpAccess::get_fullname() const {
	if (::GetTrusteeFormW((PTRUSTEEW)&Trustee) == TRUSTEE_IS_NAME)
		return ustring(::GetTrusteeNameW((PTRUSTEEW)&Trustee));
	return Sid((PSID)Trustee.ptstrName).full_name();
}

Sid		ExpAccess::get_sid() const {
	if (::GetTrusteeFormW((PTRUSTEEW)&Trustee) == TRUSTEE_IS_SID)
		return Sid((PSID)Trustee.ptstrName);
	return Sid(Trustee.ptstrName);
}

ExpAccessArray::~ExpAccessArray() {
	::LocalFree(m_eacc);
}

ExpAccessArray::ExpAccessArray(PACL acl) {
	CheckApiError(::GetExplicitEntriesFromAclW(acl, &m_cnt, (PEXPLICIT_ACCESSW*)&m_eacc));
}

///========================================================================================= WinDacl
void	WinDacl::Init(PACL acl) {
	size_t	size = WinDacl::size(acl);
	m_dacl = (PACL)CheckPointer(::LocalAlloc(LPTR, size));
	WinMem::Copy(m_dacl, acl, size);
}

void	WinDacl::Init(PSECURITY_DESCRIPTOR sd) {
	BOOL	bDaclPresent   = false;
	BOOL	bDaclDefaulted = false;
	PACL	acl = nullptr;
	CheckApi(::GetSecurityDescriptorDacl(sd, &bDaclPresent, &acl, &bDaclDefaulted));
	Init(acl);
}

WinDacl::~WinDacl() {
	if (m_dacl)
		::LocalFree(m_dacl);
}

WinDacl::WinDacl(size_t size): m_dacl(nullptr) {
	m_dacl = WinDacl::create(size);
}

WinDacl::WinDacl(PACL acl): m_dacl(nullptr) {
	Init(acl);
	CheckApi(is_valid(m_dacl));
}

WinDacl::WinDacl(PSECURITY_DESCRIPTOR pSD): m_dacl(nullptr) {
	Init(pSD);
	CheckApi(is_valid(m_dacl));
}

WinDacl::WinDacl(const ustring &name, SE_OBJECT_TYPE type): m_dacl(nullptr) {
	PSECURITY_DESCRIPTOR pSD = nullptr;
	PACL	tmp = nullptr;
	CheckApiError(::GetNamedSecurityInfoW((PWSTR)name.c_str(), type,
									   DACL_SECURITY_INFORMATION,
									   nullptr, nullptr, &tmp, nullptr, &pSD));
	Init(tmp);
	::LocalFree(pSD);
	CheckApi(is_valid(m_dacl));
}

PACL* WinDacl::operator&() {
	if (m_dacl) {
		::LocalFree(m_dacl);
		m_dacl = nullptr;
	}
	return &m_dacl;
}

//void	WinDacl::AddA(const Sid &sid) {
//	CheckApi(::AddAccessAllowedAce(m_dacl, ACL_REVISION, GENERIC_READ | GENERIC_WRITE, sid));
//}
//
//void	WinDacl::AddD(const Sid &sid) {
//	CheckApi(::AddAccessDeniedAce(m_dacl, ACL_REVISION, GENERIC_READ, sid));
//}

void	WinDacl::Add(const ExpAccess &ea) {
	WinDacl new_dacl;
	CheckApiError(::SetEntriesInAclW(1, (PEXPLICIT_ACCESSW)&ea, m_dacl, &new_dacl));
	swap(new_dacl);
}

void	WinDacl::Set(PCWSTR name, ACCESS_MASK acc) {
	WinDacl::Add(ExpAccess(name, acc, SET_ACCESS));
}

void	WinDacl::Revoke(PCWSTR name) {
	WinDacl::Add(ExpAccess(name, GENERIC_ALL, REVOKE_ACCESS));
}

void	WinDacl::Grant(PCWSTR name, ACCESS_MASK acc) {
	WinDacl::Add(ExpAccess(name, acc, GRANT_ACCESS));
}

void	WinDacl::Deny(PCWSTR name, ACCESS_MASK acc) {
	WinDacl::Add(ExpAccess(name, acc, DENY_ACCESS));
}

void	WinDacl::get_info(PACL acl, ACL_SIZE_INFORMATION &out) {
	CheckApi(is_valid(acl));
	CheckApi(::GetAclInformation(acl, &out, sizeof(out), AclSizeInformation));
}

size_t	WinDacl::count(PACL acl) {
	ACL_SIZE_INFORMATION info;
	get_info(acl, info);
	return info.AceCount;
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

size_t	WinDacl::used_bytes(PACL acl) {
	ACL_SIZE_INFORMATION	info;
	get_info(acl, info);
	return info.AclBytesInUse;
}

size_t	WinDacl::free_bytes(PACL acl) {
	ACL_SIZE_INFORMATION	info;
	get_info(acl, info);
	return info.AclBytesFree;
}

size_t	WinDacl::size(PACL acl) {
	CheckApi(is_valid(acl));
	ACL_SIZE_INFORMATION	info;
	get_info(acl, info);
	return info.AclBytesFree + info.AclBytesInUse;
}

PVOID	WinDacl::get_ace(PACL acl, size_t index) {
	PVOID	Result = nullptr;
	CheckApi(::GetAce(acl, index, &Result));
	return Result;
}

ustring WinDacl::Parse(PACL acl) {
	ustring Result = L"DACL:";
	if (!acl) {
		Result += L"\tNULL\nAll access allowed\n";
		return Result;
	}
	ACL_SIZE_INFORMATION aclSize;
	CheckApi(::GetAclInformation(acl, &aclSize, sizeof(aclSize), AclSizeInformation));
	if (aclSize.AceCount == 0)
		Result += L" empty";

	Result += ustring(L"\tACE count: ") + Num2Str(aclSize.AceCount) +
			  L"\tUse: " + Num2Str(aclSize.AclBytesInUse) + L" bytes"
			  L"\tFree: " + Num2Str(aclSize.AclBytesFree) + L" bytes" + L"\n";
	for (ULONG lIndex = 0; lIndex < aclSize.AceCount; ++lIndex) {
		ACCESS_ALLOWED_ACE *pACE;
		if (!::GetAce(acl, lIndex, (PVOID*)&pACE))
			return Result;

		Result += ustring(L"ACE [") + Num2Str(lIndex) + L"]\n";

		PSID pSID = PSIDFromPACE(pACE);
		Result = Result + L"\tACE Name: " + Sid::name(pSID).c_str() + L" (" + Sid::str(pSID).c_str() + L")";

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
		Result += BitMask<DWORD>::AsStrBin(pACE->Mask);
		Result += L"\n";
	}
	return Result;
}

void	WinDacl::detach(PACL &acl) {
	acl = m_dacl;
	m_dacl = nullptr;
}

void WinDacl::swap(WinDacl &rhs) {
	using std::swap;
	swap(m_dacl, rhs.m_dacl);
}

void WinDacl::del_inherited_aces(PACL acl) {
	ACL_SIZE_INFORMATION	info;
	get_info(acl, info);
	for (ssize_t i = (ssize_t)info.AceCount - 1; i >= 0; --i) {
		PACCESS_ALLOWED_ACE ace = (PACCESS_ALLOWED_ACE)get_ace(acl, i);
		if (ace->Header.AceFlags & INHERITED_ACE) {
			CheckApi(::DeleteAce(acl, i));
		}
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

void WinDacl::inherit(const ustring &path, SE_OBJECT_TYPE type) {
	WinSDW sd(path);
	if (sd.IsProtected())
		set(path.c_str(), sd.Dacl(), UNPROTECTED_DACL_SECURITY_INFORMATION, type);
}

void WinDacl::protect(const ustring &path, SE_OBJECT_TYPE type) {
	WinSDW sd(path);
	if (!sd.IsProtected()) {
		WinDacl::del_inherited_aces(sd.Dacl());
		set(path.c_str(), sd.Dacl(), PROTECTED_DACL_SECURITY_INFORMATION, type);
	}
}

void WinDacl::protect_copy(const ustring &path, SE_OBJECT_TYPE type) {
	WinSDW sd(path);
	if (!sd.IsProtected())
		set(path.c_str(), sd.Dacl(), PROTECTED_DACL_SECURITY_INFORMATION, type);
}

PACL WinDacl::create(size_t size) {
	PACL acl = (PACL)CheckPointer(::LocalAlloc(LPTR, size));
	CheckApi(::InitializeAcl(acl, size, ACL_REVISION));
	CheckApi(is_valid(acl));
	return acl;
}
