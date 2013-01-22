#include <libbase/messaging.hpp>

#include "MessageManager.hpp"

namespace Base {

	Observable::~Observable()
	{
		m_manager->unregister_all(this);
	}

	void Observable::register_observer(Observer * observer)
	{
		m_manager->register_observer(this, observer);
	}

	void Observable::unregister_observer(Observer * observer)
	{
		m_manager->unregister_observer(this, observer);
	}

	void Observable::notify_all(const Message & event) const
	{
		if (m_changed) {
			m_manager->notify(this, event);
			m_changed = false;
		}
	}

	void Observable::set_changed(bool changed) const
	{
		m_changed = changed;
	}

	bool Observable::get_changed() const
	{
		return m_changed;
	}

}
