/**
	win_ts

	@classes	()
	@author		© 2009 Andrew Grechkin
	@link		()
**/

#ifndef WIN_TS_HPP
#define WIN_TS_HPP

#include "win_net.h"

#include <wtsapi32.h>

///▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒ net_ts
///===================================================================================== WinTSHandle
struct		WinTSHandle {
	~WinTSHandle();

	WinTSHandle(PCWSTR host = nullptr);

	WinTSHandle(shared_ptr<RemoteConnection> conn);

	operator		HANDLE() const {
		return	m_ts;
	}

private:
	HANDLE		m_ts;
};

///===================================================================================== WinTSession
namespace	WinTSession {
	void	ConnectLocal(DWORD id, PCWSTR pass = L"");

	void	ConnectRemote(DWORD id, shared_ptr<RemoteConnection> host);

	void	Disconnect(DWORD id, PCWSTR host = L"");

	void	Disconnect(DWORD id, shared_ptr<RemoteConnection> host);

	void	LogOff(DWORD id, PCWSTR host = L"");

	void	LogOff(DWORD id, shared_ptr<RemoteConnection> host);

	DWORD	Question(DWORD id, PCWSTR ttl, PCWSTR msg, DWORD time = 60, PCWSTR host = L"");

	DWORD	Question(DWORD id, shared_ptr<RemoteConnection> host, PCWSTR ttl, PCWSTR msg, DWORD time = 60);

	DWORD	Message(DWORD id, PCWSTR ttl, PCWSTR msg, DWORD time = 60, bool wait = true, PCWSTR host = L"");

	DWORD	Message(DWORD id, shared_ptr<RemoteConnection> host, PCWSTR ttl, PCWSTR msg, DWORD time = 60, bool wait = true);

	void	Reboot(PCWSTR host);

	void	Reboot(shared_ptr<RemoteConnection> host);

	void	Turnoff(PCWSTR host);

	void	Turnoff(shared_ptr<RemoteConnection> host);
};

///======================================================================================= WinTSInfo
struct		WinTSInfo {
	DWORD					id;
	AutoUTF					sess;
	AutoUTF					user;
	AutoUTF					winSta;
	AutoUTF					client;
	WTS_CONNECTSTATE_CLASS	state;

	WinTSInfo(DWORD i, const AutoUTF &s, const AutoUTF &u, WTS_CONNECTSTATE_CLASS st): id(i), sess(s), user(u), state(st) {
	}

	PCWSTR			GetState() const {
		return	ParseState(state);
	}

	PCWSTR			ParseState(WTS_CONNECTSTATE_CLASS st) const;

	PCWSTR			ParseStateFull(WTS_CONNECTSTATE_CLASS st) const;
};

///==================================================================================== WinTSessions
class		WinTS : public MapContainer<DWORD, WinTSInfo> {
public:
	~WinTS() {
		Clear();
	}

	void				Cache(shared_ptr<RemoteConnection> conn);

	AutoUTF				Info() const {
		AutoUTF	Result;
		Result += L"Id:           ";
		Result += Num2Str((size_t)Key());
		Result += L"\n\n";
		Result += L"User name:    ";
		Result += Value().user;
		Result += L"\n\n";
		Result += L"State:        ";
		Result += Value().GetState();
		Result += L"\n\n";
		Result += L"Session:      ";
		Result += Value().sess;
		Result += L"\n\n";
		Result += L"WinStation:   ";
		Result += Value().winSta;
		Result += L"\n\n";
		Result += L"Client:       ";
		Result += Value().client;
		Result += L"\n\n";
		return	Result;
	}

	bool				FindSess(PCWSTR in) const;

	bool				FindUser(PCWSTR in) const;
};

#endif // WIN_TS_HPP
