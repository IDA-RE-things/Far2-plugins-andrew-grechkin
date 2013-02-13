#include "exception_pvt.hpp"
#include <libbase/err.hpp>
#include <libbase/logger.hpp>

namespace Ext {

	///==================================================================================== WinError
#ifndef NDEBUG
	WinError::WinError(PCSTR file, size_t line, PCSTR func) :
		AbstractError(file, line, func),
		m_code(::GetLastError())
	{
		LogNoise(L"%s\n", what().c_str());
	}

	WinError::WinError(DWORD code, PCSTR file, size_t line, PCSTR func) :
		AbstractError(file, line, func),
		m_code(code)
	{
		LogNoise(L"%s\n", what().c_str());
	}
#else
	WinError::WinError() :
		m_code(::GetLastError())
	{
		LogNoise(L"%s\n", what().c_str());
	}

	WinError::WinError(DWORD code) :
		m_code(code)
	{
		LogNoise(L"%s\n", what().c_str());
	}
#endif

	WinError * WinError::clone() const
	{
		return new WinError(*this);
	}

	ustring WinError::type() const
	{
		return L"WinError";
	}

	ustring WinError::what() const
	{
		return Base::ErrAsStr(code());
	}

	DWORD WinError::code() const
	{
		return m_code;
	}

	void WinError::format_error(Base::mstring & out) const
	{
		wchar_t buf[Base::MAX_PATH_LEN] = {0};

		_snwprintf(buf, Base::lengthof(buf), L"Error: %s", what().c_str());
		out.push_back(buf);
#ifndef NDEBUG
		_snwprintf(buf, Base::lengthof(buf), L"Exception: %s", type().c_str());
		out.push_back(buf);
		_snwprintf(buf, Base::lengthof(buf), L"Where: %s", where());
		out.push_back(buf);
#endif
	}

	///=============================================================================================
#ifndef NDEBUG
	bool HiddenFunctions::CheckApiFunc(bool r, PCSTR file, size_t line, PCSTR func)
	{
		if (!r) {
			DWORD err = ::GetLastError();
			throw WinError(err, file, line, func);
		}
		return r;
	}

	bool HiddenFunctions::CheckApiThrowErrorFunc(bool r, DWORD err, PCSTR file, size_t line, PCSTR func)
	{
		if (!r) {
			throw WinError(err, file, line, func);
		}
		return r;
	}

	DWORD HiddenFunctions::CheckApiErrorFunc(DWORD err, PCSTR file, size_t line, PCSTR func)
	{
		if (err != ERROR_SUCCESS) {
			throw WinError(err, file, line, func);
		}
		return err;
	}

	HRESULT HiddenFunctions::CheckComFunc(HRESULT res, PCSTR file, size_t line, PCSTR func)
	{
		if (FAILED(res))
			throw WinError(res, file, line, func);
		return res;
	}

	HANDLE HiddenFunctions::CheckHandleFuncHan(HANDLE hnd, PCSTR file, size_t line, PCSTR func)
	{
		if (hnd == nullptr || hnd == INVALID_HANDLE_VALUE ) {
			throw WinError(ERROR_INVALID_HANDLE, file, line, func);
		}
		return hnd;
	}

	HANDLE HiddenFunctions::CheckHandleErrFuncHan(HANDLE hnd, PCSTR file, size_t line, PCSTR func)
	{
		if (hnd == nullptr || hnd == INVALID_HANDLE_VALUE ) {
			DWORD err = ::GetLastError();
			throw WinError(err, file, line, func);
		}
		return hnd;
	}

	PVOID HiddenFunctions::CheckPointerFuncVoid(PVOID ptr, PCSTR file, size_t line, PCSTR func)
	{
		if (!ptr) {
			throw WinError(E_POINTER, file, line, func);
		}
		return ptr;
	}

	PVOID HiddenFunctions::CheckPointerErrFuncVoid(PVOID ptr, PCSTR file, size_t line, PCSTR func)
	{
		if (!ptr) {
			DWORD err = ::GetLastError();
			throw WinError(err, file, line, func);
		}
		return ptr;
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

HRESULT HiddenFunctions::CheckComFunc(HRESULT res) {
	if (FAILED(res))
	throw WinError(res);
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
#endif

}
