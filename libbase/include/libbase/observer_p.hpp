#ifndef _LIBBASE_OBSERVER_P_HPP_
#define _LIBBASE_OBSERVER_P_HPP_


#include <libbase/std.hpp>

namespace Base {

	struct Observable_p;

	///================================================================================== Observer_p
	struct Observer_p {
		virtual ~Observer_p();

		void notify(void * data = nullptr);

		Observer_p(Observable_p * observable = nullptr);

	private:
		virtual void notify_(void * data) = 0;

		Observable_p * m_observable;
	};

	///================================================================================ Observable_p
	struct Observable_p {
		virtual ~Observable_p();

		Observable_p();

		void register_observer(Observer_p * observer);

		void unregister_observer(Observer_p * observer);

		void notify(void * data = nullptr) const;

		void set_changed(bool changed);

		bool get_changed() const;

	private:
		struct impl;
		impl * m_impl;
	};

}


#endif
