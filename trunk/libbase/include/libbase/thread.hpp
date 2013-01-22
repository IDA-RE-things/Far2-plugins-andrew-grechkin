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
		typedef DWORD timeout_t;

		static const timeout_t WAIT_INFINITE = INFINITE;

		enum class Priority_t: ssize_t {
			IDLE = -15,
			LOWEST = -2,
			BELOW_NORMAL = -1,
			NORMAL = 0,
			ABOVE_NORMAL = 1,
			HIGHEST = 2,
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

		ThreadRoutine_i * get_routine() const
		{
			return m_routine;
		}

		bool suspend() const;

		bool resume() const;

		bool wait(Thread::timeout_t timeout = Thread::WAIT_INFINITE) const;

	private:
		ThreadRoutine_i * m_routine;
		handle_t m_handle;
		id_t m_id;
	};

}

#endif
