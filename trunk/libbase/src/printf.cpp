#include <libbase/console.hpp>

namespace Base {

	int consoleout(PCWSTR in, size_t len, DWORD nStdHandle) {
		DWORD written = 0;
		if (len) {
			HANDLE hStdOut = ::GetStdHandle(nStdHandle);
			if (hStdOut != INVALID_HANDLE_VALUE && !::WriteConsoleW(hStdOut, in, len, &written, nullptr)) {
				::WriteFile(hStdOut, in, len * sizeof(*in), &written, nullptr);
				written /= sizeof(*in);
			}
		}
		return written;
	}

	int vsnprintf(PWSTR buf, size_t len, PCWSTR format, va_list vl) {
		buf[len - 1] = 0;
		return ::_vsnwprintf(buf, len - 1, format, vl);
	}

	int stdvprintf(DWORD nStdHandle, PCWSTR format, va_list vl) {
		auto_array<WCHAR> buf(64 * 1024);
		vsnprintf(buf.data(), buf.size(), format, vl);
		return consoleout(buf.data(), get_str_len(buf.data()), nStdHandle);
	}

	int printf(PCWSTR format, ...) {
		va_list vl;
		va_start(vl, format);
		int ret = stdvprintf(STD_OUTPUT_HANDLE, format, vl);
		va_end(vl);
		return ret;
	}

}
