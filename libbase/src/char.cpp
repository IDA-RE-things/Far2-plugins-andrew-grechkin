#ifndef _WIN_DEF_CHAR_HPP
#define _WIN_DEF_CHAR_HPP

#include <libbase/std.hpp>

namespace Base {

	int get_type_of_char(WCHAR in) {
		WORD Result[2] = {0};
		::GetStringTypeW(CT_CTYPE1, &in, 1, Result);
		return Result[0];
	}

	bool is_eol(WCHAR in) {
		return in == L'\r' || in == L'\n';
	}

	bool is_space(WCHAR in) {
		//	return in == L' ' || in == L'\t';
		return get_type_of_char(in) & C1_SPACE;
	}

	bool is_printable(WCHAR in) {
		return !(get_type_of_char(in) & C1_CNTRL);
	}

	bool is_control(WCHAR in) {
		//	return in == L' ' || in == L'\t';
		return get_type_of_char(in) & C1_CNTRL;
	}

	bool is_upper(WCHAR in) {
		//	return ::IsCharUpperW(in);
		return get_type_of_char(in) & C1_UPPER;
	}

	bool is_lower(WCHAR in) {
		//	return ::IsCharLowerW(in);
		return get_type_of_char(in) & C1_LOWER;
	}

	bool is_alpha(WCHAR in) {
		//	return ::IsCharAlphaW(in);
		return get_type_of_char(in) & C1_ALPHA;
	}

	bool is_alnum(WCHAR in) {
		//	return ::IsCharAlphaW(in);
		int Result = get_type_of_char(in);
		return (Result & C1_ALPHA) || (Result & C1_DIGIT);
	}

	bool is_digit(WCHAR in) {
		//	return ::IsCharAlphaNumeric(in);
		return get_type_of_char(in) & C1_DIGIT;
	}

	bool is_xdigit(WCHAR in) {
		//	return ::IsCharAlphaNumeric(in);
		return get_type_of_char(in) & C1_XDIGIT;
	}

	bool is_punct(WCHAR in) {
		//	return ::IsCharAlphaNumeric(in);
		return get_type_of_char(in) & C1_PUNCT;
	}

	WCHAR to_upper(WCHAR in) {
		::CharUpperBuffW(&in, 1);
		return in;
	}

	WCHAR to_lower(WCHAR in) {
		::CharLowerBuffW(&in, 1);
		return in;
	}

}

#endif
