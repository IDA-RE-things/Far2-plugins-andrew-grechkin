/**
	win_ts

	@classes	()
	@author		© 2009 Andrew Grechkin
	@link		()
**/

#ifndef WIN_TS_HPP
#define WIN_TS_HPP

#include "win_net.h"

///▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒ net_ts
///===================================================================================== WinTSHandle
struct		WinTSHandle: private Uncopyable {
	~WinTSHandle();

	WinTSHandle();

	WinTSHandle(PCWSTR host);

	operator		HANDLE() const {
		return m_ts;
	}

private:
	HANDLE		m_ts;
};

///===================================================================================== WinTSession
namespace	WinTSession {
	void	ConnectLocal(DWORD id, PCWSTR pass = L"");

	void	ConnectRemote(DWORD id, PCWSTR host);

	void	Disconnect(DWORD id, const WinTSHandle &host = WinTSHandle());

	void	LogOff(DWORD id, const WinTSHandle &host = WinTSHandle());

	DWORD	Question(DWORD id, PCWSTR ttl, PCWSTR msg, DWORD time = 60, const WinTSHandle &host = WinTSHandle());

	DWORD	Message(DWORD id, PCWSTR ttl, PCWSTR msg, DWORD time = 60, bool wait = true, const WinTSHandle &host = WinTSHandle());

	void	Reboot(const WinTSHandle &host = WinTSHandle());

	void	Turnoff(const WinTSHandle &host = WinTSHandle());
};

///======================================================================================= WinTSInfo
class	WinTSInfo {
public:
	WinTSInfo(DWORD i, const AutoUTF &s, const AutoUTF &u, int st);

	WinTSInfo(const WinTSHandle &host, DWORD id, const AutoUTF &ws, int st);

	DWORD id() const {
		return m_id;
	}
	AutoUTF	sess() const {
		return m_sess;
	}
	AutoUTF	user() const {
		return m_user;
	}
	AutoUTF	winSta() const {
		return m_winSta;
	}
	AutoUTF	client() const {
		return m_client;
	}
	int		state() const {
		return m_state;
	}

	bool is_disconnected() const;

	operator DWORD() const {
		return m_id;
	}

private:
	DWORD	m_id;
	AutoUTF	m_sess;
	AutoUTF	m_user;
	AutoUTF	m_winSta;
	AutoUTF	m_client;
	int		m_state;
};

///==================================================================================== WinTSessions
class	WinTS: public std::vector<WinTSInfo> {
public:
	WinTS() {
	}

	WinTS(const WinTSHandle &host) {
		Cache(host);
	}

	void Cache(const WinTSHandle &host);

	bool	FindSess(PCWSTR in) const;
	bool	FindUser(PCWSTR in) const;
};

#endif
