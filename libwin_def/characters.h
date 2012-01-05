#ifndef _WIN_DEF_CHAR_HPP
#define _WIN_DEF_CHAR_HPP

#include "std.h"
#include "bit.h"

///====================================================================== Функции работы с символами
inline int GetType(WCHAR in) {
	WORD Result[2] = {0};
	::GetStringTypeW(CT_CTYPE1, &in, 1, Result);
	return Result[0];
}

inline bool IsEol(WCHAR in) {
	return in == L'\r' || in == L'\n';
}

inline bool IsSpace(WCHAR in) {
	//	return in == L' ' || in == L'\t';
	return WinFlag::Check(GetType(in), C1_SPACE);
}

inline bool IsPrint(WCHAR in) {
	return !WinFlag::Check(GetType(in), C1_CNTRL);
}

inline bool IsCntrl(WCHAR in) {
	//	return in == L' ' || in == L'\t';
	return WinFlag::Check(GetType(in), C1_CNTRL);
}

inline bool IsUpper(WCHAR in) {
	//	return ::IsCharUpperW(in);
	return WinFlag::Check(GetType(in), C1_UPPER);
}

inline bool IsLower(WCHAR in) {
	//	return ::IsCharLowerW(in);
	return WinFlag::Check(GetType(in), C1_LOWER);
}

inline bool IsAlpha(WCHAR in) {
	//	return ::IsCharAlphaW(in);
	return WinFlag::Check(GetType(in), C1_ALPHA);
}

inline bool IsAlNum(WCHAR in) {
	//	return ::IsCharAlphaW(in);
	int Result = GetType(in);
	return WinFlag::Check(Result, C1_ALPHA) || WinFlag::Check(Result, C1_DIGIT);
}

inline bool IsDigit(WCHAR in) {
	//	return ::IsCharAlphaNumeric(in);
	return WinFlag::Check(GetType(in), C1_DIGIT);
}

inline bool IsXDigit(WCHAR in) {
	//	return ::IsCharAlphaNumeric(in);
	return WinFlag::Check(GetType(in), C1_XDIGIT);
}

inline bool IsPunct(WCHAR in) {
	//	return ::IsCharAlphaNumeric(in);
	return WinFlag::Check(GetType(in), C1_PUNCT);
}

inline WCHAR ToUpper(WCHAR in) {
	::CharUpperBuffW(&in, 1);
	return in;
}

inline WCHAR ToLower(WCHAR in) {
	::CharLowerBuffW(&in, 1);
	return in;
}

#endif
