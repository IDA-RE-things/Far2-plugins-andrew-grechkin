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
	if (host && !WinStr::Empty(host)) {
		WCHAR	ipc[MAX_PATH];
		if (host[0] != PATH_SEPARATOR_C || host[1] != PATH_SEPARATOR_C)
			WinStr::Cat(ipc, NET_PREFIX, sizeofa(ipc));
		WinStr::Cat(ipc, host, sizeofa(ipc));
		if (Test(host) || Test(ipc)) {
			m_host = host;
			return;
		}
		if (user && !WinStr::Empty(user)) {
			WinStr::Cat(ipc, L"\\IPC$", sizeofa(ipc));

			NETRESOURCE	NetRes = {0};
			NetRes.dwType = RESOURCETYPE_ANY;
			NetRes.lpRemoteName = ipc;

			DWORD	err = ::WNetAddConnection2(&NetRes, pass, user, 0);
			if (err != ERROR_SUCCESS) {
				m_host.clear();
				throw	ActionError(err);
			}
			m_host = host;
			m_conn = true;
			return;
		}
		throw	ActionError(ERROR_ACCESS_DENIED);
	}
	m_host.clear();
}
void			RemoteConnection::Close() {
	if (m_conn) {
		WCHAR	ipc[MAX_PATH];
		if (m_host[0] != PATH_SEPARATOR_C || m_host[1] != PATH_SEPARATOR_C)
			WinStr::Cat(ipc, NET_PREFIX, sizeofa(ipc));
		WinStr::Cat(ipc, m_host.c_str(), sizeofa(ipc));
		WinStr::Cat(ipc, L"\\IPC$", sizeofa(ipc));
		DWORD	err = ::WNetCancelConnection2(ipc, 0, FALSE);
		if (err != NO_ERROR) {
			throw	ActionError(err);
		}
		m_host.clear();
		m_conn = false;
	}
}
bool			RemoteConnection::Test(PCWSTR host) {
	SC_HANDLE	hSC = ::OpenSCManager(host, NULL, SC_MANAGER_CONNECT);
	if (hSC != NULL) {
		::CloseServiceHandle(hSC);
		return	true;
	}
	return	false;
}
