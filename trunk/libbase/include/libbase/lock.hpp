#ifndef _LIBBASE_LOCK_HPP_
#define _LIBBASE_LOCK_HPP_

#include <libbase/auto_destroy.hpp>
#include <libbase/uncopyable.hpp>
#include <libbase/std.hpp>


namespace Base {

	namespace Lock {

		struct LockWatcher;
		struct SyncUnit_i;


		SyncUnit_i * get_CritSection();

		SyncUnit_i * get_ReadWrite();


		///=========================================================================================
		struct SyncUnit_i: public Base::Destroyable {
			LockWatcher get_lock();

			LockWatcher get_lock_read();

			void destroy() override;

		public:
			virtual ~SyncUnit_i();

			virtual void lock() = 0;

			virtual void lock_read() = 0;

			virtual void release() = 0;
		};


		///=========================================================================================
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


		///=========================================================================================
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


		///=========================================================================================
		struct CriticalSection: private Base::Uncopyable {
			~CriticalSection() {
				::DeleteCriticalSection(&m_sync);
			}

			CriticalSection()
			{
				::InitializeCriticalSection(&m_sync);
			}

			void lock() const {
				::EnterCriticalSection(&m_sync);
			}

			void release() const {
				::LeaveCriticalSection(&m_sync);
			}

		private:
			mutable CRITICAL_SECTION m_sync;
		};


		///=========================================================================================
		struct Semaphore: private Base::Uncopyable {
			enum class WaitResult: ssize_t {
				SUCCES = WAIT_OBJECT_0,
				APC = WAIT_IO_COMPLETION,
				TIMEOUT = WAIT_TIMEOUT,
			};

			static const size_t WAIT_FOREVER;

			~Semaphore();

			Semaphore(PCWSTR name = nullptr);

			HANDLE handle() const;

			WaitResult wait(size_t millisec = WAIT_FOREVER);

			void release(size_t cnt = 1);

		private:
			HANDLE m_handle;
		};

		///=========================================================================================
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
