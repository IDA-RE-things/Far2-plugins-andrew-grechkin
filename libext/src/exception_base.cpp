#include "exception_pvt.hpp"

namespace Ext {

	AbstractBaseError::~AbstractBaseError() {
	}

#ifndef NDEBUG
	AbstractBaseError::AbstractBaseError(PCSTR file, size_t line, PCSTR func):
		m_where(THROW_PLACE_STR),
		m_prev_exc(nullptr) {
	}

	AbstractBaseError::AbstractBaseError(const AbstractError & prev, PCSTR file, size_t line, PCSTR func):
		m_where(THROW_PLACE_STR),
		m_prev_exc(prev.clone()) {
	}
#else
	AbstractBaseError::AbstractBaseError():
		m_prev_exc(nullptr){
	}

	AbstractBaseError::AbstractBaseError(const AbstractError & prev):
		m_prev_exc(prev.clone()) {
	}
#endif

	PCWSTR AbstractBaseError::where() const {
#ifndef NDEBUG
		return m_where.c_str();
#else
		return L"Programm compiled with NDEBUG define";
#endif
	}

	AbstractError * AbstractBaseError::get_prev() const {
		return m_prev_exc.get();
	}

	Base::mstring AbstractBaseError::format_error() const {
		Base::mstring msg;
		format_error(msg);
		return msg;
	}

}
