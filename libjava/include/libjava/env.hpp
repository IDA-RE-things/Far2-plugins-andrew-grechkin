#ifndef _LIBJAVA_ENV_HPP_
#define _LIBJAVA_ENV_HPP_

#include <libbase/std.hpp>

#include <jni.h>


namespace Java {

	struct Class;
	struct Object;


	///========================================================================================= Env
	struct Env {
		JNIEnv * operator -> () const {
			return m_jenv;
		}

		jstring convert_string(const ustring & str) const;

		ustring convert_string(jstring str) const;

		Class get_class(const char * class_name) const;

	private:
		Env(JNIEnv * env);

		JNIEnv * m_jenv;	// native method interface

		friend struct Vm;
	};

}


#endif
