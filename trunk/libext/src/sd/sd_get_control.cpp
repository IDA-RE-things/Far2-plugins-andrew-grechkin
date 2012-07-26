#include <libext/sd.hpp>
#include <libext/exception.hpp>
#include <libext/dacl.hpp>

namespace Ext {

	WORD WinSD::get_control(PSECURITY_DESCRIPTOR sd) {
		WORD Result = 0;
		DWORD Revision;
		CheckApi(::GetSecurityDescriptorControl(sd, &Result, &Revision));
		return Result;
	}

	PSID WinSD::get_owner(PSECURITY_DESCRIPTOR sd) {
		PSID psid;
		BOOL bTmp;
		CheckApi(::GetSecurityDescriptorOwner(sd, &psid, &bTmp));
		return psid;
	}

	PSID WinSD::get_group(PSECURITY_DESCRIPTOR sd) {
		PSID psid;
		BOOL bTmp;
		CheckApi(::GetSecurityDescriptorGroup(sd, &psid, &bTmp));
		return psid;
	}

	PACL WinSD::get_dacl(PSECURITY_DESCRIPTOR sd) {
		BOOL bDaclPresent   = false;
		BOOL bDaclDefaulted = false;
		PACL Result = nullptr;
		CheckApi(::GetSecurityDescriptorDacl(sd, &bDaclPresent, &Result, &bDaclDefaulted));
		CheckApi(bDaclPresent);
		return Result;
	}

	PACL WinSD::get_sacl(PSECURITY_DESCRIPTOR sd) {
		BOOL bSaclPresent   = false;
		BOOL bSaclDefaulted = false;
		PACL Result = nullptr;
		CheckApi(::GetSecurityDescriptorSacl(sd, &bSaclPresent, &Result, &bSaclDefaulted));
		CheckApi(bSaclPresent);
		return Result;
	}

	size_t WinSD::get_dacl_size(PSECURITY_DESCRIPTOR sd) {
		BOOL bDaclPresent   = false;
		BOOL bDaclDefaulted = false;
		PACL acl = nullptr;
		CheckApi(::GetSecurityDescriptorDacl(sd, &bDaclPresent, &acl, &bDaclDefaulted));
		return bDaclPresent ? WinDacl::size(acl) : 0;
	}

	size_t WinSD::get_sacl_size(PSECURITY_DESCRIPTOR sd) {
		BOOL bSaclPresent   = false;
		BOOL bSaclDefaulted = false;
		PACL acl = nullptr;
		CheckApi(::GetSecurityDescriptorSacl(sd, &bSaclPresent, &acl, &bSaclDefaulted));
		return bSaclPresent ? WinDacl::size(acl) : 0;
	}

}
