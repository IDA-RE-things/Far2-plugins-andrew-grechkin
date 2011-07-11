#include "win_net.h"
#include <libwin_def/priv.h>

#include <sddl.h>

AutoUTF	GetOwner(const AutoUTF &path, SE_OBJECT_TYPE type) {
	WinSDW	sd(path, type);
	return sd.Owner();
}

AutoUTF	GetOwner(HANDLE hnd, SE_OBJECT_TYPE type) {
	WinSDH	sd(hnd, type);
	return sd.Owner();
}

AutoUTF	GetGroup(const AutoUTF &path, SE_OBJECT_TYPE type) {
	WinSDW	sd(path, type);
	return sd.Group();
}

AutoUTF	GetGroup(HANDLE hnd, SE_OBJECT_TYPE type) {
	WinSDH	sd(hnd, type);
	return sd.Group();
}

void	SetOwner(HANDLE handle, PSID owner, SE_OBJECT_TYPE type) {
	DWORD	err = ::SetSecurityInfo(handle, type, OWNER_SECURITY_INFORMATION,
	     	                        owner, nullptr, nullptr, nullptr);
	if (err == ERROR_INVALID_OWNER) {
		Privilege ow(SE_TAKE_OWNERSHIP_NAME);
		Privilege rn(SE_RESTORE_NAME);
		err = ::SetSecurityInfo(handle, type, OWNER_SECURITY_INFORMATION,
		                        owner, nullptr, nullptr, nullptr);
	}
	CheckApiError(err);
}

void	SetOwner(const AutoUTF &path, PSID owner, SE_OBJECT_TYPE type) {
	DWORD err = ::SetNamedSecurityInfoW(const_cast<PWSTR>(path.c_str()), type,
										OWNER_SECURITY_INFORMATION, owner, nullptr, nullptr, nullptr);
	if (err == ERROR_INVALID_OWNER) {
		Privilege ow(SE_TAKE_OWNERSHIP_NAME);
		Privilege rn(SE_RESTORE_NAME);
		err = ::SetNamedSecurityInfoW(const_cast<PWSTR>(path.c_str()), type,
									OWNER_SECURITY_INFORMATION, owner, nullptr, nullptr, nullptr);
	}
	CheckApiError(err);
}

void	SetOwnerSD(const AutoUTF &path, PSECURITY_DESCRIPTOR sd, SE_OBJECT_TYPE type) {
	SetOwner(path, WinSD::get_owner(sd), type);
}

void	SetGroup(HANDLE handle, PSID owner, SE_OBJECT_TYPE type) {
	CheckApiError(::SetSecurityInfo(handle, type, GROUP_SECURITY_INFORMATION, nullptr, owner, nullptr, nullptr));
}

void	SetGroup(const AutoUTF &path, PSID owner, SE_OBJECT_TYPE type) {
	CheckApiError(::SetNamedSecurityInfoW((PWSTR)path.c_str(), type, GROUP_SECURITY_INFORMATION, nullptr, owner, nullptr, nullptr));
}

void	SetGroupSD(const AutoUTF &path, PSECURITY_DESCRIPTOR sd, SE_OBJECT_TYPE type) {
	PSID	psid = WinSD::get_group(sd);
	SetGroup(path, psid, type);
}

void	SetDacl(HANDLE handle, PACL acl, SE_OBJECT_TYPE type) {
	CheckApiError(::SetSecurityInfo(handle, type, DACL_SECURITY_INFORMATION, nullptr, nullptr, acl, nullptr));
}

void	SetDacl(const AutoUTF &path, PACL pacl, SE_OBJECT_TYPE type) {
	CheckApiError(::SetNamedSecurityInfoW((PWSTR)path.c_str(), type, DACL_SECURITY_INFORMATION, nullptr, nullptr, pacl, nullptr));
}

