/** win_user
 *	@classes to manipulate Users
 *	@author GrAnD, 2009
 *	@link (netapi32)
**/

#include "user.h"

#include <lm.h>

///========================================================================================= NetUser
class		UserBuf {
public:
	~UserBuf() {
		if (info)
			::NetApiBufferFree(info);
	}

	UserBuf(const AutoUTF &name, const AutoUTF &dom = AutoUTF()) {
		DWORD	dwLevel = 3;
		CheckApiError(::NetUserGetInfo(dom.c_str(), name.c_str(), dwLevel, (PBYTE*)&info));
	}

	void			Set(const AutoUTF &name, const AutoUTF &dom = AutoUTF()) {
		DWORD	dwLevel = 3;
		CheckApiError(::NetUserSetInfo(dom.c_str(), name.c_str(), dwLevel, (PBYTE)info, nullptr));
	}

	LPUSER_INFO_3	operator->() const {
		return	info;
	}

private:
	LPUSER_INFO_3	info;
};

bool				NetUser::IsExist(const AutoUTF &name, const AutoUTF &dom) {
	DWORD	dwLevel = 0;
	LPUSER_INFO_0	info = nullptr;
	NET_API_STATUS	err = ::NetUserGetInfo(dom.c_str(), name.c_str(), dwLevel, (PBYTE*) & info);
	if (info)
		::NetApiBufferFree(info);
	return	err == NERR_Success;
}
bool				NetUser::IsDisabled(const AutoUTF &name, const AutoUTF &dom) {
	UserBuf	info(name, dom);
	return	info->usri3_flags & UF_ACCOUNTDISABLE;
}
bool				NetUser::IsExpired(const AutoUTF &name, const AutoUTF &dom) {
	UserBuf	info(name, dom);
	return	info->usri3_password_expired;
}

AutoUTF				NetUser::GetComm(const AutoUTF &name, const AutoUTF &dom) {
	UserBuf	info(name, dom);
	return	info->usri3_usr_comment;
}
AutoUTF				NetUser::GetDesc(const AutoUTF &name, const AutoUTF &dom) {
	UserBuf	info(name, dom);
	return	info->usri3_comment;
}
AutoUTF				NetUser::GetFName(const AutoUTF &name, const AutoUTF &dom) {
	UserBuf	info(name, dom);
	return	info->usri3_full_name;
}
AutoUTF				NetUser::GetHome(const AutoUTF &name, const AutoUTF &dom) {
	UserBuf	info(name, dom);
	return	info->usri3_home_dir;
}
AutoUTF				NetUser::GetParams(const AutoUTF &name, const AutoUTF &dom) {
	UserBuf	info(name, dom);
	return	info->usri3_profile;
}
AutoUTF				NetUser::GetProfile(const AutoUTF &name, const AutoUTF &dom) {
	UserBuf	info(name, dom);
	return	info->usri3_profile;
}
AutoUTF				NetUser::GetScript(const AutoUTF &name, const AutoUTF &dom) {
	UserBuf	info(name, dom);
	return	info->usri3_script_path;
}
AutoUTF				NetUser::GetWorkstations(const AutoUTF &name, const AutoUTF &dom) {
	UserBuf	info(name, dom);
	return	info->usri3_workstations;
}
DWORD				NetUser::GetFlags(const AutoUTF &name, const AutoUTF &dom) {
	UserBuf	info(name, dom);
	return	info->usri3_flags;
}
DWORD				NetUser::GetUID(const AutoUTF &name, const AutoUTF &dom) {
	UserBuf	info(name, dom);
	return	info->usri3_user_id;
}

void				NetUser::Add(const AutoUTF &name, const AutoUTF &pass, const AutoUTF &dom) {
	DWORD		dwLevel	= 1;
	USER_INFO_1	info = {0};
	info.usri1_name = const_cast<WCHAR*>(name.c_str());
	info.usri1_password = const_cast<WCHAR*>(pass.c_str());
	info.usri1_priv = USER_PRIV_USER;
	info.usri1_flags = UF_SCRIPT | UF_NORMAL_ACCOUNT | UF_DONT_EXPIRE_PASSWD;
	if (pass.empty())
		WinFlag::Set(info.usri1_flags, (DWORD)UF_PASSWD_NOTREQD);
	CheckApiError(::NetUserAdd(dom.c_str(), dwLevel, (PBYTE)&info, nullptr));
}
void				NetUser::Del(const AutoUTF &name, const AutoUTF &dom) {
	CheckApiError(::NetUserDel(dom.c_str(), name.c_str()));
}
void				NetUser::Disable(const AutoUTF &name, const AutoUTF &dom) {
	SetFlags(name, UF_ACCOUNTDISABLE, true, dom);
}
void				NetUser::Enable(const AutoUTF &name, const AutoUTF &dom) {
	SetFlags(name, UF_ACCOUNTDISABLE, false, dom);
}

