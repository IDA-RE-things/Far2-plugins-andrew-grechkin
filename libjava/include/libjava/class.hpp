#ifndef _LIBJAVA_CLASS_HPP_
#define _LIBJAVA_CLASS_HPP_

#include <libjava/env.hpp>


namespace Java {

	struct Class {
		~Class();

		Class(const Env & env, jclass cl);

		Class(const Env & env, const char * class_name);

		Class(const Object & object);

		Object get_object() const;

		Object get_object(const char * signature, ...) const;

		void call_method_void(const char * name, const char * signature, ...) const;

		jmethodID get_static_method(const char * name, const char * signature) const;

		jmethodID get_method(const char * name, const char * signature) const;

		jfieldID get_field(const char * name, const char * signature) const;

		void register_natives(const JNINativeMethod * methods, size_t count) const;

		operator jclass () const;

		const Env & get_env() const {
			return m_jenv;
		}

		void run() const;

	private:
		Env m_jenv;
		jclass m_class;
	};

}

#endif
