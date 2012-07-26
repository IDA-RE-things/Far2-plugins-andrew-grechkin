#include <libext/sid.hpp>
#include <libext/exception.hpp>

namespace Ext {

	PSID Sid::get_sid(WELL_KNOWN_SID_TYPE wns) {
		DWORD size = SECURITY_MAX_SID_SIZE;
		PSID m_sid = (value_type)::LocalAlloc(LPTR, SECURITY_MAX_SID_SIZE);
		CheckApi(::CreateWellKnownSid(wns, nullptr, m_sid, &size));
		return m_sid;
	}

}
