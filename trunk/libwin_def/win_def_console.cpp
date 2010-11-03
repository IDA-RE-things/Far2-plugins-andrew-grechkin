#include "win_def.h"

int consoleout(PCSTR in, DWORD nStdHandle) {
	HANDLE hStdOut = ::GetStdHandle(nStdHandle);
	if (hStdOut && hStdOut != INVALID_HANDLE_VALUE) {
		DWORD written = 0;
		DWORD len = Len(in);
		if (len && !::WriteConsoleA(hStdOut, in, len, &written, nullptr)) {
			::WriteFile(hStdOut, in, len * sizeof(*in), &written, nullptr);
			written /= sizeof(*in);
		}
		return written;
	}
	return 0;
}

int consoleoutonly(PCWSTR in) {
	HANDLE hStdOut = ::GetStdHandle(STD_OUTPUT_HANDLE);
	if (hStdOut && hStdOut != INVALID_HANDLE_VALUE) {
		DWORD written = 0;
		DWORD len = Len(in);
		len && ::WriteConsoleW(hStdOut, in, len, &written, nullptr);
		return written;
	}
	return 0;
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
	uint64_t size = MyUI64(info.nFileSizeLow, info.nFileSizeHigh);
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
