#ifndef _LIBEXT_EXCEPTION_PVT_HPP_
#define _LIBEXT_EXCEPTION_PVT_HPP_

#include <libbase/std.hpp>
#include <libbase/shared_ptr.hpp>
#include <libext/exception.hpp>

#ifdef NDEBUG
#define THROW_PLACE_STR ustring()
#else
#define THROW_PLACE_STR ThrowPlaceString(file, line, func)
#endif

namespace Ext {

	extern PCSTR const THROW_PLACE_FORMAT;

	ustring ThrowPlaceString(PCSTR file, int line, PCSTR func);

	///=========================================================================== AbstractBaseError
	struct AbstractBaseError: public AbstractError {
		virtual ~AbstractBaseError();

		virtual AbstractBaseError * clone() const = 0;

		virtual ustring type() const = 0;

		virtual ustring what() const = 0;

		virtual DWORD code() const = 0;

		virtual void format_error(Base::mstring & out) const = 0;

		virtual PCWSTR where() const;

		virtual AbstractError * get_prev() const;

		virtual Base::mstring format_error() const;

	protected:
#ifndef NDEBUG
		AbstractBaseError(PCSTR file, size_t line, PCSTR func);
		AbstractBaseError(const AbstractError & prev, PCSTR file, size_t line, PCSTR func);
#else
		AbstractBaseError();
		AbstractBaseError(const AbstractError & prev);
#endif

	private:
#ifndef NDEBUG
		ustring	m_where;
#endif
		Base::shared_ptr<AbstractError> m_prev_exc;
	};


	///==================================================================================== WinError
	struct WinError: public AbstractBaseError {
		virtual WinError * clone() const;

		virtual ustring type() const;

		virtual ustring	 what() const;

		virtual DWORD code() const;

		virtual void format_error(Base::mstring & out) const;

	protected:
#ifndef NDEBUG
		WinError(PCSTR file, size_t line, PCSTR func);
		WinError(DWORD code, PCSTR file, size_t line, PCSTR func);
#else
		WinError();
		WinError(DWORD code);
#endif

	private:
		DWORD m_code;

		friend struct HiddenFunctions;
	};


	///==================================================================================== WmiError
	struct WmiError: public WinError {
		virtual WmiError * clone() const;

		virtual ustring type() const;

		virtual ustring	 what() const;

	private:
#ifndef NDEBUG
		WmiError(HRESULT code, PCSTR file, size_t line, PCSTR func);
#else
		WmiError(HRESULT code);
#endif

		friend struct HiddenFunctions;
	};


	///================================================================================== HMailError
	struct HMailError: public WinError {
		virtual HMailError * clone() const;

		virtual ustring type() const;

		virtual ustring	 what() const;

	private:
#ifndef NDEBUG
		HMailError(HRESULT code, PCSTR file, size_t line, PCSTR func);
#else
		HMailError(HRESULT code);
#endif

		friend struct HiddenFunctions;
	};


	///================================================================================== WSockError
	struct WSockError: public WinError {
		virtual WSockError * clone() const;

		virtual ustring type() const;

	private:
#ifndef NDEBUG
		WSockError(PCSTR file, size_t line, PCSTR func);
		WSockError(DWORD code, PCSTR file, size_t line, PCSTR func);
#else
		WSockError();
		WSockError(DWORD code);
#endif

		friend struct HiddenFunctions;
	};


}


#endif
