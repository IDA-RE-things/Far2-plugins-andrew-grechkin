#ifndef WIN_NET_LOG_HPP
#define WIN_NET_LOG_HPP

#include "win_net.h"

///▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒ net_log
///========================================================================================== WinLog
class WinLog {
	HANDLE	m_hndl;

	WinLog();
public:
	~WinLog();

	WinLog(PCWSTR name);

	static void Register(PCWSTR name, PCWSTR path = nullptr);
};

#endif
