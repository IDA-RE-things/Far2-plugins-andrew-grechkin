#ifndef _LIBBASE_THREAD_HPP_
#define _LIBBASE_THREAD_HPP_

#include <libbase/std.hpp>
#include <libbase/memory.hpp>
#include <libbase/logger.hpp>
#include <libbase/queue.hpp>
#include <libbase/shared_ptr.hpp>

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


	struct ThreadRoutine_i {
		static DWORD WINAPI run_thread(void * routine);

		static VOID WINAPI alert_thread(ULONG_PTR routine);

		void post_message(size_t msg, ssize_t lparam = 0, ssize_t wparam = 0);

		virtual ~ThreadRoutine_i();

		virtual void alert(void * data);

		virtual size_t run(void * data);

	private:
		virtual void post_message_(size_t msg, ssize_t lparam, ssize_t wparam);
	};


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

		~Thread();

		Thread(ThreadRoutine_i * routine);

		Thread(ThreadRoutine_i * routine, void * data, size_t stack_size = 0);

		Thread(Thread && right);

		Thread & operator = (Thread && right);

		void swap(Thread & right);

		void alert();

		void alert(void * data);

		bool set_priority(Thread::Priority_t prio);

		bool set_io_priority(Thread::IoPriority_t prio);

		size_t get_exitcode() const;

		size_t get_id() const;

		Thread::handle_t get_handle() const;

		Thread::Priority_t get_priority() const;

		ThreadRoutine_i * get_routine() const;

		bool suspend() const;

		bool resume() const;

		bool wait(Thread::timeout_t timeout = Thread::WAIT_INFINITE) const;

	private:
		ThreadRoutine_i * m_routine;
		handle_t m_handle;
		id_t m_id;
	};


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
