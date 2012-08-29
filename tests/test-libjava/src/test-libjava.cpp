#include <iostream>

#include <libjava/jvm.hpp>
#include <libjava/exception.hpp>
#include <libjava/class.hpp>
#include <libbase/logger.hpp>

#include <string>

int main() try {
	Base::Logger::set_target(Base::Logger::get_TargetToConsole());
	Base::Logger::set_color_mode(true);
	Base::Logger::set_level(Base::Logger::LVL_TRACE);
	LogTrace();

	{
		LogTrace();
		//		Class cl(env.get_class("org/andrewgrechkin/LibTest"));
		//		Class cl(env.get_class("org/andrewgrechkin/MainWindow"));
		//		ob.call_method_void("print", "()V");
//		Java::Env env = Java::get_env();
		Java::Env env = Java::create_vm("-Djava.class.path=D:\\projects\\andrew-grechkin\\tests\\test-libjava\\Debug\\LibTest.jar");
		LogTrace();
		env.get_class("org/andrewgrechkin/MainWindow1").run();
		LogTrace();
		Java::destroy_vm();
	}
	LogTrace();

	return 0;
} catch (Ext::AbstractError & e) {
	LogError(L"exception cought: %s, %s\n", e.what().c_str(), e.where());
	return e.code();
} catch (std::exception & e) {
	LogError(L"std::exception [%S]:\n", typeid(e).name());
	LogError(L"What: %S\n", e.what());
	return 1;
}
