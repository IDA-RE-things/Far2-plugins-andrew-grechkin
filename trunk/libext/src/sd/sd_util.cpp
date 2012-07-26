#include <libext/sd.hpp>
#include <libext/exception.hpp>
#include <libext/sid.hpp>

namespace Ext {

	ustring get_owner(HANDLE hnd, SE_OBJECT_TYPE type) {
		return Sid::get_name(WinSDH(hnd, type).get_owner());
	}

	ustring get_owner(const ustring & path, SE_OBJECT_TYPE type) {
		return Sid::get_name(WinSDW(path, type).get_owner());
	}


	ustring get_group(HANDLE hnd, SE_OBJECT_TYPE type) {
		return Sid::get_name(WinSDH(hnd, type).get_group());
	}

	ustring get_group(const ustring &path, SE_OBJECT_TYPE type) {
		return Sid::get_name(WinSDW(path, type).get_group());
	}


	void set_group(HANDLE handle, PSID owner, SE_OBJECT_TYPE type) {
		CheckApiError(::SetSecurityInfo(handle, type, GROUP_SECURITY_INFORMATION, nullptr, owner, nullptr, nullptr));
	}

	void set_group(PCWSTR path, PSID owner, SE_OBJECT_TYPE type) {
		CheckApiError(::SetNamedSecurityInfoW((PWSTR)path, type, GROUP_SECURITY_INFORMATION, nullptr, owner, nullptr, nullptr));
	}

	void set_dacl(HANDLE handle, PACL acl, SE_OBJECT_TYPE type) {
		CheckApiError(::SetSecurityInfo(handle, type, DACL_SECURITY_INFORMATION, nullptr, nullptr, acl, nullptr));
	}

	void set_dacl(PCWSTR path, PACL pacl, SE_OBJECT_TYPE type) {
		CheckApiError(::SetNamedSecurityInfoW((PWSTR)path, type, DACL_SECURITY_INFORMATION, nullptr, nullptr, pacl, nullptr));
	}

//	void SetSecurity(const ustring &path, const ustring &sddl, SE_OBJECT_TYPE type) {
//		WinSDDL sd(sddl);
//		SetSecurity(path, sd, type);
//	}

}