void	SetDacl(const AutoUTF &path, PSECURITY_DESCRIPTOR sd, SE_OBJECT_TYPE type) {
	WORD	control = WinSD::get_control(sd);
	if (WinFlag::Check(control, (WORD)SE_DACL_PRESENT)) {
		DWORD	flag = (WinFlag::Check(control, (WORD)SE_DACL_PROTECTED)) ?
		     	              PROTECTED_DACL_SECURITY_INFORMATION :
		                      UNPROTECTED_DACL_SECURITY_INFORMATION;
		CheckApiError(::SetNamedSecurityInfoW((PWSTR)path.c_str(), type,
		                                       DACL_SECURITY_INFORMATION | flag, nullptr, nullptr,
		                                       WinSD::get_dacl(sd), nullptr));
	}
}

void	SetSacl(HANDLE handle, PACL acl, SE_OBJECT_TYPE type) {
	DWORD	err = ::SetSecurityInfo(handle, type, SACL_SECURITY_INFORMATION, nullptr,
	     	                        nullptr, nullptr, acl);
	if (err == ERROR_INVALID_OWNER) {
		Privilege ow(SE_SECURITY_NAME);
		err = ::SetSecurityInfo(handle, type, SACL_SECURITY_INFORMATION, nullptr,
     	                        nullptr, nullptr, acl);
	}
	CheckApiError(err);
}

void	SetSacl(const AutoUTF &path, PACL pacl, SE_OBJECT_TYPE type) {
	DWORD	err = ::SetNamedSecurityInfoW((PWSTR)path.c_str(), type, SACL_SECURITY_INFORMATION,
	     	                              nullptr, nullptr, nullptr, pacl);
	if (err == ERROR_INVALID_OWNER) {
		Privilege ow(SE_SECURITY_NAME);
		err = ::SetNamedSecurityInfoW((PWSTR)path.c_str(), type, SACL_SECURITY_INFORMATION,
     	                              nullptr, nullptr, nullptr, pacl);
	}
	CheckApiError(err);
}

void	SetSecurity(HANDLE hnd, PSECURITY_DESCRIPTOR sd, SE_OBJECT_TYPE type) {
	PSID	owner = WinSD::get_owner(sd);
	PSID	group = WinSD::get_group(sd);
	DWORD	flag =  DACL_SECURITY_INFORMATION;
	PACL	dacl = nullptr;
	if (owner)
		flag |= OWNER_SECURITY_INFORMATION;
	if (group)
		flag |= GROUP_SECURITY_INFORMATION;
	WORD	control = WinSD::get_control(sd);
	if (WinFlag::Check(control, (WORD)SE_DACL_PRESENT)) {
		flag |= (WinFlag::Check(control, (WORD)SE_DACL_PROTECTED)) ? PROTECTED_DACL_SECURITY_INFORMATION
		                                                           : UNPROTECTED_DACL_SECURITY_INFORMATION;
		dacl = WinSD::get_dacl(sd);
	}
	DWORD err = ::SetSecurityInfo(hnd, type, flag, owner, group, dacl, nullptr);

	if (err == ERROR_INVALID_OWNER) {
		Privilege ow(SE_TAKE_OWNERSHIP_NAME);
		Privilege rn(SE_RESTORE_NAME);
		err = ::SetSecurityInfo(hnd, type, flag, owner, group, dacl, nullptr);
	}
	CheckApiError(err);
}

void	SetSecurity(const AutoUTF &path, PSECURITY_DESCRIPTOR sd, SE_OBJECT_TYPE type) {
	DWORD flag = 0;
	PACL dacl = nullptr;

	WORD control = WinSD::get_control(sd);
	if (WinFlag::Check(control, (WORD)SE_DACL_PRESENT)) {
		flag = DACL_SECURITY_INFORMATION;
		flag |= WinFlag::Check(control, (WORD)SE_DACL_PROTECTED) ? PROTECTED_DACL_SECURITY_INFORMATION
		                                                          : UNPROTECTED_DACL_SECURITY_INFORMATION;
		dacl = WinSD::get_dacl(sd);
		CheckApiError(::SetNamedSecurityInfoW((PWSTR)path.c_str(), type, flag, nullptr, nullptr, dacl, nullptr));
	}

	Sid system(L"SYSTEM");
	PSID group = WinSD::get_group(sd);
	if (!group) {
		group = system;
	}
	::SetNamedSecurityInfoW((PWSTR)path.c_str(), type, GROUP_SECURITY_INFORMATION, nullptr, group, nullptr, nullptr);

	PSID owner = WinSD::get_owner(sd);
	if (owner) {
		flag |= OWNER_SECURITY_INFORMATION;
	}

	DWORD err = ::SetNamedSecurityInfoW((PWSTR)path.c_str(), type, OWNER_SECURITY_INFORMATION, owner, nullptr, nullptr, nullptr);
	if (err == ERROR_INVALID_OWNER) {
		Privilege ow(SE_TAKE_OWNERSHIP_NAME);
		Privilege rn(SE_RESTORE_NAME);
		err = ::SetNamedSecurityInfoW((PWSTR)path.c_str(), type, OWNER_SECURITY_INFORMATION, owner, nullptr, nullptr, nullptr);
	}
	CheckApiError(err);
}

