#include "win_net.h"
#include "exception.h"

#include <authz.h>

void	SetSecurity(const ustring &path, const Sid &uid, const Sid &gid, mode_t mode, bool protect, SE_OBJECT_TYPE type) {
	SetSecurity(path, WinAbsSD(uid.name(), gid.name(), mode, protect), type);
}

///========================================================================================= WinSDDL
WinAbsSD::WinAbsSD(const ustring &name, const ustring &group, mode_t mode, bool protect) {
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
	try {
		return Authz(sid).access(psd);
	} catch (...) {
	}
	return 0;
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
