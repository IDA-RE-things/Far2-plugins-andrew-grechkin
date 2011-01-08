#include "win_net.h"

#include <sddl.h>

AutoUTF					MakeSDDL(const AutoUTF &name, const AutoUTF &group, mode_t mode, bool pr, const AutoUTF dom) {
	AutoUTF	Result;
	if (!name.empty())
		Result = Result + L"O:" + Sid::str(name.c_str(), dom.c_str());
	if (!group.empty())
		Result = Result + L"G:" + Sid::str(group.c_str(), dom.c_str());
	Result += L"D:";
	if (pr)
		Result += L"P";
	Result += Mode2Sddl(name, group, mode, dom);
	return	Result;
}
AutoUTF					Mode2Sddl(const AutoUTF &name, const AutoUTF &group, mode_t mode, const AutoUTF dom) {
	AutoUTF	Result = L"(A;OICI;FA;;;BA)";
	AutoUTF	sm;
	if ((mode & 0700) && !name.empty()) {
		if ((mode & 0700) == 0700) {
			sm += L"FA";
		} else {
			if (WinFlag::Check(mode, (mode_t)S_IRUSR))
				sm += L"FR";
			if (WinFlag::Check(mode, (mode_t)S_IWUSR))
				sm += L"FWSDWDWO";
			if (WinFlag::Check(mode, (mode_t)S_IXUSR))
				sm += L"FX";
		}
		Result += AutoUTF(L"(A;OICI;") + sm + L";;;" + AutoUTF(Sid::str(name.c_str(), dom.c_str())) + L")";
	}
	sm.clear();
	if ((mode & 070) && !group.empty()) {
		if ((mode & 070) == 070) {
			sm += L"FA";
		} else {
			if (WinFlag::Check(mode, (mode_t)S_IRGRP))
				sm += L"FR";
			if (WinFlag::Check(mode, (mode_t)S_IWGRP))
				sm += L"FWSDWDWO";
			if (WinFlag::Check(mode, (mode_t)S_IXGRP))
				sm += L"FX";
		}
		Result += AutoUTF(L"(A;OICI;") + sm + L";;;" + AutoUTF(Sid::str(group.c_str(), dom.c_str())) + L")";
	}
	sm.clear();
	if (mode & 07) {
		if ((mode & 07) == 07) {
			sm += L"FA";
		} else {
			if (WinFlag::Check(mode, (mode_t)S_IROTH))
				sm += L"FR";
			if (WinFlag::Check(mode, (mode_t)S_IWOTH))
				sm += L"FWSDWDWO";
			if (WinFlag::Check(mode, (mode_t)S_IXOTH))
				sm += L"FX";
		}
		Result += AutoUTF(L"(A;OICI;") + sm + L";;;WD)";
	}
	return 	Result;
}

///=========================================================================================== WinSD
WinSD::~WinSD() {
	Free(m_pSD);
}

void	WinSD::MakeSelfRelative() {
	CheckApi(IsValid());
	if (IsSelfRelative())
		return;
	DWORD	size = Size();
	PSECURITY_DESCRIPTOR tmp = (PSECURITY_DESCRIPTOR)::LocalAlloc(LMEM_ZEROINIT, size);
	CheckApi(::MakeSelfRelativeSD(m_pSD, tmp, &size));
	using std::swap;
	swap(m_pSD, tmp);
	Free(tmp);
}

