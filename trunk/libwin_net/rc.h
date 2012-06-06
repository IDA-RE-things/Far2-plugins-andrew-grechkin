#ifndef WIN_NET_RC_HPP
#define WIN_NET_RC_HPP

#include "win_net.h"

///▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒ net_rc
///================================================================================ RemoteConnection
struct RemoteConnection {
	~RemoteConnection();

	RemoteConnection(PCWSTR host = nullptr, PCWSTR user = nullptr, PCWSTR pass = nullptr);

	void connect(PCWSTR host, PCWSTR user = nullptr, PCWSTR pass = nullptr);

	void disconnect();

	PCWSTR get_host() const;

private:
	ustring m_host;
	bool m_connected;
};

#endif
