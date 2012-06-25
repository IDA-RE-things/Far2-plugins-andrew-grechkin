#ifndef _LIBBASE_WIN64_HPP_
#define _LIBBASE_WIN64_HPP_

#include <libbase/std.hpp>

namespace Base {

	bool is_WOW64();

	bool disable_WOW64(PVOID & oldValue);

	bool enable_WOW64(PVOID & oldValue);

}

#endif
