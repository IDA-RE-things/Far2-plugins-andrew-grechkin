//#include <libwin_def/win_def.h>
#include <libwin_def/log.h>

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

void test_logger() {
	logger_init(new LogToFile(L"c:/qwe.log"), LOG_TRACE);
	LogDebug(L"sasdasd\n");
	LogWarn(L"QWQWeqweqw\n");
	LogFatal(L"zxczxcx\n");
}

int main() {
//	test_ustring();
	test_logger();
	return 0;
}