void				NetUser::SetExpire(const AutoUTF &name, bool in, const AutoUTF &dom) {
	UserBuf	info(name, dom);
	info->usri3_password_expired = (in) ? 1 : 0;
	info.Set(name, dom);
}
void				NetUser::SetName(const AutoUTF &name, const AutoUTF &in, const AutoUTF &dom) {
	DWORD		dwLevel	= 0;
	USER_INFO_0	info = {0};
	info.usri0_name = const_cast<WCHAR*>(in.c_str());
	CheckApiError(::NetUserSetInfo(dom.c_str(), name.c_str(), dwLevel, (PBYTE)&info, nullptr));
}
void				NetUser::SetPass(const AutoUTF &name, const AutoUTF &in, const AutoUTF &dom) {
	DWORD			dwLevel = 1003;
	USER_INFO_1003	info = {0};
	info.usri1003_password = const_cast<WCHAR*>(in.c_str());
	CheckApiError(::NetUserSetInfo(dom.c_str(), name.c_str(), dwLevel, (PBYTE)&info, nullptr));
}
void				NetUser::SetDesc(const AutoUTF &name, const AutoUTF &in, const AutoUTF &dom) {
	DWORD			dwLevel = 1007;
	USER_INFO_1007	info = {0};
	info.usri1007_comment	= const_cast<WCHAR*>(in.c_str());
	CheckApiError(::NetUserSetInfo(dom.c_str(), name.c_str(), dwLevel, (PBYTE)&info, nullptr));
}
void				NetUser::SetFName(const AutoUTF &name, const AutoUTF &in, const AutoUTF &dom) {
	DWORD			dwLevel = 1011;
	USER_INFO_1011	info = {0};
	info.usri1011_full_name	= const_cast<WCHAR*>(in.c_str());
	CheckApiError(::NetUserSetInfo(dom.c_str(), name.c_str(), dwLevel, (PBYTE)&info, nullptr));
}
void				NetUser::SetComm(const AutoUTF &name, const AutoUTF &in, const AutoUTF &dom) {
	DWORD			dwLevel = 1012;
	USER_INFO_1012	info = {0};
	info.usri1012_usr_comment = const_cast<WCHAR*>(in.c_str());
	CheckApiError(::NetUserSetInfo(dom.c_str(), name.c_str(), dwLevel, (PBYTE)&info, nullptr));
}
void				NetUser::SetHome(const AutoUTF &name, const AutoUTF &in, const AutoUTF &dom) {
	DWORD			dwLevel = 1006;
	USER_INFO_1006	info = {0};
	info.usri1006_home_dir	= const_cast<WCHAR*>(in.c_str());
	CheckApiError(::NetUserSetInfo(dom.c_str(), name.c_str(), dwLevel, (PBYTE)&info, nullptr));
}
void				NetUser::SetProfile(const AutoUTF &name, const AutoUTF &in, const AutoUTF &dom) {
	DWORD			dwLevel = 1052;
	USER_INFO_1052	info = {0};
	info.usri1052_profile	= const_cast<WCHAR*>(in.c_str());
	CheckApiError(::NetUserSetInfo(dom.c_str(), name.c_str(), dwLevel, (PBYTE)&info, nullptr));
}
void				NetUser::SetScript(const AutoUTF &name, const AutoUTF &in, const AutoUTF &dom) {
	DWORD			dwLevel = 1009;
	USER_INFO_1009	info = {0};
	info.usri1009_script_path	= const_cast<WCHAR*>(in.c_str());
	CheckApiError(::NetUserSetInfo(dom.c_str(), name.c_str(), dwLevel, (PBYTE)&info, nullptr));
}
void				NetUser::SetFlags(const AutoUTF &name, DWORD in, bool value, const AutoUTF &dom) {
	DWORD	dwLevel	= 1008;
	DWORD	dwFlags	= GetFlags(name, dom);
	USER_INFO_1008	info = {0};
	if (value)
		WinFlag::Set(dwFlags, in);
	else
		WinFlag::UnSet(dwFlags, in);
	info.usri1008_flags	= dwFlags;
	CheckApiError(::NetUserSetInfo(dom.c_str(), name.c_str(), dwLevel, (PBYTE)&info, nullptr));
}

///======================================================================================== SysUsers
UserInfo::UserInfo() {
	desc = fname = comm = prof = home = script = L"";
	priv = 0;
}
UserInfo::UserInfo(const AutoUTF &name, const AutoUTF &dom) {
	DWORD			dwLevel = 3;
	LPUSER_INFO_3	info = nullptr;
	CheckApiError(::NetUserGetInfo(dom.c_str(), name.c_str(), dwLevel, (PBYTE*) & info));
	desc	= info->usri3_comment;
	fname	= info->usri3_full_name;
	comm	= info->usri3_usr_comment;
	prof	= info->usri3_profile;
	home	= info->usri3_home_dir;
	script	= info->usri3_script_path;
	priv	= info->usri3_priv;
	flags	= info->usri3_flags;
	::NetApiBufferFree(info);
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
				UserInfo utmp;
				utmp.desc		= info[i].usri3_comment;
				utmp.fname		= info[i].usri3_full_name;
				utmp.comm		= info[i].usri3_usr_comment;
				utmp.prof		= info[i].usri3_profile;
				utmp.home		= info[i].usri3_home_dir;
				utmp.script		= info[i].usri3_script_path;
				utmp.priv		= info[i].usri3_priv;
				utmp.flags		= info[i].usri3_flags;
				Insert(info[i].usri3_name, utmp);
			}
			::NetApiBufferFree(info);
		}
	} while (ERROR_MORE_DATA == nStatus);
	return	(NERR_Success == nStatus);
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
				UserInfo utmp;
				utmp.desc		= infoTmp->usri3_comment;
				utmp.fname		= infoTmp->usri3_full_name;
				utmp.comm		= infoTmp->usri3_usr_comment;
				utmp.prof		= infoTmp->usri3_profile;
				utmp.home		= infoTmp->usri3_home_dir;
				utmp.script		= infoTmp->usri3_script_path;
				utmp.priv		= infoTmp->usri3_priv;
				utmp.flags		= infoTmp->usri3_flags;
				Insert(infoTmp->usri3_name, utmp);
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
				Insert(infoTmp->lgrmi1_name, UserInfo(infoTmp->lgrmi1_name, dom));
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
		Insert(name, UserInfo(name, dom));
		if (!gr.empty())
			NetGroup::AddMember(gr, name, dom);
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
