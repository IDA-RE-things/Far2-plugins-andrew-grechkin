#include <libext/command_p.hpp>

namespace Ext {

	Command_p::~Command_p() {
	}


	bool NullCommand::execute() const {
		return true;
	}

}
