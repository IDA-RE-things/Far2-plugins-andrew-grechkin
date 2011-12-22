#include "mstring.h"

///========================================================================================= mstring
mstring::impl::~impl() {
	WinMem::Free(m_data);
}

mstring::impl::impl(PCWSTR in):
			m_size(0) {
	if (!in)
		in = EMPTY_STR;
	PCWSTR	ptr = in;
	while (*ptr) {
		ptr += (Len(ptr) + 1);
		++m_size;
	}
	m_capa = ptr - in + 1;
	WinMem::Alloc(m_data, sizeof(WCHAR) * m_capa);
	WinMem::Copy(m_data, in, m_capa * sizeof(WCHAR));
}

mstring::mstring(PCWSTR in):
	m_str(new impl(in)) {
}

size_t mstring::size() const {
	return m_str->m_size;
}

size_t mstring::capacity() const {
	return m_str->m_capa;
}

PCWSTR mstring::c_str() const {
	return m_str->m_data;
}

PCWSTR mstring::operator [](int index) const {
	PCWSTR	ptr = c_str();
	int		cnt = 0;
	while (*ptr && (cnt++ < index)) {
		ptr += (Len(ptr) + 1);
	}
	return ptr;
}
