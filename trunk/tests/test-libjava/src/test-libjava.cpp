#include <iostream>

#include <libbase/logger.hpp>
#include <libext/exception.hpp>
#include <libjava/jvm.hpp>

#include <string>

int main() try {
	Base::Logger::set_target(Base::Logger::get_TargetToConsole());
	Base::Logger::set_color_mode(true);
	Base::Logger::set_level(Base::Logger::LVL_TRACE);
	LogTrace();

	{
		Java::Jvm vm;
	}
//	Sleep(10000);
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
