#include <libjava/jvm.hpp>
#include <libjava/exception.hpp>
#include <libjava/class.hpp>

namespace Java {

	///========================================================================================= Env
	jstring Env::convert_string(const ustring & str) const {
		return m_jenv->NewString(reinterpret_cast<const jchar*>(str.c_str()), static_cast<jsize>(str.length()));
	}

	ustring Env::convert_string(jstring str) const {
		ustring ret;

		const jchar * jc = m_jenv->GetStringChars(str, NULL);
		if (jc) {
			ret = ustring(reinterpret_cast<const wchar_t*>(jc));
			m_jenv->ReleaseStringChars(str, jc);
		}

		return std::move(ret);
	}

	Class Env::get_class(const char * class_name) const {
		return Class(*this, class_name);
	}

	Env::Env(JNIEnv * env):
		m_jenv(env)
	{
	}

}
