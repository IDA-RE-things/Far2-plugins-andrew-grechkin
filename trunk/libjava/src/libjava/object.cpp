#include <libjava/object.hpp>

#include <libjava/exception.hpp>
#include <libjava/class.hpp>

namespace Java {

	Object::Object(const Class & cl):
//		m_jenv(cl.get_env()),
		m_class(cl),
		m_object(m_class.get_env()->AllocObject(m_class))
	{
		CheckJavaExc(m_class.get_env());
	}

	Object::Object(const Class & cl, const char * signature, ...):
//		m_jenv(cl.get_env()),
		m_class(cl),
		m_object(nullptr)
	{
		jmethodID mid = m_class.get_method("<init>", signature);

		va_list vl;
		va_start(vl, signature);
		m_object = m_class.get_env()->NewObjectV(m_class, mid, vl);
		va_end(vl);

		CheckJavaExc(m_class.get_env());
	}

	Object::Object(const Class & cl, const char * signature, va_list vl):
//		m_jenv(cl.get_env()),
		m_class(cl),
		m_object(nullptr)
	{
		jmethodID mid = m_class.get_method("<init>", signature);

		m_object = m_class.get_env()->NewObjectV(m_class, mid, vl);
		CheckJavaExc(m_class.get_env());
	}

	Object::Object(const Env & env, jclass cl, jobject ob):
//		m_jenv(env),
		m_class(env, cl),
		m_object(ob)
	{
	}

	Class Object::get_class() const {
		return Class(*this);
	}

	Object Object::call_method_obj(const char * name, const char * signature, ...) {
		jmethodID mid = m_class.get_method(name, signature);
		CheckJavaExc(m_class.get_env());

		va_list vl;
		va_start(vl, signature);
		jobject obj = m_class.get_env()->CallObjectMethodV(m_object, mid, vl);
		va_end(vl);
		CheckJavaExc(m_class.get_env());
		return Object(m_class.get_env(), m_class, obj);
	}

	void Object::call_method_void(const char * name, const char * signature, ...) {
		jmethodID mid = m_class.get_method(name, signature);

		va_list vl;
		va_start(vl, signature);
		m_class.get_env()->CallVoidMethodV(m_object, mid, vl);
		va_end(vl);
		CheckJavaExc(m_class.get_env());
	}

	void Object::call_method_void(const char * name, const char * signature, va_list vl) {
		jmethodID mid = m_class.get_method(name, signature);

		m_class.get_env()->CallVoidMethodV(m_object, mid, vl);
		CheckJavaExc(m_class.get_env());
	}

	int32_t Object::get_field_int(const char * name) const {
		jfieldID field = m_class.get_field(name, "I");
		jint ret = m_class.get_env()->GetIntField(m_object, field);
		CheckJavaExc(m_class.get_env());
		return ret;
	}

	ustring Object::get_field_string(const char * name) const {
		jfieldID field = m_class.get_field(name, "Ljava/lang/String;");

		jobject ret = m_class.get_env()->GetObjectField(m_object, field);
		CheckJavaExc(m_class.get_env());
//		jstring str = reinterpret_cast<jstring>(get_field_object(name, "Ljava/lang/String;"));
		return m_class.get_env().convert_string((jstring)ret);
	}

	Object Object::get_field_object(const char * name, const char * signature) const {
		jfieldID field = m_class.get_field(name, signature);

		jobject obj = m_class.get_env()->GetObjectField(m_object, field);
		CheckJavaExc(m_class.get_env());
		return Object(m_class.get_env(), m_class, obj);
	}
}
