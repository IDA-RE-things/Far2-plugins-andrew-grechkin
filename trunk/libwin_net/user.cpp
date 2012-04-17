/** win_user
	@classes to manipulate Users
	@author © 2012 Andrew Grechkin
	@link (netapi32)
**/

#include "user.h"
#include "exception.h"
#include "sid.h"

#include <lm.h>

void set_pass_length(size_t len, PCWSTR srv) {
	USER_MODALS_INFO_1001 info = {(DWORD)len};
	CheckApiError(::NetUserModalsSet(srv, 1001, (PBYTE)&info, nullptr));
}

void set_pass_age(size_t age, PCWSTR srv) {
	USER_MODALS_INFO_1002 info = {(DWORD)age};
	CheckApiError(::NetUserModalsSet(srv, 1002, (PBYTE)&info, nullptr));
}

struct UserBuf {
	~UserBuf() {
		if (info)
			::NetApiBufferFree(info);
	}

	UserBuf(const ustring & name, const ustring & dom = ustring()) {
		CheckApiError(::NetUserGetInfo(dom.c_str(), name.c_str(), 3, (PBYTE*)&info));
	}

	void set(const ustring & name, const ustring & dom = ustring()) {
		CheckApiError(::NetUserSetInfo(dom.c_str(), name.c_str(), 3, (PBYTE)info, nullptr));
	}

	PUSER_INFO_3 data() const {
		return info;
	}

	PUSER_INFO_3 operator ->() const {
		return info;
	}

private:
	PUSER_INFO_3 info;
};

///============================================================================================ User
bool User::is_exist(const ustring & name, const ustring & dom) {
	const DWORD dwLevel = 0;
	LPUSER_INFO_0 info = nullptr;
	NET_API_STATUS err = ::NetUserGetInfo(dom.c_str(), name.c_str(), dwLevel, (PBYTE*) & info);
	if (info)
		::NetApiBufferFree(info);
	return err == NERR_Success;
}

bool User::is_disabled(const ustring & name, const ustring & dom) {
	UserBuf info(name, dom);
	return info->usri3_flags & UF_ACCOUNTDISABLE;
}

bool User::is_expired(const ustring & name, const ustring & dom) {
	UserBuf info(name, dom);
	return info->usri3_password_expired;
}

ustring User::get_comm(const ustring & name, const ustring & dom) {
	UserBuf info(name, dom);
	return ustring(info->usri3_usr_comment);
}

ustring User::get_desc(const ustring & name, const ustring & dom) {
	UserBuf info(name, dom);
	return ustring(info->usri3_comment);
}

ustring User::get_fname(const ustring & name, const ustring & dom) {
	UserBuf info(name, dom);
	return ustring(info->usri3_full_name);
}

ustring User::get_home(const ustring & name, const ustring & dom) {
	UserBuf info(name, dom);
	return ustring(info->usri3_home_dir);
}

ustring User::get_params(const ustring & name, const ustring & dom) {
	UserBuf info(name, dom);
	return ustring(info->usri3_profile);
}

ustring User::get_profile(const ustring & name, const ustring & dom) {
	UserBuf info(name, dom);
	return ustring(info->usri3_profile);
}

ustring User::get_script(const ustring & name, const ustring & dom) {
	UserBuf info(name, dom);
	return ustring(info->usri3_script_path);
}

ustring User::get_workstations(const ustring & name, const ustring & dom) {
	UserBuf info(name, dom);
	return ustring(info->usri3_workstations);
}

DWORD User::get_flags(const ustring & name, const ustring & dom) {
	UserBuf info(name, dom);
	return info->usri3_flags;
}

DWORD User::get_uid(const ustring & name, const ustring & dom) {
	UserBuf info(name, dom);
	return info->usri3_user_id;
}

void User::add(const ustring & name, const ustring & pass, const ustring & dom) {
	DWORD dwLevel = 1;
	USER_INFO_1 info = {0};
	info.usri1_name = const_cast<WCHAR*>(name.c_str());
	info.usri1_password = const_cast<WCHAR*>(pass.c_str());
	info.usri1_priv = USER_PRIV_USER;
	info.usri1_flags = UF_SCRIPT | UF_NORMAL_ACCOUNT | UF_DONT_EXPIRE_PASSWD;
	if (pass.empty())
		info.usri1_flags |= UF_PASSWD_NOTREQD;
	CheckApiError(::NetUserAdd(dom.c_str(), dwLevel, (PBYTE)&info, nullptr));
}

void User::del(const ustring & name, const ustring & dom) {
	CheckApiError(::NetUserDel(dom.c_str(), name.c_str()));
}

void User::disable(const ustring & name, const ustring & dom) {
	set_flags(name, UF_ACCOUNTDISABLE, true, dom);
}

void User::enable(const ustring & name, const ustring & dom) {
	set_flags(name, UF_ACCOUNTDISABLE, false, dom);
}

