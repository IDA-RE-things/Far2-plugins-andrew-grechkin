#include <libbase/thread.hpp>
#include <libbase/logger.hpp>

namespace Base {

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