void	SetSecurity(const AutoUTF &path, const AutoUTF &sddl, SE_OBJECT_TYPE type) {
	WinSDDL	sd(sddl);
	SetSecurity(path, sd, type);
}

void	SetSecurity(const AutoUTF &path, const Sid &uid, const Sid &gid, mode_t mode, bool protect, SE_OBJECT_TYPE type) {
	SetSecurity(path, WinAbsSD(uid.name(), gid.name(), mode, protect), type);
}

AutoUTF	MakeSDDL(const AutoUTF &name, const AutoUTF &group, mode_t mode, bool protect) {
	AutoUTF	Result;
	if (!name.empty())
		Result = Result + L"O:" + Sid::str(name.c_str());
	if (!group.empty())
		Result = Result + L"G:" + Sid::str(group.c_str());
	Result += L"D:";
	if (protect)
		Result += L"P";
	Result += Mode2Sddl(name, group, mode);
	return Result;
}

AutoUTF	Mode2Sddl(const AutoUTF &name, const AutoUTF &group, mode_t mode) {
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
		Result += AutoUTF(L"(A;OICI;") + sm + L";;;" + AutoUTF(Sid::str(name.c_str())) + L")";
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
		Result += AutoUTF(L"(A;OICI;") + sm + L";;;" + AutoUTF(Sid::str(group.c_str())) + L")";
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
	Free(m_sd);
}

void	WinSD::MakeSelfRelative() {
	CheckApi(is_valid(m_sd));
	if (is_selfrelative(m_sd))
		return;
	DWORD	size = Size();
	PSECURITY_DESCRIPTOR tmp = (PSECURITY_DESCRIPTOR)::LocalAlloc(LMEM_ZEROINIT, size);
	CheckApi(::MakeSelfRelativeSD(m_sd, tmp, &size));
	using std::swap;
	swap(m_sd, tmp);
	Free(tmp);
}

AutoUTF	WinSD::as_sddl(PSECURITY_DESCRIPTOR sd, SECURITY_INFORMATION in) {
	PWSTR	str;
	CheckApi(::ConvertSecurityDescriptorToStringSecurityDescriptorW(sd, SDDL_REVISION, in, &str, nullptr));
	AutoUTF Result = str;
	::LocalFree(str);
	return Result;
}

AutoUTF	WinSD::Parse(PSECURITY_DESCRIPTOR sd) {
	AutoUTF	Result;
	WORD	ctrl = get_control(sd);
	Result += L"Security descriptor:\n";
	Result += AutoUTF(L"SDDL: ") + WinSD::as_sddl(sd, DACL_SECURITY_INFORMATION);

	Result += AutoUTF(L"\nSize: ") + Num2Str(size(sd));
	Result += AutoUTF(L"\tOwner: ") + Sid::name(get_owner(sd));
	Result += AutoUTF(L"\tGroup: ") + Sid::name(get_group(sd));
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
		Result += WinDacl::Parse(get_dacl(sd));
	}
	return Result;
}

void WinSD::Free(PSECURITY_DESCRIPTOR &in) {
	if (in) {
		::LocalFree(in);
		in = nullptr;
	}
}

bool WinSD::is_protected(PSECURITY_DESCRIPTOR sd) {
	return WinFlag::Check(get_control(sd), (WORD)SE_DACL_PROTECTED);
}