static void set_info(const ustring & name, const ustring & dom, DWORD level, PVOID info) {
	CheckApiError(::NetUserSetInfo(dom.c_str(), name.c_str(), level, (PBYTE)info, nullptr));
}

void User::set_expire(const ustring & name, bool in, const ustring & dom) {
	const DWORD level = 1017;
	USER_INFO_1017 info;
	info.usri1017_acct_expires = (in) ? 1 : 0;
	set_info(name, dom, level, &info);
}

void User::set_name(const ustring & name, const ustring & in, const ustring & dom) {
	const DWORD level = 0;
	USER_INFO_0 info;
	info.usri0_name = const_cast<PWSTR>(in.c_str());
	set_info(name, dom, level, &info);
}

void User::set_pass(const ustring & name, const ustring & in, const ustring & dom) {
	const DWORD level = 1003;
	USER_INFO_1003	info;
	info.usri1003_password = const_cast<PWSTR>(in.c_str());
	set_info(name, dom, level, &info);
}

void User::set_desc(const ustring & name, const ustring & in, const ustring & dom) {
	const DWORD level = 1007;
	USER_INFO_1007 info;
	info.usri1007_comment = const_cast<PWSTR>(in.c_str());
	set_info(name, dom, level, &info);
}

void User::set_fname(const ustring & name, const ustring & in, const ustring & dom) {
	const DWORD level = 1011;
	USER_INFO_1011 info;
	info.usri1011_full_name = const_cast<PWSTR>(in.c_str());
	set_info(name, dom, level, &info);
}

void User::set_comm(const ustring & name, const ustring & in, const ustring & dom) {
	const DWORD level = 1012;
	USER_INFO_1012 info;
	info.usri1012_usr_comment = const_cast<PWSTR>(in.c_str());
	set_info(name, dom, level, &info);
}

void User::set_home(const ustring & name, const ustring & in, const ustring & dom) {
	const DWORD level = 1006;
	USER_INFO_1006 info;
	info.usri1006_home_dir = const_cast<PWSTR>(in.c_str());
	set_info(name, dom, level, &info);
}

void User::set_profile(const ustring & name, const ustring & in, const ustring & dom) {
	const DWORD level = 1052;
	USER_INFO_1052 info;
	info.usri1052_profile = const_cast<PWSTR>(in.c_str());
	set_info(name, dom, level, &info);
}

void User::set_script(const ustring & name, const ustring & in, const ustring & dom) {
	const DWORD level = 1009;
	USER_INFO_1009 info;
	info.usri1009_script_path = const_cast<PWSTR>(in.c_str());
	set_info(name, dom, level, &info);
}

void User::set_flags(const ustring & name, DWORD in, bool value, const ustring & dom) {
	const DWORD level = 1008;
	DWORD dwFlags = get_flags(name, dom);
	if (value)
		WinFlag::Set(dwFlags, in);
	else
		WinFlag::UnSet(dwFlags, in);
	USER_INFO_1008 info;
	info.usri1008_flags = dwFlags;
	set_info(name, dom, level, &info);
}

///======================================================================================== UserInfo
//UserInfo::UserInfo():
//	priv(0),
//	flags(0) {
//}

UserInfo::UserInfo(const ustring & n) :
	name(n),
	priv(0),
	flags(0) {
}

UserInfo::UserInfo(PVOID info):
	name(((PUSER_INFO_3)info)->usri3_name),
	desc(((PUSER_INFO_3)info)->usri3_comment),
	fname(((PUSER_INFO_3)info)->usri3_full_name),
	comm(((PUSER_INFO_3)info)->usri3_usr_comment),
	prof(((PUSER_INFO_3)info)->usri3_profile),
	home(((PUSER_INFO_3)info)->usri3_home_dir),
	script(((PUSER_INFO_3)info)->usri3_script_path),
	priv(((PUSER_INFO_3)info)->usri3_priv),
	flags(((PUSER_INFO_3)info)->usri3_flags) {
}

bool UserInfo::operator<(const UserInfo &rhs) const {
	return name < rhs.name;
}

bool UserInfo::operator==(const ustring & nm) const {
	return this->name == nm;
}

bool UserInfo::is_admin() const {
	return priv == USER_PRIV_ADMIN;
}

bool UserInfo::is_disabled() const {
	return flags & UF_ACCOUNTDISABLE;
}

///======================================================================================== WinUsers
WinUsers::WinUsers(bool autocache) {
	if (autocache)
		cache();
}

