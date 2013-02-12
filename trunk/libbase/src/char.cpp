#include <libbase/std.hpp>
#include <libbase/char.hpp>

namespace Base {

	ssize_t get_type_of_char(wchar_t in) {
		WORD Result[2] = {0};
		::GetStringTypeW(CT_CTYPE1, &in, 1, Result);
		return Result[0];
	}

	bool is_eol(wchar_t in) {
		return in == L'\r' || in == L'\n';
	}

	bool is_space(wchar_t in) {
		//	return in == L' ' || in == L'\t';
		return get_type_of_char(in) & C1_SPACE;
	}

	bool is_printable(wchar_t in) {
		return !(get_type_of_char(in) & C1_CNTRL);
	}

	bool is_control(wchar_t in) {
		//	return in == L' ' || in == L'\t';
		return get_type_of_char(in) & C1_CNTRL;
	}

	bool is_upper(wchar_t in) {
		//	return ::IsCharUpperW(in);
		return get_type_of_char(in) & C1_UPPER;
	}

	bool is_lower(wchar_t in) {
		//	return ::IsCharLowerW(in);
		return get_type_of_char(in) & C1_LOWER;
	}

	bool is_alpha(wchar_t in) {
		//	return ::IsCharAlphaW(in);
		return get_type_of_char(in) & C1_ALPHA;
	}

	bool is_alnum(wchar_t in) {
		//	return ::IsCharAlphaW(in);
		return get_type_of_char(in) & (C1_ALPHA | C1_DIGIT);
	}

	bool is_digit(wchar_t in) {
		//	return ::IsCharAlphaNumeric(in);
		return get_type_of_char(in) & C1_DIGIT;
	}

	bool is_xdigit(wchar_t in) {
		//	return ::IsCharAlphaNumeric(in);
		return get_type_of_char(in) & C1_XDIGIT;
	}

	bool is_punct(wchar_t in) {
		//	return ::IsCharAlphaNumeric(in);
		return get_type_of_char(in) & C1_PUNCT;
	}

	wchar_t to_upper(wchar_t in) {
		return Inplace::to_upper(in);
	}

	wchar_t to_lower(wchar_t in) {
		return Inplace::to_lower(in);
	}

	namespace Inplace
	{
		wchar_t & to_upper(wchar_t & in) {
			::CharUpperBuffW(&in, 1);
			return in;
		}

		wchar_t & to_lower(wchar_t & in) {
			::CharLowerBuffW(&in, 1);
			return in;
		}
	}

}
