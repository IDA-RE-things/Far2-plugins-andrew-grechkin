#include <libbase/std.hpp>
#include <libbase/err.hpp>
#include <libbase/console.hpp>
#include <libext/exception.hpp>

#include <stdio.h>

using namespace Base;

namespace Ext {

	PCSTR const THROW_PLACE_FORMAT = "%s: %d [%s]";

#ifdef NDEBUG
#define THROW_PLACE_STR ustring()
#else
#define THROW_PLACE_STR ThrowPlaceString(file, line, func)

	ustring ThrowPlaceString(PCSTR file, int line, PCSTR func) {
		CHAR buf[MAX_PATH];
		buf[MAX_PATH-1] = 0;
		::snprintf(buf, sizeofa(buf) - 1, THROW_PLACE_FORMAT, file, line, func);
		return cp2w(buf, CP_UTF8);
	}

#endif

	///=================================================================================== AbstractError
	AbstractError::~AbstractError() {
	}

#ifndef NDEBUG
	AbstractError::AbstractError(PCSTR file, size_t line, PCSTR func):
		m_where(THROW_PLACE_STR) {
	}

	AbstractError::AbstractError(const AbstractError & prev, PCSTR file, size_t line, PCSTR func):
		m_where(THROW_PLACE_STR),
		m_prev_exc(prev.clone()) {
	}
#else
	AbstractError::AbstractError() {
	}

	AbstractError::AbstractError(const AbstractError & prev):
		m_prev_exc(prev.clone()) {
	}
#endif

	AbstractError * AbstractError::get_prev() const {
		return m_prev_exc.get();
	}

	///======================================================================================== WinError
#ifndef NDEBUG
	WinError::WinError(PCSTR file, size_t line, PCSTR func):
		AbstractError(file, line, func),
		m_code(::GetLastError()) {
	}

	WinError::WinError(DWORD code, PCSTR file, size_t line, PCSTR func):
		AbstractError(file, line, func),
		m_code(code) {
	}
#else
	WinError::WinError():
		m_code(::GetLastError()) {
	}

	WinError::WinError(DWORD code):
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

	void WinError::format_error(Base::mstring & out) const {
		WCHAR buf[MAX_PATH_LEN] = {0};

		_snwprintf(buf, lengthof(buf), L"Error: %s", what().c_str());
		out.push_back(buf);
#ifndef NDEBUG
		_snwprintf(buf, lengthof(buf), L"Exception: %s", type().c_str());
		out.push_back(buf);
		_snwprintf(buf, lengthof(buf), L"Where: %s", where().c_str());
		out.push_back(buf);
#endif
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
		return ErrAsStrWmi(code());
	}

	///====================================================================================== HMailError
#ifndef NDEBUG
	HMailError::HMailError(HRESULT code, PCSTR file, size_t line, PCSTR func):
		WinError(code, file, line, func) {
	}
#else
	HMailError::HMailError(HRESULT code):
		WinError(code) {
	}
#endif

	HMailError * HMailError::clone() const {
		return new HMailError(*this);
	}

	ustring HMailError::type() const {
		return L"HMailError";
	}

	ustring	HMailError::what() const {
		return ErrAsStr(code(), L"hMailServer.exe");
	}

	///=================================================================================== WinLogicError
#ifndef NDEBUG
	RuntimeError::RuntimeError(const ustring & what, PCSTR file, size_t line, PCSTR func, size_t code):
		AbstractError(file, line, func),
		m_code(code),
		m_what(what) {
	}

	RuntimeError::RuntimeError(const AbstractError & prev, const ustring & what, PCSTR file, size_t line, PCSTR func, size_t code):
		AbstractError(prev, file, line, func),
		m_code(code),
		m_what(what) {
	}
#else
	RuntimeError::RuntimeError(const ustring & what, size_t code):
		m_code(code),
		m_what(what) {
	}

	RuntimeError::RuntimeError(const AbstractError & prev, const ustring & what, size_t code):
		AbstractError(prev),
		m_code(code),
		m_what(what) {
	}
#endif

