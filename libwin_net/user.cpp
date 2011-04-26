/** win_user
 *	@classes to manipulate Users
 *	@author GrAnD, 2009
 *	@link (netapi32)
**/

#include "user.h"

#include <lm.h>

void set_pass_length(size_t len, PCWSTR srv) {
	USER_MODALS_INFO_1001 info = {len};
	CheckApiError(::NetUserModalsSet(srv, 1001, (PBYTE)&info, nullptr));
}

void set_pass_age(size_t age, PCWSTR srv) {
	USER_MODALS_INFO_1002 info = {age};
	CheckApiError(::NetUserModalsSet(srv, 1002, (PBYTE)&info, nullptr));
}

///========================================================================================= NetUser
class		UserBuf {
public:
	~UserBuf() {
		if (info)
			::NetApiBufferFree(info);
	}

	UserBuf(const AutoUTF &name, const AutoUTF &dom = AutoUTF()) {
		CheckApiError(::NetUserGetInfo(dom.c_str(), name.c_str(), 3, (PBYTE*)&info));
	}

	void			Set(const AutoUTF &name, const AutoUTF &dom = AutoUTF()) {
		CheckApiError(::NetUserSetInfo(dom.c_str(), name.c_str(), 3, (PBYTE)info, nullptr));
	}

	LPUSER_INFO_3	operator->() const {
		return	info;
	}

	LPUSER_INFO_3	data() const {
		return	info;
	}
private:
	LPUSER_INFO_3	info;
};

bool	NetUser::IsExist(const AutoUTF &name, const AutoUTF &dom) {
	const DWORD dwLevel = 0;
	LPUSER_INFO_0	info = nullptr;
	NET_API_STATUS	err = ::NetUserGetInfo(dom.c_str(), name.c_str(), dwLevel, (PBYTE*) & info);
	if (info)
		::NetApiBufferFree(info);
	return	err == NERR_Success;
}
bool	NetUser::IsDisabled(const AutoUTF &name, const AutoUTF &dom) {
	UserBuf	info(name, dom);
	return	info->usri3_flags & UF_ACCOUNTDISABLE;
}
bool	NetUser::IsExpired(const AutoUTF &name, const AutoUTF &dom) {
	UserBuf	info(name, dom);
	return	info->usri3_password_expired;
}

AutoUTF	NetUser::GetComm(const AutoUTF &name, const AutoUTF &dom) {
	UserBuf	info(name, dom);
	return	AutoUTF(info->usri3_usr_comment);
}
AutoUTF	NetUser::GetDesc(const AutoUTF &name, const AutoUTF &dom) {
	UserBuf	info(name, dom);
	return	AutoUTF(info->usri3_comment);
}
AutoUTF	NetUser::GetFName(const AutoUTF &name, const AutoUTF &dom) {
	UserBuf	info(name, dom);
	return	AutoUTF(info->usri3_full_name);
}
AutoUTF	NetUser::GetHome(const AutoUTF &name, const AutoUTF &dom) {
	UserBuf	info(name, dom);
	return	AutoUTF(info->usri3_home_dir);
}
AutoUTF	NetUser::GetParams(const AutoUTF &name, const AutoUTF &dom) {
	UserBuf	info(name, dom);
	return	AutoUTF(info->usri3_profile);
}
AutoUTF	NetUser::GetProfile(const AutoUTF &name, const AutoUTF &dom) {
	UserBuf	info(name, dom);
	return	AutoUTF(info->usri3_profile);
}
AutoUTF	NetUser::GetScript(const AutoUTF &name, const AutoUTF &dom) {
	UserBuf	info(name, dom);
	return	AutoUTF(info->usri3_script_path);
}
AutoUTF	NetUser::GetWorkstations(const AutoUTF &name, const AutoUTF &dom) {
	UserBuf	info(name, dom);
	return	AutoUTF(info->usri3_workstations);
}
DWORD	NetUser::GetFlags(const AutoUTF &name, const AutoUTF &dom) {
	UserBuf	info(name, dom);
	return	info->usri3_flags;
}
DWORD	NetUser::GetUID(const AutoUTF &name, const AutoUTF &dom) {
	UserBuf	info(name, dom);
	return	info->usri3_user_id;
}

