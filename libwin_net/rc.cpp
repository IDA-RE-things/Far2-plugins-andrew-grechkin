#include "rc.h"
#include "exception.h"

///========================================================================================= Mpr_dll
struct Mpr_dll: private DynamicLibrary {
	typedef DWORD (WINAPI *FWNetAddConnection2W)(LPNETRESOURCEW, LPCWSTR, LPCWSTR, DWORD);
	typedef DWORD (WINAPI *FWNetCancelConnection2W)(LPCWSTR, DWORD, WINBOOL);

	DEFINE_FUNC(WNetAddConnection2W);
	DEFINE_FUNC(WNetCancelConnection2W);

	static Mpr_dll & inst() {
		static Mpr_dll ret;
		return ret;
	}

private:
	Mpr_dll():
		DynamicLibrary(L"mpr.dll") {
		GET_DLL_FUNC(WNetAddConnection2W);
		GET_DLL_FUNC(WNetCancelConnection2W);
	}
};

static void MakeIPCstring(PCWSTR host, WCHAR ipc[], size_t size) {
	if (host[0] != PATH_SEPARATOR_C || host[1] != PATH_SEPARATOR_C)
		Copy(ipc, NETWORK_PATH_PREFIX, size);
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
			CheckApiError(Mpr_dll::inst().WNetAddConnection2W(&NetRes, pass, user, 0));
			m_host = host;
			m_conn = true;
			return;
		} else {
			CheckApiError(Mpr_dll::inst().WNetAddConnection2W(&NetRes, NULL, NULL, 0));
			m_host = host;
			return;
		}
		CheckApiError(ERROR_BAD_NETPATH);
	}
}

void RemoteConnection::Close() {
	if (m_conn) {
		WCHAR ipc[MAX_PATH];
		MakeIPCstring(m_host.c_str(), ipc, sizeofa(ipc));
		CheckApiError(Mpr_dll::inst().WNetCancelConnection2W(ipc, 0, FALSE));
		m_conn = false;
	}
	m_host.clear();
}

PCWSTR RemoteConnection::host() const {
	return m_host.c_str();
}
