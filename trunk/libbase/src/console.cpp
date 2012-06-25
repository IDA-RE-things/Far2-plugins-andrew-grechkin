#include <libbase/console.hpp>

#include <stdio.h>

namespace Base {

	int fileout(HANDLE hndl, PCWSTR str, size_t len) {
		DWORD written = 0;
		if (hndl && hndl != INVALID_HANDLE_VALUE) {
			::WriteFile(hndl, str, len * sizeof(*str), &written, nullptr);
			written /= sizeof(*str);
		}
		return written;
	}

	int consoleout(PCSTR in, size_t len, DWORD nStdHandle) {
		DWORD written = 0;
		if (len) {
			HANDLE hStdOut = ::GetStdHandle(nStdHandle);
			if (hStdOut != INVALID_HANDLE_VALUE && !::WriteConsoleA(hStdOut, in, len, &written, nullptr)) {
				::WriteFile(hStdOut, in, len * sizeof(*in), &written, nullptr);
				written /= sizeof(*in);
			}
		}
		return written;
	}

	int consoleout(WCHAR in, DWORD nStdHandle) {
		WCHAR out[] = {in, STR_END};
		return consoleout(out, nStdHandle);
	}

	int consoleoutonly(PCWSTR in, size_t len) {
		DWORD written = 0;
		if (len) {
			HANDLE hStdOut = ::GetStdHandle(STD_OUTPUT_HANDLE);
			if (hStdOut != INVALID_HANDLE_VALUE)
				::WriteConsoleW(hStdOut, in, len, &written, nullptr);
		}
		return written;
	}

	int stdprintf(DWORD nStdHandle, PCWSTR format, ...) {
		va_list vl;
		va_start(vl, format);
		int Result = stdvprintf(nStdHandle, format, vl);
		va_end(vl);
		return Result;
	}

	int snprintf(PWSTR buff, size_t len, PCWSTR format, ...) {
		va_list vl;
		va_start(vl, format);
		int Result = vsnwprintf(buff, len, format, vl);
		va_end(vl);
		return Result;
	}

	void errx(int eval, PCSTR format, ...) {
		va_list vl;
		va_start(vl, format);
		::vprintf(format, vl);
		va_end(vl);
		exit(eval);
	}

#ifndef NDEBUG
	PCSTR FUNC_ENTER_FORMAT = "Enter >>>>>>>>    %s \t\t[%s:%d]\n";
	PCSTR FUNC_LEAVE_FORMAT = "Leave <<<<<<<<    %s\n";
	PCSTR FUNC_TRACE_FORMAT = "Trace =========== %s \t\t[%s:%d]\n";
#endif

}
