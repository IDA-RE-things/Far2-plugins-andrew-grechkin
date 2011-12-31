#include "memory.h"

namespace WinMem {
	PVOID Realloc(PVOID in, size_t size, DWORD flags) {
		if (in)
			in = ::HeapReAlloc(::GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS | flags, (PVOID)in, size);
		else
			in = ::HeapAlloc(::GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS | flags, size);
		return in;
	}
}

void auto_close<HANDLE>::close() {
	if (m_ptr && m_ptr != INVALID_HANDLE_VALUE) {
		::CloseHandle(m_ptr);
		m_ptr = nullptr;
	}
}

