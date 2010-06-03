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

///========================================================================================== WinVol
void					WinVol::Close() {
	if (m_hnd != INVALID_HANDLE_VALUE) {
		::FindVolumeClose(m_hnd);
		m_hnd = INVALID_HANDLE_VALUE;
	}
}

bool 					WinVol::Next() {
	WCHAR	buf[MAX_PATH];
	if (m_hnd != INVALID_HANDLE_VALUE) {
		ChkSucc(::FindNextVolumeW(m_hnd, buf, sizeofa(buf)));
	} else {
		m_hnd = ::FindFirstVolumeW(buf, sizeofa(buf));
		ChkSucc(m_hnd != INVALID_HANDLE_VALUE);
	}
	if (IsOK()) {
		name = buf;
	}
	return	IsOK();
}

AutoUTF					WinVol::GetPath() const {
	AutoUTF	Result;
	if (IsOK()) {
		DWORD	size;
		::GetVolumePathNamesForVolumeNameW(name.c_str(), NULL, 0, &size);
		if (::GetLastError() == ERROR_MORE_DATA) {
			WinBuf<WCHAR> buf(size);
			::GetVolumePathNamesForVolumeNameW(name.c_str(), buf, size, &size);
			Result = buf.data();
			Result.CutWord(L"\\");
		}
	}
	return	Result;
}
AutoUTF					WinVol::GetDevice() const {
	WinBuf<WCHAR> Result(MAX_PATH);
	::QueryDosDeviceW(GetPath().c_str(), Result, Result.capacity());
	return	(PWSTR)Result;
}

bool					WinVol::GetSize(uint64_t &uiUserFree, uint64_t &uiTotalSize, uint64_t &uiTotalFree) const {
	UINT	mode = ::SetErrorMode(SEM_FAILCRITICALERRORS);
	bool	Result = ::GetDiskFreeSpaceExW(name.c_str(),
										(PULARGE_INTEGER) & uiUserFree,
										(PULARGE_INTEGER) & uiTotalSize,
										(PULARGE_INTEGER) & uiTotalFree);
	::SetErrorMode(mode);
	return	Result;
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
