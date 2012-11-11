#include <libbase/thread.hpp>
#include <libbase/logger.hpp>

namespace Base {

	DWORD WINAPI ThreadRoutine_i::run_thread(void * routine) {
		return reinterpret_cast<ThreadRoutine_i*>(routine)->run(nullptr);
	}

	VOID WINAPI ThreadRoutine_i::alert_thread(ULONG_PTR routine) {
		reinterpret_cast<ThreadRoutine_i*>(routine)->alert(nullptr);
	}

	void ThreadRoutine_i::post_message(size_t msg, ssize_t lparam, ssize_t wparam) {
		post_message_(msg, lparam, wparam);
	}

	ThreadRoutine_i::~ThreadRoutine_i() {
		LogTrace();
	}

	void ThreadRoutine_i::alert(void * /*data*/) {
	}

	size_t ThreadRoutine_i::run(void * /*data*/) {
		return 0;
	}

	void ThreadRoutine_i::post_message_(size_t /*msg*/, ssize_t /*lparam*/, ssize_t /*wparam*/) {
	}

}
