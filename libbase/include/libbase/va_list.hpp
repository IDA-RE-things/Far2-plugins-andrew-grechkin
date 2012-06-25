#ifndef _LIBBASE_VALIST_HPP_
#define _LIBBASE_VALIST_HPP_

#include <libbase/std.hpp>

namespace Base {

	ustring as_str(PCWSTR format, ...);

	ustring as_str(PCWSTR format, va_list args);

}

#endif
