#include <libbase/std.hpp>
#include <libbase/err.hpp>
#include <libbase/console.hpp>
#include <libext/exception.hpp>

#include <stdio.h>

using namespace Base;

namespace Ext {

	PCSTR const THROW_PLACE_FORMAT = "%s: %d [%s]";

#ifdef NDEBUG
#else
	ustring ThrowPlaceString(PCSTR file, int line, PCSTR func) {
		CHAR buf[MAX_PATH];
		buf[MAX_PATH-1] = 0;
		::snprintf(buf, sizeofa(buf) - 1, THROW_PLACE_FORMAT, file, line, func);
		return cp2w(buf, CP_UTF8);
	}
#endif

	///=============================================================================== AbstractError
	AbstractError::~AbstractError() {
	}

}
