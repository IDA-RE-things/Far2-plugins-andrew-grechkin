﻿#include "exception_pvt.hpp"
#include <libbase/logger.hpp>

namespace Ext {

	///================================================================================ RuntimeError
	struct RuntimeError: public AbstractError {
		virtual RuntimeError * clone() const;

		virtual ustring type() const;

		virtual ustring what() const;

		virtual DWORD code() const;

		virtual void format_error(Base::mstring & out) const;

	protected:
#ifndef NDEBUG
		RuntimeError(const ustring & wh, PCSTR file, size_t line, PCSTR func, size_t code = 0);
		RuntimeError(const AbstractError & prev, const ustring & wh, PCSTR file, size_t line, PCSTR func, size_t code = 0);
#else
		RuntimeError(const ustring & wh, size_t code = 0);
		RuntimeError(const AbstractError & prev, const ustring & wh, size_t code = 0);
#endif

	private:
		size_t m_code;
		ustring m_what;

		friend struct HiddenFunctions;
	};

	///================================================================================ RuntimeError
#ifndef NDEBUG
	RuntimeError::RuntimeError(const ustring & wh, PCSTR file, size_t line, PCSTR func, size_t code) :
		AbstractError(file, line, func),
		m_code(code),
		m_what(wh)
	{
		LogNoise(L"%s\n", what().c_str());
	}

	RuntimeError::RuntimeError(const AbstractError & prev, const ustring & wh, PCSTR file, size_t line, PCSTR func, size_t code) :
		AbstractError(prev, file, line, func),
		m_code(code),
		m_what(wh)
	{
		LogNoise(L"%s\n", what().c_str());
	}
#else
	RuntimeError::RuntimeError(const ustring & wh, size_t code) :
		m_code(code),
		m_what(wh)
	{
		LogNoise(L"%s\n", what().c_str());
	}

	RuntimeError::RuntimeError(const AbstractError & prev, const ustring & wh, size_t code) :
		AbstractError(prev),
		m_code(code),
		m_what(wh)
	{
		LogNoise(L"%s\n", what().c_str());
	}
#endif

	RuntimeError * RuntimeError::clone() const
	{
		return new RuntimeError(*this);
	}

	ustring RuntimeError::type() const
	{
		return L"RuntimeError";
	}

	ustring RuntimeError::what() const
	{
		return m_what;
	}

	DWORD RuntimeError::code() const
	{
		return m_code;
	}

	void RuntimeError::format_error(Base::mstring & out) const
	{
		out.push_back(what().c_str());
		if (get_prev()) {
			get_prev()->format_error(out);
		}
	}

#ifndef NDEBUG
	void HiddenFunctions::RethrowExceptionFunc(const AbstractError & prev, const ustring & what, PCSTR file, size_t line, PCSTR func)
	{
		throw RuntimeError(prev, what, file, line, func, prev.code());
	}
#else
	void HiddenFunctions::RethrowExceptionFunc(const AbstractError & prev, const ustring & what) {
		throw RuntimeError(prev, what, prev.code());
	}
#endif
}
