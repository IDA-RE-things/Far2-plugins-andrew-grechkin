#include <libbase/std.hpp>
#include <libext/dll.hpp>
#include <libext/rc.hpp>
#include <libext/exception.hpp>

using namespace Base;

namespace Ext {

	///===================================================================================== Mpr_dll
	namespace {
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

		void MakeIPCstring(PCWSTR host, WCHAR ipc_out[], size_t size) {
			PCWSTR prefix = (host[0] != PATH_SEPARATOR_C || host[1] != PATH_SEPARATOR_C) ? NETWORK_PATH_PREFIX : EMPTY_STR;
			_snwprintf(ipc_out, size, L"%s%s%s", prefix, host, L"\\IPC$");
		}
	}

	///============================================================================ RemoteConnection
	RemoteConnection::~RemoteConnection() {
		disconnect();
	}

	RemoteConnection::RemoteConnection(PCWSTR host, PCWSTR user, PCWSTR pass):
		m_connected(false) {
		connect(host, user, pass);
	}

	void RemoteConnection::connect(PCWSTR host, PCWSTR user, PCWSTR pass) {
		disconnect();
		if (!is_str_empty(host)) {
			WCHAR ipc[MAX_PATH];
			MakeIPCstring(host, ipc, lengthof(ipc));
			NETRESOURCE NetRes = {0};
			NetRes.dwType = RESOURCETYPE_ANY;
			NetRes.lpRemoteName = ipc;
			if (is_str_empty(user)) {
				user = nullptr;
				pass = nullptr;
			}
			CheckApiError(Mpr_dll::inst().WNetAddConnection2W(&NetRes, pass, user, 0));
			m_host = host;
			m_connected = true;
		}
		//	CheckApiError(ERROR_BAD_NETPATH);
	}


	void RemoteConnection::disconnect() {
		if (m_connected) {
			WCHAR ipc[MAX_PATH];
			MakeIPCstring(m_host.c_str(), ipc, sizeofa(ipc));
			CheckApiError(Mpr_dll::inst().WNetCancelConnection2W(ipc, 0, FALSE));
			m_connected = false;
		}
		m_host.clear();
	}

	PCWSTR RemoteConnection::get_host() const {
		return m_host.c_str();
	}

}
