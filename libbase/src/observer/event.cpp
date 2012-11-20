#include <libbase/message.hpp>

namespace Base {

	Message::~Message()
	{
	}

	ssize_t Message::get_type() const
	{
		return 0;
	}

	ssize_t Message::get_code() const
	{
		return 0;
	}

	ssize_t Message::get_param() const
	{
		return 0;
	}

	void * Message::get_data() const
	{
		return nullptr;
	}

}
