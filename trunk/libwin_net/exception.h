#ifndef _WIN_NET_EXCEPTION_H_
#define _WIN_NET_EXCEPTION_H_

#include <libwin_def/win_def.h>

class	ProgrammError {
public:
	virtual ~ProgrammError() throw();

	ProgrammError(const ustring &what = ustring());
	ProgrammError(const ustring &what, PCSTR file, size_t line, PCSTR func);

	virtual ustring what() const throw();

	ustring	where() const throw() {
		return	m_where;
	}

private:
	ustring	m_what;
	ustring	m_where;
};

class	WinError: public ProgrammError {
public:
	WinError(const ustring &what = ustring());
	WinError(const ustring &what, PCSTR file, size_t line, PCSTR func);
	WinError(size_t code, const ustring &what = ustring());
	WinError(size_t code, const ustring &what, PCSTR file, size_t line, PCSTR func);

	virtual AutoUTF	 msg() const;

	size_t	code() const {
		return	m_code;
	}

protected:
	size_t	code(size_t code) {
		return	m_code = code;
	}

private:
	size_t	m_code;
};

class	ApiError: public WinError {
public:
	ApiError(const ustring &what = ustring()):
		WinError(::GetLastError(), what) {
	}
	ApiError(const ustring &what, PCSTR file, size_t line, PCSTR func):
		WinError(::GetLastError(), what, file, line, func) {
	}
	ApiError(size_t code, const ustring &what = ustring()):
		WinError(code, what) {
	}
	ApiError(size_t code, const ustring &what, PCSTR file, size_t line, PCSTR func):
		WinError(code, what, file, line, func) {
	}
};

class	WSockError: public WinError {
public:
	WSockError(const ustring &what = ustring()):
		WinError(::WSAGetLastError(), what) {
	}
	WSockError(const ustring &what, PCSTR file, size_t line, PCSTR func):
		WinError(::WSAGetLastError(), what, file, line, func) {
	}
	WSockError(size_t code, const ustring &what = ustring()):
		WinError(code, what) {
	}
	WSockError(size_t code, const ustring &what, PCSTR file, size_t line, PCSTR func):
		WinError(code, what, file, line, func) {
	}
};

class	WmiError: public WinError {
public:
	WmiError(HRESULT code, const ustring &what = ustring()):
		WinError(code, what) {
	}
	WmiError(HRESULT code, const ustring &what, PCSTR file, size_t line, PCSTR func):
		WinError(code, what, file, line, func) {
	}

	virtual AutoUTF	 msg() const;
};

#define THROW_PLACE THIS_FILE, __LINE__, __FUNCTION__
#define CheckApi(arg) (CheckApiFunc((arg), THROW_PLACE))
#define CheckApiError(arg) (CheckApiErrorFunc((arg), THROW_PLACE))
#define CheckWSock(arg) (CheckWSockFunc((arg), THROW_PLACE))
#define CheckCom(arg) (CheckComFunc((arg), THROW_PLACE))
#define CheckWmi(arg) (CheckWmiFunc((arg), THROW_PLACE))
#define CheckHandle(arg) (CheckHandleFunc((arg), THROW_PLACE))
#define CheckPointer(arg) (CheckPointerFunc((arg), THROW_PLACE))

bool	CheckApiFunc(bool r, PCSTR file, size_t line, PCSTR func);
DWORD	CheckApiErrorFunc(DWORD err, PCSTR file, size_t line, PCSTR func);
int		CheckWSockFunc(int err, PCSTR file, size_t line, PCSTR func);
HRESULT	CheckComFunc(HRESULT res, PCSTR file, size_t line, PCSTR func);
HRESULT	CheckWmiFunc(HRESULT res, PCSTR file, size_t line, PCSTR func);
template <typename Type>
Type	CheckHandleFunc(Type hnd, PCSTR file, size_t line, PCSTR func) {
	if (!hnd || hnd == INVALID_HANDLE_VALUE) {
		throw	ApiError(ERROR_INVALID_HANDLE, L"CheckHandle", file, line, func);
	}
	return	hnd;
}
template <typename Type>
Type	CheckPointerFunc(Type ptr, PCSTR file, size_t line, PCSTR func) {
	if (!ptr) {
		throw	ApiError(E_POINTER, L"CheckPointer", file, line, func);
	}
	return	ptr;
}

#endif
