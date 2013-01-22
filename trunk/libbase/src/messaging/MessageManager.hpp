#ifndef _LIBBASE_MESSAGEMANAGER_PVT_HPP_
#define _LIBBASE_MESSAGEMANAGER_PVT_HPP_

#include <stdint.h>

namespace Base {

	struct Message;
	struct Observer_p;
	struct Observable_p;

	struct MessageManager {
		virtual ~MessageManager();

		virtual void register_observer(Observable_p * subject, Observer_p * observer) = 0;

		virtual void unregister_observer(Observable_p * subject, Observer_p * observer) = 0;

		virtual void unregister_all(Observable_p * subject) = 0;

		virtual void unregister_all(Observer_p * observer) = 0;

		virtual void notify(const Observable_p * subject, const Message & event) const = 0;
	};

}

#endif