AutoUTF	WinSD::AsSddl(PSECURITY_DESCRIPTOR pSD, SECURITY_INFORMATION in) {
	PWSTR	str = nullptr;
	CheckApi(::ConvertSecurityDescriptorToStringSecurityDescriptorW(pSD, SDDL_REVISION, in, &str, nullptr));
	AutoUTF Result = str;
	::LocalFree(str);
	return	Result;
}
AutoUTF	WinSD::Parse(PSECURITY_DESCRIPTOR pSD) {
	AutoUTF	Result;
	WORD	ctrl = WinSD::GetControl(pSD);
	Result += L"Security descriptor:\n";
	Result += AutoUTF(L"SDDL: ") + WinSD::AsSddl(pSD, DACL_SECURITY_INFORMATION);

	Result += AutoUTF(L"\nSize: ") + Num2Str(WinSD::Size(pSD));
	Result += AutoUTF(L"\tOwner: ") + Sid::name(WinSD::GetOwner(pSD));
	Result += AutoUTF(L"\tGroup: ") + Sid::name(WinSD::GetGroup(pSD));
	Result += AutoUTF(L"\nControl: 0x") + Num2Str(ctrl, 16) + L" (" + BitMask<WORD>::AsStrBin(ctrl) + L") [" + BitMask<WORD>::AsStrNum(ctrl) + L"]\n";
	if (WinFlag::Check(ctrl, (WORD)SE_OWNER_DEFAULTED))
		Result += AutoUTF(L"\tSE_OWNER_DEFAULTED (") + Num2Str(SE_OWNER_DEFAULTED) + L")\n";
	if (WinFlag::Check(ctrl, (WORD)SE_GROUP_DEFAULTED))
		Result += AutoUTF(L"\tSE_GROUP_DEFAULTED (") + Num2Str(SE_GROUP_DEFAULTED) + L")\n";
	if (WinFlag::Check(ctrl, (WORD)SE_DACL_PRESENT))
		Result += AutoUTF(L"\tSE_DACL_PRESENT (") + Num2Str(SE_DACL_PRESENT) + L")\n";
	if (WinFlag::Check(ctrl, (WORD)SE_SACL_DEFAULTED))
		Result += AutoUTF(L"\tSE_DACL_DEFAULTED (") + Num2Str(SE_SACL_DEFAULTED) + L")\n";
	if (WinFlag::Check(ctrl, (WORD)SE_SACL_PRESENT))
		Result += AutoUTF(L"\tSE_DACL_PRESENT (") + Num2Str(SE_SACL_PRESENT) + L")\n";
	if (WinFlag::Check(ctrl, (WORD)SE_SACL_DEFAULTED))
		Result += AutoUTF(L"\tSE_SACL_DEFAULTED (") + Num2Str(SE_SACL_DEFAULTED) + L")\n";
	if (WinFlag::Check(ctrl, (WORD)SE_DACL_AUTO_INHERIT_REQ))
		Result += AutoUTF(L"\tSE_DACL_AUTO_INHERIT_REQ (") + Num2Str(SE_DACL_AUTO_INHERIT_REQ) + L")\n";
	if (WinFlag::Check(ctrl, (WORD)SE_SACL_AUTO_INHERIT_REQ))
		Result += AutoUTF(L"\tSE_SACL_AUTO_INHERIT_REQ (") + Num2Str(SE_SACL_AUTO_INHERIT_REQ) + L")\n";
	if (WinFlag::Check(ctrl, (WORD)SE_DACL_AUTO_INHERITED))
		Result += AutoUTF(L"\tSE_DACL_AUTO_INHERITED (") + Num2Str(SE_DACL_AUTO_INHERITED) + L")\n";
	if (WinFlag::Check(ctrl, (WORD)SE_SACL_AUTO_INHERITED))
		Result += AutoUTF(L"\tSE_SACL_AUTO_INHERITED (") + Num2Str(SE_SACL_AUTO_INHERITED) + L")\n";
	if (WinFlag::Check(ctrl, (WORD)SE_DACL_PROTECTED))
		Result += AutoUTF(L"\tSE_DACL_PROTECTED (") + Num2Str(SE_DACL_PROTECTED) + L")\n";
	if (WinFlag::Check(ctrl, (WORD)SE_SACL_PROTECTED))
		Result += AutoUTF(L"\tSE_SACL_PROTECTED (") + Num2Str(SE_SACL_PROTECTED) + L")\n";
	if (WinFlag::Check(ctrl, (WORD)SE_SELF_RELATIVE))
		Result += AutoUTF(L"\tSE_SELF_RELATIVE (") + Num2Str(SE_SELF_RELATIVE) + L")\n";
	if (WinFlag::Check(ctrl, (WORD)SE_DACL_PRESENT)) {
		WinDacl	dacl(pSD);
		Result += dacl.Parse();
	}
	return	Result;
}

///========================================================================================= WinSDDL
WinSDDL::WinSDDL(const AutoUTF &in) {
	CheckApi(::ConvertStringSecurityDescriptorToSecurityDescriptorW(in.c_str(), SDDL_REVISION_1, &m_pSD, nullptr));
}

void	WinAbsSD::Init(PSECURITY_DESCRIPTOR sd) {
	m_owner = m_group = m_dacl = m_sacl = nullptr;
	if (WinSD::IsSelfRelative(sd)) {
		DWORD	sdSize = sizeof(SECURITY_DESCRIPTOR);
		DWORD	ownerSize = SECURITY_MAX_SID_SIZE;
		DWORD	groupSize = SECURITY_MAX_SID_SIZE;
		DWORD	daclSize = WinSD::Size(sd);
		DWORD	saclSize = daclSize;
		m_pSD = (PSECURITY_DESCRIPTOR)::LocalAlloc(LMEM_ZEROINIT, sdSize);
		WinMem::Alloc(m_owner, ownerSize);
		WinMem::Alloc(m_group, groupSize);
		WinMem::Alloc(m_dacl, daclSize);
		WinMem::Alloc(m_sacl, saclSize);
		CheckApi(::MakeAbsoluteSD(sd, m_pSD, &sdSize, m_dacl, &daclSize,
								  m_sacl, &saclSize, m_owner, &ownerSize, m_group, &groupSize));
	} else {
		throw ApiError(ERROR_INVALID_SECURITY_DESCR);
	}
}

