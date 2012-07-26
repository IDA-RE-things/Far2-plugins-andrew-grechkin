#include <libext/sd.hpp>
#include <libext/exception.hpp>

namespace Ext {

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

}
