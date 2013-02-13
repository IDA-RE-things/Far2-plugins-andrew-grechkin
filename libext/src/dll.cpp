#include <libext/dll.hpp>

#include <libext/exception.hpp>

namespace Ext {

	///============================================================================== DinamicLibrary
	DynamicLibrary::~DynamicLibrary() noexcept
	{
	}

	DynamicLibrary::DynamicLibrary(PCWSTR path, DWORD flags) :
		base_type(CheckHandleErr(::LoadLibraryExW(path, nullptr, flags)), flags)
	{
	}

	DynamicLibrary::DynamicLibrary(HMODULE hndl, DWORD flags) :
		base_type(CheckHandleErr(hndl), flags)
	{
	}

	DynamicLibrary::DynamicLibrary(const this_type & rhs) :
		base_type(CheckHandleErr(::LoadLibraryExW(rhs.get_path().c_str(), nullptr, rhs.get_flags())), rhs.get_flags())
	{
	}

	DynamicLibrary::this_type & DynamicLibrary::operator = (const this_type & rhs)
	{
		if (this != &rhs)
			DynamicLibrary(rhs).swap(*this);
		return *this;
	}

	ustring DynamicLibrary::get_path() const
	{
		wchar_t buf[Base::MAX_PATH_LEN];
		CheckApi(base_type::get_path(buf, Base::lengthof(buf)));
		return ustring(buf);
	}

	FARPROC DynamicLibrary::get_function(PCSTR name) const
	{
		return CheckPointer(base_type::get_function(name));
	}


}
