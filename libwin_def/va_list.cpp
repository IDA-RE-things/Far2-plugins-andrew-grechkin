#include "va_list.h"

#include <stdio.h>

namespace windef {
	namespace {
		const size_t default_buffer_size = 4 * 1024;
	}

	ustring vargs_as_str(PCWSTR format, ...) {
		va_list args;
		va_start(args, format);
		return va_list_as_str(format, args);
	}

	ustring va_list_as_str(PCWSTR format, va_list args) {
		WCHAR buf[default_buffer_size];
		size_t size = lengthof(buf) - 1;
		buf[size] = L'\0';
		::_vsnwprintf(buf, size, format, args);
		return ustring(buf);
	}
}
