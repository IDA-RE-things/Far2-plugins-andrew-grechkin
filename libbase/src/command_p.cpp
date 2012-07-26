#include <libbase/command_p.hpp>

namespace Base {

	Command_p::~Command_p() {
	}


	size_t NullCommand::execute() {
		return true;
	}

}
