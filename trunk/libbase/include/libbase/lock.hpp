#ifndef _LIBBASE_LOCK_HPP_
#define _LIBBASE_LOCK_HPP_

#include <libbase/std.hpp>

namespace Base {

	struct LockWatcher;

	struct SyncUnit_i {
		virtual ~SyncUnit_i() = 0;

		virtual LockWatcher get_lock() = 0;

		virtual void lock() = 0;

		virtual void release() = 0;
	};

	struct LockWatcher: private Uncopyable {
		~LockWatcher();

		LockWatcher(SyncUnit_i * unit);

		LockWatcher(LockWatcher && rhs);

		LockWatcher & operator = (LockWatcher && rhs);

		void swap(LockWatcher & rhs);

	private:
		LockWatcher();

		SyncUnit_i * m_unit;
	};

	SyncUnit_i * get_LockCritSection();

}

#endif
