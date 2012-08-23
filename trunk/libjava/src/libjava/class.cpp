#include <libjava/class.hpp>
#include <libjava/exception.hpp>

namespace Java {

	Class::~Class() {
	}

	Class::Class(const Env & env, const char * class_name):
		m_jenv(env),
		m_class(m_jenv->FindClass(class_name))
	{
		CheckJavaExc(m_jenv);
	}

	Class::Class(const Object & object):
		m_jenv(object.get_env()),
		m_class(m_jenv->GetObjectClass(object))
	{
		CheckJavaExc(m_jenv);
	}

	Object Class::get_object() const {
		return Object(*this);
	}

	Object Class::get_object(const char * signature, ...) const {
		va_list vl;
		va_start(vl, signature);
		Object l_object(*this, signature, vl);
		va_end(vl);
		return l_object;
	}

	void Class::call_method_void(const char * name, const char * signature, ...) {
		jmethodID mid = m_jenv->GetStaticMethodID(m_class, name, signature);
		CheckJavaExc(m_jenv);

		va_list vl;
		va_start(vl, signature);
		m_jenv->CallStaticVoidMethodV(m_class, mid, vl);
		va_end(vl);
		CheckJavaExc(m_jenv);
	}

	jmethodID Class::get_method(const char * name, const char * signature) const {
		jmethodID mid = m_jenv->GetMethodID(m_class, name, signature);
		CheckJavaExc(m_jenv);
		return mid;
	}

	Class::operator jclass () const {
		return m_class;
	}
}
