#include <libext/sd.hpp>
#include <libext/exception.hpp>

#include <sddl.h>

namespace Ext {

	ustring WinSD::as_sddl(PSECURITY_DESCRIPTOR sd, SECURITY_INFORMATION in) {
		PWSTR str = nullptr;
		CheckApi(::ConvertSecurityDescriptorToStringSecurityDescriptorW(sd, SDDL_REVISION, in, &str, nullptr));
		ustring Result(str);
		::LocalFree(str);
		return Result;
	}

}
