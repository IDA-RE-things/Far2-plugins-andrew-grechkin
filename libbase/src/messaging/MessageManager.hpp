#ifndef _LIBBASE_MESSAGEMANAGER_PVT_HPP_
#define _LIBBASE_MESSAGEMANAGER_PVT_HPP_

#include <stdint.h>

namespace Base {

	struct Message;
	struct Observer;
	struct Observable;

	struct MessageManager {
		virtual ~MessageManager();

		virtual void register_observer(Observable * subject, Observer * observer) = 0;

		virtual void unregister_observer(Observable * subject, Observer * observer) = 0;

		virtual void unregister_all(Observable * subject) = 0;

		virtual void unregister_all(Observer * observer) = 0;

		virtual void notify(const Observable * subject, const Message & event) const = 0;
	};

}

#endif
