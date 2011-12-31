#ifndef WIN_NET_TIMER_HPP
#define WIN_NET_TIMER_HPP

#include "win_net.h"

///==================================================================================== WinSysTimers
struct		WinSysTimers {
	LARGE_INTEGER liKeBootTime;
	LARGE_INTEGER liKeSystemTime;
	LARGE_INTEGER liExpTimeZoneBias;
	ULONG uCurrentTimeZoneId;
	DWORD dwReserved;

	WinSysTimers();

	size_t	Uptime(size_t del = 1);

	ustring	UptimeAsText();
};

#endif
