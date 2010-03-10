#include "win_def.h"

namespace	WinGUID {
CStrW		Gen() {
	GUID  	guid;
	HRESULT	hr = ::CoCreateGuid(&guid);
	if (SUCCEEDED(hr)) {
		CStrW	szGUID(40);
		if (::StringFromGUID2(guid, szGUID.buffer(), szGUID.capacity()))
			return	szGUID;
	}
	return	CStrW(L"");
}
}
