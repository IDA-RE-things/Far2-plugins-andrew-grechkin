#ifndef _LIBEXT_DLL_HPP_
#define _LIBEXT_DLL_HPP_

#include <libbase/std.hpp>
#include <libbase/dll.hpp>

namespace Ext {

	///============================================================================== DinamicLibrary
	class DynamicLibrary: private Base::DynamicLibrary {
		typedef Base::DynamicLibrary base_type;
		typedef DynamicLibrary this_type;

	public:
		~DynamicLibrary() noexcept;

		DynamicLibrary(PCWSTR path, DWORD flags = 0);

		DynamicLibrary(HMODULE hndl, DWORD flags = 0);

		DynamicLibrary(const this_type & rhs);

		this_type & operator = (const this_type & rhs);

		using Base::DynamicLibrary::get_hmodule;
		using Base::DynamicLibrary::get_flags;

		ustring get_path() const;

		FARPROC get_function(PCSTR name) const;

		FARPROC get_function_nt(PCSTR name) const noexcept
		{
			return base_type::get_function(name);
		}

		void swap(this_type & rhs)
		{
			base_type::swap(rhs);
		}
	};

}

#endif
