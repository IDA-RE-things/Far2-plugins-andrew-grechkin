#include <libbase/lock.hpp>

namespace Base {
	namespace Lock {

		struct ReadWrite_impl: public SyncUnit_i {
			~ReadWrite_impl() override;

			ReadWrite_impl();

			void lock() override;

			void lock_read() override;

			void release() override;

		private:
			CRITICAL_SECTION m_cs;		// Permits exclusive access to other members
			HANDLE m_EventAllowWrite; 	// Writers wait on this if a reader has access
			HANDLE m_EventAllowRead;	// Readers wait on this if a writer has access
			ssize_t m_nActive;			// Number of threads currently with access (0=no threads, >0=# of readers, -1=1 writer)
			size_t m_nWaitingWriters;	// Number of writers waiting for access
			size_t m_nWaitingReaders;	// Number of readers waiting for access
			DWORD m_WriterThreadId;
		};

		ReadWrite_impl::ReadWrite_impl():
			m_EventAllowWrite(::CreateEventW(nullptr, FALSE, FALSE, nullptr)),
			m_EventAllowRead(::CreateEventW(nullptr, TRUE, FALSE, nullptr)),
			m_nActive(0),
			m_nWaitingWriters(0),
			m_nWaitingReaders(0),
			m_WriterThreadId(0) {
			// Initially no readers want access, no writers want access, and no threads are accessing the resource
			::InitializeCriticalSection(&m_cs);
		}

		ReadWrite_impl::~ReadWrite_impl() {
			::DeleteCriticalSection(&m_cs);
			::CloseHandle(m_EventAllowRead);
			::CloseHandle(m_EventAllowWrite);
		}

		void ReadWrite_impl::lock() {
			::EnterCriticalSection(&m_cs);
			// Are there any threads accessing the resource?
			BOOL fResourceOwned = ((m_nActive > 0) || ((m_nActive < 0) && (m_WriterThreadId != ::GetCurrentThreadId())));

			if (fResourceOwned) {
				// This writer must wait, increment the count of waiting writers
				m_nWaitingWriters++;
			} else {
				// This writer can write, decrement the count of active writers
				m_nActive--;
			}
			::LeaveCriticalSection(&m_cs);

			if (fResourceOwned) {
				// This thread must wait
				::WaitForSingleObject(m_EventAllowWrite, INFINITE);
			}
			m_WriterThreadId = ::GetCurrentThreadId();
		}

		void ReadWrite_impl::lock_read() {
			::EnterCriticalSection(&m_cs);
			// Are there writers waiting or is a writer writing?
			BOOL fResourceWritePending = (m_nWaitingWriters || (m_nActive < 0));

			if (fResourceWritePending) {
				// This reader must wait, increment the count of waiting readers
				m_nWaitingReaders++;
			} else {
				// This reader can read, increment the count of active readers
				m_nActive++;
			}
			::LeaveCriticalSection(&m_cs);

			if (fResourceWritePending) {
				::WaitForSingleObject(m_EventAllowRead, INFINITE);
			}
		}

		void ReadWrite_impl::release() {
			::EnterCriticalSection(&m_cs);
			if (m_nActive > 0) {
				m_nActive--;	// Readers have control so a reader must be done
			} else {
				m_nActive++;	// Writers have control so a writer must be done
			}

			if (m_nActive == 0) {
				m_WriterThreadId = 0;
				// No thread has access, who should wake up?
				// NOTE: It is possible that readers could never get access
				//       if there are always writers wanting to write
				if (m_nWaitingWriters > 0) {
					// Writers are waiting and they take priority over readers
					m_nActive = -1;					// A writer will get access
					m_nWaitingWriters--;			// One less writer will be waiting
					::SetEvent(m_EventAllowWrite);	// NOTE: The event will release only 1 writer thread
				} else if (m_nWaitingReaders > 0) {
					// Readers are waiting and no writers are waiting
					m_nActive = m_nWaitingReaders;	// All readers will get access
					m_nWaitingReaders = 0;			// No readers will be waiting
					::SetEvent(m_EventAllowRead);	// release all readers
					::ResetEvent(m_EventAllowRead);
				} else {
					// There are no threads waiting at all; no semaphore gets released
				}
			}
			::LeaveCriticalSection(&m_cs);
		}


		SyncUnit_i * get_ReadWrite() {
			return new ReadWrite_impl;
		}

	}
}
