#include <libbase/std.hpp>
#include <libbase/char.hpp>

namespace Base {

	namespace Inplace {

		wchar_t & to_upper(wchar_t & in)
		{
			::CharUpperBuffW(&in, 1);
			return in;
		}

		wchar_t & to_lower(wchar_t & in)
		{
			::CharLowerBuffW(&in, 1);
			return in;
		}

	}

	ssize_t get_type_of_char(wchar_t in)
	{
		WORD Result[2] = {0};
		::GetStringTypeW(CT_CTYPE1, &in, 1, Result);
		return Result[0];
	}

	bool is_eol(wchar_t in)
	{
		return in == L'\r' || in == L'\n';
	}

}
