#include "win_def.h"

bool			consoleout(PCWSTR in, DWORD nStdHandle) {
	HANDLE	hStdOut = ::GetStdHandle(nStdHandle);
	if (hStdOut != INVALID_HANDLE_VALUE) {
		DWORD	lpNumberOfCharsWritten;
		return	::WriteConsoleW(hStdOut, in, (DWORD)Len(in), &lpNumberOfCharsWritten, NULL) != 0;
	}
	return	false;
}
bool			consoleout(const CStrW &in, DWORD nStdHandle) {
	HANDLE	hStdOut = ::GetStdHandle(nStdHandle);
	if (hStdOut != INVALID_HANDLE_VALUE) {
		DWORD	lpNumberOfCharsWritten;
		return	::WriteConsoleW(hStdOut, in.c_str(), (DWORD)in.capacity(), &lpNumberOfCharsWritten, NULL) != 0;
	}
	return	false;
}
