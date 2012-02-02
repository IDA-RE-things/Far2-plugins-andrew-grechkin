#include "win_net.h"

#include "exception.h"

///================================================================================== DinamicLibrary
DynamicLibrary::~DynamicLibrary() {
	::FreeLibrary(m_hndl);
}

DynamicLibrary::DynamicLibrary(PCWSTR path, DWORD flags):
	m_hndl(CheckHandleErr(::LoadLibraryExW(path, nullptr, flags))),
	m_flags(flags) {
}

DynamicLibrary::DynamicLibrary(const this_type & rhs):
	m_hndl(CheckHandleErr(::LoadLibraryExW(rhs.get_path().c_str(), nullptr, rhs.m_flags))),
	m_flags(rhs.m_flags) {
}

DynamicLibrary::this_type & DynamicLibrary::operator =(const this_type & rhs) {
	if (this != &rhs)
		DynamicLibrary(rhs).swap(*this);
	return *this;
}

ustring DynamicLibrary::get_path() const {
	WCHAR buf[MAX_PATH_LEN];
	CheckApi(::GetModuleFileNameW(m_hndl, buf, lengthof(buf)));
	return ustring(buf);
}

FARPROC DynamicLibrary::get_function(PCSTR name) const {
	return CheckPointer(::GetProcAddress(m_hndl, name));
}

FARPROC DynamicLibrary::get_function_nt(PCSTR name) const throw() {
	return ::GetProcAddress(m_hndl, name);
}

void DynamicLibrary::swap(this_type & rhs) {
	using std::swap;
	swap(m_hndl, rhs.m_hndl);
	swap(m_flags, rhs.m_flags);
}
