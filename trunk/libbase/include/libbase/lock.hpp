#ifndef _LIBBASE_LOCK_HPP_
#define _LIBBASE_LOCK_HPP_

#include <libbase/std.hpp>

namespace Base {

	namespace Lock {

		struct int64_t_sync {
			int64_t_sync(int64_t val):
				m_value(val) {
			}

			int64_t_sync & operator = (int64_t val) {
				::InterlockedExchange64(&m_value, val);
				return *this;
			}

			int64_t_sync & operator += (int64_t val) {
				::InterlockedExchangeAdd64(&m_value, val);
				return *this;
			}

		private:
			int64_t m_value;
		};

		struct LockWatcher;

		struct SyncUnit_i {
			virtual ~SyncUnit_i() = 0;

			virtual LockWatcher get_lock() = 0;

			virtual LockWatcher get_lock_read() = 0;

			virtual void lock() = 0;

			virtual void lock_read() = 0;

			virtual void release() = 0;
		};


		struct LockWatcher: private Uncopyable {
			~LockWatcher();

			LockWatcher(SyncUnit_i * unit, bool read = false);

			LockWatcher(LockWatcher && rhs);

			LockWatcher & operator = (LockWatcher && rhs);

			void swap(LockWatcher & rhs);

		private:
			LockWatcher();

			SyncUnit_i * m_unit;
		};


		SyncUnit_i * get_CritSection();

		SyncUnit_i * get_ReadWrite();


		struct CriticalSection {
			~CriticalSection() {
				::DeleteCriticalSection(&m_sync);
			}

			CriticalSection()
			{
				::InitializeCriticalSection(&m_sync);
			}

			void lock() {
				return ::EnterCriticalSection(&m_sync);
			}

			void release() {
				::LeaveCriticalSection(&m_sync);
			}

		private:
			CRITICAL_SECTION m_sync;
		};

		struct Semaphore {
			~Semaphore() {
				::CloseHandle(m_sync);
			}

			Semaphore():
				m_sync(::CreateSemaphoreW(nullptr, 0, LONG_MAX, nullptr))
			{
			}

			HANDLE handle() const {
				return m_sync;
			}

			DWORD wait(size_t millisec) {
				return ::WaitForSingleObject(m_sync, millisec);
			}

			void release() {
				::ReleaseSemaphore(m_sync, 1, nullptr);
			}

		private:
			HANDLE m_sync;
		};

//		struct SRWlock {
//			SRWlock() {
//				::InitializeSRWLock(&m_impl);
//			}
//		private:
//			SRWLOCK m_impl;
//		};

	}
}

#endif
