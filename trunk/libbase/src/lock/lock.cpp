#include <libbase/lock.hpp>

namespace Base {
	namespace Lock {

		ScopeGuard SyncUnit_i::lock_scope()
		{
			return ScopeGuard(this, false);
		}

		ScopeGuard SyncUnit_i::lock_scope_read()
		{
			return ScopeGuard(this, true);
		}

		void SyncUnit_i::destroy() const
		{
			delete this;
		}

		SyncUnit_i::~SyncUnit_i()
		{
		}

	}
}
