#ifndef _WIN_NET_EXCEPTION_H_
#define _WIN_NET_EXCEPTION_H_

#include <libwin_def/std.h>

#include <tr1/memory>

///=================================================================================== AbstractError
class	AbstractError {
public:
	virtual ~AbstractError();

	AbstractError();
	AbstractError(const AbstractError & prev);

#ifndef NDEBUG
	AbstractError(PCSTR file, size_t line, PCSTR func);
	AbstractError(const AbstractError & prev, PCSTR file, size_t line, PCSTR func);
#endif

	virtual AbstractError * clone() const = 0;

	virtual ustring type() const = 0;

	virtual ustring	 what() const = 0;

	virtual DWORD code() const = 0;

#ifndef NDEBUG
	ustring	where() const {
		return m_where;
	}
#else
	ustring	where() const {
		return L"NDEBUG version";
	}
#endif

	AbstractError * get_prev() const;

private:
#ifndef NDEBUG
	ustring	m_where;
#endif
//	winstd::shared_ptr<AbstractError> m_prev_exc;
	std::tr1::shared_ptr<AbstractError> m_prev_exc;
};

///======================================================================================== WinError
class	WinError: public AbstractError {
public:
	WinError();
	WinError(DWORD code);

#ifndef NDEBUG
	WinError(PCSTR file, size_t line, PCSTR func);
	WinError(DWORD code, PCSTR file, size_t line, PCSTR func);
#endif

	virtual WinError * clone() const;

	virtual ustring type() const;

	virtual ustring	 what() const;

	virtual DWORD code() const;

private:
	DWORD	m_code;
};

///====================================================================================== WSockError
class	WSockError: public WinError {
public:
#ifndef NDEBUG
	WSockError(PCSTR file, size_t line, PCSTR func);
	WSockError(DWORD code, PCSTR file, size_t line, PCSTR func);
#else
	WSockError();
	WSockError(DWORD code);
#endif

	virtual WSockError * clone() const;

	virtual ustring type() const;
};

///======================================================================================== WmiError
class	WmiError: public WinError {
public:
#ifndef NDEBUG
	WmiError(HRESULT code, PCSTR file, size_t line, PCSTR func);
#else
	WmiError(HRESULT code);
#endif

	virtual WmiError * clone() const;

	virtual ustring type() const;

	virtual ustring	 what() const;
};

///=================================================================================== WinLogicError
class	RuntimeError: public AbstractError {
public:
	RuntimeError(const ustring & what, size_t code = 0);
	RuntimeError(const AbstractError & prev, const ustring & what, size_t code = 0);

#ifndef NDEBUG
	RuntimeError(const ustring & what, PCSTR file, size_t line, PCSTR func, size_t code = 0);
	RuntimeError(const AbstractError & prev, const ustring & what, PCSTR file, size_t line, PCSTR func, size_t code = 0);
#endif

	virtual RuntimeError * clone() const;

	virtual ustring type() const;

	virtual ustring	 what() const;

	virtual DWORD code() const;

private:
	size_t m_code;
	ustring	m_what;
};

///=================================================================================================
#ifndef NDEBUG

#define THROW_PLACE THIS_FILE, __LINE__, __FUNCTION__

#define CheckApi(arg) (CheckApiFunc((arg), THROW_PLACE))

#define CheckApiThrowError(arg1, arg2) (CheckApiThrowErrorFunc((arg1), (arg2), THROW_PLACE))

#define CheckApiError(arg) (CheckApiErrorFunc((arg), THROW_PLACE))

#define CheckWSock(arg) (CheckWSockFunc((arg), THROW_PLACE))

#define CheckCom(arg) (CheckComFunc((arg), THROW_PLACE))

#define CheckWmi(arg) (CheckWmiFunc((arg), THROW_PLACE))

#define CheckHandle(arg) (CheckHandleFunc((arg), THROW_PLACE))

