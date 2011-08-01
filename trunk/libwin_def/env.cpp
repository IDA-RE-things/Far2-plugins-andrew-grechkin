#include "win_def.h"

namespace	WinEnv {
ustring		Get(PCWSTR name) {
	WCHAR	buf[::GetEnvironmentVariableW(name, nullptr, 0)];
	::GetEnvironmentVariableW(name, buf, sizeofa(buf));
	return buf;
}
bool		Set(PCWSTR name, PCWSTR val) {
	return ::SetEnvironmentVariableW(name, val) != 0;
}
bool		Add(PCWSTR name, PCWSTR val) {
	WCHAR	buf[::GetEnvironmentVariableW(name, nullptr, 0) + Len(val)];
	::GetEnvironmentVariableW(name, buf, sizeofa(buf));
	Cat(buf, val);
	return ::SetEnvironmentVariableW(name, buf) != 0;
}
bool		Del(PCWSTR name) {
	return ::SetEnvironmentVariableW(name, nullptr) != 0;
}
}