bool WinSD::is_selfrelative(PSECURITY_DESCRIPTOR sd) {
	return WinFlag::Check(get_control(sd), (WORD)SE_SELF_RELATIVE);
}

WORD WinSD::get_control(PSECURITY_DESCRIPTOR sd) {
	WORD	Result = 0;
	DWORD	Revision;
	CheckApi(::GetSecurityDescriptorControl(sd, &Result, &Revision));
	return Result;
}

size_t WinSD::size(PSECURITY_DESCRIPTOR sd) {
	CheckApi(is_valid(sd));
	return ::GetSecurityDescriptorLength(sd);
}

PSID WinSD::get_owner(PSECURITY_DESCRIPTOR sd) {
	PSID	psid;
	BOOL	bTmp;
	CheckApi(::GetSecurityDescriptorOwner(sd, &psid, &bTmp));
	return psid;
}

PSID WinSD::get_group(PSECURITY_DESCRIPTOR sd) {
	PSID	psid;
	BOOL	bTmp;
	CheckApi(::GetSecurityDescriptorGroup(sd, &psid, &bTmp));
	return psid;
}

PACL WinSD::get_dacl(PSECURITY_DESCRIPTOR sd) {
	BOOL	bDaclPresent   = false;
	BOOL	bDaclDefaulted = false;
	PACL	Result = nullptr;
	CheckApi(::GetSecurityDescriptorDacl(sd, &bDaclPresent, &Result, &bDaclDefaulted));
	CheckApi(bDaclPresent);
	return Result;
}

void WinSD::set_control(PSECURITY_DESCRIPTOR sd, WORD flag, bool s) {
	CheckApi(::SetSecurityDescriptorControl(sd, flag, s ? flag : 0));
}

void WinSD::set_owner(PSECURITY_DESCRIPTOR sd, PSID pSid, bool deflt) {
	CheckApi(::SetSecurityDescriptorOwner(sd, pSid, deflt));
}

void WinSD::set_group(PSECURITY_DESCRIPTOR sd, PSID pSid, bool deflt) {
	CheckApi(::SetSecurityDescriptorGroup(sd, pSid, deflt));
}

void WinSD::set_dacl(PSECURITY_DESCRIPTOR sd, PACL acl) {
	CheckApi(::SetSecurityDescriptorDacl(sd, true, acl, false));
}

void WinSD::set_sacl(PSECURITY_DESCRIPTOR sd, PACL acl) {
	CheckApi(::SetSecurityDescriptorSacl(sd, true, acl, false));
}

///========================================================================================= WinSDDL
WinSDDL::WinSDDL(const AutoUTF &in) {
	CheckApi(::ConvertStringSecurityDescriptorToSecurityDescriptorW(in.c_str(), SDDL_REVISION_1, &m_sd, nullptr));
}

void	WinAbsSD::Init(PSECURITY_DESCRIPTOR sd) {
	m_owner = m_group = m_dacl = m_sacl = nullptr;
//	if (WinSD::is_selfrelative(sd)) {
		DWORD	sdSize = sizeof(SECURITY_DESCRIPTOR);
		DWORD	ownerSize = SECURITY_MAX_SID_SIZE;
		DWORD	groupSize = SECURITY_MAX_SID_SIZE;
		DWORD	daclSize = WinDacl::size(WinSD::get_dacl(sd));
		DWORD	saclSize = daclSize;
		m_sd = (PSECURITY_DESCRIPTOR)::LocalAlloc(LPTR, sdSize);
		m_owner = (PSID)::LocalAlloc(LPTR, ownerSize);
		m_group = (PSID)::LocalAlloc(LPTR, groupSize);
		m_dacl = (PACL)::LocalAlloc(LPTR, daclSize);
		m_sacl = (PACL)::LocalAlloc(LPTR, saclSize);
		CheckApi(::MakeAbsoluteSD(sd, m_sd, &sdSize, m_dacl, &daclSize,
								  m_sacl, &saclSize, m_owner, &ownerSize, m_group, &groupSize));
//	} else {
//		throw ApiError(ERROR_INVALID_SECURITY_DESCR);
//	}
}

