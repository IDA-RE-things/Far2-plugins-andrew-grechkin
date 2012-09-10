#include <libbase/observer_p.hpp>
#include <libbase/lock.hpp>

namespace Base {

	Observer_p::~Observer_p() {
		unsubscribe();
	}

	Observer_p::Observer_p():
		m_observable(nullptr)
	{
	}

	Observer_p::Observer_p(Observable_p * observer):
		m_observable(observer)
	{
		m_observable->register_observer(this);
	}

	void Observer_p::subscribe(Observable_p * observable) {
		unsubscribe();
		m_observable = observable;
		m_observable->register_observer(this);
	}

	void Observer_p::unsubscribe() {
		if (m_observable) {
			m_observable->unregister_observer(this);
			m_observable = nullptr;
		}
	}


	struct Observable_p::impl: private std::vector<Observer_p*> {
		~impl() {
			delete m_sync;
		}

		impl() :
			m_sync(Lock::get_CritSection()),
			m_changed(false)
		{
		}

		void add(Observer_p* observer) {
			auto lock(m_sync->get_lock());
			push_back(observer);
		}

		void erase(Observer_p* observer) {
			auto lock(m_sync->get_lock());
			iterator it = std::find(begin(), end(), observer);
			if (it != end())
				vector::erase(it);
		}

		void notify_all(void * data) {
			if (m_changed) {
				auto lock(m_sync->get_lock_read());
				if (m_changed) {
					iterator it_end = end();
					for (iterator it = begin(); it != it_end; ++it) {
						(*it)->notify(data);
					}
					m_changed = false;
				}
			}
		}

		void set_changed(bool changed) {
			m_changed = changed;
		}

		bool get_changed() const {
			return m_changed;
		}

	private:
		Lock::SyncUnit_i* m_sync;
		bool m_changed;
	};

	Observable_p::~Observable_p() {
	}

	Observable_p::Observable_p() :
		m_impl(new impl) {
	}

	void Observable_p::register_observer(Observer_p* observer) {
		m_impl->add(observer);
	}

	void Observable_p::unregister_observer(Observer_p* observer) {
		m_impl->erase(observer);
	}

	void Observable_p::notify_all(void* data) const {
		m_impl->notify_all(data);
	}

	void Observable_p::set_changed(bool changed) {
		m_impl->set_changed(changed);
	}

	bool Observable_p::get_changed() const {
		return m_impl->get_changed();
	}

}

