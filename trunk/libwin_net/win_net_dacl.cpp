#include "win_net.h"

#ifndef ACCESS_ALLOWED_OBJECT_ACE_TYPE
#define ACCESS_ALLOWED_OBJECT_ACE_TYPE          (0x5)
#define ACCESS_DENIED_OBJECT_ACE_TYPE           (0x6)
#define SYSTEM_AUDIT_OBJECT_ACE_TYPE            (0x7)
#endif

struct		WinAceTypes {
	BYTE  lACEType;
	PCWSTR pszTypeName;
} aceTypes[6] = {
	{ACCESS_ALLOWED_ACE_TYPE, L"ACCESS_ALLOWED_ACE_TYPE"},
	{ACCESS_DENIED_ACE_TYPE, L"ACCESS_DENIED_ACE_TYPE"},
	{SYSTEM_AUDIT_ACE_TYPE, L"SYSTEM_AUDIT_ACE_TYPE"},
	{ACCESS_ALLOWED_OBJECT_ACE_TYPE, L"ACCESS_ALLOWED_OBJECT_ACE_TYPE"},
	{ACCESS_DENIED_OBJECT_ACE_TYPE, L"ACCESS_DENIED_OBJECT_ACE_TYPE"},
	{SYSTEM_AUDIT_OBJECT_ACE_TYPE, L"SYSTEM_AUDIT_OBJECT_ACE_TYPE"}
};
struct		WinAceFlags {
	ULONG lACEFlag;
	PCWSTR pszFlagName;
} aceFlags[7] = {
	{INHERITED_ACE, L"INHERITED_ACE"},
	{CONTAINER_INHERIT_ACE, L"CONTAINER_INHERIT_ACE"},
	{OBJECT_INHERIT_ACE, L"OBJECT_INHERIT_ACE"},
	{INHERIT_ONLY_ACE, L"INHERIT_ONLY_ACE"},
	{NO_PROPAGATE_INHERIT_ACE, L"NO_PROPAGATE_INHERIT_ACE"},
	{FAILED_ACCESS_ACE_FLAG, L"FAILED_ACCESS_ACE_FLAG"},
	{SUCCESSFUL_ACCESS_ACE_FLAG, L"SUCCESSFUL_ACCESS_ACE_FLAG"}
};

///========================================================================================= WinDacl
void					WinDacl::Init(PACL pACL) {
	size_t	size = GetSize(pACL);
	if (size) {
		needDelete = WinMem::Alloc(m_PACL, size);
		WinMem::Copy(m_PACL, pACL, size);
	}
}
void					WinDacl::Init(PSECURITY_DESCRIPTOR pSD) {
	BOOL	bDaclPresent   = true;
	BOOL	bDaclDefaulted = false;
	PACL	tmp = nullptr;
	CheckAPI(::GetSecurityDescriptorDacl(pSD, &bDaclPresent, &tmp, &bDaclDefaulted));
	Init(tmp);
}

WinDacl::WinDacl(size_t size): m_PACL(nullptr), needDelete(false) {
	needDelete = WinMem::Alloc(m_PACL, size);
	CheckAPI(::InitializeAcl(m_PACL, size, ACL_REVISION));
}
WinDacl::WinDacl(PACL pACL): m_PACL(nullptr), needDelete(false) {
	Init(pACL);
	CheckAPI(Valid());
}
WinDacl::WinDacl(PSECURITY_DESCRIPTOR pSD): m_PACL(nullptr), needDelete(false) {
	Init(pSD);
	CheckAPI(Valid());
}
WinDacl::WinDacl(const AutoUTF &name, SE_OBJECT_TYPE type): m_PACL(nullptr), needDelete(false) {
	PSECURITY_DESCRIPTOR pSD = nullptr;
	PACL	tmp = nullptr;
	CheckError(::GetNamedSecurityInfoW((PWSTR)name.c_str(), type,
									   DACL_SECURITY_INFORMATION,
									   nullptr, nullptr, &tmp, nullptr, &pSD));
	Init(tmp);
	::LocalFree(pSD);
	CheckAPI(Valid());
}

void					WinDacl::AddA(const Sid &sid) {
	CheckAPI(::AddAccessAllowedAce(m_PACL, ACL_REVISION, GENERIC_READ | GENERIC_WRITE, sid));
}
void					WinDacl::AddD(const Sid &sid) {
	CheckAPI(::AddAccessDeniedAce(m_PACL, ACL_REVISION, GENERIC_READ, sid));
}

