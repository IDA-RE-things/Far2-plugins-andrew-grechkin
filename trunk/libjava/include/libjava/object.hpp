#ifndef _LIBJAVA_OBJECT_HPP_
#define _LIBJAVA_OBJECT_HPP_

#include <libjava/jvm.hpp>

namespace Java {

	struct Object {
		Object(const Class & cl);

		Object(const Class & cl, const char * signature, ...);

		Object(const Class & cl, const char * signature, va_list vl);

		Class get_class() const;

		void call_method_void(const char * name, const char * signature, ...);

		Object call_method_obj(const char * name, const char * signature, ...);

		Object get_field_object(const char * name, const char * type) const;

		int32_t get_field_int(const char * name) const;

		ustring get_field_string(const char * name) const;

		operator jobject () const {
			return m_object;
		}

		Env get_env() const {
			return m_jenv;
		}

	private:
		Object(const Env & jenv, jclass cl, jobject ob);

		Env m_jenv;
		jclass m_class;
		jobject m_object;
	};

}

#endif
