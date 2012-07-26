#ifndef _LIBBASE_COMMAND_P_HPP_
#define _LIBBASE_COMMAND_P_HPP_

#include <libbase/std.hpp>

namespace Base {

	///=================================================================================== Command_p
	struct Command_p {
		virtual ~Command_p();

		virtual size_t execute() = 0;
	};

	struct NullCommand: public Command_p {
		virtual size_t execute();
	};

}

#endif
