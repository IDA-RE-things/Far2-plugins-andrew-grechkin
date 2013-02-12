#include <libbase/env.hpp>
#include <libbase/pcstr.hpp>
#include <libbase/str.hpp>

namespace Base {
	namespace Env {

		ustring get(PCWSTR name) {
			wchar_t buf[::GetEnvironmentVariableW(name, nullptr, 0)];
			::GetEnvironmentVariableW(name, buf, sizeofa(buf));
			return ustring(buf);
		}

		bool set(PCWSTR name, PCWSTR val) {
			return ::SetEnvironmentVariableW(name, val) != 0;
		}

		bool add(PCWSTR name, PCWSTR val) {
			wchar_t buf[::GetEnvironmentVariableW(name, nullptr, 0) + Str::length(val)];
			::GetEnvironmentVariableW(name, buf, sizeofa(buf));
			Str::cat(buf, val);
			return ::SetEnvironmentVariableW(name, buf) != 0;
		}

		bool del(PCWSTR name) {
			return ::SetEnvironmentVariableW(name, nullptr) != 0;
		}

	}
}