void					WinDacl::GetAclInfo(PACL pACL, ACL_SIZE_INFORMATION &out) {
	CheckAPI(::GetAclInformation(pACL, &out, sizeof(out), AclSizeInformation));
}
void					WinDacl::DelInheritedAces(PACL pACL) {
	ACL_SIZE_INFORMATION	info;
	GetAclInfo(pACL, info);
	for (ULONG lIndex = 0; lIndex < info.AceCount; ++lIndex) {
		ACCESS_ALLOWED_ACE	*pACE = (ACCESS_ALLOWED_ACE*)GetAce(pACL, lIndex);
		if (pACE->Header.AceFlags & INHERITED_ACE) {
			CheckAPI(::DeleteAce(pACL, lIndex--));
		}
	}
}
size_t					WinDacl::CountAces(PACL pACL) {
	ACL_SIZE_INFORMATION	info;
	GetAclInfo(pACL, info);
	return	info.AceCount;
}
size_t					WinDacl::CountAces(PACL pACL, size_t &sz, bool inh) {
	size_t	Result = 0;
	ACL_SIZE_INFORMATION	info;
	GetAclInfo(pACL, info);
	sz = 0;
	for (ULONG lIndex = 0; lIndex < info.AceCount; ++lIndex) {
		ACCESS_ALLOWED_ACE	*pACE = (ACCESS_ALLOWED_ACE*)GetAce(pACL, lIndex);
		if (inh || !(pACE->Header.AceFlags & INHERITED_ACE)) {
			PSID	pSID = PSIDFromPACE(pACE);
			++Result;
			sz += Sid::size(pSID);
		}
	}
	return	Result;
}
size_t					WinDacl::GetUsed(PACL pACL) {
	ACL_SIZE_INFORMATION	info;
	GetAclInfo(pACL, info);
	return	info.AclBytesInUse;
}
size_t					WinDacl::GetFree(PACL pACL) {
	ACL_SIZE_INFORMATION	info;
	GetAclInfo(pACL, info);
	return	info.AclBytesFree;
}
size_t					WinDacl::GetSize(PACL pACL) {
	ACL_SIZE_INFORMATION	info;
	GetAclInfo(pACL, info);
	return	info.AclBytesFree + info.AclBytesInUse;
}
PVOID					WinDacl::GetAce(PACL pACL, size_t index) {
	PVOID	Result = nullptr;
	CheckAPI(::GetAce(pACL, index, &Result));
	return	Result;
}

AutoUTF					WinDacl::Parse(PACL pACL) {
	AutoUTF Result = L"DACL:";
	if (pACL == nullptr) {
		Result += L"\tNULL\nAll access allowed\n";
		return	Result;
	}
	ACL_SIZE_INFORMATION aclSize = {0};
	CheckAPI(::GetAclInformation(pACL, &aclSize, sizeof(aclSize), AclSizeInformation));
	if (aclSize.AceCount == 0)
		Result += L" empty";

	Result += AutoUTF(L"\tACE count: ") + Num2Str(aclSize.AceCount) +
			  L"\tUse: " + Num2Str(aclSize.AclBytesInUse) + L" bytes"
			  L"\tFree: " + Num2Str(aclSize.AclBytesFree) + L" bytes" + L"\n";
	for (ULONG lIndex = 0; lIndex < aclSize.AceCount; ++lIndex) {
		ACCESS_ALLOWED_ACE *pACE;
		if (!::GetAce(pACL, lIndex, (PVOID*)&pACE))
			return	Result;

		Result += AutoUTF(L"ACE [") + Num2Str(lIndex) + L"]\n";

		PSID pSID = PSIDFromPACE(pACE);
		Result = Result + L"\tACE Name: " + Sid::name(pSID).c_str() + L" (" + Sid::str(pSID).c_str() + L")";

		ULONG lIndex2 = 6;
		PCWSTR pszString = L"Unknown ACE Type";
		while (lIndex2--) {
			if (pACE->Header.AceType == aceTypes[lIndex2].lACEType) {
				pszString = aceTypes[lIndex2].pszTypeName;
				break;
			}
		}
		Result += AutoUTF(L"\tAceType: ") + pszString;
		Result += L"\n\tACE Flags:";
		lIndex2 = 7;
		while (lIndex2--) {
			if ((pACE->Header.AceFlags & aceFlags[lIndex2].lACEFlag) != 0) {
				Result += AutoUTF(L" ") + aceFlags[lIndex2].pszFlagName;
			}
		}
		Result += L"\n\tACE Mask: ";
		Result += BitMask<DWORD>::AsStrBin(pACE->Mask);
		Result += L"\n";
	}
	return	Result;
}

void					WinDacl::Inherit(const AutoUTF &path, SE_OBJECT_TYPE type) {
	WinDacl	dacl(path, type);
//	dacl.DelInheritedAces();
	dacl.SetTo(UNPROTECTED_DACL_SECURITY_INFORMATION, path, type);
}
void					WinDacl::Protect(const AutoUTF &path, bool copy, SE_OBJECT_TYPE type) {
	WinDacl	dacl(path, type);
	if (!copy) {
		dacl.DelInheritedAces();
	}
	dacl.SetTo(PROTECTED_DACL_SECURITY_INFORMATION, path, type);
}