WinAbsSD::~WinAbsSD() {
	WinMem::Free(m_owner);
	WinMem::Free(m_group);
	WinMem::Free(m_dacl);
	WinMem::Free(m_sacl);
}
WinAbsSD::WinAbsSD() {
	m_owner = m_group = m_dacl = m_sacl = nullptr;
	m_pSD = (PSECURITY_DESCRIPTOR)::LocalAlloc(LMEM_ZEROINIT, sizeof(SECURITY_DESCRIPTOR));
	CheckApi(::InitializeSecurityDescriptor(m_pSD, SECURITY_DESCRIPTOR_REVISION));
}
WinAbsSD::WinAbsSD(PCWSTR usr, PCWSTR grp, mode_t mode) {
	m_owner = m_group = m_dacl = m_sacl = nullptr;
	m_pSD = (PSECURITY_DESCRIPTOR)::LocalAlloc(LMEM_ZEROINIT, sizeof(SECURITY_DESCRIPTOR));
	CheckApi(::InitializeSecurityDescriptor(m_pSD, SECURITY_DESCRIPTOR_REVISION));

	DWORD	ownerSize = SECURITY_MAX_SID_SIZE;
	WinMem::Alloc(m_owner, ownerSize);
	Sid(usr, EMPTY).copy_to(m_owner, ownerSize);

	DWORD	groupSize = SECURITY_MAX_SID_SIZE;
	WinMem::Alloc(m_group, groupSize);
	Sid(grp, EMPTY).copy_to(m_group, groupSize);

	PSID sidTbl[] = {m_owner, m_group, NULL};
	WinMem::Alloc(sidTbl[2], SECURITY_MAX_SID_SIZE);
	SidString(L"S-1-1-0").copy_to(sidTbl[2], SECURITY_MAX_SID_SIZE);

	SetOwner(m_pSD, m_owner);
	SetGroup(m_pSD, m_group);

	const size_t ACL_SIZE = 1024;
	WinMem::Alloc(m_dacl, 1024);
	CheckApi(::InitializeAcl(m_dacl, ACL_SIZE, ACL_REVISION));

	DWORD aAceMsk[] = {FILE_GENERIC_READ | STANDARD_RIGHTS_READ, FILE_GENERIC_WRITE | FILE_DELETE_CHILD | STANDARD_RIGHTS_WRITE, FILE_GENERIC_EXECUTE};
	DWORD dAceMsk[] = {FILE_GENERIC_READ & ~SYNCHRONIZE, (FILE_GENERIC_WRITE) & ~SYNCHRONIZE,
					   FILE_GENERIC_EXECUTE & ~SYNCHRONIZE
					  };
	bool	ok = true;
	for (size_t iBit = 0; iBit < 9; ++iBit) {
		if ((mode >> (8 - iBit) & 0x1) != 0 && aAceMsk[iBit % 3] != 0)
			ok = ok && AddAccessAllowedAce(m_dacl, ACL_REVISION, aAceMsk[iBit % 3], sidTbl[iBit / 3]);
		else if (dAceMsk[iBit % 3] != 0 && ((iBit / 3) < 2))
			ok = ok && AddAccessDeniedAce(m_dacl, ACL_REVISION, dAceMsk[iBit % 3], sidTbl[iBit / 3]);
	}
	CheckApi(::IsValidAcl(m_dacl));
	SetDacl(m_pSD, m_dacl);
	CheckApi(::IsValidSecurityDescriptor(m_pSD));
}

