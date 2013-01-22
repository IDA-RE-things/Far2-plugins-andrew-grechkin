#include <libbase/std.hpp>
#include <libbase/lock.hpp>

namespace Base {
	namespace Lock {

		struct CriticalSection_impl: public SyncUnit_i, private CriticalSection {
			~CriticalSection_impl();

			void lock() override;

			void lock_read() override;

			void release() override;
		};

		CriticalSection_impl::~CriticalSection_impl()
		{
		}

		void CriticalSection_impl::lock()
		{
			CriticalSection::lock();
		}

		void CriticalSection_impl::lock_read()
		{
			CriticalSection::lock();
		}

		void CriticalSection_impl::release()
		{
			CriticalSection::release();
		}

		SyncUnit_i * get_CritSection()
		{
			return new CriticalSection_impl;
		}

	}
}
