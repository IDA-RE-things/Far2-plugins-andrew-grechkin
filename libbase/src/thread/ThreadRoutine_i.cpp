#include <libbase/thread.hpp>
#include <libbase/logger.hpp>

namespace Base {

	DWORD WINAPI ThreadRoutine_i::run_thread(void * routine)
	{
		LogTrace();
		return reinterpret_cast<ThreadRoutine_i*>(routine)->run(nullptr);
	}

	VOID WINAPI ThreadRoutine_i::alert_thread(ULONG_PTR routine)
	{
		LogTrace();
		reinterpret_cast<ThreadRoutine_i*>(routine)->alert(nullptr);
	}

	ThreadRoutine_i::~ThreadRoutine_i()
	{
		LogTrace();
	}

	void ThreadRoutine_i::alert(void * /*data*/)
	{
		LogTrace();
	}

	size_t ThreadRoutine_i::run(void * /*data*/)
	{
		LogTrace();
		return 0;
	}

	void ThreadRoutine_i::post_message(const Message & message) const
	{
		(void)message;
		LogDebug(L"type: %Id, code: %Id, param: %Id, data: %p\n", message.get_type(), message.get_code(), message.get_param(), message.get_data());
	}

}
