#ifndef _LIBBASE_CONSOLE_HPP_
#define _LIBBASE_CONSOLE_HPP_

#include <libbase/std.hpp>
#include <libbase/str.hpp>

//#include <stdio.h>

namespace Base {

	int fileout(HANDLE hndl, PCWSTR str, size_t len);

	int consoleout(PCSTR in, size_t len, DWORD nStdHandle = STD_OUTPUT_HANDLE);

	int consoleout(PCWSTR in, size_t len, DWORD nStdHandle = STD_OUTPUT_HANDLE);

	int consoleout(WCHAR in, DWORD nStdHandle = STD_OUTPUT_HANDLE);

	int consoleoutonly(PCWSTR in, size_t len);

	inline int consoleout(PCWSTR in, DWORD nStdHandle = STD_OUTPUT_HANDLE) {
		return consoleout(in, Str::length(in), nStdHandle);
	}

	inline int consoleout(const ustring &in, DWORD nStdHandle = STD_OUTPUT_HANDLE/*STD_ERROR_HANDLE*/) {
		return consoleout(in.c_str(), nStdHandle);
	}

	inline int consoleoutonly(PCWSTR in) {
		return consoleoutonly(in, Str::length(in));
	}

	///================================================================================= Console out
	int vsnprintf(PWSTR buf, size_t len, PCWSTR format, va_list vl);

	int stdvprintf(DWORD nStdHandle, PCWSTR format, va_list vl);

	int stdprintf(DWORD nStdHandle, PCWSTR format, ...);

	int printf(PCWSTR format, ...);

	inline int vprintf(PCWSTR format, va_list vl) {
		return stdvprintf(STD_OUTPUT_HANDLE, format, vl);
	}

	int snprintf(PWSTR buff, size_t len, PCWSTR format, ...);

	void errx(int eval, PCSTR format, ...);

	///================================================================================ ConsoleColor
	struct ConsoleColor {
		~ConsoleColor();

		ConsoleColor(WORD color);

		void restore();

	private:
		bool save();

		WORD m_color;
	};

#ifndef NDEBUG
	extern PCSTR FUNC_ENTER_FORMAT;
	extern PCSTR FUNC_LEAVE_FORMAT;
	extern PCSTR FUNC_TRACE_FORMAT;

#define FuncLogger() struct FL_struc__ { \
	FL_struc__(const char * fl, int l, const char * f):_fn(f) {printf(FUNC_ENTER_FORMAT, _fn, fl, l);} \
	~FL_struc__() {printf(FUNC_LEAVE_FORMAT, _fn);} \
	const char * _fn; \
} tmp_struct(THIS_FILE, __LINE__, __PRETTY_FUNCTION__);

#define FuncTrace() printf(FUNC_TRACE_FORMAT, __PRETTY_FUNCTION__, THIS_FILE, __LINE__);

#else

#define FuncLogger()
#define FuncTrace()

#endif

}

#endif
