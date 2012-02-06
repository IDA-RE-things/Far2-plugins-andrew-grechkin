/*
 * lock.h
 *
 *  Created on: 24.08.2011
 *      Author: Andrew Grechkin
 */

#ifndef __WIN__LOCK_H_
#define __WIN__LOCK_H_

#include <windows.h>

struct CriticalSection {
	class Lock {
		typedef Lock this_type;

	public:
		~Lock();

		Lock(const this_type & cs);

		Lock & operator = (const this_type & cs);

		void swap(this_type & rhs);

	private:
		Lock(PCRITICAL_SECTION cs);

		PCRITICAL_SECTION m_cs;

		friend class CriticalSection;
	};

	~CriticalSection();

	CriticalSection();

	Lock lock() const;

private:
	CRITICAL_SECTION m_cs;
};

#endif
