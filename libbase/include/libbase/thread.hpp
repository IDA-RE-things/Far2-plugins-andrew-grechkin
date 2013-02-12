#ifndef _LIBBASE_THREAD_HPP_
#define _LIBBASE_THREAD_HPP_

#include <libbase/std.hpp>
#include <libbase/uncopyable.hpp>
#include <libbase/thread/ThreadRoutine_i.hpp>
#include <libbase/thread/ThreadPool.hpp>

namespace Base {

//	template <typename Type, size_t (Type::*mem_func)(void *)>
//	DWORD WINAPI member_thunk(void * ptr)
//	{ // http://www.rsdn.ru/Forum/Message.aspx?mid=753888&only=1
//		return (static_cast<Type*>(ptr)->*mem_func)(nullptr);
//	}
//
//	template <typename Type, void (Type::*mem_func)(void *)>
//	VOID WINAPI apc_thunk(ULONG_PTR ptr)
//	{
//		(((Type*)(ptr))->*mem_func)(nullptr);
//	}

	struct Thread: private Uncopyable {
		typedef HANDLE handle_t;
		typedef DWORD id_t;

		enum class Priority_t: ssize_t {
			IDLE          = THREAD_PRIORITY_IDLE,
			LOWEST        = THREAD_PRIORITY_LOWEST,
			BELOW_NORMAL  = THREAD_PRIORITY_BELOW_NORMAL,
			NORMAL        = THREAD_PRIORITY_NORMAL,
			ABOVE_NORMAL  = THREAD_PRIORITY_ABOVE_NORMAL,
			HIGHEST       = THREAD_PRIORITY_HIGHEST,
			TIME_CRITICAL = THREAD_PRIORITY_TIME_CRITICAL,
		};

		enum class IoPriority_t: ssize_t {
			VERY_LOW,
			LOW,
			NORMAL,
			HIGH,
			CRITICAL,
		};

		~Thread() noexcept;

		Thread(ThreadRoutine_i * routine);

		Thread(ThreadRoutine_i * routine, void * data, size_t stack_size = 0);

		Thread(Thread && right);

		Thread & operator = (Thread && right);

		void swap(Thread & right) noexcept;

		void alert();

		void alert(void * data);

		bool set_priority(Thread::Priority_t prio);

		bool set_io_priority(Thread::IoPriority_t prio);

		size_t get_exitcode() const;

		id_t get_id() const
		{
			return m_id;
		}

		Thread::handle_t get_handle() const
		{
			return m_handle;
		}

		Thread::Priority_t get_priority() const;

		Thread::IoPriority_t get_io_priority() const;

		ThreadRoutine_i * get_routine() const
		{
			return m_routine;
		}

		bool suspend() const;

		bool resume() const;

		WaitResult_t wait(Timeout_t timeout = WAIT_FOREVER) const;

	private:
		ThreadRoutine_i * m_routine;
		handle_t m_handle;
		id_t m_id;
	};

	PCWSTR as_str(Thread::Priority_t prio);

	PCWSTR as_str(Thread::IoPriority_t prio);
}

#endif
