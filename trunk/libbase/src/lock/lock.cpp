#include <libbase/lock.hpp>

namespace Base {
	namespace Lock {

		LockWatcher SyncUnit_i::get_lock()
		{
			return LockWatcher(this, false);
		}

		LockWatcher SyncUnit_i::get_lock_read()
		{
			return LockWatcher(this, true);
		}

		void SyncUnit_i::destroy()
		{
			delete this;
		}

		SyncUnit_i::~SyncUnit_i()
		{
		}

	}
}
