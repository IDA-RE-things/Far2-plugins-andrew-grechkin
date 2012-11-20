#ifndef _LIBBASE_CHANNELMANAGER_PVT_HPP_
#define _LIBBASE_CHANNELMANAGER_PVT_HPP_

#include <stdint.h>

namespace Base {

	struct Observer_p;
	struct Observable_p;

	struct ChangeManager {
		virtual ~ChangeManager();

		virtual void register_observer(Observable_p * subject, Observer_p * observer) = 0;

		virtual void unregister_observer(Observable_p * subject, Observer_p * observer) = 0;

		virtual void unregister_all(Observable_p * subject) = 0;

		virtual void unregister_all(Observer_p * observer) = 0;

		virtual void notify(const Observable_p * subject, const Event & event) const = 0;
	};

	ChangeManager * get_simple_change_manager();

}

#endif
