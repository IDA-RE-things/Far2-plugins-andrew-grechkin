#ifndef _LIBBASE_THREADPOOL_HPP_
#define _LIBBASE_THREADPOOL_HPP_

#include <libbase/std.hpp>
#include <libbase/uncopyable.hpp>

namespace Base {

//	struct ThreadPool: private Base::Uncopyable {
//		typedef Thread::handle_t handle_t;
//
//		~ThreadPool() {
//		}
//
//		size_t size() const {
//			return m_threads.size();
//		}
//
//		void push_back(Thread thread) {
//			m_threads.push_back(thread);
//			m_handles.push_back(thread.get_handle());
//		}
//
//		void push_back(ThreadRoutine_i * routine);
//
//		Thread & back() {
//			return m_threads.back();
//		}
//
//		Thread & operator [] (size_t i) {
//			return m_threads[i];
//		}
//
//		bool wait_all(Thread::timeout_t timeout = Thread::WAIT_INFINITE) const {
//			return ::WaitForMultipleObjects(m_handles.size(), &m_handles[0], TRUE, timeout) == WAIT_OBJECT_0;
//		}
//
//		size_t wait_any(Thread::timeout_t timeout = Thread::WAIT_INFINITE) const {
//			return ::WaitForMultipleObjects(m_handles.size(), &m_handles[0], FALSE, timeout) - WAIT_OBJECT_0;
//		}
//
//	private:
//		std::vector<Thread> m_threads;
//		std::vector<Thread::handle_t> m_handles;
//	};

}

#endif
