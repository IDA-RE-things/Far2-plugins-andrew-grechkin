#include "win_net.h"

#include <sddl.h>

AutoUTF					MakeSDDL(const AutoUTF &name, const AutoUTF &group, mode_t mode, bool pr, const AutoUTF dom) {
	AutoUTF	Result;
	if (!name.empty())
		Result = Result + L"O:" + Sid::AsStr(name.c_str(), dom.c_str());
	if (!group.empty())
		Result = Result + L"G:" + Sid::AsStr(group.c_str(), dom.c_str());
	Result += L"D:";
	if (pr)
		Result += L"P";
	Result += Mode2Sddl(name, group, mode, dom);
	return	Result;
}
AutoUTF					AsSddl(PSECURITY_DESCRIPTOR pSD, SECURITY_INFORMATION in) {
	PWSTR	str = NULL;
	CheckAPI(::ConvertSecurityDescriptorToStringSecurityDescriptorW(pSD, SDDL_REVISION, in, &str, NULL));
	AutoUTF Result = str;
	::LocalFree(str);
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
		Result += AutoUTF(L"(A;OICI;") + sm + L";;;" + AutoUTF(Sid::AsStr(name.c_str(), dom.c_str())) + L")";
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
		Result += AutoUTF(L"(A;OICI;") + sm + L";;;" + AutoUTF(Sid::AsStr(group.c_str(), dom.c_str())) + L")";
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

AutoUTF					Parse(PSECURITY_DESCRIPTOR pSD) {
	AutoUTF	Result;
	WORD	ctrl = WinSD::Control(pSD);
	Result += L"Security descriptor:\n";
	Result += AutoUTF(L"SDDL: ") + ::AsSddl(pSD, DACL_SECURITY_INFORMATION);

	Result += AutoUTF(L"\nSize: ") + Num2Str(WinSD::Size(pSD));
	Result += AutoUTF(L"\tOwner: ") + Sid::AsName(WinSD::GetOwner(pSD));
	Result += AutoUTF(L"\tGroup: ") + Sid::AsName(WinSD::GetGroup(pSD));
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
///=========================================================================================== WinSD
// public
WinSD::WinSD(const AutoUTF &in): m_PSD(NULL), m_type(SE_UNKNOWN_OBJECT_TYPE) {
	CheckAPI(::ConvertStringSecurityDescriptorToSecurityDescriptorW(in.c_str(), SDDL_REVISION_1, &m_PSD, NULL));
}

void					WinSD::MakeSelfRelative() {
	CheckAPI(Valid());
	if (IsSelfRelative())
		return;
	DWORD	size = Size();
	PSECURITY_DESCRIPTOR tmp = (PSECURITY_DESCRIPTOR)::LocalAlloc(LMEM_ZEROINIT, size);
	CheckAPI(::MakeSelfRelativeSD(m_PSD, tmp, &size));
	Swp(m_PSD, tmp);
	Free(tmp);
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
										OWNER_SECURITY_INFORMATION, owner, NULL, NULL, NULL);
	if (setpriv) {
		if (pTO)
			WinPriv::Disable(SE_TAKE_OWNERSHIP_NAME);
		if (pRe)
			WinPriv::Disable(SE_RESTORE_NAME);
	}
	CheckError(err);
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
	WORD	control = WinSD::Control(pSD);
	if (WinFlag::Check(control, (WORD)SE_DACL_PRESENT)) {
		DWORD	flag = (WinFlag::Check(control, (WORD)SE_DACL_PROTECTED)) ? PROTECTED_DACL_SECURITY_INFORMATION : UNPROTECTED_DACL_SECURITY_INFORMATION;
		CheckError(::SetNamedSecurityInfoW((PWSTR)path.c_str(), type, DACL_SECURITY_INFORMATION | flag, NULL, NULL, WinSD::DACL(pSD), NULL));
	}
}
void					SetSecurity(const AutoUTF &path, PSECURITY_DESCRIPTOR pSD, SE_OBJECT_TYPE type) {
	DWORD	flag =  DACL_SECURITY_INFORMATION;
	PACL	dacl = NULL;
	PSID	owner = WinSD::GetOwner(pSD);
	PSID	group = WinSD::GetGroup(pSD);
	if (owner)
		WinFlag::Set(flag, (DWORD)OWNER_SECURITY_INFORMATION);
	if (group)
		WinFlag::Set(flag, (DWORD)GROUP_SECURITY_INFORMATION);
	WORD	control = WinSD::Control(pSD);
	if (WinFlag::Check(control, (WORD)SE_DACL_PRESENT)) {
		flag |= (WinFlag::Check(control, (WORD)SE_DACL_PROTECTED)) ? PROTECTED_DACL_SECURITY_INFORMATION : UNPROTECTED_DACL_SECURITY_INFORMATION;
		dacl = WinSD::DACL(pSD);
	}
	CheckError(::SetNamedSecurityInfoW((PWSTR)path.c_str(), type, flag, owner, group, dacl, NULL));
}
void					SetSecurity(HANDLE hnd, PSECURITY_DESCRIPTOR pSD, SE_OBJECT_TYPE type) {
	DWORD	flag =  DACL_SECURITY_INFORMATION;
	PACL	dacl = NULL;
	PSID	owner = WinSD::GetOwner(pSD);
	PSID	group = WinSD::GetGroup(pSD);
	if (owner)
		WinFlag::Set(flag, (DWORD)OWNER_SECURITY_INFORMATION);
	if (group)
		WinFlag::Set(flag, (DWORD)GROUP_SECURITY_INFORMATION);
	WORD	control = WinSD::Control(pSD);
	if (WinFlag::Check(control, (WORD)SE_DACL_PRESENT)) {
		flag |= (WinFlag::Check(control, (WORD)SE_DACL_PROTECTED)) ? PROTECTED_DACL_SECURITY_INFORMATION : UNPROTECTED_DACL_SECURITY_INFORMATION;
		dacl = WinSD::DACL(pSD);
	}
	CheckError(::SetSecurityInfo(hnd, type, flag, owner, group, dacl, NULL));
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
