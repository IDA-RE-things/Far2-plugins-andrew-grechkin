#include "win_def.h"

namespace WinEnv {
	ustring get(PCWSTR name) {
		WCHAR buf[::GetEnvironmentVariableW(name, nullptr, 0)];
		::GetEnvironmentVariableW(name, buf, sizeofa(buf));
		return buf;
	}

	bool set(PCWSTR name, PCWSTR val) {
		return ::SetEnvironmentVariableW(name, val) != 0;
	}

	bool add(PCWSTR name, PCWSTR val) {
		WCHAR buf[::GetEnvironmentVariableW(name, nullptr, 0) + Len(val)];
		::GetEnvironmentVariableW(name, buf, sizeofa(buf));
		Cat(buf, val);
		return ::SetEnvironmentVariableW(name, buf) != 0;
	}

	bool del(PCWSTR name) {
		return ::SetEnvironmentVariableW(name, nullptr) != 0;
	}
}
