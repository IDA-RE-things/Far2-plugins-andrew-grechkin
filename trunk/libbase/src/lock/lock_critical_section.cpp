#include <libbase/std.hpp>
#include <libbase/lock.hpp>


namespace Base {

	namespace Lock {

		struct CriticalSection_impl: public SyncUnit_i {
			~CriticalSection_impl() override;

			void lock() override;

			void lock_read() override;

			void release() override;

		private:
			CriticalSection_impl();

			CRITICAL_SECTION m_cs;

			friend SyncUnit_i * get_CritSection();
		};

		CriticalSection_impl::~CriticalSection_impl() {
			::DeleteCriticalSection(&m_cs);
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
