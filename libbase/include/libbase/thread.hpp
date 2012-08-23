#ifndef _LIBBASE_THREAD_HPP_
#define _LIBBASE_THREAD_HPP_

#include <libbase/std.hpp>
#include <libbase/memory.hpp>
#include <libbase/logger.hpp>
#include <libbase/queue.hpp>
#include <libbase/shared_ptr.hpp>

namespace Base {

	template <typename Type, size_t (Type::*mem_func)(void *)>
	DWORD WINAPI member_thunk(void * ptr)
	{ // http://www.rsdn.ru/Forum/Message.aspx?mid=753888&only=1
		return (static_cast<Type*>(ptr)->*mem_func)(nullptr);
	}

	template <typename Type, void (Type::*mem_func)(void *)>
	VOID WINAPI apc_thunk(ULONG_PTR ptr)
	{
		(((Type*)(ptr))->*mem_func)(nullptr);
	}


	struct ThreadRoutine_i {
		void post_message(size_t msg, ssize_t lparam = 0, ssize_t wparam = 0);

		virtual ~ThreadRoutine_i();

		virtual void alert(void * data) = 0;

		virtual size_t run(void * data) = 0;

	private:
		virtual void post_message_(size_t msg, ssize_t lparam, ssize_t wparam) = 0;
	};


	struct ThreadParameters {
		ThreadRoutine_i * routine;
		void * data;

		ThreadParameters(ThreadRoutine_i * r, void * d = nullptr);

		static DWORD WINAPI run_thread_with_param(void * param);

		static VOID WINAPI alert_thread_with_param(ULONG_PTR param);
	};


	struct Thread: private Uncopyable {
		typedef HANDLE handle_t;
		typedef DWORD id_t;
		typedef DWORD timeout_t;
//		typedef Base::shared_ptr<ThreadRoutine_i> routine_t;

		static const timeout_t WAIT_INFINITE = INFINITE;

		enum priority_t {
			PRIORITY_IDLE = -15,
			PRIORITY_LOWEST = -2,
			PRIORITY_BELOW_NORMAL = -1,
			PRIORITY_NORMAL = 0,
			PRIORITY_ABOVE_NORMAL = 1,
			PRIORITY_HIGHEST = 2,
		};

		enum io_priority_t {
			IO_PRIORITY_VERY_LOW,
			IO_PRIORITY_LOW,
			IO_PRIORITY_NORMAL,
			IO_PRIORITY_HIGH,
			IO_PRIORITY_CRITICAL,
		};

		~Thread();

		Thread(ThreadRoutine_i * routine, void * data = nullptr, size_t stack_size = 0);

		Thread(Thread && rhs);

		Thread & operator = (Thread && rhs);

		void swap(Thread & rhs);

		bool set_priority(Thread::priority_t prio);

		bool set_io_priority(Thread::io_priority_t prio);

		size_t get_exitcode() const;

		size_t get_id() const;

		Thread::handle_t get_handle() const;

		Thread::priority_t get_priority() const;

		ThreadRoutine_i * get_routine() const;

		bool suspend() const;

		bool resume() const;

		bool wait(Thread::timeout_t timeout = Thread::WAIT_INFINITE) const;

		void alert(void * data = nullptr);

	private:
		ThreadRoutine_i * m_routine;
		HANDLE m_handle;
		Thread::id_t m_id;
	};


	struct ThreadsHolder: private Base::Uncopyable {
		typedef Thread::handle_t handle_t;

		~ThreadsHolder();

		size_t size() const;

		void add(Thread && thread);

		void add(ThreadRoutine_i * routine);

		Thread & back();

		Thread & operator [] (size_t i);

		bool wait_all(Thread::timeout_t timeout = Thread::WAIT_INFINITE) const;

		size_t wait_any(Thread::timeout_t timeout = Thread::WAIT_INFINITE) const;

		void alert(void * data = nullptr);

	private:
		std::vector<Thread> m_threads;
		std::vector<Thread::handle_t> m_handles;
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
