#include "win_net.h"

///================================================================================ RemoteConnection
RemoteConnection::~RemoteConnection() {
	Close();
}
RemoteConnection::RemoteConnection(PCWSTR host): m_conn(false) {
	Open(host);
}
void			RemoteConnection::Open(PCWSTR host, PCWSTR user, PCWSTR pass) {
	Close();
	if (host && !Empty(host)) {
		WCHAR	ipc[MAX_PATH] = {0};
		if (host[0] != PATH_SEPARATOR_C || host[1] != PATH_SEPARATOR_C)
			Cat(ipc, NET_PREFIX, sizeofa(ipc));
		Cat(ipc, host, sizeofa(ipc));
		if (TestConn(host) || TestConn(ipc)) {
			m_host = host;
			return;
		}
		if (user && !Empty(user)) {
			Cat(ipc, L"\\IPC$", sizeofa(ipc));

			NETRESOURCE	NetRes = {0};
			NetRes.dwType = RESOURCETYPE_ANY;
			NetRes.lpRemoteName = ipc;
			CheckError(::WNetAddConnection2(&NetRes, pass, user, 0));
			m_host = host;
			m_conn = true;
			return;
		}
		throw	ApiError(ERROR_ACCESS_DENIED);
	}
	m_host.clear();
}
void			RemoteConnection::Close() {
	if (m_conn) {
		WCHAR	ipc[MAX_PATH] = {0};
		if (m_host[0] != PATH_SEPARATOR_C || m_host[1] != PATH_SEPARATOR_C)
			Cat(ipc, NET_PREFIX, sizeofa(ipc));
		Cat(ipc, m_host.c_str(), sizeofa(ipc));
		Cat(ipc, L"\\IPC$", sizeofa(ipc));
		CheckError(::WNetCancelConnection2(ipc, 0, FALSE));
		m_host.clear();
		m_conn = false;
	}
}
bool			RemoteConnection::TestConn(PCWSTR host) const {
	SC_HANDLE	hSC = ::OpenSCManager(host, null_ptr, SC_MANAGER_CONNECT);
	if (hSC != null_ptr) {
		::CloseServiceHandle(hSC);
		return	true;
	}
	return	false;
}
