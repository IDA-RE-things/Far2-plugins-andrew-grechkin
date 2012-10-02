#include <libbase/observer_p.hpp>


namespace Base {

	Observer_p::~Observer_p() {
		m_manager->unregister_all(this);
	}

	Observer_p::Observer_p():
		m_manager(get_simple_change_manager())
	{
	}

}

