#include <libext/rc.hpp>
#include <libext/dll.hpp>
#include <libext/exception.hpp>
#include <libbase/std.hpp>
#include <libbase/logger.hpp>
#include <libbase/str.hpp>

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

		ustring MakeIPCstring(const ustring & host) {
			PCWSTR prefix = (host[0] != PATH_SEPARATOR_C || host[1] != PATH_SEPARATOR_C) ? NETWORK_PATH_PREFIX : EMPTY_STR;
			wchar_t ipc[MAX_PATH]; ipc[0] = 0;
			_snwprintf(ipc, Base::lengthof(ipc), L"%s%s%s", prefix, host.c_str(), L"\\IPC$");
			return ustring(ipc);
		}
	}

	///============================================================================ RemoteConnection
	RemoteConnection::~RemoteConnection() {
		LogTrace();
		disconnect();
	}

	RemoteConnection::RemoteConnection(const ustring & host, PCWSTR user, PCWSTR pass):
		m_connected(false) {
		LogTrace();
		connect(host, user, pass);
	}

	void RemoteConnection::connect(const ustring & host, PCWSTR user, PCWSTR pass) {
		if (this == nullptr)
			return;
		disconnect();
		if (!host.empty()) {
			LogTrace();
			ustring ipc = MakeIPCstring(host);
			NETRESOURCE NetRes = {0};
			NetRes.dwType = RESOURCETYPE_ANY;
			NetRes.lpRemoteName = (PWSTR)ipc.c_str();
			if (Str::is_empty(user)) {
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
		if (this == nullptr)
			return;
		if (m_connected) {
			LogTrace();
			ustring ipc = MakeIPCstring(m_host);
			CheckApiError(Mpr_dll::inst().WNetCancelConnection2W(ipc.c_str(), 0, FALSE));
			m_connected = false;
		}
		m_host.clear();
	}

	ustring RemoteConnection::get_host() const {
		if (this == nullptr)
			return ustring();
		return m_host;
	}

}
