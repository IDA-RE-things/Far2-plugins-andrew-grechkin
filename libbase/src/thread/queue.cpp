#include <libbase/message.hpp>

#include <libbase/logger.hpp>
#include <libbase/lock.hpp>

#include <deque>

namespace Base {

	struct Queue_impl: private Lock::CriticalSection, private Lock::Semaphore, private std::deque<Message> {
		void post_message(value_type const& message);

		bool get_message(value_type & message, size_t timeout_msec);
	};

	void Queue_impl::post_message(value_type const& message)
	{
		CriticalSection::lock();
		emplace_back(message);
		Semaphore::release(1);
		CriticalSection::release();
	}

	bool Queue_impl::get_message(value_type & message, size_t timeout_msec)
	{
		bool ret = false;
		if (Semaphore::wait(timeout_msec) == Semaphore::WaitResult::SUCCES) {
			CriticalSection::lock();
			message = front();
			pop_front();
			CriticalSection::release();
			ret = true;
		}
		return ret;
	}

	const size_t Queue::WAIT_FOREVER = Lock::Semaphore::WAIT_FOREVER;

	Queue::~Queue()
	{
		delete m_impl;
	}

	Queue::Queue():
		m_impl(new Queue_impl)
	{
	}

	void Queue::put_message(Message const& message)
	{
		return m_impl->post_message(message);
	}

	bool Queue::get_message(Message & message, size_t timeout_msec)
	{
		return m_impl->get_message(message, timeout_msec);
	}

}
