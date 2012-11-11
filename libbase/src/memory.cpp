#include <libbase/memory.hpp>


namespace Base {

	namespace Memory {

		PVOID realloc_v(PVOID in, size_t size, DWORD flags) {
			return in ? ::HeapReAlloc(::GetProcessHeap(), flags, in, size) : ::HeapAlloc(::GetProcessHeap(), flags, size);
		}

		void free_v(PVOID in) {
			if (in)
				::HeapFree(::GetProcessHeap(), 0, in);
		}

	}

	auto_close<HANDLE>::auto_close(const this_type & rhs):
		m_ptr(nullptr) {
		::DuplicateHandle(::GetCurrentProcess(), rhs.m_ptr, ::GetCurrentProcess(), &m_ptr, 0, FALSE, DUPLICATE_SAME_ACCESS);
	}

	auto_close<HANDLE> & auto_close<HANDLE>::operator = (const this_type & rhs) {
		if (this != &rhs)
			this_type(rhs).swap(*this);
		return *this;
	}

	auto_close<HANDLE>::auto_close(this_type && rhs):
		m_ptr(nullptr) {
		swap(rhs);
	}

	auto_close<HANDLE> & auto_close<HANDLE>::operator = (this_type && rhs) {
		if (this != (this_type*)&(size_t&)rhs) // amazing operator & bypass
			this_type(std::move(rhs)).swap(*this);
		return *this;
	}

	auto_close<HANDLE>::value_type * auto_close<HANDLE>::operator & () {
		close();
		return &m_ptr;
	}


	void auto_close<HANDLE>::close() {
		if (is_valid()) {
			::CloseHandle(m_ptr);
			m_ptr = nullptr;
		}
	}

}
