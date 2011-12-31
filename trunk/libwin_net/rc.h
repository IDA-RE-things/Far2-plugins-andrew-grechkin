#ifndef WIN_NET_RC_HPP
#define WIN_NET_RC_HPP

#include "win_net.h"

///▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒ net_rc
///================================================================================ RemoteConnection
struct RemoteConnection {
	~RemoteConnection() {
		Close();
	}

	RemoteConnection(PCWSTR host = nullptr, PCWSTR user = nullptr, PCWSTR pass = nullptr): m_conn(false) {
		Open(host, user, pass);
	}

	void Open(PCWSTR host, PCWSTR user = nullptr, PCWSTR pass = nullptr);

	void Close();

	PCWSTR host() const;

private:
	ustring	m_host;
	bool	m_conn;
};

#endif
