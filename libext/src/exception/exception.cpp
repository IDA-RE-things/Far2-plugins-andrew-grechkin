#include <libbase/std.hpp>
#include <libbase/err.hpp>
#include <libbase/console.hpp>
#include <libext/exception.hpp>

#include <stdio.h>

using namespace Base;

namespace Ext {

	PCWSTR const THROW_PLACE_FORMAT = L"%S: %d [%S]";

#ifdef NDEBUG
#else
	ustring ThrowPlaceString(PCSTR file, int line, PCSTR func) {
		wchar_t buf[MAX_PATH];
		::_snwprintf(buf, Base::lengthof(buf), THROW_PLACE_FORMAT, file, line, func);
		return ustring(buf);
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