WinAbsSD::~WinAbsSD() {
	if (m_owner)
		::LocalFree(m_owner);
	if (m_group)
		::LocalFree(m_group);
	if (m_dacl)
		::LocalFree(m_dacl);
	if (m_sacl)
		::LocalFree(m_sacl);
}

WinAbsSD::WinAbsSD() {
	m_owner = m_group = m_dacl = m_sacl = nullptr;
	m_sd = (PSECURITY_DESCRIPTOR)::LocalAlloc(LPTR, sizeof(SECURITY_DESCRIPTOR));
	CheckApi(::InitializeSecurityDescriptor(m_sd, SECURITY_DESCRIPTOR_REVISION));
}

WinAbsSD::WinAbsSD(const AutoUTF &name, const AutoUTF &group, bool protect) {
	m_owner = m_group = m_dacl = m_sacl = nullptr;
	m_sd = (PSECURITY_DESCRIPTOR)::LocalAlloc(LPTR, sizeof(SECURITY_DESCRIPTOR));
	CheckApi(::InitializeSecurityDescriptor(m_sd, SECURITY_DESCRIPTOR_REVISION));
	WinDacl dacl(64);
	dacl.detach(m_dacl);

	if (!name.empty()) {
		try {
			Sid usr(name);
			DWORD	ownerSize = SECURITY_MAX_SID_SIZE;
			m_owner = (PSID)::LocalAlloc(LPTR, ownerSize);
			usr.copy_to(m_owner, ownerSize);
		} catch (...) {
		}
	}

	if (!group.empty()) {
		try {
			DWORD	groupSize = SECURITY_MAX_SID_SIZE;
			m_group = (PSID)::LocalAlloc(LPTR, groupSize);
			Sid grp(group);
			grp.copy_to(m_group, groupSize);
		} catch (...) {
		}
	}

	set_owner(m_sd, m_owner);
	set_group(m_sd, m_group);
	set_dacl(m_sd, m_dacl);
	CheckApi(::IsValidSecurityDescriptor(m_sd));
	Protect(protect);
}

WinAbsSD::WinAbsSD(const AutoUTF &name, const AutoUTF &group, mode_t mode, bool protect) {
	m_owner = m_group = m_dacl = m_sacl = nullptr;
	m_sd = (PSECURITY_DESCRIPTOR)::LocalAlloc(LPTR, sizeof(SECURITY_DESCRIPTOR));
	CheckApi(::InitializeSecurityDescriptor(m_sd, SECURITY_DESCRIPTOR_REVISION));

	WinDacl dacl(1024);
	dacl.Set(Sid(WinWorldSid).name().c_str(), mode2access((mode) & 07));
	dacl.Set(Sid(WinBuiltinIUsersSid).name().c_str(), mode2access((mode) & 07));
	dacl.Set(Sid(WinIUserSid).name().c_str(), mode2access((mode) & 07));
	dacl.Set(SidString(L"S-1-5-32-544").name().c_str(), mode2access(07));
	dacl.Set(SidString(L"S-1-5-20").name().c_str(), mode2access(07));
	dacl.Set(SidString(L"S-1-5-19").name().c_str(), mode2access(07));
	if (!name.empty()) {
		try {
			Sid usr(name);
			DWORD	ownerSize = SECURITY_MAX_SID_SIZE;
			m_owner = (PSID)::LocalAlloc(LPTR, ownerSize);
			usr.copy_to(m_owner, ownerSize);
			dacl.Set(usr.name().c_str(), mode2access((mode >> 6) & 07));
		} catch (...) {
		}
	}

	if (!group.empty()) {
		try {
			DWORD	groupSize = SECURITY_MAX_SID_SIZE;
			m_group = (PSID)::LocalAlloc(LPTR, groupSize);
			Sid grp(group);
			grp.copy_to(m_group, groupSize);
			dacl.Set(grp.name().c_str(), mode2access((mode >> 3) & 07));
		} catch (...) {
		}
	}

	CheckApi(WinDacl::is_valid(dacl));
	dacl.detach(m_dacl);

	set_owner(m_sd, m_owner);
	set_group(m_sd, m_group);
	set_dacl(m_sd, m_dacl);
	CheckApi(::IsValidSecurityDescriptor(m_sd));
	Protect(protect);
}

