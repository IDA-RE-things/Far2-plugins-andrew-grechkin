#include "exception.h"
#include <libwin_def/win_def.h>

#include <stdio.h>

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
	return ustring(buf, CP_UTF8);
}

///=================================================================================== AbstractError
AbstractError::~AbstractError() {
}

AbstractError::AbstractError() {
}

AbstractError::AbstractError(PCSTR file, size_t line, PCSTR func):
	m_where(THROW_PLACE_STR) {
}

AbstractError::AbstractError(const AbstractError &prev, PCSTR file, size_t line, PCSTR func):
	m_where(THROW_PLACE_STR),
	m_prev_exc(prev.clone()) {
}

ustring	AbstractError::where() const {
	return m_where;
}

AbstractError * AbstractError::get_prev() const {
	return m_prev_exc.get();
}

///======================================================================================== WinError
WinError::WinError():
	m_code(::GetLastError()) {
}

WinError::WinError(PCSTR file, size_t line, PCSTR func):
	AbstractError(file, line, func),
	m_code(::GetLastError()) {
}

WinError::WinError(DWORD code):
	m_code(code) {
}

WinError::WinError(DWORD code, PCSTR file, size_t line, PCSTR func):
	AbstractError(file, line, func),
	m_code(code) {
}

WinError * WinError::clone() const {
//	printf(L"WinError::clone()\n");
	return new WinError(* this);
}

ustring WinError::type() const {
	return L"WinError";
}

ustring	WinError::msg() const {
	return ErrAsStr(code());
}

ustring WinError::what() const {
	return ErrAsStr(code());
}

DWORD WinError::code() const {
	return m_code;
}

///====================================================================================== WSockError
WSockError::WSockError(DWORD code):
	WinError(code) {
}

WSockError::WSockError(DWORD code, PCSTR file, size_t line, PCSTR func):
	WinError(code, file, line, func) {
}

WSockError * WSockError::clone() const {
//	printf(L"WSockError::clone()\n");
	return new WSockError(* this);
}

ustring WSockError::type() const {
	return L"WSockError";
}

///======================================================================================== WmiError
WmiError::WmiError(HRESULT code):
	WinError(code) {
}

WmiError::WmiError(HRESULT code, PCSTR file, size_t line, PCSTR func):
	WinError(code, file, line, func) {
}

WmiError * WmiError::clone() const {
//	printf(L"WmiError::clone()\n");
 	return new WmiError(* this);
}

ustring WmiError::type() const {
	return L"WmiError";
}

ustring	WmiError::msg() const {
	return ErrWmiAsStr(code());
}

///=================================================================================== WinLogicError
RuntimeError::RuntimeError(const ustring &what):
	m_what(what) {
}

RuntimeError::RuntimeError(const ustring &what, PCSTR file, size_t line, PCSTR func):
	AbstractError(file, line, func),
	m_what(what) {
}

RuntimeError::RuntimeError(const AbstractError &prev, const ustring &what):
	AbstractError(prev),
	m_what(what) {
}

RuntimeError::RuntimeError(const AbstractError &prev, const ustring &what, PCSTR file, size_t line, PCSTR func):
	AbstractError(prev, file, line, func),
	m_what(what) {
}

RuntimeError * RuntimeError::clone() const {
//	printf(L"RuntimeError::clone()\n");
	return new RuntimeError(* this);
}

ustring RuntimeError::type() const {
	return L"RuntimeError";
}

ustring	RuntimeError::msg() const {
	return m_what;
}

ustring RuntimeError::what() const {
	return m_what;
}

DWORD RuntimeError::code() const {
	return 0;
}

///=================================================================================================
bool	CheckApiFunc(bool r, PCSTR file, size_t line, PCSTR func) {
	if (!r) {
		throw	WinError(::GetLastError(), file, line, func);
	}
	return r;
}

DWORD	CheckApiErrorFunc(DWORD err, PCSTR file, size_t line, PCSTR func) {
	if (err != ERROR_SUCCESS) {
		throw WinError(err, file, line, func);
	}
	return err;
}

int		CheckWSockFunc(int err, PCSTR file, size_t line, PCSTR func)  {
	if (err) {
		throw	WSockError(err, file, line, func);
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

void	RethrowExceptionFunc(const AbstractError &prev, const ustring &what, PCSTR file, size_t line, PCSTR func) {
//	printf(L"RethrowExceptionFunc()\n");
	throw RuntimeError(prev, what, file, line, func);
}
