#include <libbase/thread.hpp>

namespace Base {

	struct ThreadParameters {
		static DWORD WINAPI run_thread_with_param(void * param);

		static VOID WINAPI alert_thread_with_param(ULONG_PTR param);

		ThreadParameters(ThreadRoutine_i * r, void * d = nullptr);

		ThreadRoutine_i * routine;
		void * data;
	};


	ThreadParameters::ThreadParameters(ThreadRoutine_i * r, void * d):
		routine(r),
		data(d)
	{
	}

	DWORD WINAPI ThreadParameters::run_thread_with_param(void * param) {
		ThreadParameters l_param(*(ThreadParameters*)param);
		delete (ThreadParameters*)param;
		return l_param.routine->run(l_param.data);
	}

	VOID WINAPI ThreadParameters::alert_thread_with_param(ULONG_PTR param) {
		ThreadParameters l_param(*(ThreadParameters*)param);
		delete (ThreadParameters*)param;
		return l_param.routine->alert(l_param.data);
	}


	Thread::Thread(ThreadRoutine_i * routine, void * data, size_t stack_size):
		m_routine(routine),
		m_handle(::CreateThread(nullptr, stack_size, ThreadParameters::run_thread_with_param, new ThreadParameters(m_routine, data), CREATE_SUSPENDED, &m_id))
	{
	}

	void Thread::alert(void * data) {
		::QueueUserAPC(ThreadParameters::alert_thread_with_param, m_handle, (ULONG_PTR)new ThreadParameters(m_routine, data));
	}

}
