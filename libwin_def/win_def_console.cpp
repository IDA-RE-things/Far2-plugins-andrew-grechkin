#include "win_def.h"

int			logLevel = LOG_INFO;

int				consoleout(PCWSTR in, DWORD nStdHandle) {
	HANDLE	hStdOut = ::GetStdHandle(nStdHandle);
	if (hStdOut != INVALID_HANDLE_VALUE) {
		DWORD	lpNumberOfCharsWritten;
		::WriteConsoleW(hStdOut, in, (DWORD)Len(in), &lpNumberOfCharsWritten, NULL);
		return	lpNumberOfCharsWritten;
	}
	return	0;
}
int				consoleout(const CStrW &in, DWORD nStdHandle) {
	HANDLE	hStdOut = ::GetStdHandle(nStdHandle);
	if (hStdOut != INVALID_HANDLE_VALUE) {
		DWORD	lpNumberOfCharsWritten;
		::WriteConsoleW(hStdOut, in.c_str(), (DWORD)in.capacity(), &lpNumberOfCharsWritten, NULL);
		return	lpNumberOfCharsWritten;
	}
	return	0;
}
int				printf(PCWSTR format, ...) {
	WCHAR	buff[8*1024];
	{
		va_list	vl;
		va_start(vl, format);
		_vsnwprintf(buff, sizeofa(buff), format, vl);
		va_end(vl);
	}
	return	consoleout(buff, STD_OUTPUT_HANDLE);
}

///========================================================================================= Logging
void			setLogLevel(LogLevel lvl) {
	logLevel = lvl;
}
void			logDebug(PCWSTR message, ...) {
	if (logLevel <= LOG_DEBUG) {
		va_list argp;
		wprintf(L"    ");
		va_start(argp, message);
		vwprintf(message, argp);
		va_end(argp);
		wprintf(L"\n");
	}
}
void			logError(PCWSTR format, ...) {
	WCHAR	buff[8*1024];
	consoleout(L"ERROR: ", STD_ERROR_HANDLE);
	{
		va_list	vl;
		va_start(vl, format);
		_vsnwprintf(buff, sizeofa(buff), format, vl);
		va_end(vl);
	}
	consoleout(buff, STD_ERROR_HANDLE);
	consoleout(L"\n", STD_ERROR_HANDLE);
}

void			logInfo(PCWSTR message, ...) {
	if (logLevel <= LOG_INFO) {
		va_list argp;
		va_start(argp, message);
		vwprintf(message, argp);
		va_end(argp);
		wprintf(L"\n");
	}
}
void			logCounter(PCWSTR message, ...) {
	if (logLevel <= LOG_INFO) {
		va_list argp;
		va_start(argp, message);
		vwprintf(message, argp);
		va_end(argp);
		wprintf(L"\r");
	}
}
void			logVerbose(PCWSTR message, ...) {
	if (logLevel <= LOG_VERBOSE) {
		va_list argp;
		wprintf(L"  ");
		va_start(argp, message);
		vwprintf(message, argp);
		va_end(argp);
		wprintf(L"\n");
	}
}
void			logFile(WIN32_FIND_DATA info) {
	uint64_t	size = MyUI64(info.nFileSizeLow, info.nFileSizeHigh);
	logDebug(L"%s   found: \"%s\" (Size=%I64i,%s%s%s%s%s%s%s%s%s%s%s)",
			 FILE_ATTRIBUTE_DIRECTORY    &info.dwFileAttributes ? L"Dir " : L"File",
			 info.cFileName,
			 size,
			 FILE_ATTRIBUTE_ARCHIVE      &info.dwFileAttributes ? L"ARCHIVE " : L"",
			 FILE_ATTRIBUTE_COMPRESSED   &info.dwFileAttributes ? L"COMPRESSED " : L"",
			 FILE_ATTRIBUTE_ENCRYPTED    &info.dwFileAttributes ? L"ENCRYPTED " : L"",
			 FILE_ATTRIBUTE_HIDDEN       &info.dwFileAttributes ? L"HIDDEN " : L"",
			 FILE_ATTRIBUTE_NORMAL       &info.dwFileAttributes ? L"NORMAL " : L"",
			 FILE_ATTRIBUTE_OFFLINE      &info.dwFileAttributes ? L"OFFLINE " : L"",
			 FILE_ATTRIBUTE_READONLY     &info.dwFileAttributes ? L"READONLY " : L"",
			 FILE_ATTRIBUTE_REPARSE_POINT&info.dwFileAttributes ? L"REPARSE_POINT " : L"",
			 FILE_ATTRIBUTE_SPARSE_FILE  &info.dwFileAttributes ? L"SPARSE " : L"",
			 FILE_ATTRIBUTE_SYSTEM       &info.dwFileAttributes ? L"SYSTEM " : L"",
			 FILE_ATTRIBUTE_TEMPORARY    &info.dwFileAttributes ? L"TEMP " : L"");
}
