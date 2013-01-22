#ifndef _LIBBASE_COMMAND_P_HPP_
#define _LIBBASE_COMMAND_P_HPP_

#include <libbase/std.hpp>

namespace Base {

	struct Command_p {
		virtual ~Command_p();

		virtual ssize_t execute();
	};

}

#endif