	RuntimeError * RuntimeError::clone() const {
		//	printf(L"RuntimeError::clone()\n");
		return new RuntimeError(*this);
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

	void RuntimeError::format_error(Base::mstring & out) const {
		out.push_back(what().c_str());
		if (get_prev()) {
			get_prev()->format_error(out);
		}
	}

	///=================================================================================================
#ifndef NDEBUG
	bool HiddenFunctions::CheckApiFunc(bool r, PCSTR file, size_t line, PCSTR func) {
		if (!r) {
			DWORD err = ::GetLastError();
			throw WinError(err, file, line, func);
		}
		return r;
	}

	bool HiddenFunctions::CheckApiThrowErrorFunc(bool r, DWORD err, PCSTR file, size_t line, PCSTR func) {
		if (!r) {
			throw WinError(err, file, line, func);
		}
		return r;
	}

	DWORD HiddenFunctions::CheckApiErrorFunc(DWORD err, PCSTR file, size_t line, PCSTR func) {
		if (err != ERROR_SUCCESS) {
			throw WinError(err, file, line, func);
		}
		return err;
	}

	HRESULT HiddenFunctions::CheckHMailErrorFunc(HRESULT err, PCSTR file, size_t line, PCSTR func) {
		if (err != ERROR_SUCCESS) {
			throw HMailError(err, file, line, func);
		}
		return err;
	}

	HRESULT HiddenFunctions::CheckComFunc(HRESULT res, PCSTR file, size_t line, PCSTR func) {
		if (FAILED(res))
			throw	WinError(res, file, line, func);
		return res;
	}

	HRESULT HiddenFunctions::CheckWmiFunc(HRESULT res, PCSTR file, size_t line, PCSTR func) {
		if (res != S_OK)
			throw	WmiError(res, file, line, func);
		return res;
	}

	HANDLE HiddenFunctions::CheckHandleFuncHan(HANDLE hnd, PCSTR file, size_t line, PCSTR func) {
		if (!hnd || hnd == INVALID_HANDLE_VALUE) {
			throw WinError(ERROR_INVALID_HANDLE, file, line, func);
		}
		return hnd;
	}

	HANDLE HiddenFunctions::CheckHandleErrFuncHan(HANDLE hnd, PCSTR file, size_t line, PCSTR func) {
		if (!hnd || hnd == INVALID_HANDLE_VALUE) {
			DWORD err = ::GetLastError();
			throw WinError(err, file, line, func);
		}
		return hnd;
	}

	PVOID HiddenFunctions::CheckPointerFuncVoid(PVOID ptr, PCSTR file, size_t line, PCSTR func) {
		if (!ptr) {
			throw WinError(E_POINTER, file, line, func);
		}
		return ptr;
	}

	PVOID HiddenFunctions::CheckPointerErrFuncVoid(PVOID ptr, PCSTR file, size_t line, PCSTR func) {
		if (!ptr) {
			DWORD err = ::GetLastError();
			throw WinError(err, file, line, func);
		}
		return ptr;
	}

	void HiddenFunctions::RethrowExceptionFunc(const AbstractError & prev, const ustring & what, PCSTR file, size_t line, PCSTR func) {
		//	printf(L"RethrowExceptionFunc()\n");
		throw RuntimeError(prev, what, file, line, func, prev.code());
	}

#else

	bool HiddenFunctions::CheckApiFunc(bool r) {
		if (!r) {
			DWORD err = ::GetLastError();
			throw WinError(err);
		}
		return r;
	}

	bool HiddenFunctions::CheckApiThrowErrorFunc(bool r, DWORD err) {
		if (!r) {
			throw WinError(err);
		}
		return r;
	}

	DWORD HiddenFunctions::CheckApiErrorFunc(DWORD err) {
		if (err != ERROR_SUCCESS) {
			throw WinError(err);
		}
		return err;
	}

	HRESULT HiddenFunctions::CheckHMailErrorFunc(HRESULT err) {
		if (err != ERROR_SUCCESS) {
			throw HMailError(err);
		}
		return err;
	}

	HRESULT HiddenFunctions::CheckComFunc(HRESULT res) {
		if (FAILED(res))
			throw WinError(res);
		return res;
	}

	HRESULT HiddenFunctions::CheckWmiFunc(HRESULT res) {
		if (res != S_OK)
			throw WmiError(res);
		return res;
	}

	HANDLE HiddenFunctions::CheckHandleFuncHan(HANDLE hnd) {
		if (!hnd || hnd == INVALID_HANDLE_VALUE) {
			throw WinError(ERROR_INVALID_HANDLE);
		}
		return hnd;
	}

	HANDLE HiddenFunctions::CheckHandleErrFuncHan(HANDLE hnd) {
		if (!hnd || hnd == INVALID_HANDLE_VALUE) {
			DWORD err = ::GetLastError();
			throw WinError(err);
		}
		return hnd;
	}

	PVOID HiddenFunctions::CheckPointerFuncVoid(PVOID ptr) {
		if (!ptr) {
			throw WinError(E_POINTER);
		}
		return ptr;
	}

	PVOID HiddenFunctions::CheckPointerErrFuncVoid(PVOID ptr) {
		if (!ptr) {
			DWORD err = ::GetLastError();
			throw WinError(err);
		}
		return ptr;
	}

	void HiddenFunctions::RethrowExceptionFunc(const AbstractError & prev, const ustring & what) {
		//	printf(L"RethrowExceptionFunc()\n");
		throw RuntimeError(prev, what, prev.code());
	}
#endif

}
