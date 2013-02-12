#ifndef _LIBBASE_DLL_HPP_
#define _LIBBASE_DLL_HPP_

#include <libbase/std.hpp>
#include <libbase/uncopyable.hpp>

namespace Base {

	///============================================================================== DinamicLibrary
	struct DynamicLibrary: private Uncopyable {
		virtual ~DynamicLibrary() noexcept
		{
			::FreeLibrary(m_hnd);
		}

		virtual bool is_valid() const noexcept
		{
			return m_hnd;
		}

	public:
		DynamicLibrary(PCWSTR path, DWORD flags = 0) noexcept :
			m_hnd(::LoadLibraryExW(path, nullptr, flags)),
			m_flags(flags)
		{
		}

		DynamicLibrary(HMODULE hndl, DWORD flags) noexcept :
			m_hnd(hndl),
			m_flags(flags)
		{
		}

		DynamicLibrary(DynamicLibrary && right) noexcept :
			m_hnd(right.m_hnd),
			m_flags(right.m_flags)
		{
			right.m_hnd = nullptr;
			right.m_flags = 0;
		}

		DynamicLibrary & operator = (DynamicLibrary && right) noexcept
		{
			if (this != &right)
				DynamicLibrary(std::move(right)).swap(*this);
			return *this;
		}

		void swap(DynamicLibrary & right) noexcept
		{
			using std::swap;
			swap(m_hnd, right.m_hnd);
			swap(m_flags, right.m_flags);
		}

		operator HMODULE () const noexcept
		{
			return m_hnd;
		}

		HMODULE get_hmodule() const noexcept
		{
			return m_hnd;
		}

		DWORD get_flags() const noexcept
		{
			return m_flags;
		}

		bool get_path(PWSTR path, size_t size) const noexcept
		{
			return ::GetModuleFileNameW(m_hnd, path, size);
		}

		FARPROC get_function(PCSTR name) const noexcept
		{
			return ::GetProcAddress(m_hnd, name);
		}

	private:
		HMODULE m_hnd;
		DWORD m_flags;
	};

}

#endif
