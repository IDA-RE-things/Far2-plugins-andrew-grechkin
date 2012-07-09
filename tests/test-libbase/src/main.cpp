#include <libbase/logger.hpp>
#include <libbase/bit.hpp>

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

void test_logger() {
	Logger::init(Logger::get_TargetToSys(L"Qwertyu"), Logger::LVL_TRACE);
//	Logger::init(Logger::get_TargetToFile(L"c:/qwe.log"), Logger::LVL_TRACE);
//	Logger::init(Logger::get_TargetToConsole(), Logger::LVL_TRACE);
//	Logger::set_color_mode(true);
//	Logger::set_wideness(Logger::WIDE_FULL);
	LogTrace();
	LogDebug(L"sasdasd\n");
	LogInfo(L"sasdasd\n");
	LogReport(L"sasdasd\n");
	LogAtten(L"sasdasd\n");
	LogWarn(L"QWQWeqweqw\n");
	LogError(L"QWQWeqweqw\n");
	LogFatal(L"zxczxcx\n");

	size_t lim = 5;
	lim = WinBit::Limit<size_t>(lim);
}

int main() {
//	test_ustring();
	test_logger();
	return 0;
}
