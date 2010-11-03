#include "win_net.h"

namespace WinGUID {
	AutoUTF Gen() {
		GUID guid;
		HRESULT hr = ::CoCreateGuid(&guid);
		if (SUCCEEDED(hr)) {
			WCHAR szGUID[40];
			if (::StringFromGUID2(guid, szGUID, sizeofa(szGUID)))
				return szGUID;
		}
		return AutoUTF();
	}
}

///========================================================================================== WinNet
namespace	WinNet {
AutoUTF		GetCompName(COMPUTER_NAME_FORMAT cnf) {
	DWORD	size = 0;
	::GetComputerNameExW(cnf, nullptr, &size);
	WCHAR	buf[size];
	::GetComputerNameExW(cnf, buf, &size);
	return	buf;
}
}

///==================================================================================== WinSysTimers
AutoUTF					WinSysTimers::UptimeAsText() {
	size_t	uptime = Uptime();
	size_t	ud = uptime / (60 * 60 * 24);
	uptime %= (60 * 60 * 24);
	size_t	uh = uptime / (60 * 60);
	uptime %= (60 * 60);
	size_t	um = uptime  / 60;
	size_t	us = uptime % 60;
	AutoUTF	Result;
	if (ud) {
		Result += Num2Str(ud);
		Result += L"d ";
	}

	Result += Num2Str(uh);
	Result += L":";
	Result += Num2Str(um);
	Result += L":";
	Result += Num2Str(us);
	return	Result;
}
