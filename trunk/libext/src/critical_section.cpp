/*
 * critical_section.cpp
 *
 *  Created on: 24.08.2011
 *      Author: Andrew Grechkin
 */

#include <libbase/std.hpp>

#include <libext/lock.hpp>

namespace Ext {

CriticalSection::Lock::~Lock() {
	if (m_cs)
		::LeaveCriticalSection(m_cs);
}

CriticalSection::Lock::Lock(PCRITICAL_SECTION cs):
	m_cs(nullptr) {
	::EnterCriticalSection(cs);
	m_cs = cs;
}

CriticalSection::Lock::Lock(const this_type & cs):
	m_cs(nullptr) {
	::EnterCriticalSection(cs.m_cs);
	m_cs = cs.m_cs;
}

CriticalSection::Lock & CriticalSection::Lock::operator =(const this_type & cs) {
	if (m_cs != cs.m_cs)
		Lock(cs).swap(*this);
	return *this;
}

void CriticalSection::Lock::swap(this_type & rhs) {
	using std::swap;
	swap(m_cs, rhs.m_cs);
}


CriticalSection::~CriticalSection() {
	::DeleteCriticalSection(&m_cs);
}

CriticalSection::CriticalSection() {
	::InitializeCriticalSection(&m_cs);
}

CriticalSection::Lock CriticalSection::lock() const {
	return Lock((PCRITICAL_SECTION)&m_cs);
}

}
