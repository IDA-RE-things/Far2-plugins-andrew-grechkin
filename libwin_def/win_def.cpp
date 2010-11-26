#include "win_def.h"

///========================================================================================== WinVol
void WinVol::Close() {
	if (m_hnd != INVALID_HANDLE_VALUE) {
		::FindVolumeClose(m_hnd);
		m_hnd = INVALID_HANDLE_VALUE;
	}
}

bool WinVol::Next() {
	WCHAR buf[MAX_PATH];
	if (m_hnd != INVALID_HANDLE_VALUE) {
		ChkSucc(::FindNextVolumeW(m_hnd, buf, sizeofa(buf)));
	} else {
		m_hnd = ::FindFirstVolumeW(buf, sizeofa(buf));
		ChkSucc(m_hnd != INVALID_HANDLE_VALUE);
	}
	if (IsOK()) {
		name = buf;
	}
	return IsOK();
}

AutoUTF WinVol::GetPath() const {
	AutoUTF Result;
	if (IsOK()) {
		DWORD size;
		::GetVolumePathNamesForVolumeNameW(name.c_str(), nullptr, 0, &size);
		if (::GetLastError() == ERROR_MORE_DATA) {
			WinBuf<WCHAR> buf(size);
			::GetVolumePathNamesForVolumeNameW(name.c_str(), buf, size, &size);
			Result = buf.data();
			CutWord(Result, L"\\");
		}
	}
	return Result;
}

AutoUTF WinVol::GetDevice() const {
	WinBuf<WCHAR> Result(MAX_PATH);
	::QueryDosDeviceW(GetPath().c_str(), Result, Result.capacity());
	return (PWSTR)Result;
}

bool WinVol::GetSize(uint64_t &uiUserFree, uint64_t &uiTotalSize, uint64_t &uiTotalFree) const {
	UINT mode = ::SetErrorMode(SEM_FAILCRITICALERRORS);
	bool Result = ::GetDiskFreeSpaceExW(name.c_str(), (PULARGE_INTEGER)&uiUserFree,
	                                    (PULARGE_INTEGER)&uiTotalSize,
	                                    (PULARGE_INTEGER)&uiTotalFree);
	::SetErrorMode(mode);
	return Result;
}

///====================================================================================== WinSysInfo
WinSysInfo::WinSysInfo() {
	if (Win64::IsWOW64())
		::GetNativeSystemInfo((LPSYSTEM_INFO)this);
	else
		::GetSystemInfo((LPSYSTEM_INFO)this);
}

size_t WinSysInfo::Uptime(size_t del) {
	return 0;//::GetTickCount64() / del;
}

///===================================================================================== Binary type
NamedValues<DWORD> BinaryType[] = {{(DWORD)-1, L"UNKNOWN"}, {SCS_32BIT_BINARY, L"x32"},
                                   {SCS_64BIT_BINARY, L"x64"}, {SCS_DOS_BINARY, L"dos"},
                                   {SCS_OS216_BINARY, L"os2x16"}, {SCS_PIF_BINARY, L"pif"},
                                   {SCS_POSIX_BINARY, L"posix"}, {SCS_WOW_BINARY, L"x16"}, };
