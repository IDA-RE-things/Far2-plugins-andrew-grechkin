#ifndef _WIN_NET_EXCEPTION_H_
#define _WIN_NET_EXCEPTION_H_

#include <libbase/std.hpp>
#include <libbase/str.hpp>

#include <vector>
#include <tr1/memory>

#define THROW_PLACE THIS_FILE, __LINE__, __PRETTY_FUNCTION__

namespace Ext {

	///=============================================================================== AbstractError
	struct AbstractError {
		virtual ~AbstractError();

		virtual AbstractError * clone() const = 0;

		virtual ustring type() const = 0;

		virtual ustring	 what() const = 0;

		virtual DWORD code() const = 0;

		virtual void format_error(std::vector<ustring> & out) const = 0;

		ustring	where() const {
#ifndef NDEBUG
			return m_where;
#else
			return ustring(L"Programm compiled with NDEBUG define");
#endif
		}

		AbstractError * get_prev() const;

	protected:
#ifndef NDEBUG
		AbstractError(PCSTR file, size_t line, PCSTR func);
		AbstractError(const AbstractError & prev, PCSTR file, size_t line, PCSTR func);
#else
		AbstractError();
		AbstractError(const AbstractError & prev);
#endif

	private:
#ifndef NDEBUG
		ustring	m_where;
#endif
		std::tr1::shared_ptr<AbstractError> m_prev_exc;
	};

	///==================================================================================== WinError
	struct WinError: public AbstractError {
		virtual WinError * clone() const;

		virtual ustring type() const;

		virtual ustring	 what() const;

		virtual DWORD code() const;

		virtual void format_error(std::vector<ustring> & out) const;

	protected:
#ifndef NDEBUG
		WinError(PCSTR file, size_t line, PCSTR func);
		WinError(DWORD code, PCSTR file, size_t line, PCSTR func);
#else
		WinError();
		WinError(DWORD code);
#endif

	private:
		DWORD m_code;

		friend struct HiddenFunctions;
	};

	///================================================================================== WSockError
	struct WSockError: public WinError {
		virtual WSockError * clone() const;

		virtual ustring type() const;

	private:
#ifndef NDEBUG
		WSockError(PCSTR file, size_t line, PCSTR func);
		WSockError(DWORD code, PCSTR file, size_t line, PCSTR func);
#else
		WSockError();
		WSockError(DWORD code);
#endif

		friend struct HiddenFunctions;
	};

	///==================================================================================== WmiError
	struct WmiError: public WinError {
		virtual WmiError * clone() const;

		virtual ustring type() const;

		virtual ustring	 what() const;

	private:
#ifndef NDEBUG
		WmiError(HRESULT code, PCSTR file, size_t line, PCSTR func);
#else
		WmiError(HRESULT code);
#endif

		friend struct HiddenFunctions;
	};

	///================================================================================== HMailError
	struct HMailError: public WinError {
		virtual HMailError * clone() const;

		virtual ustring type() const;

		virtual ustring	 what() const;

	private:
#ifndef NDEBUG
		HMailError(HRESULT code, PCSTR file, size_t line, PCSTR func);
#else
		HMailError(HRESULT code);
#endif

		friend struct HiddenFunctions;
	};

	///=============================================================================== WinLogicError
	struct RuntimeError: public AbstractError {
		virtual RuntimeError * clone() const;

		virtual ustring type() const;

		virtual ustring	 what() const;

		virtual DWORD code() const;

		virtual void format_error(std::vector<ustring> & out) const;

	protected:
#ifndef NDEBUG
		RuntimeError(const ustring & what, PCSTR file, size_t line, PCSTR func, size_t code = 0);
		RuntimeError(const AbstractError & prev, const ustring & what, PCSTR file, size_t line, PCSTR func, size_t code = 0);
#else
		RuntimeError(const ustring & what, size_t code = 0);
		RuntimeError(const AbstractError & prev, const ustring & what, size_t code = 0);
#endif

	private:
		size_t m_code;
		ustring	m_what;

		friend struct HiddenFunctions;
	};

	///=============================================================================================
#ifndef NDEBUG

#define CheckApi(arg) (Ext::HiddenFunctions::CheckApiFunc((arg), THROW_PLACE))

#define CheckApiThrowError(arg1, arg2) (Ext::HiddenFunctions::CheckApiThrowErrorFunc((arg1), (arg2), THROW_PLACE))

#define CheckApiError(arg) (Ext::HiddenFunctions::CheckApiErrorFunc((arg), THROW_PLACE))

#define CheckHMailError(arg) (Ext::HiddenFunctions::CheckHMailErrorFunc((arg), THROW_PLACE))

#define CheckWSock(arg) (Ext::HiddenFunctions::CheckWSockFunc((arg), THROW_PLACE))

#define CheckCom(arg) (Ext::HiddenFunctions::CheckComFunc((arg), THROW_PLACE))

#define CheckWmi(arg) (Ext::HiddenFunctions::CheckWmiFunc((arg), THROW_PLACE))

#define CheckHandle(arg) (Ext::HiddenFunctions::CheckHandleFunc((arg), THROW_PLACE))

#define CheckHandleErr(arg) (Ext::HiddenFunctions::CheckHandleErrFunc((arg), THROW_PLACE))

#define CheckPointer(arg) (Ext::HiddenFunctions::CheckPointerFunc((arg), THROW_PLACE))

#define CheckPointerErr(arg) (Ext::HiddenFunctions::CheckPointerErrFunc((arg), THROW_PLACE))

#define Rethrow(arg1, arg2) (Ext::HiddenFunctions::RethrowExceptionFunc((arg1), (arg2), THROW_PLACE))

