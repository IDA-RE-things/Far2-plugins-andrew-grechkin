#include <libext/sid.hpp>
#include <libext/exception.hpp>
#include <libbase/memory.hpp>

extern "C" {
	WINADVAPI BOOL WINAPI ConvertSidToStringSidW(PSID Sid, PWSTR * StringSid);
}


namespace Ext {

	ustring Sid::as_str() const {
		return this_type::as_str(m_sid);
	}

	// PSID to sid string
	ustring Sid::as_str(value_type psid) {
		check(psid);
		Base::auto_close<PWSTR> ret(nullptr, ::LocalFree);
		CheckApi(::ConvertSidToStringSidW(psid, &ret));
		return ustring(ret);
	}

}
