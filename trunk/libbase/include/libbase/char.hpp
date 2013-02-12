#ifndef _LIBBASE_CHAR_HPP_
#define _LIBBASE_CHAR_HPP_

#include <libbase/std.hpp>

namespace Base {

	ssize_t get_type_of_char(wchar_t in);

	bool is_eol(wchar_t in);

	bool is_space(wchar_t in);

	bool is_printable(wchar_t in);

	bool is_control(wchar_t in);

	bool is_upper(wchar_t in);

	bool is_lower(wchar_t in);

	bool is_alpha(wchar_t in);

	bool is_alnum(wchar_t in);

	bool is_digit(wchar_t in);

	bool is_xdigit(wchar_t in);

	bool is_punct(wchar_t in);

	wchar_t to_upper(wchar_t in);

	wchar_t to_lower(wchar_t in);

	namespace Inplace
	{
		wchar_t & to_upper(wchar_t & in);

		wchar_t & to_lower(wchar_t & in);
	}

}

#endif
