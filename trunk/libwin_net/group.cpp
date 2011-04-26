/** win_net_group
 *	@classes to manipulate Groups
 *	@author GrAnD, 2009
 *	@link (netapi32)
 **/

#include "win_net.h"

#include "user.h"

#include <lm.h>

///======================================================================================== NetGroup
class GroupBuf {
	const static DWORD level = 1;
	PLOCALGROUP_INFO_1 info;
public:
	~GroupBuf() {
		::NetApiBufferFree(info);
	}

	GroupBuf(const AutoUTF &name, const AutoUTF &dom = AutoUTF()) {
		CheckApiError(::NetLocalGroupGetInfo(dom.c_str(), name.c_str(), level, (PBYTE*)&info));
	}

	PLOCALGROUP_INFO_1 operator->() const {
		return info;
	}
};

bool NetGroup::IsExist(const AutoUTF &name, const AutoUTF &dom) {
	try {
		GroupBuf(name, dom);
	} catch (WinError &e) {
		if (NERR_GroupNotFound == e.code())
			return false;
		throw;
	}
	return true;
}

bool NetGroup::IsMember(const AutoUTF &name, const AutoUTF &in, const AutoUTF &dom) {
	SysUsers members(false);
	members.CacheByGroup(name, dom);
	return members.Find(in);
}

void NetGroup::Add(const AutoUTF &name, const AutoUTF &comm, const AutoUTF &dom) {
	const DWORD level = 1;
	LOCALGROUP_INFO_1 info = {const_cast<PWSTR> (name.c_str()), const_cast<PWSTR> (comm.c_str())};
	CheckApiError(::NetLocalGroupAdd(dom.c_str(), level, (PBYTE)&info, nullptr));
}

void NetGroup::Del(const AutoUTF &name, const AutoUTF &dom) {
	CheckApiError(::NetLocalGroupDel(dom.c_str(), name.c_str()));
}

void NetGroup::Rename(const AutoUTF &name, const AutoUTF &in, const AutoUTF &dom) {
	const DWORD level = 0;
	LOCALGROUP_INFO_0 info = {const_cast<PWSTR> (in.c_str())};
	CheckApiError(::NetLocalGroupSetInfo(dom.c_str(), name.c_str(), level, (PBYTE)&info, nullptr));
}

void NetGroup::AddMember(const AutoUTF &name, const Sid &user, const AutoUTF &dom) {
	const DWORD level = 0;
	LOCALGROUP_MEMBERS_INFO_0 info = {user};
	CheckApiError(::NetLocalGroupAddMembers(dom.c_str(), name.c_str(), level, (PBYTE)&info, 1));
}

void NetGroup::AddMember(const SidString &gid, const Sid &user, const AutoUTF &dom) {
	AddMember(gid.name(), user, dom);
}

void NetGroup::DelMember(const AutoUTF &name, const Sid &user, const AutoUTF &dom) {
	const DWORD level = 0;
	LOCALGROUP_MEMBERS_INFO_0 info = {user};
	CheckApiError(::NetLocalGroupDelMembers(dom.c_str(), name.c_str(), level, (PBYTE)&info, 1));
}

AutoUTF NetGroup::GetComm(const AutoUTF &name, const AutoUTF &dom) {
	GroupBuf info(name, dom);
	return info->lgrpi1_comment;
}

void NetGroup::SetComm(const AutoUTF &name, const AutoUTF &in, const AutoUTF &dom) {
	const DWORD level = 1002;
	GROUP_INFO_1002 info = {const_cast<PWSTR> (in.c_str())};
	CheckApiError(::NetLocalGroupSetInfo(dom.c_str(), name.c_str(), level, (PBYTE)&info, nullptr));
}

