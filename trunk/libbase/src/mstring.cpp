#include <libbase/mstring.hpp>

#include <libbase/memory.hpp>
#include <libbase/str.hpp>

///========================================================================================= mstring
namespace Base {

	mstring::impl::~impl() {
		Memory::free(m_data);
	}

	mstring::impl::impl(PCWSTR in) :
		m_size(0) {
		if (!in)
			in = EMPTY_STR;
		PCWSTR ptr = in;
		while (*ptr) {
			ptr += (get_str_len(ptr) + 1);
			++m_size;
		}
		m_capa = ptr - in + 1;
		Memory::alloc(m_data, sizeof(WCHAR) * m_capa);
		Memory::copy(m_data, in, m_capa * sizeof(WCHAR));
	}

	mstring::mstring(PCWSTR in) :
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

	PCWSTR mstring::operator [](size_t index) const {
		PCWSTR ptr = c_str();
		size_t cnt = 0;
		while (*ptr && (cnt++ < index)) {
			ptr += (get_str_len(ptr) + 1);
		}
		return ptr;
	}

}
