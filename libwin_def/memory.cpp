#include "memory.h"

void auto_close<HANDLE>::close() {
	if (m_ptr && m_ptr != INVALID_HANDLE_VALUE) {
		::CloseHandle(m_ptr);
		m_ptr = nullptr;
	}
}
