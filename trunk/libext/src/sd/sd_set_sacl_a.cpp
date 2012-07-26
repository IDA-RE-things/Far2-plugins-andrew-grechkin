#include <libext/sd.hpp>
#include <libext/exception.hpp>
#include <libext/priv.hpp>

namespace Ext {

	void set_sacl(HANDLE handle, PACL acl, SE_OBJECT_TYPE type) {
		DWORD err = ::SetSecurityInfo(handle, type, SACL_SECURITY_INFORMATION, nullptr,
		                              nullptr, nullptr, acl);
		if (err == ERROR_INVALID_OWNER) {
			WinPriv::modify(SE_SECURITY_NAME, true);
			err = ::SetSecurityInfo(handle, type, SACL_SECURITY_INFORMATION, nullptr,
			                        nullptr, nullptr, acl);
		}
		CheckApiError(err);
	}

}
