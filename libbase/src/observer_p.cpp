#include <libbase/observer_p.hpp>
#include <libbase/lock.hpp>

namespace Base {

	Observer_p::~Observer_p() {
		if (m_observable) {
			m_observable->unregister_observer(this);
			m_observable = nullptr;
		}
	}

	void Observer_p::notify(void * data) {
		notify_(data);
	}

	Observer_p::Observer_p(Observable_p* observer) :
		m_observable(observer) {
	}


	struct Observable_p::impl: private std::vector<Observer_p*> {
		~impl() {
			delete m_sync;
		}

		impl() :
			m_sync(Lock::get_CritSection()) {
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

		void notify(void* data) {
			auto lock(m_sync->get_lock_read());
			if (m_changed) {
				iterator it_end = end();
				for (iterator it = begin(); it != it_end; ++it) {
					(*it)->notify(data);
				}
				m_changed = false;
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
		delete m_impl;
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

	void Observable_p::notify(void* data) const {
		m_impl->notify(data);
	}

	void Observable_p::set_changed(bool changed) {
		m_impl->set_changed(changed);
	}

	bool Observable_p::get_changed() const {
		return m_impl->get_changed();
	}

}

