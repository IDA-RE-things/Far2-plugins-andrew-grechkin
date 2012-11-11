#include <libcom/safearray.hpp>
#include <libext/exception.hpp>


namespace Com {

	SafeArray::~SafeArray() {
		::SafeArrayUnlock(m_ptr);
	}

	SafeArray::SafeArray(VARTYPE type, size_t size):
		m_ptr(CheckPointer(::SafeArrayCreateVector(type, 0, size))) {
		::SafeArrayLock(m_ptr);
	}

	SafeArray::SafeArray(SAFEARRAY * ptr):
		m_ptr(CheckPointer(ptr)) {
		::SafeArrayLock(m_ptr);
	}

	SafeArray::SafeArray(const Variant &var):
		m_ptr(CheckPointer(var.parray)) {
		::SafeArrayLock(m_ptr);
	}

	size_t SafeArray::dims() const {
		return m_ptr->cDims;
	}

	size_t SafeArray::size() const {
		return m_ptr->rgsabound[0].cElements;
	}

}
