/*
 * lock.h
 *
 *  Created on: 24.08.2011
 *      Author: Andrew Grechkin
 */

#ifndef __WIN__LOCK_H_
#define __WIN__LOCK_H_

#include <windows.h>

class CriticalSectionLock {
public:
	~CriticalSectionLock();

	CriticalSectionLock(const CriticalSectionLock & cs);

	CriticalSectionLock& operator=(const CriticalSectionLock &cs);

	operator PCRITICAL_SECTION() const {
		return m_cs;
	}

	void swap(CriticalSectionLock & rhs);

private:
	CriticalSectionLock(PCRITICAL_SECTION cs);

	PCRITICAL_SECTION m_cs;

	friend class CriticalSectionLocker;
};

class CriticalSectionLocker {
public:
	typedef CriticalSectionLock Lock;

	~CriticalSectionLocker();

	CriticalSectionLocker();

	Lock lock() const;

private:
	CRITICAL_SECTION m_cs;
};

#endif
