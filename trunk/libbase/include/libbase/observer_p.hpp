#ifndef _LIBBASE_OBSERVER_P_HPP_
#define _LIBBASE_OBSERVER_P_HPP_

#include <stdint.h>

namespace Base {

	struct ChangeManager;

	///======================================================================================= Event
	struct Event {
		virtual ~Event();

		virtual ssize_t get_type() const;

		virtual ssize_t get_code() const;

		virtual void * get_data() const;
	};

	///================================================================================== Observer_p
	struct Observer_p {
		virtual ~Observer_p();

		virtual void notify(Event const& event) = 0;

	public:
		Observer_p();

		Observer_p(ChangeManager * manager) :
			m_manager(manager)
		{
		}

	private:
		ChangeManager * m_manager;
	};

	///================================================================================ Observable_p
	struct Observable_p {
		virtual ~Observable_p();

	public:
		Observable_p();

		Observable_p(ChangeManager * manager) :
			m_manager(manager),
			m_changed(false)
		{
		}

		void register_observer(Observer_p * observer);

		void unregister_observer(Observer_p * observer);

		void notify_all(Event const& event) const;

		void set_changed(bool changed) const;

		bool get_changed() const;

	private:
		ChangeManager * m_manager;
		mutable bool m_changed;
	};

}

#endif
