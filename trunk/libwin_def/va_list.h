#ifndef _WIN_DEF_VALIST_HPP
#define _WIN_DEF_VALIST_HPP

#include "str.h"

#include <stdarg.h>

namespace windef {
	ustring vargs_as_str(PCWSTR format, ...);

	ustring va_list_as_str(PCWSTR format, va_list args);
}

#endif
