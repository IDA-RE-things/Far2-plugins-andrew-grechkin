#include <libjava/exception.hpp>

#include <libbase/logger.hpp>

namespace Java {
	ustring ErrAsStr(jint code) {
		switch (code) {
			case JNI_OK:
				return ustring(L"Java OK");
			case JNI_ERR:
				return ustring(L"Java unknown error");
			case JNI_EDETACHED:
				return ustring(L"Java thread detached from the VM");
			case JNI_EVERSION:
				return ustring(L"JNI version error");
			case JNI_ENOMEM:
				return ustring(L"Not enough memory");
			case JNI_EEXIST:
				return ustring(L"VM already exist");
			case JNI_EINVAL:
				return ustring(L"Invalid argument");
		}
		return ustring(L"Unknown error");
	}

	///=================================================================================== JavaError
#ifndef NDEBUG
	JavaError::JavaError(jint code, PCSTR file, size_t line, PCSTR func):
		AbstractError(file, line, func),
		m_code(code)
	{
	}
#else
	JavaError::JavaError(jint code):
		m_code(code)
	{
	}
#endif

	JavaError * JavaError::clone() const {
		//	printf(L"WmiError::clone()\n");
		return new JavaError(*this);
	}

	ustring JavaError::type() const {
		return L"JavaError";
	}

	ustring	JavaError::what() const {
		return ErrAsStr(code());
	}

	DWORD JavaError::code() const {
		return m_code;
	}

	void JavaError::format_error(Base::mstring & out) const {
		WCHAR buf[Base::MAX_PATH_LEN] = {0};

		_snwprintf(buf, Base::lengthof(buf), L"Error: %s", what().c_str());
		out.push_back(buf);
#ifndef NDEBUG
		_snwprintf(buf, Base::lengthof(buf), L"Exception: %s", type().c_str());
		out.push_back(buf);
		_snwprintf(buf, Base::lengthof(buf), L"Where: %s", where());
		out.push_back(buf);
#endif
	}


	///=============================================================================== JavaException
#ifndef NDEBUG
	JavaException::JavaException(const Env & env, PCSTR file, size_t line, PCSTR func):
		AbstractError(file, line, func),
		m_jenv(env),
		m_code(0)
	{
	}
#else
	JavaException::JavaException(const Env & env):
		m_jenv(env),
		m_code(0)
	{
	}
#endif

	JavaException * JavaException::clone() const {
		//	printf(L"WmiError::clone()\n");
		return new JavaException(*this);
	}

	ustring JavaException::type() const {
		return L"JavaException";
	}

	ustring	JavaException::what() const {
		jthrowable e = m_jenv->ExceptionOccurred();
		m_jenv->ExceptionClear();
		jclass ex_class = m_jenv->GetObjectClass(e);
		jmethodID mid = m_jenv->GetMethodID(ex_class, "toString", "()Ljava/lang/String;");
		if (mid) {
			jstring j_str = static_cast<jstring>(m_jenv->CallObjectMethod(e, mid));
			return m_jenv.convert_string(j_str);
		}
		return L"JVM unknown exception";
	}

	DWORD JavaException::code() const {
		return m_code;
	}

	void JavaException::format_error(Base::mstring & out) const {
		WCHAR buf[Base::MAX_PATH_LEN] = {0};

		_snwprintf(buf, Base::lengthof(buf), L"Error: %s", what().c_str());
		out.push_back(buf);
#ifndef NDEBUG
		_snwprintf(buf, Base::lengthof(buf), L"Exception: %s", type().c_str());
		out.push_back(buf);
		_snwprintf(buf, Base::lengthof(buf), L"Where: %s", where());
		out.push_back(buf);
#endif
	}


	///=============================================================================================
#ifndef NDEBUG
	jint HiddenFunctions::CheckJavaFunc(jint res, PCSTR file, size_t line, PCSTR func) {
		LogDebug(L"result: %d\n", res);
		if (res != JNI_OK)
			throw JavaError(res, file, line, func);
		return res;
	}

	JNIEnv * HiddenFunctions::CheckJavaExcFunc(const Env & env, PCSTR file, size_t line, PCSTR func) {
		if (env->ExceptionCheck())
			throw JavaException(env, file, line, func);
		return env;
	}
#else
	jint HiddenFunctions::CheckJavaFunc(jint res) {
		LogDebug(L"result: %d\n", res);
		if (res != JNI_OK)
			throw JavaError(res);
		return res;
	}

	JNIEnv * HiddenFunctions::CheckJavaExcFunc(const Env & env) {
		if (env->ExceptionCheck())
			throw JavaException(env);
		return env;
	}
#endif

}
