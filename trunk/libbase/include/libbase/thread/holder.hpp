#ifndef _LIBBASE_THREAD_HOLDER_HPP_
#define _LIBBASE_THREAD_HOLDER_HPP_

#include <libbase/thread.hpp>

namespace Base {

	struct ThreadsHolder: private Base::Uncopyable {

		~ThreadsHolder();

		void alert();

		void alert(void * data);

		void add(Thread && thread);

		void add(ThreadRoutine_i * routine);

		Thread & back();

		const Thread & back() const;

		Thread & operator [] (size_t i);

		const Thread & operator [] (size_t i) const ;

		bool wait_all(Timeout_t timeout = WAIT_FOREVER) const;

		size_t wait_any(Timeout_t timeout = WAIT_FOREVER) const;

		size_t size() const;

	private:
		std::vector<Thread::handle_t> m_handles;
		std::vector<Thread> m_threads;
	};

}

#endif
