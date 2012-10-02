#ifndef _LIBJAVA_OBJECT_HPP_
#define _LIBJAVA_OBJECT_HPP_

#include <libjava/env.hpp>
#include <libjava/class.hpp>


namespace Java {

	struct Object {
		Object(const Class & cl);

		Object(const Class & cl, const char * signature, ...);

		Object(const Class & cl, const char * signature, va_list vl);

		Class get_class() const;

		void call_method_void(const char * name, const char * signature, ...);

		void call_method_void(const char * name, const char * signature, va_list vl);

		Object call_method_obj(const char * name, const char * signature, ...);

		Object get_field_object(const char * name, const char * signature) const;

		int32_t get_field_int(const char * name) const;

		ustring get_field_string(const char * name) const;

		operator jobject () const {
			return m_object;
		}

		const Env & get_env() const {
			return m_class.get_env();
		}

	private:
		Object(const Env & jenv, jclass cl, jobject ob);

//		Env m_jenv;
//		jclass m_class;
		Class m_class;
		jobject m_object;
	};

}

#endif
