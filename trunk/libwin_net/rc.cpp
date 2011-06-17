#include "win_net.h"

static void MakeIPCstring(PCWSTR host, WCHAR ipc[], size_t size) {
	if (host[0] != PATH_SEPARATOR_C || host[1] != PATH_SEPARATOR_C)
		Copy(ipc, NET_PREFIX, size);
	else
		ipc[0] = L'\0';
	Cat(ipc, host, size);
	Cat(ipc, L"\\IPC$", size);
}

///================================================================================ RemoteConnection
void RemoteConnection::Open(PCWSTR host, PCWSTR user, PCWSTR pass) {
	Close();
	if (host && !Empty(host)) {
		WCHAR ipc[MAX_PATH];
		MakeIPCstring(host, ipc, sizeofa(ipc));
		NETRESOURCE NetRes = {0};
		NetRes.dwType = RESOURCETYPE_ANY;
		NetRes.lpRemoteName = ipc;
		if (user && !Empty(user)) {
			CheckApiError(::WNetAddConnection2(&NetRes, pass, user, 0));
			m_host = host;
			m_conn = true;
			return;
		} else {
			CheckApiError(::WNetAddConnection2(&NetRes, NULL, NULL, 0));
			m_host = host;
			return;
		}
		throw WinError(ERROR_BAD_NETPATH);
	}
}

void RemoteConnection::Close() {
	if (m_conn) {
		WCHAR ipc[MAX_PATH];
		MakeIPCstring(m_host.c_str(), ipc, sizeofa(ipc));
		CheckApiError(::WNetCancelConnection2(ipc, 0, FALSE));
		m_conn = false;
	}
	m_host.clear();
}
