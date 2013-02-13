#include <libext/priv.hpp>
#include <libext/exception.hpp>

using namespace Base;

namespace Ext {

	///==================================================================================== WinToken
	WinToken::WinToken(ACCESS_MASK mask)
	{
		CheckApi(::OpenProcessToken(::GetCurrentProcess(), mask, &m_token));
	}

	WinToken::WinToken(HANDLE process, ACCESS_MASK mask)
	{
		CheckApi(::OpenProcessToken(process, mask, &m_token));
	}

	bool WinToken::check_membership(PSID sid, HANDLE token)
	{
		BOOL ret;
		CheckApi(::CheckTokenMembership(token, sid, &ret));
		return ret;
	}

	///===================================================================================== WinPriv
	namespace WinPriv {
		bool operator ==(const LUID & lhs, const LUID & rhs)
		{
			return lhs.LowPart == rhs.LowPart && lhs.HighPart == rhs.HighPart;
		}

		LUID as_luid(PCWSTR priv_name)
		{
			LUID luid;
			CheckApi(::LookupPrivilegeValueW(nullptr, priv_name, &luid));
			return luid;
		}

		bool is_exist(HANDLE token, const LUID & priv)
		{
			DWORD size = 0;
			::GetTokenInformation(token, TokenPrivileges, nullptr, 0, &size);
			CheckApi(::GetLastError() == ERROR_INSUFFICIENT_BUFFER);

			auto_buf<PTOKEN_PRIVILEGES> ptp(size);
			CheckApi(::GetTokenInformation(token, TokenPrivileges, ptp, ptp.size(), &size));

			for (DWORD i = 0; i < ptp->PrivilegeCount; ++i) {
				if (ptp->Privileges[i].Luid == priv) {
					return true;
				}
			}
			return false;
		}

		bool is_exist(HANDLE token, PCWSTR priv_name)
		{
			return is_exist(token, as_luid(priv_name));
		}

		bool is_exist(const LUID & priv)
		{
			return is_exist(WinToken(), priv);
		}

		bool is_exist(PCWSTR priv_name)
		{
			return is_exist(WinToken(), priv_name);
		}

		bool is_enabled(HANDLE token, const LUID & priv)
		{
			BOOL ret = false;
			PRIVILEGE_SET ps = {0};
			ps.PrivilegeCount = 1;
			ps.Privilege[0].Luid = priv;

			CheckApi(::PrivilegeCheck(token, &ps, &ret));
			return ret;
		}

		bool is_enabled(HANDLE token, PCWSTR priv_name)
		{
			return is_enabled(token, as_luid(priv_name));
		}

		bool is_enabled(const LUID & priv)
		{
			return is_enabled(WinToken(), priv);
		}

		bool is_enabled(PCWSTR priv_name)
		{
			return is_enabled(WinToken(), priv_name);
		}

		void modify(HANDLE token, const LUID & priv, bool enable)
		{
			TOKEN_PRIVILEGES tp = {0};
			tp.PrivilegeCount = 1;
			tp.Privileges[0].Luid = priv;
			tp.Privileges[0].Attributes = (enable) ? SE_PRIVILEGE_ENABLED : 0;
			CheckApi(::AdjustTokenPrivileges(token, false, &tp, sizeof(tp), nullptr, nullptr));
		}

		void modify(HANDLE token, PCWSTR priv_name, bool enable)
		{
			modify(token, as_luid(priv_name), enable);
		}

		void modify(const LUID & priv, bool enable)
		{
			modify(WinToken(TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES), priv, enable);
		}

		void modify(PCWSTR priv_name, bool enable)
		{
			modify(WinToken(TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES), priv_name, enable);
		}

		ustring get_name(PCWSTR priv_name)
		{
			DWORD size = 0, lang = 0;
			::LookupPrivilegeDisplayNameW(nullptr, priv_name, nullptr, &size, &lang);
			CheckApi(::GetLastError() == ERROR_INSUFFICIENT_BUFFER);
			auto_array<wchar_t> name(size);
			CheckApi(::LookupPrivilegeDisplayNameW(nullptr, priv_name, name, &size, &lang));
			return ustring(name);
		}
	}

	///=================================================================================== Privilege
	Privilege::~Privilege()
	{
		if (m_disable) {
			auto_close<HANDLE> token;
			if (::OpenProcessToken(::GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &token)) {
				m_tp.Privileges[0].Attributes = 0;
				::AdjustTokenPrivileges(token, false, &m_tp, sizeof(m_tp), nullptr, nullptr);
			}
		}
	}

	Privilege::Privilege(PCWSTR priv_name) :
		m_disable(false)
	{
		auto_close<HANDLE> token;
		if (::OpenProcessToken(::GetCurrentProcess(), TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES, &token)) {
			BOOL Result = false;
			PRIVILEGE_SET ps = {0};
			ps.PrivilegeCount = 1;
			ps.Privilege[0].Luid = WinPriv::as_luid(priv_name);
			if (::PrivilegeCheck(token, &ps, &Result) && !Result) {
				m_tp.PrivilegeCount = ps.PrivilegeCount;
				m_tp.Privileges[0].Luid = ps.Privilege[0].Luid;
				m_tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
				m_disable = ::AdjustTokenPrivileges(token, false, &m_tp, sizeof(m_tp), nullptr, nullptr);
			}
		}
	}

}