///======================================================================================= SysGroups
bool SysGroups::Cache(const AutoUTF &dom) {
	// Cache all groups.
	LPLOCALGROUP_INFO_1 info, infoTmp;
	DWORD dwLevel = 1;
	DWORD dwPrefMaxLen = MAX_PREFERRED_LENGTH;
	DWORD dwEntriesRead = 0;
	DWORD dwTotalEntries = 0;
	ULONG_PTR dwResumeHandle = 0;
	NET_API_STATUS nStatus;

	Clear();
	this->dom = dom;
	do {
		info = infoTmp = nullptr;
		nStatus = ::NetLocalGroupEnum(dom.c_str(), dwLevel, (PBYTE*)&info, dwPrefMaxLen,
		                              &dwEntriesRead, &dwTotalEntries, &dwResumeHandle);
		if (NERR_Success == nStatus || ERROR_MORE_DATA == nStatus) {
			infoTmp = info;
			for (DWORD i = 0; (i < dwEntriesRead) && (infoTmp != nullptr); ++i, ++infoTmp) {
				GroupInfo gtmp;
				gtmp.comm = infoTmp->lgrpi1_comment;
				Insert(infoTmp->lgrpi1_name, gtmp);
			}
		}
		if (info)
			::NetApiBufferFree(info);
	} while (ERROR_MORE_DATA == nStatus);
	return (NERR_Success == nStatus);
}

bool SysGroups::CacheByUser(const AutoUTF &name, const AutoUTF &dom) {
	// Cache groups that contains USER "name".
	DWORD dwLevel = 0;
	DWORD dwFlags = LG_INCLUDE_INDIRECT;
	DWORD dwPrefMaxLen = MAX_PREFERRED_LENGTH;
	DWORD dwEntriesRead = 0;
	DWORD dwTotalEntries = 0;

	Clear();
	this->dom = dom;
	LPGROUP_USERS_INFO_0 info = nullptr;
	NET_API_STATUS nStatus = ::NetUserGetLocalGroups(dom.c_str(), name.c_str(), dwLevel, dwFlags,
	                                                 (PBYTE*)&info, dwPrefMaxLen, &dwEntriesRead,
	                                                 &dwTotalEntries);
	if (nStatus == NERR_Success || nStatus == ERROR_MORE_DATA) {
		for (DWORD i = 0; i < dwEntriesRead; ++i) {
			GroupInfo gtmp;
			gtmp.comm = NetGroup::GetComm(info[i].grui0_name, dom);
			Insert(info[i].grui0_name, gtmp);
		}
		::NetApiBufferFree(info);
	}
	return (NERR_Success == nStatus);
}

AutoUTF SysGroups::GetName() const {
	return (ValidPtr()) ? Key() : L"";
}

AutoUTF SysGroups::GetComm() const {
	return (ValidPtr()) ? Value().comm : L"";
}

void SysGroups::Add(const AutoUTF &name) {
	if (!Exist(name)) {
		NetGroup::Add(name, dom);
		Insert(name, GroupInfo());
	}
}

void SysGroups::Del() {
	if (ValidPtr()) {
		NetGroup::Del(Key(), dom);
		Erase();
	}
}

void SysGroups::SetName(const AutoUTF &in) {
	if (ValidPtr()) {
		NetGroup::Rename(Key(), in, dom);
		GroupInfo info = Value();
		Erase();
		Insert(in, info);
	}
}

void SysGroups::SetComm(const AutoUTF &in) {
	if (ValidPtr()) {
		NetGroup::SetComm(Key(), in, dom);
		Value().comm = in;
	}
}

///======================================================================================= WinAccess
bool WinAccess::Cache() {
	bool Result = false;
	if (acl) {
		ACL_SIZE_INFORMATION sizeinfo = {0};
		if (::GetAclInformation(acl, &sizeinfo, sizeof(sizeinfo), AclSizeInformation)) {
			Result = true;
			for (size_t index = 0; index < sizeinfo.AceCount; ++index) {
				ACCESS_ALLOWED_ACE* pACE;
				if (!::GetAce(acl, index, (PVOID*)&pACE))
					break;

				PSID pSID = PSIDFromPACE(pACE);
				AutoUTF name = Sid::name(pSID);
				AccessInfo info;

				info.mask = pACE->Mask;
				if (pACE->Header.AceType == ACCESS_ALLOWED_ACE_TYPE) {
					info.type = L"allow";
				} else {
					info.type = L"deny";
				}
				size_t tmp = 0;
				if (WinFlag::Check(info.mask, (DWORD)FILE_GENERIC_READ))
					tmp += 4;
				if (WinFlag::Check(info.mask, (DWORD)FILE_GENERIC_WRITE)
				    && WinFlag::Check(info.mask, (DWORD)DELETE))
					tmp += 2;
				if (WinFlag::Check(info.mask, (DWORD)FILE_GENERIC_EXECUTE))
					tmp += 1;
				info.unix = Num2Str(tmp);
				Insert(name, info);
			}
		}
	}
	return Result;
}
