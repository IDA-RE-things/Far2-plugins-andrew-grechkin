#include <libext/sd.hpp>
#include <libext/exception.hpp>
#include <libext/priv.hpp>

namespace Ext {

	void set_owner(HANDLE handle, PSID owner, SE_OBJECT_TYPE type) {
		DWORD err = ::SetSecurityInfo(handle, type, OWNER_SECURITY_INFORMATION,
		                              owner, nullptr, nullptr, nullptr);
		if (err == ERROR_INVALID_OWNER) {
			WinPriv::modify(SE_TAKE_OWNERSHIP_NAME, true);
//			Privilege rn(SE_RESTORE_NAME);
			err = ::SetSecurityInfo(handle, type, OWNER_SECURITY_INFORMATION,
			                        owner, nullptr, nullptr, nullptr);
		}
		CheckApiError(err);
	}

}
