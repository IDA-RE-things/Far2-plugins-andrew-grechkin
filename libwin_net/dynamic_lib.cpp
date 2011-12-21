#include "win_net.h"

#include "exception.h"

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
