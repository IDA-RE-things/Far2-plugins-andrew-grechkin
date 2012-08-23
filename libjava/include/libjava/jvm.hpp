#ifndef _LIBJAVA_JVM_HPP_
#define _LIBJAVA_JVM_HPP_

#include <libext/dll.hpp>

#include <jni.h>

namespace Java {

	const ssize_t DEFAULT_VERSION = JNI_VERSION_1_6;

	enum JvmType {
		JVM_JRE,
		JVM_JDK,
	};

	///========================================================================================= Env
	struct Object;
	struct Class;

	struct Env {
		JNIEnv * operator -> () const {
			return m_jenv;
		}

		operator JNIEnv * () const {
			return m_jenv;
		}

		JNIEnv ** get_env_ptr() {
			return &m_jenv;
		}

		jstring convert_string(const ustring & str) const;

		ustring convert_string(jstring str) const;

		Class get_class(const char * class_name) const;

	private:
		Env(JNIEnv * env = nullptr);

		JNIEnv * m_jenv;	// native method interface

		friend struct Jvm;
	};


	///================================================================================== AttachArgs
	struct AttachArgs: public JavaVMAttachArgs {
		AttachArgs();
	};


	///===================================================================================== Jvm_dll
	struct Jvm_dll: private Ext::DynamicLibrary {
		typedef jint (JNICALL *FJNI_CreateJavaVM)(JavaVM **pvm, void **penv, void *args);
		typedef jint (JNICALL *FJNI_GetDefaultJavaVMInitArgs)(void *args);
		typedef jint (JNICALL *FJNI_GetCreatedJavaVMs)(JavaVM **, jsize, jsize *);

		DEFINE_FUNC(JNI_CreateJavaVM);
		DEFINE_FUNC(JNI_GetDefaultJavaVMInitArgs);
		DEFINE_FUNC(JNI_GetCreatedJavaVMs);

		static Jvm_dll & inst();

		JavaVMInitArgs get_default_init_args() const;

	private:
		~Jvm_dll();

		Jvm_dll();

		static HMODULE find_and_load_dll();

		static PCWSTR const DLL_NAME;
	};


	///========================================================================================== VM
	struct Jvm {
		~Jvm();

		Jvm();

		Env create_vm();

		Env attach_current_thread(JavaVMAttachArgs args);

		void detach_current_thread();

		Env get_env() const;

	private:
		JavaVM * m_jvm;		// Java VM
		Env m_jenv;
	};

}


#endif
