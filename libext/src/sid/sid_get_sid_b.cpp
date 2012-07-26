#include <libext/sid.hpp>
#include <libext/exception.hpp>

namespace Ext {

	PSID Sid::get_sid(PCWSTR name, PCWSTR srv) {
		DWORD size_sid = SECURITY_MAX_SID_SIZE;
		PSID m_sid = (value_type)::LocalAlloc(LPTR, size_sid);
		DWORD size_dom = MAX_PATH;
		WCHAR dom[size_dom];
		SID_NAME_USE type;
		CheckApi(::LookupAccountNameW(srv, name, m_sid, &size_sid, dom, &size_dom, &type));
		return m_sid;
	}

}
