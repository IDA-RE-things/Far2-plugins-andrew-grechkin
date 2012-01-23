#include "console.h"

#include <stdio.h>

int	consoleout(PCSTR in, size_t len, DWORD nStdHandle) {
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

int	consoleout(WCHAR in, DWORD nStdHandle) {
	WCHAR out[] = {in, STR_END};
	return consoleout(out, nStdHandle);
}

int	consoleoutonly(PCWSTR in, size_t len) {
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
	vprintf(format, vl);
	va_end(vl);
	exit(eval);
}

///========================================================================================= Logging
int logLevel = LOG_INFO;

void setLogLevel(WinLogLevel lvl) {
	logLevel = lvl;
}

void logError(PCWSTR format, ...) {
	ConsoleColor col(FOREGROUND_INTENSITY | FOREGROUND_RED);
	va_list vl;
	va_start(vl, format);
	stdprintf(STD_ERROR_HANDLE, L"ERROR: ");
	stdvprintf(STD_ERROR_HANDLE, format, vl);
	va_end(vl);
}

void logError(DWORD errNumber, PCWSTR format, ...) {
	ConsoleColor col(FOREGROUND_INTENSITY | FOREGROUND_RED);
	va_list vl;
	va_start(vl, format);
	stdprintf(STD_ERROR_HANDLE, L"ERROR [%i]: %s\n\t", errNumber, ErrAsStr(errNumber).c_str());
	stdvprintf(STD_ERROR_HANDLE, format, vl);
	va_end(vl);
}

void logDebug(PCWSTR format, ...) {
	if (logLevel <= LOG_DEBUG) {
		va_list vl;
		va_start(vl, format);
		vprintf(format, vl);
		va_end(vl);
	}
}

void logInfo(PCWSTR format, ...) {
	if (logLevel <= LOG_INFO) {
		va_list vl;
		va_start(vl, format);
		vprintf(format, vl);
		va_end(vl);
	}
}

void logVerbose(PCWSTR format, ...) {
	if (logLevel <= LOG_VERBOSE) {
		va_list vl;
		va_start(vl, format);
		vprintf(format, vl);
		va_end(vl);
	}
}

void logCounter(PCWSTR format, ...) {
	if (logLevel >= LOG_VERBOSE && logLevel < LOG_ERROR) {
		if (consoleoutonly(L"\r")) {
			va_list vl;
			va_start(vl, format);
			WCHAR buff[8 * 1024];
			vsnprintf(buff, sizeofa(buff), format, vl);
			consoleoutonly(buff);
			consoleoutonly(L"\r");
			va_end(vl);
		}
	}
}

void logFile(WIN32_FIND_DATA info) {
	uint64_t size = HighLow64(info.nFileSizeHigh, info.nFileSizeLow);
	logDebug(L"%s   found: \"%s\" (Size=%I64i,%s%s%s%s%s%s%s%s%s%s%s)\n", FILE_ATTRIBUTE_DIRECTORY
	    & info.dwFileAttributes ? L"Dir " : L"File", info.cFileName, size, FILE_ATTRIBUTE_ARCHIVE
	    & info.dwFileAttributes ? L"ARCHIVE " : L"", FILE_ATTRIBUTE_COMPRESSED
	    & info.dwFileAttributes ? L"COMPRESSED " : L"", FILE_ATTRIBUTE_ENCRYPTED
	    & info.dwFileAttributes ? L"ENCRYPTED " : L"", FILE_ATTRIBUTE_HIDDEN
	    & info.dwFileAttributes ? L"HIDDEN " : L"",
	         FILE_ATTRIBUTE_NORMAL & info.dwFileAttributes ? L"NORMAL " : L"",
	         FILE_ATTRIBUTE_OFFLINE & info.dwFileAttributes ? L"OFFLINE " : L"",
	         FILE_ATTRIBUTE_READONLY & info.dwFileAttributes ? L"READONLY " : L"",
	         FILE_ATTRIBUTE_REPARSE_POINT & info.dwFileAttributes ? L"REPARSE_POINT " : L"",
	         FILE_ATTRIBUTE_SPARSE_FILE & info.dwFileAttributes ? L"SPARSE " : L"",
	         FILE_ATTRIBUTE_SYSTEM & info.dwFileAttributes ? L"SYSTEM " : L"",
	         FILE_ATTRIBUTE_TEMPORARY & info.dwFileAttributes ? L"TEMP " : L"");
}


#ifndef NDEBUG
	PCSTR FUNC_ENTER_FORMAT = "Enter >>>>>>>>    %s \t\t[%s:%d]\n";
	PCSTR FUNC_LEAVE_FORMAT = "Leave <<<<<<<<    %s\n";
	PCSTR FUNC_TRACE_FORMAT = "Trace =========== %s \t\t[%s:%d]\n";
#endif
