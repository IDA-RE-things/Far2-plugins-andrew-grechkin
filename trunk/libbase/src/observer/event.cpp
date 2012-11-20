#include <libbase/observer_p.hpp>

namespace Base {

	Event::~Event()
	{
	}

	ssize_t Event::get_type() const
	{
		return 0;
	}

	ssize_t Event::get_code() const
	{
		return 0;
	}

	void * Event::get_data() const
	{
		return nullptr;
	}

}
