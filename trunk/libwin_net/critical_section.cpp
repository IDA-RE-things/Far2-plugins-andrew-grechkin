/*
 * critical_section.cpp
 *
 *  Created on: 24.08.2011
 *      Author: Andrew Grechkin
 */

#include <libwin_def/std.h>

#include "lock.h"

CriticalSectionLock::~CriticalSectionLock() {
	if (m_cs)
		::LeaveCriticalSection(m_cs);
}

CriticalSectionLock::CriticalSectionLock(PCRITICAL_SECTION cs):
	m_cs(nullptr) {
	::EnterCriticalSection(cs);
	m_cs = cs;
}

CriticalSectionLock& CriticalSectionLock::operator=(const CriticalSectionLock &cs) {
	CriticalSectionLock(cs).swap(*this);
	return *this;
}

void CriticalSectionLock::swap(CriticalSectionLock & rhs) {
	using std::swap;
	swap(m_cs, rhs.m_cs);
}


CriticalSectionLocker::~CriticalSectionLocker() {
	::DeleteCriticalSection(&m_cs);
}

CriticalSectionLocker::CriticalSectionLocker() {
	::InitializeCriticalSection(&m_cs);
}

CriticalSectionLocker::Lock CriticalSectionLocker::lock() const {
	return Lock((PCRITICAL_SECTION)&m_cs);
}
