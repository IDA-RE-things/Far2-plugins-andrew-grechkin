#include <libjava/object.hpp>

#include <libjava/exception.hpp>
#include <libjava/class.hpp>

namespace Java {

	Object::Object(const Class & cl):
		m_jenv(cl.get_env()),
		m_class(cl),
		m_object(m_jenv->AllocObject(m_class))
	{
		CheckJavaExc(m_jenv);
	}

	Object::Object(const Class & cl, const char * signature, ...):
		m_jenv(cl.get_env()),
		m_class(cl),
		m_object(nullptr)
	{
		jmethodID mid = m_jenv->GetMethodID(m_class, "<init>", signature);
		CheckJavaExc(m_jenv);

		va_list vl;
		va_start(vl, signature);
		m_object = m_jenv->NewObjectV(m_class, mid, vl);
		va_end(vl);

		CheckJavaExc(m_jenv);
	}

	Object::Object(const Class & cl, const char * signature, va_list vl):
		m_jenv(cl.get_env()),
		m_class(cl),
		m_object(nullptr)
	{
		jmethodID mid = m_jenv->GetMethodID(m_class, "<init>", signature);
		CheckJavaExc(m_jenv);

		m_object = m_jenv->NewObjectV(m_class, mid, vl);
		CheckJavaExc(m_jenv);
	}

	Object::Object(const Env & env, jclass cl, jobject ob):
		m_jenv(env),
		m_class(cl),
		m_object(ob)
	{
	}

	Class Object::get_class() const {
		return Class(*this);
	}

	Object Object::call_method_obj(const char * name, const char * signature, ...) {
		jmethodID mid = m_jenv->GetMethodID(m_class, name, signature);
		CheckJavaExc(m_jenv);

		va_list vl;
		va_start(vl, signature);
		jobject obj = m_jenv->CallObjectMethodV(m_object, mid, vl);
		va_end(vl);
		CheckJavaExc(m_jenv);
		return Object(m_jenv, m_class, obj);
	}

	void Object::call_method_void(const char * name, const char * signature, ...) {
		jmethodID mid = m_jenv->GetMethodID(m_class, name, signature);
		CheckJavaExc(m_jenv);

		va_list vl;
		va_start(vl, signature);
		m_jenv->CallVoidMethodV(m_object, mid, vl);
		va_end(vl);
		CheckJavaExc(m_jenv);
	}

	void Object::call_method_void(const char * name, const char * signature, va_list vl) {
		jmethodID mid = m_jenv->GetMethodID(m_class, name, signature);
		CheckJavaExc(m_jenv);

		m_jenv->CallVoidMethodV(m_object, mid, vl);
		CheckJavaExc(m_jenv);
	}

	int32_t Object::get_field_int(const char * name) const {
		jfieldID field = m_jenv->GetFieldID(m_class, name, "I");
		CheckJavaExc(m_jenv);
		jint ret = m_jenv->GetIntField(m_object, field);
		CheckJavaExc(m_jenv);
		return ret;
	}

	ustring Object::get_field_string(const char* name) const {
		jfieldID field = m_jenv->GetFieldID(m_class, name, "Ljava/lang/String;");
		CheckJavaExc(m_jenv);
		jobject ret = m_jenv->GetObjectField(m_object, field);
		CheckJavaExc(m_jenv);
//		jstring str = reinterpret_cast<jstring>(get_field_object(name, "Ljava/lang/String;"));
		return m_jenv.convert_string((jstring)ret);
	}

	Object Object::get_field_object(const char* name, const char* type) const {
		jfieldID field = m_jenv->GetFieldID(m_class, name, type);
		CheckJavaExc(m_jenv);

		jobject obj = m_jenv->GetObjectField(m_object, field);
		CheckJavaExc(m_jenv);
		return Object(m_jenv, m_class, obj);
	}
}
