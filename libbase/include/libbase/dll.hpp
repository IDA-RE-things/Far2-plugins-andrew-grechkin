#ifndef _LIBBASE_DLL_HPP_
#define _LIBBASE_DLL_HPP_

#include <libbase/std.hpp>
#include <libbase/uncopyable.hpp>

namespace Base {

	struct DynamicLibrary: private Uncopyable {
		virtual ~DynamicLibrary()
		{
			::FreeLibrary(m_hnd);
		}

		virtual bool is_valid() const
		{
			return m_hnd;
		}

	public:
		DynamicLibrary(PCWSTR path) :
			m_hnd(::LoadLibraryW(path))
		{
		}

		DynamicLibrary(DynamicLibrary && right) :
			m_hnd(right.m_hnd)
		{
			right.m_hnd = nullptr;
		}

		DynamicLibrary & operator = (DynamicLibrary && right)
		{
			if (this != &right)
				DynamicLibrary(std::move(right)).swap(*this);
			return *this;
		}

		void swap(DynamicLibrary & right)
		{
			using std::swap;
			swap(m_hnd, right.m_hnd);
		}

		FARPROC get_function(PCSTR name) const
		{
			return ::GetProcAddress(m_hnd, name);
		}

	private:
		HMODULE m_hnd;
	};

}

#endif
