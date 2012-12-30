#include <libbase/lock.hpp>

namespace Base {
	namespace Lock {

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

		LockWatcher::LockWatcher(LockWatcher && right):
			m_unit(right.m_unit)
		{
			right.m_unit = nullptr;
		}

		LockWatcher & LockWatcher::operator = (LockWatcher && right) {
			if (this != &right)
				LockWatcher(std::move(right)).swap(*this);
			return *this;
		}

		void LockWatcher::swap(LockWatcher & right) {
			using std::swap;
			swap(m_unit, right.m_unit);
		}

		LockWatcher::LockWatcher():
			m_unit(nullptr)
		{
		}

	}
}
