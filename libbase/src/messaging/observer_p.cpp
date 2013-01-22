#include <libbase/messaging.hpp>

#include "MessageManager.hpp"

namespace Base {

	Observer_p::~Observer_p()
	{
		m_manager->unregister_all(this);
	}

}
