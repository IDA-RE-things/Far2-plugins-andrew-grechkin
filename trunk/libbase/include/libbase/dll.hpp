#ifndef _LIBBASE_DLL_HPP_
#define _LIBBASE_DLL_HPP_

#include <libbase/std.hpp>

namespace Base {

	struct DynamicLibrary: private Uncopyable {
		virtual ~DynamicLibrary() {
			::FreeLibrary(m_hnd);
		}

		virtual bool is_valid() const {
			return m_hnd;
		}

		FARPROC get_function(PCSTR name) const {
			return ::GetProcAddress(m_hnd, name);
		}

		DynamicLibrary(PCWSTR path) :
			m_hnd(::LoadLibraryW(path)) {
		}

	private:
		HMODULE m_hnd;
	};

}

#endif
