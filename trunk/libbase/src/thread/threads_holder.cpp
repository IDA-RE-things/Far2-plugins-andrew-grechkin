#include <libbase/thread.hpp>

namespace Base {

	ThreadsHolder::~ThreadsHolder() {
	}

	size_t ThreadsHolder::size() const {
		return m_handles.size();
	}

	void ThreadsHolder::add(Thread && thread) {
		m_threads.push_back(std::move(thread));
		m_handles.push_back(m_threads.back().get_handle());
	}

	void ThreadsHolder::add(ThreadRoutine_i * routine) {
		m_threads.emplace_back(routine);
		m_handles.push_back(m_threads.back().get_handle());
	}

	Thread & ThreadsHolder::back() {
		return m_threads.back();
	}

	Thread & ThreadsHolder::operator [] (size_t i) {
		return m_threads[i];
	}

	bool ThreadsHolder::wait_all(Thread::timeout_t timeout) const {
		return ::WaitForMultipleObjects(m_handles.size(), &m_handles[0], TRUE, timeout) == WAIT_OBJECT_0;
	}

	size_t ThreadsHolder::wait_any(Thread::timeout_t timeout) const {
		return ::WaitForMultipleObjects(m_handles.size(), &m_handles[0], FALSE, timeout) - WAIT_OBJECT_0;
	}

	void ThreadsHolder::alert(void * data) {
		for (size_t i = 0; i < m_threads.size(); ++i) {
			m_threads[i].alert(data);
		}
	}

}
