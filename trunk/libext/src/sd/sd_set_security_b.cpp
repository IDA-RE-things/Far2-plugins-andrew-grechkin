#include <libext/sd.hpp>
#include <libext/exception.hpp>
#include <libext/priv.hpp>
#include <libbase/bit.hpp>

namespace Ext {

	void set_security(HANDLE hnd, PSECURITY_DESCRIPTOR sd, SE_OBJECT_TYPE type) {
		DWORD flag = 0;

		PSID owner = WinSD::get_owner(sd);
		if (owner)
			flag |= OWNER_SECURITY_INFORMATION;

		PSID group = WinSD::get_group(sd);
		if (group)
			flag |= GROUP_SECURITY_INFORMATION;

		WORD control = WinSD::get_control(sd);

		PACL dacl = nullptr;
		if (Base::WinFlag::Check(control, (WORD)SE_DACL_PRESENT)) {
			dacl = WinSD::get_dacl(sd);
			flag |= DACL_SECURITY_INFORMATION;
			flag |= (Base::WinFlag::Check(control, (WORD)SE_DACL_PROTECTED)) ?
				PROTECTED_DACL_SECURITY_INFORMATION : UNPROTECTED_DACL_SECURITY_INFORMATION;
		}

		PACL sacl = nullptr;
//		if (Base::WinFlag::Check(control, (WORD)SE_SACL_PRESENT)) {
//			sacl = WinSD::get_sacl(sd);
//			flag |= SACL_SECURITY_INFORMATION;
//			flag |= (Base::WinFlag::Check(control, (WORD)SE_SACL_PROTECTED)) ?
//				PROTECTED_SACL_SECURITY_INFORMATION : UNPROTECTED_SACL_SECURITY_INFORMATION;
//		}

		DWORD err = ::SetSecurityInfo(hnd, type, flag, owner, group, dacl, sacl);

		if (err == ERROR_INVALID_OWNER) {
			WinPriv::modify(SE_TAKE_OWNERSHIP_NAME, true);
			WinPriv::modify(SE_RESTORE_NAME, true);
			err = ::SetSecurityInfo(hnd, type, flag, owner, group, dacl, sacl);
		}
		CheckApiError(err);
	}

}
