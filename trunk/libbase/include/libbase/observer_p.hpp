#ifndef _LIBBASE_OBSERVER_P_HPP_
#define _LIBBASE_OBSERVER_P_HPP_


#include <libbase/std.hpp>
#include <libbase/shared_ptr.hpp>

namespace Base {

	struct Observable_p;


	///================================================================================== Observer_p
	struct Observer_p {
		virtual ~Observer_p();

		virtual void notify(void * data) = 0;

		Observer_p();

		Observer_p(Observable_p * observable);

		void subscribe(Observable_p * observable);

		void unsubscribe();

	private:
		Observable_p * m_observable;
	};


	///================================================================================ Observable_p
	struct Observable_p {
		virtual ~Observable_p();

		Observable_p();

		void register_observer(Observer_p * observer);

		void unregister_observer(Observer_p * observer);

		void notify_all(void * data = nullptr) const;

		void set_changed(bool changed);

		bool get_changed() const;

	private:
		struct impl;
		Base::shared_ptr<impl> m_impl;
	};

}


#endif
