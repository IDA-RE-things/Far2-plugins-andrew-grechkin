#include "win_net.h"

#include "exception.h"

///========================================================================================== WinNet
namespace	WinNet {
ustring		GetCompName(COMPUTER_NAME_FORMAT cnf) {
	DWORD	size = 0;
	::GetComputerNameExW(cnf, nullptr, &size);
	WCHAR	buf[size];
	::GetComputerNameExW(cnf, buf, &size);
	return buf;
}
}

///==================================================================================== WinSysTimers
WinSysTimers::WinSysTimers() {
	WinMem::Zero(*this);
	typedef LONG(WINAPI * PROCNTQSI)(UINT, PVOID, ULONG, PULONG);

	PROCNTQSI NtQuerySystemInformation;

	NtQuerySystemInformation = (PROCNTQSI)::GetProcAddress(::GetModuleHandleW(L"ntdll"), "NtQuerySystemInformation");

	if (!NtQuerySystemInformation)
		return;

	NtQuerySystemInformation(3, this, sizeof(*this), 0);
}

size_t	WinSysTimers::Uptime(size_t del) {
	ULONGLONG	start = liKeBootTime.QuadPart;
	ULONGLONG	now = liKeSystemTime.QuadPart;
	ULONGLONG	Result = (now - start) / 10000000LL;
	return Result / del;
}

ustring	WinSysTimers::UptimeAsText() {
	size_t	uptime = Uptime();
	size_t	ud = uptime / (60 * 60 * 24);
	uptime %= (60 * 60 * 24);
	size_t	uh = uptime / (60 * 60);
	uptime %= (60 * 60);
	size_t	um = uptime  / 60;
	size_t	us = uptime % 60;
	ustring	Result;
	if (ud) {
		Result += Num2Str(ud);
		Result += L"d ";
	}

	Result += Num2Str(uh);
	Result += L":";
	Result += Num2Str(um);
	Result += L":";
	Result += Num2Str(us);
	return Result;
}

///===================================================================================== FileVersion
namespace {
	struct DLL_version {
		typedef DWORD (WINAPI *FGetFileVersionInfoSizeW)(LPCWSTR, LPDWORD);
		typedef WINBOOL (WINAPI *FGetFileVersionInfoW)(LPCWSTR, DWORD, DWORD, LPVOID);
		typedef WINBOOL (WINAPI *FVerQueryValueW)(const LPVOID, LPCWSTR, LPVOID *, PUINT);

		FGetFileVersionInfoSizeW GetFileVersionInfoSizeW;
		FGetFileVersionInfoW GetFileVersionInfoW;
		FVerQueryValueW VerQueryValueW;

		static DLL_version & inst() {
			static DLL_version ret;
			return ret;
		}

	private:
		DLL_version():
			dll(L"version.dll") {
			GetFileVersionInfoSizeW = (FGetFileVersionInfoSizeW)dll.get_function("GetFileVersionInfoSizeW");
			GetFileVersionInfoW = (FGetFileVersionInfoW)dll.get_function("GetFileVersionInfoW");
			VerQueryValueW = (FVerQueryValueW)dll.get_function("VerQueryValueW");
		}

		DynamicLibrary dll;
	};
}

FileVersion::FileVersion(PCWSTR path) {
	DWORD size = DLL_version::inst().GetFileVersionInfoSizeW(path, nullptr);
	CheckApi(size);
	auto_array<BYTE> data(size);
	CheckApi(DLL_version::inst().GetFileVersionInfoW(path, 0, size, data.data()));
	UINT bufLen;
	VS_FIXEDFILEINFO * ffi;
	CheckApi(DLL_version::inst().VerQueryValueW(data, L"\\", (PVOID*)&ffi, &bufLen));
	WCHAR tmp[MAX_PATH];
	_snwprintf(tmp, sizeofa(tmp), L"%d.%d"
	           ,HIWORD(ffi->dwFileVersionMS)
	           ,LOWORD(ffi->dwFileVersionMS)
//	           ,HIWORD(ffi->dwFileVersionLS)
//	           LOWORD(ffi->dwFileVersionLS)
	);
	m_ver = tmp;
}

///================================================================================== DinamicLibrary
DynamicLibrary::~DynamicLibrary() {
	::FreeLibrary(m_hnd);
}

DynamicLibrary::DynamicLibrary(const ustring & path):
	m_hnd(CheckHandleErr(::LoadLibraryW(path.c_str()))),
	m_path(path) {
}

DynamicLibrary::DynamicLibrary(const this_type & rhs):
	m_hnd(CheckHandleErr(::LoadLibraryW(rhs.m_path.c_str()))),
	m_path(rhs.m_path) {
}

DynamicLibrary::this_type & DynamicLibrary::operator =(const this_type & rhs) {
	if (this != &rhs)
		DynamicLibrary(rhs).swap(*this);
	return *this;
}

FARPROC DynamicLibrary::get_function_nt(PCSTR name) const throw() {
	return ::GetProcAddress(m_hnd, name);
}

FARPROC DynamicLibrary::get_function(PCSTR name) const {
	return CheckPointer(::GetProcAddress(m_hnd, name));
}

void DynamicLibrary::swap(this_type & rhs) {
	using std::swap;
	swap(m_hnd, rhs.m_hnd);
	swap(m_path, rhs.m_path);
}
