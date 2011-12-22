#include "exception.h"
#include <libwin_def/win_def.h>
#include <libwin_def/console.h>

#include <stdio.h>

PCSTR const THROW_PLACE_FORMAT = "%s: %d [%s]";

#ifdef NDEBUG
#define THROW_PLACE_STR ustring()
#else
#define THROW_PLACE_STR ThrowPlaceString(file, line, func)

ustring ThrowPlaceString(PCSTR file, int line, PCSTR func) {
	CHAR buf[MAX_PATH];
	buf[MAX_PATH-1] = 0;
	::snprintf(buf, sizeofa(buf) - 1, THROW_PLACE_FORMAT, file, line, func);
	return ustring(buf, CP_UTF8);
}

#endif

///=================================================================================== AbstractError
AbstractError::~AbstractError() {
}

AbstractError::AbstractError() {
}

AbstractError::AbstractError(const AbstractError & prev):
	m_prev_exc(prev.clone()) {
}

#ifndef NDEBUG
AbstractError::AbstractError(PCSTR file, size_t line, PCSTR func):
	m_where(THROW_PLACE_STR) {
}

AbstractError::AbstractError(const AbstractError & prev, PCSTR file, size_t line, PCSTR func):
	m_where(THROW_PLACE_STR),
	m_prev_exc(prev.clone()) {
}
#endif

AbstractError * AbstractError::get_prev() const {
	return m_prev_exc.get();
}

///======================================================================================== WinError
WinError::WinError():
	m_code(::GetLastError()) {
}

WinError::WinError(DWORD code):
	m_code(code) {
}

#ifndef NDEBUG
WinError::WinError(PCSTR file, size_t line, PCSTR func):
	AbstractError(file, line, func),
	m_code(::GetLastError()) {
}

WinError::WinError(DWORD code, PCSTR file, size_t line, PCSTR func):
	AbstractError(file, line, func),
	m_code(code) {
}
#endif

WinError * WinError::clone() const {
//	printf(L"WinError::clone()\n");
	return new WinError(* this);
}

ustring WinError::type() const {
	return L"WinError";
}

ustring WinError::what() const {
	return ErrAsStr(code());
}

DWORD WinError::code() const {
	return m_code;
}

DWORD WinError::format_error() const {
	return WinError::format_error(*this);
}

DWORD WinError::format_error(const WinError & e) {
	printf(L"Exception: %s\nError: %s\nWhere: %s\n", e.type().c_str(), e.what().c_str(), e.where().c_str());
	return e.code();
}

///======================================================================================== WmiError
#ifndef NDEBUG
WmiError::WmiError(HRESULT code, PCSTR file, size_t line, PCSTR func):
	WinError(code, file, line, func) {
}
#else
WmiError::WmiError(HRESULT code):
	WinError(code) {
}
#endif

WmiError * WmiError::clone() const {
//	printf(L"WmiError::clone()\n");
 	return new WmiError(* this);
}

ustring WmiError::type() const {
	return L"WmiError";
}

ustring	WmiError::what() const {
	return ErrWmiAsStr(code());
}

///=================================================================================== WinLogicError
RuntimeError::RuntimeError(const ustring & what, size_t code):
	m_code(code),
	m_what(what) {
}

RuntimeError::RuntimeError(const AbstractError & prev, const ustring & what, size_t code):
	AbstractError(prev),
	m_code(code),
	m_what(what) {
}

#ifndef NDEBUG
RuntimeError::RuntimeError(const ustring & what, PCSTR file, size_t line, PCSTR func, size_t code):
	AbstractError(file, line, func),
	m_code(code),
	m_what(what) {
}

RuntimeError::RuntimeError(const AbstractError & prev, const ustring &what, PCSTR file, size_t line, PCSTR func, size_t code):
	AbstractError(prev, file, line, func),
	m_code(code),
	m_what(what) {
}
#endif

RuntimeError * RuntimeError::clone() const {
//	printf(L"RuntimeError::clone()\n");
	return new RuntimeError(* this);
}

ustring RuntimeError::type() const {
	return L"RuntimeError";
}

ustring RuntimeError::what() const {
	return m_what;
}

DWORD RuntimeError::code() const {
	return m_code;
}

