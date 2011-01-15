#include "exception.h"

#ifndef NDEBUG
#define THROW_PLACE_STR ThrowPlaceString(file, line, func)
#else
#define THROW_PLACE_STR ustring()
#endif

PCSTR const THROW_PLACE_FORMAT = "%s: %d [%s]";

ustring ThrowPlaceString(PCSTR file, int line, PCSTR func) {
	CHAR buf[MAX_PATH];
	buf[MAX_PATH-1] = 0;
	::snprintf(buf, sizeofa(buf) - 1, THROW_PLACE_FORMAT, file, line, func);
	return	ustring(buf, CP_UTF8);
}

ProgrammError::~ProgrammError() throw() {
}

ProgrammError::ProgrammError(const ustring &what):
		m_what(what) {
}

ProgrammError::ProgrammError(const ustring &what, PCSTR file, size_t line, PCSTR func):
		m_what(what),
		m_where(THROW_PLACE_STR) {
}

ustring	ProgrammError::what() const throw() {
	return	m_what;
}

WinError::WinError(const ustring &what):
	ProgrammError(what),
	m_code(0) {
}

WinError::WinError(const ustring &what, PCSTR file, size_t line, PCSTR func):
	ProgrammError(what, file, line, func),
	m_code(0) {
}

WinError::WinError(size_t code, const ustring &what):
	ProgrammError(what),
	m_code(code) {
}

WinError::WinError(size_t code, const ustring &what, PCSTR file, size_t line, PCSTR func):
	ProgrammError(what, file, line, func),
	m_code(code) {
}

ustring	 WinError::msg() const {
	return	ErrAsStr(code());
}

ustring	 WmiError::msg() const {
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
		throw	ApiError(res, L"CheckWmi", file, line, func);
	return	res;
}
