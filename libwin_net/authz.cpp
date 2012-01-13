#include "win_net.h"
#include "exception.h"
#include "dacl.h"
#include "sid.h"
#include "sd.h"

#include <authz.h>

namespace {
	///=================================================================================== Authz_dll
	struct Authz_dll: private DynamicLibrary {
		typedef WINBOOL (WINAPI *FAuthzFreeContext)(AUTHZ_CLIENT_CONTEXT_HANDLE);
		typedef WINBOOL (WINAPI *FAuthzFreeResourceManager)(AUTHZ_RESOURCE_MANAGER_HANDLE);
		typedef WINBOOL (WINAPI *FAuthzInitializeResourceManager)(DWORD, PFN_AUTHZ_DYNAMIC_ACCESS_CHECK, PFN_AUTHZ_COMPUTE_DYNAMIC_GROUPS, PFN_AUTHZ_FREE_DYNAMIC_GROUPS, PCWSTR, PAUTHZ_RESOURCE_MANAGER_HANDLE);
		typedef WINBOOL (WINAPI *FAuthzInitializeContextFromSid)(DWORD, PSID, AUTHZ_RESOURCE_MANAGER_HANDLE, PLARGE_INTEGER, LUID, PVOID, PAUTHZ_CLIENT_CONTEXT_HANDLE);
		typedef WINBOOL (WINAPI *FAuthzAccessCheck)(DWORD, AUTHZ_CLIENT_CONTEXT_HANDLE, PAUTHZ_ACCESS_REQUEST, AUTHZ_AUDIT_EVENT_HANDLE, PSECURITY_DESCRIPTOR, PSECURITY_DESCRIPTOR *, DWORD, PAUTHZ_ACCESS_REPLY, PAUTHZ_ACCESS_CHECK_RESULTS_HANDLE);

		DEFINE_FUNC(AuthzFreeContext);
		DEFINE_FUNC(AuthzFreeResourceManager);
		DEFINE_FUNC(AuthzInitializeResourceManager);
		DEFINE_FUNC(AuthzInitializeContextFromSid);
		DEFINE_FUNC(AuthzAccessCheck);

		operator AUTHZ_RESOURCE_MANAGER_HANDLE() const {
			return m_hnd;
		}

		static Authz_dll & inst() {
			static Authz_dll ret;
			return ret;
		}

	private:
		~Authz_dll() {
			AuthzFreeResourceManager(m_hnd);
		}

		Authz_dll():
			DynamicLibrary(L"Authz.dll") {
			GET_DLL_FUNC(AuthzFreeContext);
			GET_DLL_FUNC(AuthzFreeResourceManager);
			GET_DLL_FUNC(AuthzInitializeResourceManager);
			GET_DLL_FUNC(AuthzInitializeContextFromSid);
			GET_DLL_FUNC(AuthzAccessCheck);
			CheckApi(AuthzInitializeResourceManager(AUTHZ_RM_FLAG_NO_AUDIT, nullptr,
			                                        nullptr, nullptr, nullptr, &m_hnd));
		}

		AUTHZ_RESOURCE_MANAGER_HANDLE m_hnd;
	};
}

void SetSecurity(const ustring & path, const Sid & uid, const Sid & gid, mode_t mode, bool protect, SE_OBJECT_TYPE type) {
	SetSecurity(path, WinAbsSD(uid.get_name(), gid.get_name(), mode, protect), type);
}

///========================================================================================= WinSDDL
WinAbsSD::WinAbsSD(const ustring &name, const ustring &group, mode_t mode, bool protect) {
	m_owner = m_group = m_dacl = m_sacl = nullptr;
	m_sd = (PSECURITY_DESCRIPTOR)::LocalAlloc(LPTR, sizeof(SECURITY_DESCRIPTOR));
	CheckApi(::InitializeSecurityDescriptor(m_sd, SECURITY_DESCRIPTOR_REVISION));

	WinDacl dacl(1024);
	dacl.set_access(Sid(WinWorldSid).get_name().c_str(), mode2access((mode) & 07));
	dacl.set_access(Sid(WinBuiltinIUsersSid).get_name().c_str(), mode2access((mode) & 07));
	dacl.set_access(Sid(WinIUserSid).get_name().c_str(), mode2access((mode) & 07));
	dacl.set_access(SidString(L"S-1-5-32-544").get_name().c_str(), mode2access(07));
	dacl.set_access(SidString(L"S-1-5-20").get_name().c_str(), mode2access(07));
	dacl.set_access(SidString(L"S-1-5-19").get_name().c_str(), mode2access(07));
	if (!name.empty()) {
		try {
			Sid usr(name);
			DWORD	ownerSize = SECURITY_MAX_SID_SIZE;
			m_owner = (PSID)::LocalAlloc(LPTR, ownerSize);
			usr.copy_to(m_owner, ownerSize);
			dacl.set_access(usr.get_name().c_str(), mode2access((mode >> 6) & 07));
		} catch (...) {
		}
	}

	if (!group.empty()) {
		try {
			DWORD	groupSize = SECURITY_MAX_SID_SIZE;
			m_group = (PSID)::LocalAlloc(LPTR, groupSize);
			Sid grp(group);
			grp.copy_to(m_group, groupSize);
			dacl.set_access(grp.get_name().c_str(), mode2access((mode >> 3) & 07));
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
	dacl.set_access(Sid(WinWorldSid).get_name().c_str(), mode2access((mode) & 07));
	dacl.set_access(Sid(WinBuiltinIUsersSid).get_name().c_str(), mode2access((mode) & 07));
	dacl.set_access(Sid(WinIUserSid).get_name().c_str(), mode2access((mode) & 07));
	dacl.set_access(SidString(L"S-1-5-32-544").get_name().c_str(), mode2access(07));
	dacl.set_access(SidString(L"S-1-5-20").get_name().c_str(), mode2access(07));
	dacl.set_access(SidString(L"S-1-5-19").get_name().c_str(), mode2access(07));
	try {
		Sid ow(WinCreatorOwnerSid);
		dacl.set_access(ow.get_name().c_str(), mode2access((mode >> 6) & 07));
	} catch (...) {
	}
	try {
		Sid gr(WinCreatorGroupSid);
		dacl.set_access(gr.get_name().c_str(), mode2access((mode >> 3) & 07));
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
struct Authz {
	~Authz() {
		Authz_dll::inst().AuthzFreeContext(m_cln);
	}

	Authz(PSID sid) {
		LUID unusedId = {0};
		CheckApi(Authz_dll::inst().AuthzInitializeContextFromSid(0, sid, Authz_dll::inst(), nullptr, unusedId, nullptr, &m_cln));
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

		CheckApi(Authz_dll::inst().AuthzAccessCheck(0, m_cln, &AccessRequest, nullptr, psd, nullptr, 0, &AccessReply, nullptr));
		return *(PACCESS_MASK)(AccessReply.GrantedAccessMask);
	}

private:
	AUTHZ_CLIENT_CONTEXT_HANDLE m_cln;
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
