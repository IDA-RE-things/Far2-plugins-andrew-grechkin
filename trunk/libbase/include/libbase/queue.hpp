#ifndef _LIBBASE_QUEUE_HPP_
#define _LIBBASE_QUEUE_HPP_

#include <libbase/std.hpp>
#include <libbase/logger.hpp>
#include <libbase/lock.hpp>

#include <deque>

namespace Base {

	struct Queue: private Lock::CriticalSection, private Lock::Semaphore {
		struct Message {
			size_t msg;
			ssize_t lparam;
			ssize_t wparam;

			Message(size_t m = 0, ssize_t lp = 0, ssize_t wp = 0):
				msg(m),
				lparam(lp),
				wparam(wp)
			{
			}
		};

		Queue()
		{
		}

		using Lock::Semaphore::handle;
		using Lock::Semaphore::wait;

		void post_message(size_t msg, ssize_t lparam = 0, ssize_t wparam = 0) {
			CriticalSection::lock();
			m_queue.emplace_back(msg, lparam, wparam);
			CriticalSection::release();
		}

		Message get_message() {
			Message tmp;
			CriticalSection::lock();
			if (!m_queue.empty()) {
				tmp = m_queue.front();
				m_queue.pop_front();
			}
			CriticalSection::release();
			return tmp;
		}

	private:
		std::deque<Message> m_queue;
	};

}

#endif
