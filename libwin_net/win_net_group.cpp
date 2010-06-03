/** win_net_group
 *	@classes to manipulate Groups
 *	@author GrAnD, 2009
 *	@link (netapi32)
**/

#include "win_net.h"

///======================================================================================== NetGroup
class		GroupBuf {
	LPLOCALGROUP_INFO_1	info;
public:
	~GroupBuf() {
		if (info)
			::NetApiBufferFree(info);
	}
	GroupBuf(const AutoUTF &name, const AutoUTF &dom = L"") {
		DWORD	dwLevel = 1;
		CheckNetApi(::NetLocalGroupGetInfo(dom.c_str(), name.c_str(), dwLevel, (PBYTE*)&info));
	}

	LPLOCALGROUP_INFO_1	operator->() const {
		return	info;
	}
};

bool				NetGroup::IsExist(const AutoUTF &name, const AutoUTF &dom) {
	DWORD				dwLevel = 1;
	LPLOCALGROUP_INFO_1	info = NULL;
	NET_API_STATUS		err = ::NetLocalGroupGetInfo(dom.c_str(), name.c_str(), dwLevel, (PBYTE*) & info);
	if (info)
		::NetApiBufferFree(info);
	return	err == NERR_Success;
}
bool				NetGroup::IsMember(const AutoUTF &name, const AutoUTF &in, const AutoUTF &dom) {
	SysUsers	members(false);
	members.CacheByGroup(name, dom);
	return	members.Find(in);
}

AutoUTF				NetGroup::GetComm(const AutoUTF &name, const AutoUTF &dom) {
	GroupBuf	info(name, dom);
	return	info->lgrpi1_comment;
}
/*
AutoUTF			NetGroup::GetName(const AutoUTF &name, const AutoUTF &dom) {
	GroupBuf	info(name, dom);
	return	info->lgrpi1_name;
}
DWORD			NetGroup::GetGID(const AutoUTF &name, const AutoUTF &dom) {
	GroupBuf	info(name, dom);
	return	info->grpi2_group_id;
}
*/

void				NetGroup::Add(const AutoUTF &name, const AutoUTF &dom) {
	DWORD				dwLevel = 0;
	LOCALGROUP_INFO_0	info = {0};
	info.lgrpi0_name = const_cast<WCHAR*>(name.c_str());
	CheckNetApi(::NetLocalGroupAdd(dom.c_str(), dwLevel, (PBYTE)&info, NULL));
}
void				NetGroup::Del(const AutoUTF &name, const AutoUTF &dom) {
	CheckNetApi(::NetLocalGroupDel(dom.c_str(), name.c_str()));
}

void				NetGroup::AddMember(const AutoUTF &name, const AutoUTF &user, const AutoUTF &dom) {
	DWORD	dwLevel = 0;
	LOCALGROUP_MEMBERS_INFO_0	info = {0};
	Sid		sid(user.c_str(), L"");
	if (sid.Valid()) {
		info.lgrmi0_sid = sid;
		CheckNetApi(::NetLocalGroupAddMembers(dom.c_str(), name.c_str(), dwLevel, (PBYTE)&info, 1));
	}
}
void				NetGroup::AddMemberGid(const AutoUTF &gid, const AutoUTF &user, const AutoUTF &dom) {
	AddMember(Sid(gid).AsName(), user, dom);
}
void				NetGroup::DelMember(const AutoUTF &name, const AutoUTF &user, const AutoUTF &dom) {
	DWORD	dwLevel = 0;
	LOCALGROUP_MEMBERS_INFO_0 info = {0};
	Sid		sid(user.c_str(), L"");
	if (sid.Valid()) {
		info.lgrmi0_sid	= sid;
		CheckNetApi(::NetLocalGroupDelMembers(dom.c_str(), name.c_str(), dwLevel, (PBYTE)&info, 1));
	}
}
void				NetGroup::SetName(const AutoUTF &name, const AutoUTF &in, const AutoUTF &dom) {
	DWORD	dwLevel = 0;
	LOCALGROUP_INFO_0 info	= {0};
	info.lgrpi0_name = const_cast<WCHAR*>(in.c_str());
	CheckNetApi(::NetLocalGroupSetInfo(dom.c_str(), name.c_str(), dwLevel, (PBYTE)&info, NULL));
}
void				NetGroup::SetComm(const AutoUTF &name, const AutoUTF &in, const AutoUTF &dom) {
	DWORD	dwLevel = 1002;
	GROUP_INFO_1002	info = {0};
	info.grpi1002_comment	= const_cast<WCHAR*>(in.c_str());
	CheckNetApi(::NetLocalGroupSetInfo(dom.c_str(), name.c_str(), dwLevel, (PBYTE)&info, NULL));
}