#define CheckHandleErr(arg) (CheckHandleErrFunc((arg), THROW_PLACE))

#define CheckPointer(arg) (CheckPointerFunc((arg), THROW_PLACE))

#define Rethrow(arg1, arg2) (RethrowExceptionFunc((arg1), (arg2), THROW_PLACE))

bool	CheckApiFunc(bool r, PCSTR file, size_t line, PCSTR func);

bool	CheckApiThrowErrorFunc(bool r, DWORD err, PCSTR file, size_t line, PCSTR func);

DWORD	CheckApiErrorFunc(DWORD err, PCSTR file, size_t line, PCSTR func);

int		CheckWSockFunc(int err, PCSTR file, size_t line, PCSTR func);

HRESULT	CheckComFunc(HRESULT res, PCSTR file, size_t line, PCSTR func);

HRESULT	CheckWmiFunc(HRESULT res, PCSTR file, size_t line, PCSTR func);

HANDLE	CheckHandleFuncHan(HANDLE hnd, PCSTR file, size_t line, PCSTR func);

HANDLE	CheckHandleErrFuncHan(HANDLE hnd, PCSTR file, size_t line, PCSTR func);

PVOID	CheckPointerFuncVoid(PVOID ptr, PCSTR file, size_t line, PCSTR func);

template <typename Type>
Type	CheckHandleFunc(Type hnd, PCSTR file, size_t line, PCSTR func) {
	return (Type)CheckHandleFuncHan((HANDLE)hnd, file, line, func);
}

template <typename Type>
Type	CheckHandleErrFunc(Type hnd, PCSTR file, size_t line, PCSTR func) {
	return (Type)CheckHandleErrFuncHan((HANDLE)hnd, file, line, func);
}

template <typename Type>
Type	CheckPointerFunc(Type ptr, PCSTR file, size_t line, PCSTR func) {
	return (Type)CheckPointerFuncVoid((PVOID)ptr, file, line, func);
}

void	RethrowExceptionFunc(const AbstractError & prev, const ustring & what, PCSTR file, size_t line, PCSTR func);

#else

#define CheckApi(arg) (CheckApiFunc((arg)))

#define CheckApiThrowError(arg1, arg2) (CheckApiThrowErrorFunc((arg1), (arg2)))

#define CheckApiError(arg) (CheckApiErrorFunc((arg)))

#define CheckWSock(arg) (CheckWSockFunc((arg)))

#define CheckCom(arg) (CheckComFunc((arg)))

#define CheckWmi(arg) (CheckWmiFunc((arg)))

#define CheckHandle(arg) (CheckHandleFunc((arg)))

#define CheckHandleErr(arg) (CheckHandleErrFunc((arg)))

#define CheckPointer(arg) (CheckPointerFunc((arg)))

#define Rethrow(arg1, arg2) (RethrowExceptionFunc((arg1), (arg2)))

bool	CheckApiFunc(bool r);

bool	CheckApiThrowErrorFunc(bool r, DWORD err);

DWORD	CheckApiErrorFunc(DWORD err);

int		CheckWSockFunc(int err);

HRESULT	CheckComFunc(HRESULT res);

HRESULT	CheckWmiFunc(HRESULT res);

HANDLE	CheckHandleFuncHan(HANDLE hnd);

HANDLE	CheckHandleErrFuncHan(HANDLE hnd);

PVOID	CheckPointerFuncVoid(PVOID ptr);

template <typename Type>
Type	CheckHandleFunc(Type hnd) {
	return (Type)CheckHandleFuncHan((HANDLE)hnd);
}

template <typename Type>
Type	CheckHandleErrFunc(Type hnd) {
	return (Type)CheckHandleErrFuncHan((HANDLE)hnd);
}

template <typename Type>
Type	CheckPointerFunc(Type ptr) {
	return (Type)CheckPointerFuncVoid((PVOID)ptr);
}

void	RethrowExceptionFunc(const AbstractError & prev, const ustring & what);
#endif

#endif
