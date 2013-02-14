#ifndef _LIBBASE_CHAR_HPP_
#define _LIBBASE_CHAR_HPP_

#include <libbase/std.hpp>

namespace Base {

	namespace Inplace {
		wchar_t & to_upper(wchar_t & in);

		wchar_t & to_lower(wchar_t & in);
	}

	ssize_t get_type_of_char(wchar_t in);

	bool is_eol(wchar_t in);

	inline bool is_space(wchar_t in)
	{
		//	return in == L' ' || in == L'\t';
		return get_type_of_char(in) & C1_SPACE;
	}

	inline bool is_printable(wchar_t in)
	{
		return !(get_type_of_char(in) & C1_CNTRL);
	}

	inline bool is_control(wchar_t in)
	{
		//	return in == L' ' || in == L'\t';
		return get_type_of_char(in) & C1_CNTRL;
	}

	inline bool is_upper(wchar_t in)
	{
		//	return ::IsCharUpperW(in);
		return get_type_of_char(in) & C1_UPPER;
	}

	inline bool is_lower(wchar_t in)
	{
		//	return ::IsCharLowerW(in);
		return get_type_of_char(in) & C1_LOWER;
	}

	inline bool is_alpha(wchar_t in)
	{
		//	return ::IsCharAlphaW(in);
		return get_type_of_char(in) & C1_ALPHA;
	}

	inline bool is_alnum(wchar_t in)
	{
		//	return ::IsCharAlphaW(in);
		return get_type_of_char(in) & (C1_ALPHA | C1_DIGIT);
	}

	inline bool is_digit(wchar_t in)
	{
		//	return ::IsCharAlphaNumeric(in);
		return get_type_of_char(in) & C1_DIGIT;
	}

	inline bool is_xdigit(wchar_t in)
	{
		//	return ::IsCharAlphaNumeric(in);
		return get_type_of_char(in) & C1_XDIGIT;
	}

	inline bool is_punct(wchar_t in)
	{
		//	return ::IsCharAlphaNumeric(in);
		return get_type_of_char(in) & C1_PUNCT;
	}

	inline wchar_t to_upper(wchar_t in)
	{
		return Inplace::to_upper(in);
	}

	inline wchar_t to_lower(wchar_t in)
	{
		return Inplace::to_lower(in);
	}

}

#endif