void	NetUser::Add(const AutoUTF &name, const AutoUTF &pass, const AutoUTF &dom) {
	DWORD		dwLevel	= 1;
	USER_INFO_1	info = {0};
	info.usri1_name = const_cast<WCHAR*>(name.c_str());
	info.usri1_password = const_cast<WCHAR*>(pass.c_str());
	info.usri1_priv = USER_PRIV_USER;
	info.usri1_flags = UF_SCRIPT | UF_NORMAL_ACCOUNT | UF_DONT_EXPIRE_PASSWD;
	if (pass.empty())
		info.usri1_flags |= UF_PASSWD_NOTREQD;
	CheckApiError(::NetUserAdd(dom.c_str(), dwLevel, (PBYTE)&info, nullptr));
}
void	NetUser::Del(const AutoUTF &name, const AutoUTF &dom) {
	CheckApiError(::NetUserDel(dom.c_str(), name.c_str()));
}
void	NetUser::Disable(const AutoUTF &name, const AutoUTF &dom) {
	SetFlags(name, UF_ACCOUNTDISABLE, true, dom);
}
void	NetUser::Enable(const AutoUTF &name, const AutoUTF &dom) {
	SetFlags(name, UF_ACCOUNTDISABLE, false, dom);
}

static void set_info(const AutoUTF &name, const AutoUTF &dom, DWORD level, PVOID info) {
	CheckApiError(::NetUserSetInfo(dom.c_str(), name.c_str(), level, (PBYTE)info, nullptr));
}

void	NetUser::SetExpire(const AutoUTF &name, bool in, const AutoUTF &dom) {
	const DWORD level = 1017;
	USER_INFO_1017 info;
	info.usri1017_acct_expires = (in) ? 1 : 0;
	set_info(name, dom, level, &info);
}
void	NetUser::SetName(const AutoUTF &name, const AutoUTF &in, const AutoUTF &dom) {
	const DWORD level = 0;
	USER_INFO_0	info;
	info.usri0_name = const_cast<PWSTR>(in.c_str());
	set_info(name, dom, level, &info);
}
void	NetUser::SetPass(const AutoUTF &name, const AutoUTF &in, const AutoUTF &dom) {
	const DWORD level = 1003;
	USER_INFO_1003	info;
	info.usri1003_password = const_cast<PWSTR>(in.c_str());
	set_info(name, dom, level, &info);
}
void	NetUser::SetDesc(const AutoUTF &name, const AutoUTF &in, const AutoUTF &dom) {
	const DWORD level = 1007;
	USER_INFO_1007	info;
	info.usri1007_comment = const_cast<PWSTR>(in.c_str());
	set_info(name, dom, level, &info);
}
void	NetUser::SetFName(const AutoUTF &name, const AutoUTF &in, const AutoUTF &dom) {
	const DWORD level = 1011;
	USER_INFO_1011	info;
	info.usri1011_full_name = const_cast<PWSTR>(in.c_str());
	set_info(name, dom, level, &info);
}
void	NetUser::SetComm(const AutoUTF &name, const AutoUTF &in, const AutoUTF &dom) {
	const DWORD level = 1012;
	USER_INFO_1012	info;
	info.usri1012_usr_comment = const_cast<PWSTR>(in.c_str());
	set_info(name, dom, level, &info);
}
void	NetUser::SetHome(const AutoUTF &name, const AutoUTF &in, const AutoUTF &dom) {
	const DWORD level = 1006;
	USER_INFO_1006	info;
	info.usri1006_home_dir = const_cast<PWSTR>(in.c_str());
	set_info(name, dom, level, &info);
}
void	NetUser::SetProfile(const AutoUTF &name, const AutoUTF &in, const AutoUTF &dom) {
	const DWORD level = 1052;
	USER_INFO_1052	info;
	info.usri1052_profile = const_cast<PWSTR>(in.c_str());
	set_info(name, dom, level, &info);
}
void	NetUser::SetScript(const AutoUTF &name, const AutoUTF &in, const AutoUTF &dom) {
	const DWORD level = 1009;
	USER_INFO_1009	info;
	info.usri1009_script_path = const_cast<PWSTR>(in.c_str());
	set_info(name, dom, level, &info);
}
void	NetUser::SetFlags(const AutoUTF &name, DWORD in, bool value, const AutoUTF &dom) {
	const DWORD level = 1008;
	DWORD	dwFlags	= GetFlags(name, dom);
	if (value)
		WinFlag::Set(dwFlags, in);
	else
		WinFlag::UnSet(dwFlags, in);
	USER_INFO_1008	info;
	info.usri1008_flags	= dwFlags;
	set_info(name, dom, level, &info);
}

///======================================================================================== SysUsers
//UserInfo::UserInfo():
//	priv(0),
//	flags(0) {
//}

