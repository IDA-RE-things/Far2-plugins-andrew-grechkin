#ifndef _LIBBASE_QUEUE_HPP_
#define _LIBBASE_QUEUE_HPP_

#include <libbase/std.hpp>
#include <libbase/logger.hpp>
#include <libbase/lock.hpp>

#include <deque>


namespace Base {

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


	struct Queue: private Lock::CriticalSection, private Lock::Semaphore {
		Queue()
		{
		}

		void post_message(size_t msg, ssize_t lparam = 0, ssize_t wparam = 0) {
			CriticalSection::lock();
			m_queue.emplace_back(msg, lparam, wparam);
			Semaphore::release(1);
			CriticalSection::release();
		}

		bool get_message(Message & out, size_t timeout_msec = Semaphore::WAIT_FOREVER) {
			bool ret = false;
			if (Semaphore::wait(timeout_msec) == Semaphore::SUCCES) {
				CriticalSection::lock();
				out = m_queue.front();
				m_queue.pop_front();
				CriticalSection::release();
				ret = true;
			}
			return ret;
		}

	private:
		std::deque<Message> m_queue;
	};

}

#endif
