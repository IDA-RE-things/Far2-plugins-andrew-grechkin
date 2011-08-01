#ifndef _WIN_NET_EXCEPTION_H_
#define _WIN_NET_EXCEPTION_H_

#include <libwin_def/std.h>

#include <tr1/memory>

///=================================================================================== AbstractError
class	AbstractError {
public:
	virtual ~AbstractError();

	AbstractError();
	AbstractError(PCSTR file, size_t line, PCSTR func);
	AbstractError(const AbstractError &prev, PCSTR file, size_t line, PCSTR func);

	virtual AbstractError * clone() const = 0;

	virtual ustring type() const = 0;

	virtual ustring	 msg() const = 0;

	virtual ustring	 what() const = 0;

	virtual DWORD code() const = 0;

	ustring	where() const;

	AbstractError * get_prev() const;

private:
	ustring	m_where;
//	winstd::shared_ptr<AbstractError> m_prev_exc;
	std::tr1::shared_ptr<AbstractError> m_prev_exc;
};

///======================================================================================== WinError
class	WinError: public AbstractError {
public:
	WinError();
	WinError(PCSTR file, size_t line, PCSTR func);
	WinError(DWORD code);
	WinError(DWORD code, PCSTR file, size_t line, PCSTR func);

	virtual WinError * clone() const;

	virtual ustring type() const;

	virtual ustring	 msg() const;

	virtual ustring	 what() const;

	virtual DWORD code() const;

private:
	DWORD	m_code;
};

///====================================================================================== WSockError
class	WSockError: public WinError {
public:
	WSockError():
		WinError(::WSAGetLastError()) {
	}
	WSockError(PCSTR file, size_t line, PCSTR func):
		WinError(::WSAGetLastError(), file, line, func) {
	}
	WSockError(DWORD code);
	WSockError(DWORD code, PCSTR file, size_t line, PCSTR func);

	virtual WSockError * clone() const;

	virtual ustring type() const;
};

///======================================================================================== WmiError
class	WmiError: public WinError {
public:
	WmiError(HRESULT code);
	WmiError(HRESULT code, PCSTR file, size_t line, PCSTR func);

	virtual WmiError * clone() const;

	virtual ustring type() const;

	virtual ustring	 msg() const;
};

///=================================================================================== WinLogicError
class	RuntimeError: public AbstractError {
public:
	RuntimeError(const ustring &what);
	RuntimeError(const ustring &what, PCSTR file, size_t line, PCSTR func);

	RuntimeError(const AbstractError &prev, const ustring &what);
	RuntimeError(const AbstractError &prev, const ustring &what, PCSTR file, size_t line, PCSTR func);

	virtual RuntimeError * clone() const;

	virtual ustring type() const;

	virtual ustring	 msg() const;

	virtual ustring	 what() const;

	virtual DWORD code() const;

private:
	ustring	m_what;
};

///=================================================================================================
#define THROW_PLACE THIS_FILE, __LINE__, __FUNCTION__
#define CheckApi(arg) (CheckApiFunc((arg), THROW_PLACE))
#define CheckApiError(arg) (CheckApiErrorFunc((arg), THROW_PLACE))
#define CheckWSock(arg) (CheckWSockFunc((arg), THROW_PLACE))
#define CheckCom(arg) (CheckComFunc((arg), THROW_PLACE))
#define CheckWmi(arg) (CheckWmiFunc((arg), THROW_PLACE))
#define CheckHandle(arg) (CheckHandleFunc((arg), THROW_PLACE))
#define CheckHandleErr(arg) (CheckHandleErrFunc((arg), THROW_PLACE))
#define CheckPointer(arg) (CheckPointerFunc((arg), THROW_PLACE))

#define Rethrow(arg1, arg2) (RethrowExceptionFunc((arg1), (arg2), THROW_PLACE))

bool	CheckApiFunc(bool r, PCSTR file, size_t line, PCSTR func);

DWORD	CheckApiErrorFunc(DWORD err, PCSTR file, size_t line, PCSTR func);

int		CheckWSockFunc(int err, PCSTR file, size_t line, PCSTR func);

HRESULT	CheckComFunc(HRESULT res, PCSTR file, size_t line, PCSTR func);

HRESULT	CheckWmiFunc(HRESULT res, PCSTR file, size_t line, PCSTR func);

template <typename Type>
Type	CheckHandleFunc(Type hnd, PCSTR file, size_t line, PCSTR func) {
	if (!hnd || hnd == INVALID_HANDLE_VALUE) {
		throw	WinError(ERROR_INVALID_HANDLE, file, line, func);
	}
	return hnd;
}

template <typename Type>
Type	CheckHandleErrFunc(Type hnd, PCSTR file, size_t line, PCSTR func) {
	if (!hnd || hnd == INVALID_HANDLE_VALUE) {
		throw	WinError(::GetLastError(), file, line, func);
	}
	return hnd;
}

template <typename Type>
Type	CheckPointerFunc(Type ptr, PCSTR file, size_t line, PCSTR func) {
	if (!ptr) {
		throw	WinError(E_POINTER, file, line, func);
	}
	return ptr;
}

void	RethrowExceptionFunc(const AbstractError &prev, const ustring &what, PCSTR file, size_t line, PCSTR func);

#endif
