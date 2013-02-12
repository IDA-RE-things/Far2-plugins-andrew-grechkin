#include <libbase/thread/holder.hpp>

#include <algorithm>
#include <functional>
#include <iterator>
#include <memory>


namespace Base {

	ThreadsHolder::~ThreadsHolder() {
	}

	void ThreadsHolder::alert() {
		using namespace std::placeholders;
		void (Thread::*func)() = &Thread::alert;
		std::for_each(m_threads.begin(), m_threads.end(), std::bind(func, _1));
	}

	void ThreadsHolder::alert(void * data) {
		using namespace std::placeholders;
		void (Thread::*func)(void *) = &Thread::alert;
		std::for_each(m_threads.begin(), m_threads.end(), std::bind(func, _1, data));
	}

	void ThreadsHolder::add(Thread && thread) {
		m_threads.push_back(std::move(thread));
		m_handles.emplace_back(m_threads.back().get_handle());
	}

	void ThreadsHolder::add(ThreadRoutine_i * routine) {
		m_threads.emplace_back(routine);
		m_handles.emplace_back(m_threads.back().get_handle());
	}

	Thread & ThreadsHolder::back() {
		return m_threads.back();
	}

	const Thread & ThreadsHolder::back() const {
		return m_threads.back();
	}

	Thread & ThreadsHolder::operator [] (size_t i) {
		return m_threads[i];
	}

	const Thread & ThreadsHolder::operator [] (size_t i) const {
		return m_threads[i];
	}

	bool ThreadsHolder::wait_all(Timeout_t timeout) const {
		return ::WaitForMultipleObjects(m_handles.size(), &m_handles[0], TRUE, timeout) == WAIT_OBJECT_0;
	}

	size_t ThreadsHolder::wait_any(Timeout_t timeout) const {
		return ::WaitForMultipleObjects(m_handles.size(), &m_handles[0], FALSE, timeout) - WAIT_OBJECT_0;
	}

	size_t ThreadsHolder::size() const {
		return m_handles.size();
	}

}
