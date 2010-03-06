#include "win_def.h"

///========================================================================================== WinNet
namespace	WinNet {
bool		GetCompName(CStrW &buf, COMPUTER_NAME_FORMAT cnf) {
	DWORD	size = 0;
	::GetComputerNameExW(cnf, NULL, &size);
	buf.reserve(size);
	return	::GetComputerNameExW(cnf, buf.buffer(), &size) != 0;
}
}
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
