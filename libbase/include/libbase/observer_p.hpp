#ifndef _LIBBASE_OBSERVER_P_HPP_
#define _LIBBASE_OBSERVER_P_HPP_

#include <stdint.h>


namespace Base {

	struct Observer_p;
	struct Observable_p;


	///======================================================================================= Event
	struct Event {
		Event();

		size_t type;
		ssize_t code;
		void * userData;
	};


	///=============================================================================== ChangeManager
	struct ChangeManager {
		virtual ~ChangeManager();

		virtual void register_observer(Observable_p * subject, Observer_p * observer) = 0;

		virtual void unregister_observer(Observable_p * subject, Observer_p * observer) = 0;

		virtual void unregister_all(Observable_p * subject) = 0;

		virtual void unregister_all(Observer_p * observer) = 0;

		virtual void notify(const Observable_p * subject, const Event & event) const = 0;
	};


	ChangeManager * get_simple_change_manager();


	///================================================================================== Observer_p
	struct Observer_p {
		virtual ~Observer_p();

		virtual void notify(const Event & event) = 0;

	public:
		Observer_p();

		Observer_p(ChangeManager * manager):
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

		Observable_p(ChangeManager * manager):
			m_manager(manager),
			m_changed(false)
		{
		}

		void register_observer(Observer_p * observer);

		void unregister_observer(Observer_p * observer);

		void notify_all(const Event & event) const;

		void set_changed(bool changed) const;

		bool get_changed() const;

	private:
		ChangeManager * m_manager;
		mutable bool m_changed;
	};

}


#endif
