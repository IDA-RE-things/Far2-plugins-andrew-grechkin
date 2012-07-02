#ifndef _LIBEXT_COMMAND_P_HPP_
#define _LIBEXT_COMMAND_P_HPP_

#include <libbase/std.hpp>

namespace Ext {

	///===================================================================================== Command
	struct Command_p {
		virtual ~Command_p();

		virtual bool execute() const = 0;
	};

	struct NullCommand: public Command_p {
		virtual bool execute() const;
	};

}

#endif
