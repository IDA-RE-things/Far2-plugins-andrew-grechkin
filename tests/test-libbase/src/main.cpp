#include <libbase/logger.hpp>
#include <libbase/bit.hpp>
#include <libbase/backtrace.hpp>

//void test_ustring() {
//	PCWSTR pcwstr = L"PCWSTR";
//	PWSTR pwstr = (PWSTR)L"pwstr";
//	ustring str1(6, L'h');
//	ustring str2(pcwstr);
//	ustring str3 = pwstr;
//
//	printf(L"str1: %s\n", str1.c_str());
//	printf(L"str2: %s\n", str2.c_str());
//	printf(L"str3: %s\n\n", str3.c_str());
//
//	str1 = L"que1";
//	str2 = str3;
//	str3 = pwstr;
//
//	printf(L"str1: %s\n", str1.c_str());
//	printf(L"str2: %s\n", str2.c_str());
//	printf(L"str3: %s\n", str3.c_str());
//
//}
using namespace Base;

DWORD thread(void * /*tgt*/) {
//	Logger::Target_i * con = (Logger::Target_i *)tgt;
	LogTrace();
	for (size_t i = 0; i < 100; ++i) {
//		tgt->out(L"a", 1);
		LogDebug(L"i: %Id\n", i);
//		::Sleep(100);
	}
	LogTrace();
//	con->out(L"t\n", 2);
	return 0;
}


void test_backtrace() {
	LogTrace();
	Base::Backtrace bt;
	for (auto it = bt.begin(); it != bt.end(); ++it) {
		LogInfo(L"%s\n", it->AsStr().c_str());
	}
}

void test_logger() {
	Base::Logger::set_target(Base::Logger::get_TargetToConsole());
	Base::Logger::set_level(Base::Logger::Level::Info);
	Logger::set_color_mode(true);
	LogTrace();
////	Logger::set_target(Logger::get_TargetToFile(L"c:/qwe.log"));
//	Logger::set_target(Logger::get_TargetToConsole());
//	Logger::set_level(Logger::LVL_TRACE);
//	Logger::set_wideness(Logger::WIDE_FULL);
//	LogTrace();
//	LogDebug(L"sasdasd\n");
//	LogInfo(L"sasdasd\n");
//	LogReport(L"sasdasd\n");
//	LogAtten(L"sasdasd\n");
//	LogWarn(L"QWQWeqweqw\n");
//	LogError(L"QWQWeqweqw\n");
//	LogFatal(L"zxczxcx\n");
//
//	HANDLE th_id[2] {
//		::CreateThread(nullptr, 0, &thread, nullptr, 0, nullptr),
//		::CreateThread(nullptr, 0, &thread, nullptr, 0, nullptr),
//	};
//
//	LogTrace();
//	::WaitForMultipleObjects(2, th_id, TRUE, INFINITE);
//	LogTrace();
}

int main() {
//	test_ustring();
	test_logger();
	test_backtrace();
	return 0;
}
