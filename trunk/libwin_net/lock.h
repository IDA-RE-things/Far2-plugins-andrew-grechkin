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

	CriticalSectionLock(PCRITICAL_SECTION cs);

	CriticalSectionLock& operator=(const CriticalSectionLock &cs);

	operator PCRITICAL_SECTION() const {
		return m_cs;
	}

	void swap(CriticalSectionLock & rhs);

private:
	PCRITICAL_SECTION m_cs;
};

#endif
