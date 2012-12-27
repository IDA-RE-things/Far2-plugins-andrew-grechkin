#include <libbase/command_p.hpp>

namespace Base {

	Command_p::~Command_p()
	{
	}

	ssize_t Command_p::execute()
	{
		return true;
	}

}
