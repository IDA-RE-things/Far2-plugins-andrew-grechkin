#include <libext/sd.hpp>
#include <libext/exception.hpp>
#include <libext/priv.hpp>

namespace Ext {

	void set_sacl(PCWSTR path, PACL pacl, SE_OBJECT_TYPE type) {
		DWORD err = ::SetNamedSecurityInfoW((PWSTR)path, type, SACL_SECURITY_INFORMATION,
		                                    nullptr, nullptr, nullptr, pacl);
		if (err == ERROR_INVALID_OWNER) {
			WinPriv::modify(SE_SECURITY_NAME, true);
			err = ::SetNamedSecurityInfoW((PWSTR)path, type, SACL_SECURITY_INFORMATION,
			                              nullptr, nullptr, nullptr, pacl);
		}
		CheckApiError(err);
	}

}
