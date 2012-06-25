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

	void auto_close<HANDLE>::close() {
		if (m_ptr && m_ptr != INVALID_HANDLE_VALUE) {
			::CloseHandle(m_ptr);
			m_ptr = nullptr;
		}
	}

}
