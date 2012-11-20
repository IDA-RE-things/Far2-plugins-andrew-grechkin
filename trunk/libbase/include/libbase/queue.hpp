#ifndef _LIBBASE_QUEUE_HPP_
#define _LIBBASE_QUEUE_HPP_

#include <stdint.h>
#include <libbase/uncopyable.hpp>

namespace Base {

	struct Message;
	struct Queue_impl;

	struct Queue: private Base::Uncopyable {

		static const size_t WAIT_FOREVER;

		~Queue();

		Queue();

		void post_message(Message const& message);

		bool get_message(Message & message, size_t timeout_msec = WAIT_FOREVER);

	private:
		Queue_impl * m_impl;
	};

}

#endif