///=================================================================================================
namespace HiddenFunctions {
#ifndef NDEBUG
	bool	CheckApiFunc(bool r, PCSTR file, size_t line, PCSTR func) {
		if (!r) {
			throw WinError(::GetLastError(), file, line, func);
		}
		return r;
	}

	bool	CheckApiThrowErrorFunc(bool r, DWORD err, PCSTR file, size_t line, PCSTR func) {
		if (!r) {
			throw WinError(err, file, line, func);
		}
		return r;
	}

	DWORD	CheckApiErrorFunc(DWORD err, PCSTR file, size_t line, PCSTR func) {
		if (err != ERROR_SUCCESS) {
			throw WinError(err, file, line, func);
		}
		return err;
	}

	HRESULT	CheckComFunc(HRESULT res, PCSTR file, size_t line, PCSTR func) {
		if (FAILED(res))
			throw	WinError(res, file, line, func);
		return res;
	}

	HRESULT	CheckWmiFunc(HRESULT res, PCSTR file, size_t line, PCSTR func) {
		if (res != S_OK)
			throw	WmiError(res, file, line, func);
		return res;
	}

	HANDLE	CheckHandleFuncHan(HANDLE hnd, PCSTR file, size_t line, PCSTR func) {
		if (!hnd || hnd == INVALID_HANDLE_VALUE) {
			throw	WinError(ERROR_INVALID_HANDLE, file, line, func);
		}
		return hnd;
	}

	HANDLE	CheckHandleErrFuncHan(HANDLE hnd, PCSTR file, size_t line, PCSTR func) {
		if (!hnd || hnd == INVALID_HANDLE_VALUE) {
			DWORD err = ::GetLastError();
			throw	WinError(err, file, line, func);
		}
		return hnd;
	}

	PVOID	CheckPointerFuncVoid(PVOID ptr, PCSTR file, size_t line, PCSTR func) {
		if (!ptr) {
			throw WinError(E_POINTER, file, line, func);
		}
		return ptr;
	}

	PVOID	CheckPointerErrFuncVoid(PVOID ptr, PCSTR file, size_t line, PCSTR func) {
		if (!ptr) {
			throw WinError(::GetLastError(), file, line, func);
		}
		return ptr;
	}

	void	RethrowExceptionFunc(const AbstractError & prev, const ustring & what, PCSTR file, size_t line, PCSTR func) {
		//	printf(L"RethrowExceptionFunc()\n");
		throw RuntimeError(prev, what, file, line, func);
	}

#else

	bool	CheckApiFunc(bool r) {
		if (!r) {
			throw WinError(::GetLastError());
		}
		return r;
	}

	bool	CheckApiThrowErrorFunc(bool r, DWORD err) {
		if (!r) {
			throw WinError(err);
		}
		return r;
	}

	DWORD	CheckApiErrorFunc(DWORD err) {
		if (err != ERROR_SUCCESS) {
			throw WinError(err);
		}
		return err;
	}

	HRESULT	CheckComFunc(HRESULT res) {
		if (FAILED(res))
			throw	WinError(res);
		return res;
	}

	HRESULT	CheckWmiFunc(HRESULT res) {
		if (res != S_OK)
			throw	WmiError(res);
		return res;
	}

	HANDLE	CheckHandleFuncHan(HANDLE hnd) {
		if (!hnd || hnd == INVALID_HANDLE_VALUE) {
			throw	WinError(ERROR_INVALID_HANDLE);
		}
		return hnd;
	}

	HANDLE	CheckHandleErrFuncHan(HANDLE hnd) {
		if (!hnd || hnd == INVALID_HANDLE_VALUE) {
			DWORD err = ::GetLastError();
			throw	WinError(err);
		}
		return hnd;
	}

	PVOID	CheckPointerFuncVoid(PVOID ptr) {
		if (!ptr) {
			throw WinError(E_POINTER);
		}
		return ptr;
	}

	PVOID	CheckPointerErrFuncVoid(PVOID ptr) {
		if (!ptr) {
			throw WinError(::GetLastError());
		}
		return ptr;
	}

	void	RethrowExceptionFunc(const AbstractError & prev, const ustring & what) {
		//	printf(L"RethrowExceptionFunc()\n");
		throw RuntimeError(prev, what);
	}

#endif
}