UserInfo::UserInfo(const AutoUTF &n) :
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

bool UserInfo::operator==(const AutoUTF &nm) const {
	return this->name == nm;
}

bool UserInfo::is_admin() const {
	return priv == USER_PRIV_ADMIN;
}

bool UserInfo::is_disabled() const {
	return flags & UF_ACCOUNTDISABLE;
}

SysUsers::SysUsers(bool autocache) {
	if (autocache)
		Cache();
}

bool				SysUsers::Cache(const AutoUTF &dom) {
	// Cache all users.
	DWORD dwLevel = 3;
	DWORD dwPrefMaxLen = MAX_PREFERRED_LENGTH;
	DWORD dwEntriesRead = 0;
	DWORD dwTotalEntries = 0;
	DWORD dwResumeHandle = 0;
	NET_API_STATUS nStatus;
	Clear();
	this->dom = dom;
	do {
		LPUSER_INFO_3 info = nullptr;
		nStatus = ::NetUserEnum(dom.c_str(),
								dwLevel,
								FILTER_NORMAL_ACCOUNT,
								(PBYTE*) & info,
								dwPrefMaxLen,
								&dwEntriesRead,
								&dwTotalEntries,
								&dwResumeHandle);
		if (NERR_Success == nStatus || ERROR_MORE_DATA == nStatus) {
			for (DWORD i = 0; i < dwEntriesRead; ++i) {
				Insert(info[i].usri3_name, UserInfo(info));
			}
			::NetApiBufferFree(info);
		}
	} while (nStatus == ERROR_MORE_DATA);
	return	(nStatus == NERR_Success);
}
bool				SysUsers::CacheByPriv(DWORD priv, const AutoUTF &dom) {
	// Cache users by priveleges.
	LPUSER_INFO_3 info, infoTmp;
	DWORD dwLevel = 3;
	DWORD dwPrefMaxLen = MAX_PREFERRED_LENGTH;
	DWORD dwEntriesRead = 0;
	DWORD dwTotalEntries = 0;
	DWORD dwResumeHandle = 0;
	NET_API_STATUS nStatus;
	Clear();
	this->dom = dom;
	do {
		info = infoTmp = nullptr;
		nStatus = ::NetUserEnum(dom.c_str(),
								dwLevel,
								FILTER_NORMAL_ACCOUNT, // global users
								(PBYTE*) & info,
								dwPrefMaxLen,
								&dwEntriesRead,
								&dwTotalEntries,
								&dwResumeHandle);
		if (NERR_Success == nStatus || ERROR_MORE_DATA == nStatus) {
			infoTmp = info;
			for (DWORD i = 0; (i < dwEntriesRead) && (infoTmp != nullptr); ++i, ++infoTmp) {
				if (priv != infoTmp->usri3_priv)
					continue;
				Insert(infoTmp->usri3_name, UserInfo(info));
			}
		}
		if (info)
			::NetApiBufferFree(info);
	} while (ERROR_MORE_DATA == nStatus);
	return	(NERR_Success == nStatus);
}
bool				SysUsers::CacheByGroup(const AutoUTF &name, const AutoUTF &dom) {
	// Cache members of group "name".
	LPLOCALGROUP_MEMBERS_INFO_1 info, infoTmp;
	DWORD dwLevel = 1;
	DWORD dwPrefMaxLen = MAX_PREFERRED_LENGTH;
	DWORD dwEntriesRead = 0;
	DWORD dwTotalEntries = 0;
	ULONG_PTR dwResumeHandle = 0;
	NET_API_STATUS nStatus;

	Clear();
	this->gr  = name;
	this->dom = dom;
	do {
		info = infoTmp = nullptr;
		nStatus = ::NetLocalGroupGetMembers(dom.c_str(),
											name.c_str(),
											dwLevel,
											(PBYTE*) & info,
											dwPrefMaxLen,
											&dwEntriesRead,
											&dwTotalEntries,
											&dwResumeHandle);
		if (NERR_Success == nStatus || ERROR_MORE_DATA == nStatus) {
			infoTmp = info;
			for (DWORD i = 0; (i < dwEntriesRead) && (infoTmp != nullptr); ++i, ++infoTmp) {
				Insert(infoTmp->lgrmi1_name, UserInfo(UserBuf(infoTmp->lgrmi1_name, dom).data()));
			}
		}
		if (info)
			::NetApiBufferFree(info);
	} while (ERROR_MORE_DATA == nStatus);
	return	(NERR_Success == nStatus);
}
bool				SysUsers::CacheByGid(const AutoUTF &gid, const AutoUTF &dom) {
	return	(CacheByGroup(SidString(gid).name(), dom));
}