bool WinUsers::cache(const ustring & dom) {
	const DWORD dwLevel = 3, dwPrefMaxLen = MAX_PREFERRED_LENGTH;
	DWORD dwEntriesRead = 0, dwTotalEntries = 0, dwResumeHandle = 0;
	NET_API_STATUS nStatus;
	clear();
	m_group.clear();
	m_dom = dom;
	do {
		PUSER_INFO_3 info = nullptr;
		nStatus = ::NetUserEnum(m_dom.c_str(), dwLevel, FILTER_NORMAL_ACCOUNT, (PBYTE*) &info,
								dwPrefMaxLen, &dwEntriesRead, &dwTotalEntries, &dwResumeHandle);
		if (NERR_Success == nStatus || ERROR_MORE_DATA == nStatus) {
			PUSER_INFO_3 ptr = info;
			for (DWORD i = 0; i < dwEntriesRead && ptr; ++i, ++ptr) {
				push_back(UserInfo(ptr));
			}
			::NetApiBufferFree(info);
		}
	} while (nStatus == ERROR_MORE_DATA);
	return (nStatus == NERR_Success);
}

bool WinUsers::cache_by_priv(DWORD priv, const ustring & dom) {
	const DWORD dwLevel = 3, dwPrefMaxLen = MAX_PREFERRED_LENGTH;
	DWORD dwEntriesRead = 0, dwTotalEntries = 0, dwResumeHandle = 0;
	NET_API_STATUS nStatus;
	clear();
	m_group.clear();
	m_dom = dom;
	do {
		PUSER_INFO_3 info = nullptr;
		nStatus = ::NetUserEnum(m_dom.c_str(), dwLevel, FILTER_NORMAL_ACCOUNT, (PBYTE*) &info,
								dwPrefMaxLen, &dwEntriesRead, &dwTotalEntries, &dwResumeHandle);
		if (NERR_Success == nStatus || ERROR_MORE_DATA == nStatus) {
			PUSER_INFO_3 ptr = info;
			for (DWORD i = 0; i < dwEntriesRead && ptr; ++i, ++ptr) {
				if (priv != ptr->usri3_priv) {
					continue;
				}
				push_back(UserInfo(ptr));
			}
			::NetApiBufferFree(info);
		}
	} while (ERROR_MORE_DATA == nStatus);
	return (NERR_Success == nStatus);
}

bool WinUsers::cache_by_group(const ustring & group, const ustring & dom) {
	// Cache members of group "name".
	const DWORD dwLevel = 1, dwPrefMaxLen = MAX_PREFERRED_LENGTH;
	DWORD dwEntriesRead = 0, dwTotalEntries = 0;
	ULONG_PTR dwResumeHandle = 0;
	NET_API_STATUS nStatus;
	clear();
	m_group = group;
	m_dom = dom;
	do {
		PLOCALGROUP_MEMBERS_INFO_1 info = nullptr;
		nStatus = ::NetLocalGroupGetMembers(m_dom.c_str(), group.c_str(), dwLevel, (PBYTE*) &info,
											dwPrefMaxLen, &dwEntriesRead, &dwTotalEntries, &dwResumeHandle);
		if (NERR_Success == nStatus || ERROR_MORE_DATA == nStatus) {
			PLOCALGROUP_MEMBERS_INFO_1 ptr = info;
			for (DWORD i = 0; i < dwEntriesRead && ptr; ++i, ++ptr) {
				push_back(UserInfo(UserBuf(ptr->lgrmi1_name).data()));
			}
			::NetApiBufferFree(info);
		}
	} while (ERROR_MORE_DATA == nStatus);
	return (NERR_Success == nStatus);
}

bool WinUsers::cache_by_gid(const ustring & gid, const ustring & dom) {
	return cache_by_group(SidString(gid).get_name(), dom);
}

WinUsers::iterator WinUsers::find(const ustring & name) {
	return std::find(begin(), end(), name);
}

void WinUsers::add(const ustring & name, const ustring & pass) {
	User::add(name, pass);
	push_back(UserInfo(UserBuf(name).data()));
	if (!m_group.empty())
		Group::add_member(m_group, Sid(name));
}

void WinUsers::del(const ustring & name) {
	iterator it = find(name);
	if (it != end())
		del(it);
}

void WinUsers::del(iterator it) {
	User::del(it->name);
	erase(it);
}

void WinUsers::rename(const ustring & name, const ustring & new_name) {
	iterator it = find(name);
	if (it != end())
		rename(it, new_name);
}

void WinUsers::rename(iterator it, const ustring & new_name) {
	User::set_name(it->name, new_name);
	it->name = new_name;
}

WinUsersByPriv::WinUsersByPriv(DWORD priv, const ustring & dom):
	WinUsers(false) {
	cache_by_priv(priv, dom);
}

WinUsersByGroup::WinUsersByGroup(const ustring & group, const ustring & dom):
	WinUsers(false) {
	cache_by_group(group, dom);
}

WinUsersByGid::WinUsersByGid(const ustring & gid, const ustring & dom):
	WinUsers(false) {
	cache_by_gid(gid, dom);
}
