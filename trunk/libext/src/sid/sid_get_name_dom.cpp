#include <libext/sid.hpp>
#include <libext/exception.hpp>

namespace Ext {

	void Sid::get_name_dom(value_type sid, ustring & name, ustring & dom, PCWSTR srv) {
		check(sid);
		DWORD size_nam = 0;
		DWORD size_dom = 0;
		SID_NAME_USE type;

		// determine size of name
		::LookupAccountSidW(srv, sid, nullptr, &size_nam, nullptr, &size_dom, &type);
		CheckApi(::GetLastError() == ERROR_INSUFFICIENT_BUFFER);
		wchar_t pName[size_nam];
		wchar_t pDom[size_dom];

		CheckApi(::LookupAccountSidW(srv, sid, pName, &size_nam, pDom, &size_dom, &type));
		name = pName;
		dom = pDom;
	}

}
