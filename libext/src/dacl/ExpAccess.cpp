#include <libext/dacl.hpp>
#include <libext/exception.hpp>

namespace Ext {

	///=================================================================================== ExpAccess
	ExpAccess::ExpAccess(PCWSTR name, ACCESS_MASK acc, ACCESS_MODE mode, DWORD inh) {
		::BuildExplicitAccessWithNameW(this, (PWSTR)name, acc, mode, inh);
	}

	ExpAccess::ExpAccess(PSID sid, ACCESS_MASK acc, ACCESS_MODE mode, DWORD inh) {
		::BuildExplicitAccessWithNameW(this, (PWSTR)Base::EMPTY_STR, acc, mode, inh);
		this->Trustee.ptstrName = (LPWSTR)sid;
		this->Trustee.TrusteeForm = TRUSTEE_IS_SID;
	}

	ustring ExpAccess::get_name() const {
		TRUSTEE_FORM tf = ::GetTrusteeFormW((PTRUSTEEW)&Trustee);
		if (tf == TRUSTEE_IS_NAME)
			return ustring(::GetTrusteeNameW((PTRUSTEEW)&Trustee));
		else if (tf != TRUSTEE_IS_SID)
			CheckApiError(ERROR_INVALID_PARAMETER);
		return Sid((PSID)Trustee.ptstrName).get_name();
	}

	Sid ExpAccess::get_sid() const {
		if (::GetTrusteeFormW((PTRUSTEEW)&Trustee) != TRUSTEE_IS_SID)
			CheckApiError(ERROR_INVALID_PARAMETER);
		return Sid((PSID)Trustee.ptstrName);
	}

}