bool				SysUsers::IsAdmin() const {
	return	(ValidPtr()) ? Value().priv  == USER_PRIV_ADMIN : false;
}
bool				SysUsers::IsDisabled() const {
	return	(ValidPtr()) ? Value().flags & UF_ACCOUNTDISABLE : false;
}

AutoUTF				SysUsers::GetName() const {
	return	(ValidPtr()) ? Key() : L"";
}
AutoUTF				SysUsers::GetDesc() const {
	return	(ValidPtr()) ? Value().desc : L"";
}
AutoUTF				SysUsers::GetFName() const {
	return	(ValidPtr()) ? Value().fname : L"";
}
AutoUTF				SysUsers::GetComm() const {
	return	(ValidPtr()) ? Value().comm : L"";
}
AutoUTF				SysUsers::GetProfile() const {
	return	(ValidPtr()) ? Value().prof : L"";
}
AutoUTF				SysUsers::GetHome() const {
	return	(ValidPtr()) ? Value().home : L"";
}
AutoUTF				SysUsers::GetScript() const {
	return	(ValidPtr()) ? Value().script : L"";
}
DWORD				SysUsers::GetPriv() const {
	return	(ValidPtr()) ? Value().priv : 0;
}
DWORD				SysUsers::GetFlags() const {
	return	(ValidPtr()) ? Value().flags : 0;
}

void				SysUsers::Add(const AutoUTF &name, const AutoUTF &pass) {
	if (!NetUser::IsExist(name)) {
		NetUser::Add(name, pass, dom);
		Insert(name, UserInfo(UserBuf(name, dom).data()));
		if (!gr.empty())
			NetGroup::AddMember(gr, Sid(name), dom);
	}
}
void				SysUsers::Del() {
	if (ValidPtr()) {
		NetUser::Del(Key(), dom);
		Erase();
	}
}
void				SysUsers::Del(const AutoUTF &name) {
	if (Find(name))
		Del();
}

void				SysUsers::Disable() {
	if (ValidPtr()) {
		NetUser::Disable(Key(), dom);
		Value().flags = NetUser::GetFlags(Key(), dom);
	}
}
void				SysUsers::Disable(const AutoUTF &name) {
	if (Find(name))
		Disable();
}
void				SysUsers::Enable() {
	if (ValidPtr()) {
		NetUser::Enable(Key(), dom);
		Value().flags = NetUser::GetFlags(Key(), dom);
	}
}
void				SysUsers::Enable(const AutoUTF &name) {
	if (Find(name))
		Enable();
}

void				SysUsers::SetName(const AutoUTF &in) {
	if (ValidPtr()) {
		NetUser::SetName(Key(), in, dom);
		UserInfo info = Value();
		Erase();
		Insert(in, info);
	}
}
void				SysUsers::SetPass(const AutoUTF &in) {
	if (ValidPtr()) {
		NetUser::SetPass(Key(), in, dom);
	}
}
void				SysUsers::SetDesc(const AutoUTF &in) {
	if (ValidPtr()) {
		NetUser::SetDesc(Key(), in, dom);
		Value().desc = in;
	}
}
void				SysUsers::SetFName(const AutoUTF &in) {
	if (ValidPtr()) {
		NetUser::SetFName(Key(), in, dom);
		Value().fname = in;
	}
}
void				SysUsers::SetComm(const AutoUTF &in) {
	if (ValidPtr()) {
		NetUser::SetComm(Key(), in, dom);
		Value().comm = in;
	}
}
void				SysUsers::SetProfile(const AutoUTF &in) {
	if (ValidPtr()) {
		NetUser::SetProfile(Key(), in, dom);
		Value().prof = in;
	}
}
void				SysUsers::SetHome(const AutoUTF &in) {
	if (ValidPtr()) {
		NetUser::SetHome(Key(), in, dom);
		Value().home = in;
	}
}
void				SysUsers::SetScript(const AutoUTF &in) {
	if (ValidPtr()) {
		NetUser::SetScript(Key(), in, dom);
		Value().script = in;
	}
}
void				SysUsers::SetFlags(DWORD in, bool value) {
	if (ValidPtr()) {
		NetUser::SetFlags(Key(), in, value, dom);
		Value().flags = in;
	}
}

