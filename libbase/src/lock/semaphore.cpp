#include <libbase/lock.hpp>

namespace Base {
	namespace Lock {

		const size_t Semaphore::WAIT_FOREVER = INFINITE;

		Semaphore::~Semaphore() {
			::CloseHandle(m_handle);
		}

		Semaphore::Semaphore(PCWSTR name):
			m_handle(::CreateSemaphoreW(nullptr, 0, LONG_MAX, name))
		{
		}

		HANDLE Semaphore::handle() const {
			return m_handle;
		}

		Semaphore::WaitResult Semaphore::wait(size_t millisec) {
			return (WaitResult)::WaitForSingleObjectEx(m_handle, millisec, true);
		}

		void Semaphore::release(size_t cnt) {
			::ReleaseSemaphore(m_handle, cnt, nullptr);
		}

	}
}
