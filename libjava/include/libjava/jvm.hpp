#ifndef _LIBJAVA_JVM_HPP_
#define _LIBJAVA_JVM_HPP_

#include <libbase/std.hpp>

#include <jni.h>

namespace Java {

	struct Object;
	struct Class;


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


	///================================================================================== AttachArgs
	struct AttachArgs: public JavaVMAttachArgs {
		AttachArgs();
	};


	///=============================================================================================
	Env create_vm(PCSTR class_path = "");

	void destroy_vm();

	Env attach_current_thread(const JavaVMAttachArgs & args);

	void detach_current_thread();

	Env get_env();
}


#endif
