#include <libbase/lock.hpp>

namespace Base {
	namespace Lock {

		ScopeGuard::~ScopeGuard() {
			if (m_unit) {
				m_unit->release();
				m_unit = nullptr;
			}
		}

		ScopeGuard::ScopeGuard(SyncUnit_i * unit, bool read):
			m_unit(unit)
		{
			read ? m_unit->lock_read() : m_unit->lock();
		}

		ScopeGuard::ScopeGuard(ScopeGuard && right):
			m_unit(right.m_unit)
		{
			right.m_unit = nullptr;
		}

		ScopeGuard & ScopeGuard::operator = (ScopeGuard && right) {
			if (this != &right)
				ScopeGuard(std::move(right)).swap(*this);
			return *this;
		}

		void ScopeGuard::swap(ScopeGuard & right) {
			using std::swap;
			swap(m_unit, right.m_unit);
		}

	}
}
