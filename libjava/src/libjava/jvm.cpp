#include <libjava/jvm.hpp>
#include <libjava/exception.hpp>
#include <libjava/class.hpp>
#include <libjava/object.hpp>

#include <libbase/logger.hpp>
#include <libbase/va_list.hpp>
#include <libext/reg.hpp>

#include <cassert>

namespace Java {

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

	///========================================================================================= Env
	jstring Env::convert_string(const ustring & str) const {
		return m_jenv->NewString(reinterpret_cast<const jchar*>(str.c_str()), static_cast<jsize>(str.length()));
	}

	ustring Env::convert_string(jstring str) const {
		ustring ret;

		const jchar * jc = m_jenv->GetStringChars(str, NULL);
		if (jc) {
			ret = ustring(reinterpret_cast<const wchar_t*>(jc));
			m_jenv->ReleaseStringChars(str, jc);
		}

		return std::move(ret);
	}

	Class Env::get_class(const char * class_name) const {
		return Class(*this, class_name);
	}

	Env::Env(JNIEnv * env):
		m_jenv(env)
	{
//		LogDebug(L"env: %p\n", env);
	}

	///================================================================================== AttachArgs
	AttachArgs::AttachArgs()
	{
		version = DEFAULT_VERSION;
		name = nullptr;
		group = nullptr;
	}

	///===================================================================================== Jvm_dll
	Jvm_dll & Jvm_dll::inst() {
		static Jvm_dll ret;
		return ret;
	}

	Jvm_dll::~Jvm_dll() {
	}

	Jvm_dll::Jvm_dll():
		DynamicLibrary(find_and_load_dll())
	{
		LogTrace();
		GET_DLL_FUNC(JNI_CreateJavaVM);
		GET_DLL_FUNC(JNI_GetDefaultJavaVMInitArgs);
		GET_DLL_FUNC(JNI_GetCreatedJavaVMs);
		LogTrace();
	}

	HMODULE Jvm_dll::find_and_load_dll() {
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

	PCWSTR const Jvm_dll::DLL_NAME = L"jvm.dll";


	///========================================================================================== VM
	Jvm::~Jvm() {
		m_jvm->DestroyJavaVM();
	}

	Jvm::Jvm():
		m_jvm(nullptr),
		m_jenv(nullptr)
	{
		LogTrace();
		Jvm_dll::inst();

		m_jenv = create_vm();

		Env env(get_env());

//		Class cl(env.get_class("org/andrewgrechkin/LibTest"));
		Class cl(env.get_class("org/andrewgrechkin/MainWindow"));

		cl.call_method_void("main", "([Ljava/lang/String;)V");

//		LogTrace();
//		Object ob(cl);
//
//		LogTrace();
//		ob.call_method_void("print", "()V");

		LogTrace();
	}

	Env Jvm::create_vm() {
		LogTrace();

		//Create class path
		astring class_path = "-Djava.class.path=D:\\projects\\andrew-grechkin\\tests\\test-libjava\\Debug\\LibTest.jar";
		JavaVMOption options[1];
		options[0].optionString = &class_path.front();

		//Initialize Java Virtual Machine
		JavaVMInitArgs vm_args;
		vm_args.version = JNI_VERSION_1_6;
		CheckJava(JNI_GetDefaultJavaVMInitArgs(&vm_args));
		vm_args.nOptions = Base::lengthof(options);
		vm_args.options = options;
		vm_args.ignoreUnrecognized = JNI_TRUE;

		LogDebug(L"version: %d\n", vm_args.version);
		LogDebug(L"options: %p\n", vm_args.options);
		LogDebug(L"options cnt: %d\n", vm_args.nOptions);
		LogDebug(L"ignoreUnrecognized: %d\n", vm_args.ignoreUnrecognized);

		CheckJava(JNI_CreateJavaVM(&m_jvm, reinterpret_cast<void**>(m_jenv.get_env_ptr()), &vm_args));

		LogDebug(L"Env: %p\n", (JNIEnv*)m_jenv);

		return m_jenv;
	}

	Env Jvm::attach_current_thread(JavaVMAttachArgs args) {
		LogTrace();
		JNIEnv * l_jenv = nullptr;
		CheckJava(m_jvm->AttachCurrentThread(reinterpret_cast<void**>(&l_jenv), &args));
		return Env(l_jenv);
	}

	void Jvm::detach_current_thread() {
		CheckJava(m_jvm->DetachCurrentThread());
	}

	JavaVMInitArgs Jvm_dll::get_default_init_args() const {
		JavaVMInitArgs ret;
		ret.version = JNI_VERSION_1_6;
		JNI_GetDefaultJavaVMInitArgs(&ret);
		return ret;
	}

	Env Jvm::get_env() const {
		return Env(m_jenv);
	}


}

