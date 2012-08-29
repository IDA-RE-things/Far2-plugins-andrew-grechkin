#ifndef _LIBJAVA_EXCEPTION_HPP_
#define _LIBJAVA_EXCEPTION_HPP_

#include <libext/exception.hpp>
#include <libjava/jvm.hpp>


namespace Java {

	///=================================================================================== JavaError
	struct JavaError: public Ext::AbstractError {
		virtual JavaError * clone() const;

		virtual ustring type() const;

		virtual ustring	 what() const;

		virtual DWORD code() const;

		virtual void format_error(Base::mstring & out) const;

	private:
#ifndef NDEBUG
		JavaError(jint code, PCSTR file, size_t line, PCSTR func);
#else
		JavaError(jint code);
#endif

		DWORD m_code;

		friend struct HiddenFunctions;
	};


	///=============================================================================== JavaException
	struct JavaException: public Ext::AbstractError {
		virtual JavaException * clone() const;

		virtual ustring type() const;

		virtual ustring	 what() const;

		virtual DWORD code() const;

		virtual void format_error(Base::mstring & out) const;

	private:
#ifndef NDEBUG
		JavaException(const Env & env, PCSTR file, size_t line, PCSTR func);
#else
		JavaException(const Env & env);
#endif

		Env m_jenv;
		DWORD m_code;

		friend struct HiddenFunctions;
	};


	///=============================================================================================
#ifndef NDEBUG

#define CheckJavaErr(arg) (Java::HiddenFunctions::CheckJavaErrFunc((arg), THROW_PLACE))

#define CheckJavaThrowErr(arg1, arg2) (Java::HiddenFunctions::CheckJavaThrowErrFunc((arg1), (arg2), THROW_PLACE))

#define CheckJavaExc(arg) (Java::HiddenFunctions::CheckJavaExcFunc((arg), THROW_PLACE))

	struct HiddenFunctions {
		static jint CheckJavaErrFunc(jint res, PCSTR file, size_t line, PCSTR func);
		static jint CheckJavaThrowErrFunc(bool r, jint err, PCSTR file, size_t line, PCSTR func);
		static Env CheckJavaExcFunc(const Env & env, PCSTR file, size_t line, PCSTR func);
	};

#else

#define CheckJavaErr(arg) (Java::HiddenFunctions::CheckJavaErrFunc((arg)))

#define CheckJavaThrowErr(arg1, arg2) (Java::HiddenFunctions::CheckJavaThrowErrFunc((arg1), (arg2)))

#define CheckJavaExc(arg) (Java::HiddenFunctions::CheckJavaExcFunc((arg)))

	struct HiddenFunctions {
		static jint CheckJavaErrFunc(jint res);
		static jint CheckJavaThrowErrFunc(bool r, jint err);
		static Env CheckJavaExcFunc(const Env & env);
	};

#endif


}


#endif
