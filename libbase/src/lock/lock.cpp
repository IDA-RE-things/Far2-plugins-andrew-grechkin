#include <libbase/lock.hpp>

namespace Base {
	namespace Lock {

		LockWatcher SyncUnit_i::get_lock() {
			return LockWatcher(this, false);
		}

		LockWatcher SyncUnit_i::get_lock_read() {
			return LockWatcher(this, true);
		}

		void SyncUnit_i::destroy() {
			delete this;
		}

		SyncUnit_i::~SyncUnit_i() {
		}


		LockWatcher::~LockWatcher() {
			if (m_unit) {
				m_unit->release();
				m_unit = nullptr;
			}
		}

		LockWatcher::LockWatcher(SyncUnit_i * unit, bool read):
			m_unit(unit)
		{
			read ? m_unit->lock_read() : m_unit->lock();
		}

		LockWatcher::LockWatcher(LockWatcher && rhs):
			m_unit(nullptr)
		{
			swap(rhs);
		}

		LockWatcher & LockWatcher::operator = (LockWatcher && rhs) {
			if (this != &rhs) {
				LockWatcher(std::move(rhs)).swap(*this);
			}
			return *this;
		}

		void LockWatcher::swap(LockWatcher & rhs) {
			using std::swap;
			swap(m_unit, rhs.m_unit);
		}

		LockWatcher::LockWatcher():
			m_unit(nullptr)
		{
		}

	}
}
