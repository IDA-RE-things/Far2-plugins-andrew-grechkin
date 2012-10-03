#include <libbase/std.hpp>
#include <libbase/err.hpp>
#include <libbase/console.hpp>
#include <libext/exception.hpp>

#include <stdio.h>

using namespace Base;

namespace Ext {

	PCSTR const THROW_PLACE_FORMAT = "%s: %d [%s]";

#ifdef NDEBUG
#else
	ustring ThrowPlaceString(PCSTR file, int line, PCSTR func) {
		CHAR buf[MAX_PATH];
		buf[MAX_PATH-1] = 0;
		::snprintf(buf, sizeofa(buf) - 1, THROW_PLACE_FORMAT, file, line, func);
		return cp2w(buf, CP_UTF8);
	}
#endif

	///=============================================================================== AbstractError
	AbstractError::~AbstractError() {
	}

#ifndef NDEBUG
	AbstractError::AbstractError(PCSTR file, size_t line, PCSTR func):
		m_where(THROW_PLACE_STR),
		m_prev_exc(nullptr) {
	}

	AbstractError::AbstractError(const AbstractError & prev, PCSTR file, size_t line, PCSTR func):
		m_where(THROW_PLACE_STR),
		m_prev_exc(prev.clone()) {
	}
#else
	AbstractError::AbstractError():
		m_prev_exc(nullptr){
	}

	AbstractError::AbstractError(const AbstractError & prev):
		m_prev_exc(prev.clone()) {
	}
#endif

	PCWSTR AbstractError::where() const {
#ifndef NDEBUG
		return m_where.c_str();
#else
		return L"Programm compiled with NDEBUG define";
#endif
	}

	AbstractError * AbstractError::get_prev() const {
		return m_prev_exc.get();
	}

	Base::mstring AbstractError::format_error() const {
		Base::mstring msg;
		format_error(msg);
		return msg;
	}
}
