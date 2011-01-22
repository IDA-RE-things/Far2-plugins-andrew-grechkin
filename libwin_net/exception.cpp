#include "exception.h"

#ifndef NDEBUG
#define THROW_PLACE_STR ThrowPlaceString(file, line, func)
#else
#define THROW_PLACE_STR AutoUTF()
#endif

PCSTR const THROW_PLACE_FORMAT = "%s: %d [%s]";

AutoUTF ThrowPlaceString(PCSTR file, int line, PCSTR func) {
	CHAR buf[MAX_PATH];
	buf[MAX_PATH-1] = 0;
	::snprintf(buf, sizeofa(buf) - 1, THROW_PLACE_FORMAT, file, line, func);
	return	AutoUTF(buf, CP_UTF8);
}

ProgrammError::~ProgrammError() throw() {
}

ProgrammError::ProgrammError(const AutoUTF &what):
		m_what(what) {
}

ProgrammError::ProgrammError(const AutoUTF &what, PCSTR file, size_t line, PCSTR func):
		m_what(what),
		m_where(THROW_PLACE_STR) {
}

AutoUTF	ProgrammError::what() const throw() {
	return	m_what;
}

WinError::WinError(const AutoUTF &what):
	ProgrammError(what),
	m_code(0) {
}

WinError::WinError(const AutoUTF &what, PCSTR file, size_t line, PCSTR func):
	ProgrammError(what, file, line, func),
	m_code(0) {
}

WinError::WinError(ssize_t code, const AutoUTF &what):
	ProgrammError(what),
	m_code(code) {
}

WinError::WinError(ssize_t code, const AutoUTF &what, PCSTR file, size_t line, PCSTR func):
	ProgrammError(what, file, line, func),
	m_code(code) {
}

AutoUTF	 WinError::msg() const {
	return	ErrAsStr(code());
}

AutoUTF	 WmiError::msg() const {
	return	ErrWmiAsStr(code());
}

bool	CheckApiFunc(bool r, PCSTR file, size_t line, PCSTR func) {
	if (!r) {
		throw	ApiError(::GetLastError(), L"CheckApi", file, line, func);
	}
	return r;
}

DWORD	CheckApiErrorFunc(DWORD err, PCSTR file, size_t line, PCSTR func) {
	if (err != ERROR_SUCCESS) {
		throw	ApiError(err, L"CheckApi", file, line, func);
	}
	return	err;
}

int		CheckWSockFunc(int err, PCSTR file, size_t line, PCSTR func)  {
	if (err) {
		throw	WSockError(err, L"CheckWSock", file, line, func);
	}
	return	err;
}

HRESULT	CheckComFunc(HRESULT res, PCSTR file, size_t line, PCSTR func) {
	if (FAILED(res))
		throw	ApiError(res, L"CheckCom", file, line, func);
	return	res;
}

HRESULT	CheckWmiFunc(HRESULT res, PCSTR file, size_t line, PCSTR func) {
	if (res != S_OK)
		throw	WmiError(res, L"CheckWmi", file, line, func);
	return	res;
}