///======================================================================================= SysGroups
bool				SysGroups::Cache(const AutoUTF &dom) {
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
		info = infoTmp = NULL;
		nStatus = ::NetLocalGroupEnum(dom.c_str(),
									  dwLevel,
									  (PBYTE*) & info,
									  dwPrefMaxLen,
									  &dwEntriesRead,
									  &dwTotalEntries,
									  &dwResumeHandle);
		if (NERR_Success == nStatus || ERROR_MORE_DATA == nStatus) {
			infoTmp = info;
			for (DWORD i = 0; (i < dwEntriesRead) && (infoTmp != NULL); ++i, ++infoTmp) {
				GroupInfo	gtmp;
				gtmp.comm	= infoTmp->lgrpi1_comment;
				Insert(infoTmp->lgrpi1_name, gtmp);
			}
		}
		if (info)
			::NetApiBufferFree(info);
	} while (ERROR_MORE_DATA == nStatus);
	return	(NERR_Success == nStatus);
}
bool				SysGroups::CacheByUser(const AutoUTF &name, const AutoUTF &dom) {
	// Cache groups that contains USER "name".
	DWORD dwLevel = 0;
	DWORD dwFlags = LG_INCLUDE_INDIRECT;
	DWORD dwPrefMaxLen = MAX_PREFERRED_LENGTH;
	DWORD dwEntriesRead = 0;
	DWORD dwTotalEntries = 0;

	Clear();
	this->dom = dom;
	LPGROUP_USERS_INFO_0 info = NULL;
	NET_API_STATUS nStatus = ::NetUserGetLocalGroups(dom.c_str(),
							 name.c_str(),
							 dwLevel,
							 dwFlags,
							 (PBYTE*) & info,
							 dwPrefMaxLen,
							 &dwEntriesRead,
							 &dwTotalEntries);
	if (nStatus == NERR_Success || nStatus == ERROR_MORE_DATA) {
		for (DWORD i = 0; i < dwEntriesRead; ++i) {
			GroupInfo gtmp;
			gtmp.comm		= NetGroup::GetComm(info[i].grui0_name, dom);
			Insert(info[i].grui0_name, gtmp);
		}
		::NetApiBufferFree(info);
	}
	return	(NERR_Success == nStatus);
}

AutoUTF				SysGroups::GetName() const {
	return	(ValidPtr()) ? Key() : L"";
}
AutoUTF				SysGroups::GetComm() const {
	return	(ValidPtr()) ? Value().comm : L"";
}

void				SysGroups::Add(const AutoUTF &name) {
	if (!Exist(name)) {
		NetGroup::Add(name, dom);
		Insert(name, GroupInfo());
	}
}
void				SysGroups::Del() {
	if (ValidPtr()) {
		NetGroup::Del(Key(), dom);
		Erase();
	}
}
void				SysGroups::SetName(const AutoUTF &in) {
	if (ValidPtr()) {
		NetGroup::SetName(Key(), in, dom);
		GroupInfo info = Value();
		Erase();
		Insert(in, info);
	}
}
void				SysGroups::SetComm(const AutoUTF &in) {
	if (ValidPtr()) {
		NetGroup::SetComm(Key(), in, dom);
		Value().comm = in;
	}
}

///======================================================================================= WinAccess
bool			WinAccess::Cache() {
	bool	Result = false;
	if (pACL) {
		ACL_SIZE_INFORMATION sizeinfo = {0};
		if (::GetAclInformation(pACL, &sizeinfo, sizeof(sizeinfo), AclSizeInformation)) {
			Result = true;
			for (size_t index = 0; index < sizeinfo.AceCount; ++index) {
				ACCESS_ALLOWED_ACE* pACE;
				if (!::GetAce(pACL, index, (PVOID*)&pACE))
					break;

				PSID	pSID = PSIDFromPACE(pACE);
				AutoUTF	name = Sid::AsName(pSID);
				AccessInfo info;

				info.mask = pACE->Mask;
				if (pACE->Header.AceType == ACCESS_ALLOWED_ACE_TYPE) {
					info.type = L"allow";
				} else {
					info.type = L"deny";
				}
				size_t tmp = 0;
				if (WinFlag<DWORD>::Check(info.mask, FILE_GENERIC_READ))
					tmp += 4;
				if (WinFlag<DWORD>::Check(info.mask, FILE_GENERIC_WRITE) && WinFlag<DWORD>::Check(info.mask, DELETE))
					tmp += 2;
				if (WinFlag<DWORD>::Check(info.mask, FILE_GENERIC_EXECUTE))
					tmp += 1;
				info.unix = Num2Str(tmp);
				Insert(name, info);
			}
		}
	}
	return	Result;
}
