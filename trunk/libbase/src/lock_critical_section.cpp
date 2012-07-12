#include <libbase/lock.hpp>

namespace Base {
	namespace Lock {

		struct CriticalSection_impl: public SyncUnit_i {
			virtual ~CriticalSection_impl();

			virtual LockWatcher get_lock();

			virtual LockWatcher get_lock_read();

			virtual void lock();

			virtual void lock_read();

			virtual void release();

			CriticalSection_impl();

		private:
			CRITICAL_SECTION m_cs;
		};

		CriticalSection_impl::~CriticalSection_impl() {
			::DeleteCriticalSection(&m_cs);
		}

		LockWatcher CriticalSection_impl::get_lock() {
			return LockWatcher(this);
		}

		LockWatcher CriticalSection_impl::get_lock_read() {
			return LockWatcher(this, true);
		}

		void CriticalSection_impl::lock() {
			::EnterCriticalSection(&m_cs);
		}

		void CriticalSection_impl::lock_read() {
			::EnterCriticalSection(&m_cs);
		}

		void CriticalSection_impl::release() {
			::LeaveCriticalSection(&m_cs);
		}

		CriticalSection_impl::CriticalSection_impl() {
			::InitializeCriticalSection(&m_cs);
		}


		SyncUnit_i * get_CritSection() {
			return new CriticalSection_impl;
		}

	}
}
