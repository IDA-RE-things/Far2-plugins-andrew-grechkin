#ifndef _LIBEXT_DLL_HPP_
#define _LIBEXT_DLL_HPP_

#include <libbase/std.hpp>

namespace Ext {

	///============================================================================== DinamicLibrary
	class DynamicLibrary {
		typedef DynamicLibrary this_type;

	public:
		~DynamicLibrary();

		DynamicLibrary(PCWSTR path, DWORD flags = 0);

		DynamicLibrary(HMODULE hndl, DWORD flags = 0);

		DynamicLibrary(const this_type & rhs);

		this_type & operator = (const this_type & rhs);

		HMODULE get_hmodule() const;

		DWORD get_flags() const;

		ustring get_path() const;

		FARPROC get_function(PCSTR name) const;

		FARPROC get_function_nt(PCSTR name) const throw();

		FARPROC operator [] (PCSTR name) const throw();

		void swap(this_type & rhs);

	private:
		HMODULE m_hndl;
		DWORD m_flags;
	};

}

#endif
