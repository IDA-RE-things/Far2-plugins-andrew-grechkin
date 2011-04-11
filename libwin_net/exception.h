#ifndef _WIN_NET_EXCEPTION_H_
#define _WIN_NET_EXCEPTION_H_

#include <libwin_def/std.h>

class	ProgrammError {
public:
	virtual ~ProgrammError() throw();

	ProgrammError(const AutoUTF &what = AutoUTF());
	ProgrammError(const AutoUTF &what, PCSTR file, size_t line, PCSTR func);

	virtual AutoUTF what() const throw();

	AutoUTF	where() const throw() {
		return	m_where;
	}

private:
	AutoUTF	m_what;
	AutoUTF	m_where;
};

class	WinError: public ProgrammError {
public:
	WinError(const AutoUTF &what = AutoUTF());
	WinError(const AutoUTF &what, PCSTR file, size_t line, PCSTR func);
	WinError(ssize_t code, const AutoUTF &what = AutoUTF());
	WinError(ssize_t code, const AutoUTF &what, PCSTR file, size_t line, PCSTR func);

	virtual AutoUTF	 msg() const;

	ssize_t	code() const {
		return	m_code;
	}

protected:
	ssize_t	code(ssize_t code) {
		return	m_code = code;
	}

private:
	ssize_t	m_code;
};

class	ApiError: public WinError {
public:
	ApiError(const AutoUTF &what = AutoUTF()):
		WinError(::GetLastError(), what) {
	}
	ApiError(const AutoUTF &what, PCSTR file, size_t line, PCSTR func):
		WinError(::GetLastError(), what, file, line, func) {
	}
	ApiError(ssize_t code, const AutoUTF &what = AutoUTF()):
		WinError(code, what) {
	}
	ApiError(ssize_t code, const AutoUTF &what, PCSTR file, size_t line, PCSTR func):
		WinError(code, what, file, line, func) {
	}
};

class	WSockError: public WinError {
public:
	WSockError(const AutoUTF &what = AutoUTF()):
		WinError(::WSAGetLastError(), what) {
	}
	WSockError(const AutoUTF &what, PCSTR file, size_t line, PCSTR func):
		WinError(::WSAGetLastError(), what, file, line, func) {
	}
	WSockError(ssize_t code, const AutoUTF &what = AutoUTF()):
		WinError(code, what) {
	}
	WSockError(ssize_t code, const AutoUTF &what, PCSTR file, size_t line, PCSTR func):
		WinError(code, what, file, line, func) {
	}
};

class	WmiError: public WinError {
public:
	WmiError(HRESULT code, const AutoUTF &what = AutoUTF()):
		WinError(code, what) {
	}
	WmiError(HRESULT code, const AutoUTF &what, PCSTR file, size_t line, PCSTR func):
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
#define CheckHandleErr(arg) (CheckHandleErrFunc((arg), THROW_PLACE))
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
Type	CheckHandleErrFunc(Type hnd, PCSTR file, size_t line, PCSTR func) {
	if (!hnd || hnd == INVALID_HANDLE_VALUE) {
		throw	ApiError(::GetLastError(), L"CheckHandleErr", file, line, func);
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
