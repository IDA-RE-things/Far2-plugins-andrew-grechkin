#ifndef _LIBJAVA_CLASS_HPP_
#define _LIBJAVA_CLASS_HPP_

#include <libjava/jvm.hpp>
#include <libjava/object.hpp>


namespace Java {
	struct Class {
		~Class();

		Class(const Env & env, const char * class_name);

		Class(const Object & object);

		Object get_object() const;

		Object get_object(const char * signature, ...) const;

		void call_method_void(const char * name, const char * signature, ...);

		jmethodID get_method(const char * name, const char * signature) const;

		operator jclass () const;

		Env get_env() const {
			return m_jenv;
		}

	private:
		Env m_jenv;
		jclass m_class;
	};
}

#endif