	struct HiddenFunctions {
		static bool CheckApiFunc(bool r, PCSTR file, size_t line, PCSTR func);

		static bool CheckApiThrowErrorFunc(bool r, DWORD err, PCSTR file, size_t line, PCSTR func);

		static DWORD CheckApiErrorFunc(DWORD err, PCSTR file, size_t line, PCSTR func);

		static HRESULT CheckHMailErrorFunc(HRESULT err, PCSTR file, size_t line, PCSTR func);

		static int CheckWSockFunc(int err, PCSTR file, size_t line, PCSTR func);

		static HRESULT CheckComFunc(HRESULT res, PCSTR file, size_t line, PCSTR func);

		static HRESULT CheckWmiFunc(HRESULT res, PCSTR file, size_t line, PCSTR func);

		static HANDLE CheckHandleFuncHan(HANDLE hnd, PCSTR file, size_t line, PCSTR func);

		static HANDLE CheckHandleErrFuncHan(HANDLE hnd, PCSTR file, size_t line, PCSTR func);

		static PVOID CheckPointerFuncVoid(PVOID ptr, PCSTR file, size_t line, PCSTR func);

		static PVOID CheckPointerErrFuncVoid(PVOID ptr, PCSTR file, size_t line, PCSTR func);

		template <typename Type>
		static Type CheckHandleFunc(Type hnd, PCSTR file, size_t line, PCSTR func) {
			return (Type)CheckHandleFuncHan((HANDLE)hnd, file, line, func);
		}

		template <typename Type>
		static Type CheckHandleErrFunc(Type hnd, PCSTR file, size_t line, PCSTR func) {
			return (Type)CheckHandleErrFuncHan((HANDLE)hnd, file, line, func);
		}

		template <typename Type>
		static Type CheckPointerFunc(Type ptr, PCSTR file, size_t line, PCSTR func) {
			return (Type)CheckPointerFuncVoid((PVOID)ptr, file, line, func);
		}

		template <typename Type>
		static Type CheckPointerErrFunc(Type ptr, PCSTR file, size_t line, PCSTR func) {
			return (Type)CheckPointerErrFuncVoid((PVOID)ptr, file, line, func);
		}

		static void RethrowExceptionFunc(const AbstractError & prev, const ustring & what, PCSTR file, size_t line, PCSTR func);
	};

#else

#define CheckApi(arg) (Ext::HiddenFunctions::CheckApiFunc((arg)))

#define CheckApiThrowError(arg1, arg2) (Ext::HiddenFunctions::CheckApiThrowErrorFunc((arg1), (arg2)))

#define CheckApiError(arg) (Ext::HiddenFunctions::CheckApiErrorFunc((arg)))

#define CheckHMailError(arg) (Ext::HiddenFunctions::CheckHMailErrorFunc((arg)))

#define CheckWSock(arg) (Ext::HiddenFunctions::CheckWSockFunc((arg)))

#define CheckCom(arg) (Ext::HiddenFunctions::CheckComFunc((arg)))

#define CheckWmi(arg) (Ext::HiddenFunctions::CheckWmiFunc((arg)))

#define CheckHandle(arg) (Ext::HiddenFunctions::CheckHandleFunc((arg)))

#define CheckHandleErr(arg) (Ext::HiddenFunctions::CheckHandleErrFunc((arg)))

#define CheckPointer(arg) (Ext::HiddenFunctions::CheckPointerFunc((arg)))

#define CheckPointerErr(arg) (Ext::HiddenFunctions::CheckPointerErrFunc((arg)))

#define Rethrow(arg1, arg2) (Ext::HiddenFunctions::RethrowExceptionFunc((arg1), (arg2)))

	struct HiddenFunctions {
		static bool CheckApiFunc(bool r);

		static bool CheckApiThrowErrorFunc(bool r, DWORD err);

		static DWORD CheckApiErrorFunc(DWORD err);

		static HRESULT CheckHMailErrorFunc(HRESULT err);

		static int CheckWSockFunc(int err);

		static HRESULT CheckComFunc(HRESULT res);

		static HRESULT CheckWmiFunc(HRESULT res);

		static HANDLE CheckHandleFuncHan(HANDLE hnd);

		static HANDLE CheckHandleErrFuncHan(HANDLE hnd);

		static PVOID CheckPointerFuncVoid(PVOID ptr);

		static PVOID CheckPointerErrFuncVoid(PVOID ptr);

		template <typename Type>
		static Type CheckHandleFunc(Type hnd) {
			return (Type)CheckHandleFuncHan((HANDLE)hnd);
		}

		template <typename Type>
		static Type CheckHandleErrFunc(Type hnd) {
			return (Type)CheckHandleErrFuncHan((HANDLE)hnd);
		}

		template <typename Type>
		static Type CheckPointerFunc(Type ptr) {
			return (Type)CheckPointerFuncVoid((PVOID)ptr);
		}

		template <typename Type>
		static Type CheckPointerErrFunc(Type ptr) {
			return (Type)CheckPointerErrFuncVoid((PVOID)ptr);
		}

		static void RethrowExceptionFunc(const AbstractError & prev, const ustring & what);
	};
#endif

}

#endif
