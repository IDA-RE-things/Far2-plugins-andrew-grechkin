#include <libjava/jvm.hpp>
#include <libjava/env.hpp>
#include <libjava/exception.hpp>
#include <libjava/class.hpp>
#include <libjava/object.hpp>

#include <libext/dll.hpp>
#include <libext/reg.hpp>
#include <libbase/logger.hpp>
#include <libbase/str.hpp>

#include <cassert>

namespace Java {

	const ssize_t DEFAULT_VERSION = JNI_VERSION_1_6;

	enum JvmType {
		JVM_JRE,
		JVM_JDK,
	};


	namespace Register {
//		static PCWSTR const RUNTIME_LIB = L"RuntimeLib";
		static PCWSTR const HOME_KEY = L"JavaHome";
//		static PCWSTR const HOME_ENVIRONMENT = L"JAVA_HOME";
		static PCWSTR const CURRENT_VERSION = L"CurrentVersion";
		static PCWSTR const PATHS[] = {
			L"SOFTWARE\\JavaSoft\\Java Runtime Environment",
			L"SOFTWARE\\JavaSoft\\Java Development Kit",
		};

		ustring get_version(JvmType type);

		ustring get_parameter(JvmType type, PCWSTR name);

		ustring get_version(JvmType type) {
			using Ext::Register;
			Register reg(PATHS[type], HKEY_LOCAL_MACHINE);
			return reg.get(CURRENT_VERSION, Base::EMPTY_STR);
		}

		ustring get_parameter(JvmType type, PCWSTR name) {
			using Ext::Register;
			return Register(PATHS[type], HKEY_LOCAL_MACHINE).open_subkey_stored_in_key(CURRENT_VERSION).get(name, Base::EMPTY_STR);
		}

	}

	///================================================================================== AttachArgs
	AttachArgs::AttachArgs()
	{
		version = DEFAULT_VERSION;
		name = nullptr;
		group = nullptr;
	}

	///===================================================================================== Jvm_dll
	struct Jvm_dll: private Ext::DynamicLibrary {
		typedef jint (JNICALL *FJNI_CreateJavaVM)(JavaVM **pvm, void **penv, void *args);
		typedef jint (JNICALL *FJNI_GetCreatedJavaVMs)(JavaVM **, jsize, jsize *);
		typedef jint (JNICALL *FJNI_GetDefaultJavaVMInitArgs)(void *args);


		DEFINE_FUNC(JNI_CreateJavaVM);
		DEFINE_FUNC(JNI_GetCreatedJavaVMs);

		Jvm_dll():
			DynamicLibrary(find_and_load_dll())
		{
			GET_DLL_FUNC(JNI_CreateJavaVM);
			GET_DLL_FUNC(JNI_GetCreatedJavaVMs);
			GET_DLL_FUNC(JNI_GetDefaultJavaVMInitArgs);
		}

		JavaVMInitArgs get_default_init_args() const {
			JavaVMInitArgs ret;
			ret.version = DEFAULT_VERSION;
			CheckJavaErr(JNI_GetDefaultJavaVMInitArgs(&ret));
			return ret;
		}

	private:
		DEFINE_FUNC(JNI_GetDefaultJavaVMInitArgs);

		static HMODULE find_and_load_dll() {
			HMODULE ret = ::LoadLibraryW(DLL_NAME);
			if (!ret) {
				static PCWSTR const paths[] = {
					L"bin\\server",
					L"bin\\client",
					L"jre\\bin\\server",
					L"jre\\bin\\client",
				};
				ustring home_path(Register::get_parameter(JVM_JRE, Register::HOME_KEY));
				for (PCWSTR sub_path : paths) {
					ustring dll_path(Base::as_str(L"%s\\%s\\%s", home_path.c_str(), sub_path, DLL_NAME));
					ret = ::LoadLibraryW(dll_path.c_str());
					if (ret)
						break;
				}
			}

			return ret;
		}

		static PCWSTR const DLL_NAME;
	};

	PCWSTR const Jvm_dll::DLL_NAME = L"jvm.dll";


	///========================================================================================= Jvm
	struct Vm: private Jvm_dll {

		static Vm & inst() {
			static Vm ret;
			return ret;
		}

		Env get_env() const;

		Env create(PCSTR class_path);

		void destroy();

		Env attach(const JavaVMAttachArgs & args) const;

		void detach() const;

	private:
		~Vm();

		Vm();

		JavaVM * m_jvm;		// Java VM
		ssize_t m_version;
	};

	Vm::~Vm() {
		destroy_vm();
	}

	Vm::Vm():
		m_jvm(nullptr),
		m_version(DEFAULT_VERSION)
	{
	}

	Env Vm::get_env() const {
		LogTrace();
		CheckJavaThrowErr(m_jvm, JNI_EDETACHED);

		JNIEnv * tmp = nullptr;
		CheckJavaErr(m_jvm->GetEnv((void**)&tmp, m_version));
		return Env(tmp);
	}

	Env Vm::create(PCSTR class_path) {
		LogTrace();
		if (m_jvm)
			return get_env();
//		CheckJavaThrowErr(m_jvm == nullptr, JNI_EEXIST);

		JNIEnv * tmp = nullptr;
		//Create class path
		JavaVMOption options[1];
		options[0].optionString = (PSTR)class_path;

		//Initialize Java Virtual Machine
		JavaVMInitArgs vm_args = get_default_init_args();
		vm_args.nOptions = Base::lengthof(options);
		vm_args.options = options;
		vm_args.ignoreUnrecognized = JNI_TRUE;

		LogDebug(L"version: 0x%x\n", vm_args.version);
		LogDebug(L"options: %p\n", vm_args.options);
		LogDebug(L"options cnt: %d\n", vm_args.nOptions);
		LogDebug(L"ignoreUnrecognized: %d\n", vm_args.ignoreUnrecognized);

		CheckJavaErr(JNI_CreateJavaVM(&m_jvm, (void**)&tmp, &vm_args));
		return Env(tmp);
	}

	void Vm::destroy() {
		LogTrace();
		if (!m_jvm)
			return;
		m_jvm->DestroyJavaVM();
		m_jvm = nullptr;
	}

	Env Vm::attach(const JavaVMAttachArgs & args) const {
		LogTrace();
		CheckJavaThrowErr(m_jvm, JNI_ERR);
		JNIEnv * l_jenv = nullptr;
		CheckJavaErr(m_jvm->AttachCurrentThread((void**)&l_jenv, (void*)&args));
		return Env(l_jenv);
	}

	void Vm::detach() const {
		LogTrace();
		CheckJavaThrowErr(m_jvm, JNI_EDETACHED);
		CheckJavaErr(m_jvm->DetachCurrentThread());
	}

	///=============================================================================================
	Env create_vm(PCSTR class_path) {
		return Vm::inst().create(class_path);
	}

	void destroy_vm() {
		Vm::inst().destroy();
	}

	Env attach_current_thread(const JavaVMAttachArgs & args) {
		return Vm::inst().attach(args);
	}

	void detach_current_thread() {
		Vm::inst().detach();
	}

	Env get_env() {
		return Vm::inst().get_env();
	}

}