void					SetOwner(const AutoUTF &path, PSID owner, bool setpriv, SE_OBJECT_TYPE type) {
	bool	pTO = false, pRe = false;
	if (setpriv) {
		pTO = !WinPriv::IsEnabled(SE_TAKE_OWNERSHIP_NAME);
		pRe = !WinPriv::IsEnabled(SE_RESTORE_NAME);
		if (pTO)
			WinPriv::Enable(SE_TAKE_OWNERSHIP_NAME);
		if (pRe)
			WinPriv::Enable(SE_RESTORE_NAME);
	}
	DWORD	err = ::SetNamedSecurityInfoW(const_cast<WCHAR*>(path.c_str()), type,
										OWNER_SECURITY_INFORMATION, owner, nullptr, nullptr, nullptr);
	if (setpriv) {
		if (pTO)
			WinPriv::Disable(SE_TAKE_OWNERSHIP_NAME);
		if (pRe)
			WinPriv::Disable(SE_RESTORE_NAME);
	}
	CheckApiError(err);
}
void					SetOwnerSD(const AutoUTF &path, PSECURITY_DESCRIPTOR pSD, SE_OBJECT_TYPE type) {
	PSID	psid = WinSD::GetOwner(pSD);
	SetOwner(path, psid, true, type);
}
void					SetGroupSD(const AutoUTF &path, PSECURITY_DESCRIPTOR pSD, SE_OBJECT_TYPE type) {
	PSID	psid = WinSD::GetGroup(pSD);
	SetGroup(path, psid, type);
}
void					SetDacl(const AutoUTF &path, PSECURITY_DESCRIPTOR pSD, SE_OBJECT_TYPE type) {
	WORD	control = WinSD::GetControl(pSD);
	if (WinFlag::Check(control, (WORD)SE_DACL_PRESENT)) {
		DWORD	flag = (WinFlag::Check(control, (WORD)SE_DACL_PROTECTED)) ? PROTECTED_DACL_SECURITY_INFORMATION : UNPROTECTED_DACL_SECURITY_INFORMATION;
		CheckApiError(::SetNamedSecurityInfoW((PWSTR)path.c_str(), type, DACL_SECURITY_INFORMATION | flag, nullptr, nullptr, WinSD::GetDacl(pSD), nullptr));
	}
}
void					SetSecurity(const AutoUTF &path, PSECURITY_DESCRIPTOR pSD, SE_OBJECT_TYPE type) {
	DWORD	flag =  DACL_SECURITY_INFORMATION;
	PACL	dacl = nullptr;
	PSID	owner = WinSD::GetOwner(pSD);
	PSID	group = WinSD::GetGroup(pSD);
	if (owner)
		WinFlag::Set(flag, (DWORD)OWNER_SECURITY_INFORMATION);
	if (group)
		WinFlag::Set(flag, (DWORD)GROUP_SECURITY_INFORMATION);
	WORD	control = WinSD::GetControl(pSD);
	if (WinFlag::Check(control, (WORD)SE_DACL_PRESENT)) {
		flag |= (WinFlag::Check(control, (WORD)SE_DACL_PROTECTED)) ? PROTECTED_DACL_SECURITY_INFORMATION : UNPROTECTED_DACL_SECURITY_INFORMATION;
		dacl = WinSD::GetDacl(pSD);
	}
	CheckApiError(::SetNamedSecurityInfoW((PWSTR)path.c_str(), type, flag, owner, group, dacl, nullptr));
}
void					SetSecurity(HANDLE hnd, PSECURITY_DESCRIPTOR pSD, SE_OBJECT_TYPE type) {
	DWORD	flag =  DACL_SECURITY_INFORMATION;
	PACL	dacl = nullptr;
	PSID	owner = WinSD::GetOwner(pSD);
	PSID	group = WinSD::GetGroup(pSD);
	if (owner)
		WinFlag::Set(flag, (DWORD)OWNER_SECURITY_INFORMATION);
	if (group)
		WinFlag::Set(flag, (DWORD)GROUP_SECURITY_INFORMATION);
	WORD	control = WinSD::GetControl(pSD);
	if (WinFlag::Check(control, (WORD)SE_DACL_PRESENT)) {
		flag |= (WinFlag::Check(control, (WORD)SE_DACL_PROTECTED)) ? PROTECTED_DACL_SECURITY_INFORMATION : UNPROTECTED_DACL_SECURITY_INFORMATION;
		dacl = WinSD::GetDacl(pSD);
	}
	CheckApiError(::SetSecurityInfo(hnd, type, flag, owner, group, dacl, nullptr));
}

AutoUTF					GetOwner(const AutoUTF &path, SE_OBJECT_TYPE type) {
	WinSDW	sd(path, type);
	return	sd.Owner();
}
AutoUTF					GetGroup(const AutoUTF &path, SE_OBJECT_TYPE type) {
	WinSDW	sd(path, type);
	return	sd.Group();
}
AutoUTF					GetOwner(HANDLE hnd, SE_OBJECT_TYPE type) {
	WinSDH	sd(hnd, type);
	return	sd.Owner();
}
AutoUTF					GetGroup(HANDLE hnd, SE_OBJECT_TYPE type) {
	WinSDH	sd(hnd, type);
	return	sd.Group();
}
