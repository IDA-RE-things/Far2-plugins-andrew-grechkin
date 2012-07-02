#include <libext/exception.hpp>

namespace Ext {

///====================================================================================== WSockError
#ifndef NDEBUG
WSockError::WSockError(PCSTR file, size_t line, PCSTR func):
	WinError(::WSAGetLastError(), file, line, func) {
}
WSockError::WSockError(DWORD code, PCSTR file, size_t line, PCSTR func):
	WinError(code, file, line, func) {
}
#else
WSockError::WSockError():
	WinError(::WSAGetLastError()) {
}
WSockError::WSockError(DWORD code):
	WinError(code) {
}
#endif

WSockError * WSockError::clone() const {
	return new WSockError(* this);
}

ustring WSockError::type() const {
	return L"WSockError";
}

///=================================================================================================
#ifndef NDEBUG
int HiddenFunctions::CheckWSockFunc(int err, PCSTR file, size_t line, PCSTR func)  {
	if (err) {
		throw WSockError(::WSAGetLastError(), file, line, func);
	}
	return err;
}
#else
int HiddenFunctions::CheckWSockFunc(int err)  {
	if (err) {
		throw WSockError(::WSAGetLastError());
	}
	return err;
}
#endif

}