///======================================================================================== WinUsers
WinUsers::WinUsers(bool autocache) {
	if (autocache)
		Cache();
}

bool WinUsers::Cache() {
	// Cache all users.
	DWORD dwLevel = 3;
	DWORD dwPrefMaxLen = MAX_PREFERRED_LENGTH;
	DWORD dwEntriesRead = 0;
	DWORD dwTotalEntries = 0;
	DWORD dwResumeHandle = 0;
	NET_API_STATUS nStatus;
	clear();
	do {
		LPUSER_INFO_3 info = nullptr;
		nStatus = ::NetUserEnum(nullptr,
								dwLevel,
								FILTER_NORMAL_ACCOUNT,
								(PBYTE*) & info,
								dwPrefMaxLen,
								&dwEntriesRead,
								&dwTotalEntries,
								&dwResumeHandle);
		if (NERR_Success == nStatus || ERROR_MORE_DATA == nStatus) {
			for (DWORD i = 0; i < dwEntriesRead; ++i) {
				push_back(UserInfo(info));
			}
			::NetApiBufferFree(info);
		}
	} while (nStatus == ERROR_MORE_DATA);
	return	(nStatus == NERR_Success);
}

bool WinUsers::CacheByPriv(DWORD priv) {
	// Cache users by priveleges.
	LPUSER_INFO_3 info, infoTmp;
	DWORD dwLevel = 3;
	DWORD dwPrefMaxLen = MAX_PREFERRED_LENGTH;
	DWORD dwEntriesRead = 0;
	DWORD dwTotalEntries = 0;
	DWORD dwResumeHandle = 0;
	NET_API_STATUS nStatus;
	clear();
	do {
		info = infoTmp = nullptr;
		nStatus = ::NetUserEnum(nullptr,
								dwLevel,
								FILTER_NORMAL_ACCOUNT, // global users
								(PBYTE*) & info,
								dwPrefMaxLen,
								&dwEntriesRead,
								&dwTotalEntries,
								&dwResumeHandle);
		if (NERR_Success == nStatus || ERROR_MORE_DATA == nStatus) {
			infoTmp = info;
			for (DWORD i = 0; (i < dwEntriesRead) && (infoTmp != nullptr); ++i, ++infoTmp) {
				if (priv != infoTmp->usri3_priv)
					continue;
				push_back(UserInfo(info));
			}
		}
		if (info)
			::NetApiBufferFree(info);
	} while (ERROR_MORE_DATA == nStatus);
	return	(NERR_Success == nStatus);
}

bool WinUsers::CacheByGroup(const AutoUTF &group) {
	// Cache members of group "name".
	LPLOCALGROUP_MEMBERS_INFO_1 info, infoTmp;
	DWORD dwLevel = 1;
	DWORD dwPrefMaxLen = MAX_PREFERRED_LENGTH;
	DWORD dwEntriesRead = 0;
	DWORD dwTotalEntries = 0;
	ULONG_PTR dwResumeHandle = 0;
	NET_API_STATUS nStatus;

	clear();
	this->gr  = group;
	do {
		info = infoTmp = nullptr;
		nStatus = ::NetLocalGroupGetMembers(nullptr,
		                                    group.c_str(),
											dwLevel,
											(PBYTE*) & info,
											dwPrefMaxLen,
											&dwEntriesRead,
											&dwTotalEntries,
											&dwResumeHandle);
		if (NERR_Success == nStatus || ERROR_MORE_DATA == nStatus) {
			infoTmp = info;
			for (DWORD i = 0; (i < dwEntriesRead) && (infoTmp != nullptr); ++i, ++infoTmp) {
				push_back(UserInfo(UserBuf(infoTmp->lgrmi1_name).data()));
			}
		}
		if (info)
			::NetApiBufferFree(info);
	} while (ERROR_MORE_DATA == nStatus);
	return	(NERR_Success == nStatus);
}

bool WinUsers::CacheByGid(const AutoUTF &gid) {
	return	(CacheByGroup(SidString(gid).name()));
}

void WinUsers::Add(const AutoUTF &name, const AutoUTF &pass) {
	NetUser::Add(name, pass);
	push_back(UserInfo(UserBuf(name).data()));
	if (!gr.empty())
		NetGroup::AddMember(gr, Sid(name));
}

void WinUsers::Del(const AutoUTF &name) {
	iterator it = find(begin(), end(), name);
	if (it != end())
		Del(it);
}

void WinUsers::Del(iterator it) {
	NetUser::Del(it->name);
	erase(it);
}
