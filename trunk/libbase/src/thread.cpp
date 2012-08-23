#include <libbase/thread.hpp>

namespace Base {

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

}