WinAbsSD::WinAbsSD(mode_t mode, bool protect) {
	m_owner = m_group = m_dacl = m_sacl = nullptr;

	WinDacl dacl(1024);
	dacl.Set(Sid(WinWorldSid).name().c_str(), mode2access((mode) & 07));
	dacl.Set(Sid(WinBuiltinIUsersSid).name().c_str(), mode2access((mode) & 07));
	dacl.Set(Sid(WinIUserSid).name().c_str(), mode2access((mode) & 07));
	dacl.Set(SidString(L"S-1-5-32-544").name().c_str(), mode2access(07));
	dacl.Set(SidString(L"S-1-5-20").name().c_str(), mode2access(07));
	dacl.Set(SidString(L"S-1-5-19").name().c_str(), mode2access(07));
	try {
		Sid ow(WinCreatorOwnerSid);
		dacl.Set(ow.name().c_str(), mode2access((mode >> 6) & 07));
	} catch (...) {
	}
	try {
		Sid gr(WinCreatorGroupSid);
		dacl.Set(gr.name().c_str(), mode2access((mode >> 3) & 07));
	} catch (...) {
	}
	CheckApi(WinDacl::is_valid(dacl));

	m_sd = (PSECURITY_DESCRIPTOR)::LocalAlloc(LPTR, SECURITY_DESCRIPTOR_MIN_LENGTH);
	CheckApi(::InitializeSecurityDescriptor(m_sd, SECURITY_DESCRIPTOR_REVISION));
	dacl.detach(m_dacl);
	set_dacl(m_sd, m_dacl);
	CheckApi(::IsValidSecurityDescriptor(m_sd));
	Protect(protect);
}

WinAbsSD::WinAbsSD(PSID ow, PSID gr, PACL da, bool protect) {
	const DWORD sidSize = SECURITY_MAX_SID_SIZE;
	m_owner = m_group = m_dacl = m_sacl = nullptr;

	m_sd = (PSECURITY_DESCRIPTOR)::LocalAlloc(LPTR, sizeof(SECURITY_DESCRIPTOR));
	CheckApi(::InitializeSecurityDescriptor(m_sd, SECURITY_DESCRIPTOR_REVISION));

	try {
		Sid usr(ow);
		m_owner = (PSID)::LocalAlloc(LPTR, sidSize);
		usr.copy_to(m_owner, sidSize);
		set_owner(m_sd, m_owner);
	} catch (...) {
	}

	try {
		Sid grp(gr);
		m_group = (PSID)::LocalAlloc(LPTR, sidSize);
		grp.copy_to(m_group, sidSize);
		set_group(m_sd, m_group);
	} catch (...) {
	}

	try {
		WinDacl(da).detach(m_dacl);
		set_dacl(m_sd, m_dacl);
	} catch (...) {
	}

	CheckApi(::IsValidSecurityDescriptor(m_sd));
	Protect(protect);
}

///========================================================================================== WinSDH
void	WinSDH::Get() {
	Free(m_sd);
	CheckApiError(::GetSecurityInfo(m_hnd, m_type,
									OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION |
									DACL_SECURITY_INFORMATION,
									nullptr, nullptr, nullptr, nullptr, &m_sd));
}

void	WinSDH::Set() const {
	SetSecurity(m_hnd, m_sd, m_type);
}

///========================================================================================== WinSDW
void	WinSDW::Get() {
	Free(m_sd);
	CheckApiError(::GetNamedSecurityInfoW((PWSTR)m_name.c_str(), m_type,
										  OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION |
										  DACL_SECURITY_INFORMATION,
										  nullptr, nullptr, nullptr, nullptr, &m_sd));
}

void	WinSDW::Set() const {
	SetSecurity(m_name, m_sd, m_type);
}

void	WinSDW::Set(const AutoUTF &path) const {
	SetSecurity(path, m_sd, m_type);
}
