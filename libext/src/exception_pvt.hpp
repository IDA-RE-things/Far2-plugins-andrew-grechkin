#ifndef _LIBEXT_EXCEPTION_PVT_HPP_
#define _LIBEXT_EXCEPTION_PVT_HPP_

#include <libbase/std.hpp>
#include <libext/exception.hpp>

namespace Ext {

	///==================================================================================== WinError
	struct WinError: public AbstractError {
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
