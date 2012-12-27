#ifndef _LIBBASE_CHAR_HPP_
#define _LIBBASE_CHAR_HPP_

#include <libbase/std.hpp>

namespace Base {

	ssize_t get_type_of_char(WCHAR in);

	bool is_eol(WCHAR in);

	bool is_space(WCHAR in);

	bool is_printable(WCHAR in);

	bool is_control(WCHAR in);

	bool is_upper(WCHAR in);

	bool is_lower(WCHAR in);

	bool is_alpha(WCHAR in);

	bool is_alnum(WCHAR in);

	bool is_digit(WCHAR in);

	bool is_xdigit(WCHAR in);

	bool is_punct(WCHAR in);

	WCHAR to_upper(WCHAR in);

	WCHAR to_lower(WCHAR in);

	namespace Inplace
	{
		WCHAR & to_upper(WCHAR & in);

		WCHAR & to_lower(WCHAR & in);
	}

}

#endif
