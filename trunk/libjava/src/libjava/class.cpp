#include <libjava/class.hpp>
#include <libjava/exception.hpp>
#include <libjava/object.hpp>
#include <libbase/logger.hpp>

namespace Java {

	Class::~Class() {
	}

	Class::Class(const Env & env, jclass cl):
		m_jenv(env),
		m_class(cl)
	{
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

	void Class::call_method_void(const char * name, const char * signature, ...) const {
		jmethodID mid = get_static_method(name, signature);

		va_list vl;
		va_start(vl, signature);
		m_jenv->CallStaticVoidMethodV(m_class, mid, vl);
		va_end(vl);
		CheckJavaExc(m_jenv);
	}

	jmethodID Class::get_static_method(const char * name, const char * signature) const {
		jmethodID mid = m_jenv->GetStaticMethodID(m_class, name, signature);
		CheckJavaExc(m_jenv);
		return mid;
	}

	jmethodID Class::get_method(const char * name, const char * signature) const {
		jmethodID mid = m_jenv->GetMethodID(m_class, name, signature);
		CheckJavaExc(m_jenv);
		return mid;
	}

	jfieldID Class::get_field(const char * name, const char * signature) const {
		jfieldID fid = m_jenv->GetFieldID(m_class, name, signature);
		CheckJavaExc(m_jenv);
		return fid;
	}

	Class::operator jclass () const {
		return m_class;
	}

	void Class::register_natives(const JNINativeMethod * methods, size_t count) const {
		m_jenv->RegisterNatives(m_class, methods, count);
		CheckJavaExc(m_jenv);
	}

	void Class::run() const {
		call_method_void("main", "([Ljava/lang/String;)V", 0);
	}

}
