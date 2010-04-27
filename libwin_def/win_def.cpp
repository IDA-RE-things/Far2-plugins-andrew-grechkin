#include "win_def.h"

namespace	WinGUID {
AutoUTF		Gen() {
	GUID  	guid;
	HRESULT	hr = ::CoCreateGuid(&guid);
	if (SUCCEEDED(hr)) {
		WCHAR	szGUID[40];
		if (::StringFromGUID2(guid, szGUID, sizeofa(szGUID)))
			return	szGUID;
	}
	return	AutoUTF();
}
}

///====================================================================================== WinSysInfo
WinSysInfo::WinSysInfo() {
	if (Win64::IsWOW64())
		::GetNativeSystemInfo((LPSYSTEM_INFO)this);
	else
		::GetSystemInfo((LPSYSTEM_INFO)this);
}
size_t		WinSysInfo::Uptime(size_t del) {
	return	0;//::GetTickCount64() / del;
}

///========================================================================================= WinPerf
WinPerf::WinPerf() {
	WinMem::Zero(*this);
	typedef	BOOL (WINAPI * PFUNC)(PPERFORMANCE_INFORMATION pPerformanceInformation, DWORD cb);
	PFUNC	ProcAddr = (PFUNC)::GetProcAddress(::LoadLibraryW(L"psapi.dll"), "GetPerformanceInfo");
	if (ProcAddr)
		ProcAddr(this, sizeof(*this));
}

uintmax_t	Mega2Bytes(size_t in) {
	uintmax_t Result = -1ll;
	if (in > 0)
		Result = (uintmax_t)in << 20; // * 1024 * 1024;
	return	Result;
}
size_t		Bytes2Mega(uintmax_t in) {
	uintmax_t Result = 0ll;
	if (in > 0)
		Result = in >> 20; // / 1024 / 1024
	return	Result;
}
