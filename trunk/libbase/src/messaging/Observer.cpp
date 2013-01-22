#include <libbase/messaging.hpp>

#include "MessageManager.hpp"

namespace Base {

	Observer::~Observer()
	{
		m_manager->unregister_all(this);
	}

}
