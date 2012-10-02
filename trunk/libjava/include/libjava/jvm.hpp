#ifndef _LIBJAVA_JVM_HPP_
#define _LIBJAVA_JVM_HPP_

#include <libbase/std.hpp>

#include <jni.h>


namespace Java {

	struct Env;


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
