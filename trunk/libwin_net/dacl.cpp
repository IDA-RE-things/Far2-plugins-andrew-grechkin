#include "win_net.h"

#include <authz.h>

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

AutoUTF ExpAccess::get_name() const {
	if (::GetTrusteeFormW((PTRUSTEEW)&Trustee) == TRUSTEE_IS_NAME)
		return AutoUTF(::GetTrusteeNameW((PTRUSTEEW)&Trustee));
	return Sid((PSID)Trustee.ptstrName).name();
}

AutoUTF ExpAccess::get_fullname() const {
	if (::GetTrusteeFormW((PTRUSTEEW)&Trustee) == TRUSTEE_IS_NAME)
		return AutoUTF(::GetTrusteeNameW((PTRUSTEEW)&Trustee));
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

///=========================================================================================== Authz
class Authz {
public:
	~Authz() {
        ::AuthzFreeContext(m_cln);
        ::AuthzFreeResourceManager(m_hnd);
	}

	Authz(PSID sid) {
		CheckApi(::AuthzInitializeResourceManager(AUTHZ_RM_FLAG_NO_AUDIT, nullptr,
		                                          nullptr, nullptr, nullptr, &m_hnd));
		LUID unusedId = {0};
		CheckApi(::AuthzInitializeContextFromSid(0, sid, m_hnd, nullptr, unusedId, nullptr, &m_cln));
	}

	operator AUTHZ_RESOURCE_MANAGER_HANDLE() const {
		return m_hnd;
	}

	ACCESS_MASK access(PSECURITY_DESCRIPTOR psd) {
		AUTHZ_ACCESS_REQUEST AccessRequest = {0};
		AccessRequest.DesiredAccess = MAXIMUM_ALLOWED;
		AccessRequest.PrincipalSelfSid = NULL;
		AccessRequest.ObjectTypeList = NULL;
		AccessRequest.ObjectTypeListLength = 0;
		AccessRequest.OptionalArguments = NULL;

		BYTE     Buffer[1024];
		RtlZeroMemory(Buffer, sizeof(Buffer));
		AUTHZ_ACCESS_REPLY AccessReply = {0};
		AccessReply.ResultListLength = 1;
		AccessReply.GrantedAccessMask = (PACCESS_MASK) (Buffer);
		AccessReply.Error = (PDWORD) (Buffer + sizeof(ACCESS_MASK));

		CheckApi(::AuthzAccessCheck(0, m_cln, &AccessRequest, nullptr, psd, nullptr, 0, &AccessReply, nullptr));
		return *(PACCESS_MASK)(AccessReply.GrantedAccessMask);
	}

private:
	AUTHZ_RESOURCE_MANAGER_HANDLE m_hnd;
	AUTHZ_CLIENT_CONTEXT_HANDLE   m_cln;
};

ACCESS_MASK eff_rights(const PSECURITY_DESCRIPTOR psd, PSID sid) {
//	ACCESS_MASK ret;
//	CheckApiError(::GetEffectiveRightsFromAclW(acl, (PTRUSTEEW)&tr, &ret));
//	return ret;
	return Authz(sid).access(psd);
}

size_t access2mode(ACCESS_MASK acc) {
	if (((acc & GENERIC_ALL) == GENERIC_ALL) || ((acc & FILE_ALL_ACCESS) == FILE_ALL_ACCESS))
		return 7;
	size_t ret = 0;
	if (((acc & GENERIC_READ) == GENERIC_READ) || ((acc & FILE_GENERIC_READ) == FILE_GENERIC_READ))
		ret += 4;
	if (((acc & GENERIC_WRITE) == GENERIC_WRITE) || ((acc & FILE_GENERIC_WRITE) == FILE_GENERIC_WRITE))
		ret += 2;
	if (((acc & GENERIC_EXECUTE) == GENERIC_EXECUTE) || ((acc & FILE_GENERIC_EXECUTE) == FILE_GENERIC_EXECUTE))
		ret += 1;
	return ret;
}

ACCESS_MASK mode2access(size_t mode) {
	if (mode == 7)
		return FILE_ALL_ACCESS;//GENERIC_ALL;
	ACCESS_MASK ret = 0;
	if (mode & 4)
		ret |= FILE_GENERIC_READ;
	if (mode & 2)
		ret |= FILE_GENERIC_WRITE;
	if (mode & 1)
		ret |= FILE_GENERIC_EXECUTE;
	return ret;
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

WinDacl::WinDacl(const AutoUTF &name, SE_OBJECT_TYPE type): m_dacl(nullptr) {
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
	CheckApiError(::SetEntriesInAclW(1, (PEXPLICIT_ACCESS)&ea, m_dacl, &new_dacl));
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
	ACL_SIZE_INFORMATION	info;
	get_info(acl, info);
	return	info.AceCount;
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
//	return	Result;
//}

size_t	WinDacl::used_bytes(PACL acl) {
	ACL_SIZE_INFORMATION	info;
	get_info(acl, info);
	return	info.AclBytesInUse;
}

size_t	WinDacl::free_bytes(PACL acl) {
	ACL_SIZE_INFORMATION	info;
	get_info(acl, info);
	return	info.AclBytesFree;
}

size_t	WinDacl::size(PACL acl) {
	CheckApi(is_valid(acl));
	ACL_SIZE_INFORMATION	info;
	get_info(acl, info);
	return	info.AclBytesFree + info.AclBytesInUse;
}

PVOID	WinDacl::get_ace(PACL acl, size_t index) {
	PVOID	Result = nullptr;
	CheckApi(::GetAce(acl, index, &Result));
	return	Result;
}

AutoUTF WinDacl::Parse(PACL acl) {
	AutoUTF Result = L"DACL:";
	if (!acl) {
		Result += L"\tNULL\nAll access allowed\n";
		return	Result;
	}
	ACL_SIZE_INFORMATION aclSize;
	CheckApi(::GetAclInformation(acl, &aclSize, sizeof(aclSize), AclSizeInformation));
	if (aclSize.AceCount == 0)
		Result += L" empty";

	Result += AutoUTF(L"\tACE count: ") + Num2Str(aclSize.AceCount) +
			  L"\tUse: " + Num2Str(aclSize.AclBytesInUse) + L" bytes"
			  L"\tFree: " + Num2Str(aclSize.AclBytesFree) + L" bytes" + L"\n";
	for (ULONG lIndex = 0; lIndex < aclSize.AceCount; ++lIndex) {
		ACCESS_ALLOWED_ACE *pACE;
		if (!::GetAce(acl, lIndex, (PVOID*)&pACE))
			return	Result;

		Result += AutoUTF(L"ACE [") + Num2Str(lIndex) + L"]\n";

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
		Result += AutoUTF(L"\tAceType: ") + pszString;
		Result += L"\n\tACE Flags:";
		lIndex2 = 7;
		while (lIndex2--) {
			if ((pACE->Header.AceFlags & aceFlags[lIndex2].value) != 0) {
				Result += AutoUTF(L" ") + aceFlags[lIndex2].name;
			}
		}
		Result += L"\n\tACE Mask: ";
		Result += BitMask<DWORD>::AsStrBin(pACE->Mask);
		Result += L"\n";
	}
	return	Result;
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

void WinDacl::inherit(PCWSTR path, SE_OBJECT_TYPE type) {
	WinSDW sd(path);
	if (sd.IsProtected())
		set(path, sd.Dacl(), UNPROTECTED_DACL_SECURITY_INFORMATION, type);
}

void WinDacl::protect(PCWSTR path, SE_OBJECT_TYPE type) {
	WinSDW sd(path);
	if (!sd.IsProtected()) {
		WinDacl::del_inherited_aces(sd.Dacl());
		set(path, sd.Dacl(), PROTECTED_DACL_SECURITY_INFORMATION, type);
	}
}

void WinDacl::protect_copy(PCWSTR path, SE_OBJECT_TYPE type) {
	WinSDW sd(path);
	if (!sd.IsProtected())
		set(path, sd.Dacl(), PROTECTED_DACL_SECURITY_INFORMATION, type);
}

PACL WinDacl::create(size_t size) {
	PACL acl = (PACL)CheckPointer(::LocalAlloc(LPTR, size));
	CheckApi(::InitializeAcl(acl, size, ACL_REVISION));
	CheckApi(is_valid(acl));
	return acl;
}
