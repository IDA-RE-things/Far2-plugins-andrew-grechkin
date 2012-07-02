/** win_net_group
	@classes to manipulate Groups
	@author © 2012 Andrew Grechkin
	@link (netapi32)
 **/

#include <libext/user.hpp>
#include <libext/exception.hpp>
#include <libext/sid.hpp>

#include <lm.h>

namespace Ext {

	///=========================================================================================== Group
	struct GroupBuf {
		~GroupBuf() {
			::NetApiBufferFree(info);
		}

		GroupBuf(const ustring & name, const ustring & dom = ustring()) {
			CheckApiError(::NetLocalGroupGetInfo(dom.c_str(), name.c_str(), level, (PBYTE*)&info));
		}

		PLOCALGROUP_INFO_1 operator ->() const {
			return info;
		}

	private:
		const static DWORD level = 1;
		PLOCALGROUP_INFO_1 info;
	};

	bool Group::is_exist(const ustring & name, const ustring & dom) {
		try {
			GroupBuf(name, dom);
		} catch (WinError & e) {
			if (NERR_GroupNotFound == e.code())
				return false;
			throw;
		}
		return true;
	}

	bool Group::is_member(const ustring & group, const ustring & user, const ustring & dom) {
		WinUsers members(false);
		members.cache_by_group(group, dom);
		return members.find(user) != members.end();
	}

	void Group::add(const ustring & name, const ustring & comm, const ustring & dom) {
		const DWORD level = 1;
		LOCALGROUP_INFO_1 info = {const_cast<PWSTR>(name.c_str()), const_cast<PWSTR>(comm.c_str())};
		CheckApiError(::NetLocalGroupAdd(dom.c_str(), level, (PBYTE)&info, nullptr));
	}

	void Group::del(const ustring & name, const ustring & dom) {
		CheckApiError(::NetLocalGroupDel(dom.c_str(), name.c_str()));
	}

	void Group::add_member(const ustring & name, const Sid & user, const ustring & dom) {
		const DWORD level = 0;
		LOCALGROUP_MEMBERS_INFO_0 info = {user};
		CheckApiError(::NetLocalGroupAddMembers(dom.c_str(), name.c_str(), level, (PBYTE)&info, 1));
	}

	void Group::add_member(const SidString & gid, const Sid & user, const ustring & dom) {
		add_member(gid.get_name(), user, dom);
	}

	void Group::del_member(const ustring & name, const Sid & user, const ustring & dom) {
		const DWORD level = 0;
		LOCALGROUP_MEMBERS_INFO_0 info = {user};
		CheckApiError(::NetLocalGroupDelMembers(dom.c_str(), name.c_str(), level, (PBYTE)&info, 1));
	}

	ustring Group::get_comm(const ustring & name, const ustring & dom) {
		GroupBuf info(name, dom);
		return info->lgrpi1_comment;
	}

	void Group::set_name(const ustring & name, const ustring & in, const ustring & dom) {
		const DWORD level = 0;
		LOCALGROUP_INFO_0 info = {const_cast<PWSTR> (in.c_str())};
		CheckApiError(::NetLocalGroupSetInfo(dom.c_str(), name.c_str(), level, (PBYTE)&info, nullptr));
	}

	void Group::set_comm(const ustring & name, const ustring & in, const ustring & dom) {
		const DWORD level = 1002;
		GROUP_INFO_1002 info = {const_cast<PWSTR> (in.c_str())};
		CheckApiError(::NetLocalGroupSetInfo(dom.c_str(), name.c_str(), level, (PBYTE)&info, nullptr));
	}

	///======================================================================================= GroupInfo
	GroupInfo::GroupInfo(PVOID info):
		name(((PLOCALGROUP_INFO_1)info)->lgrpi1_name),
		comm(((PLOCALGROUP_INFO_1)info)->lgrpi1_comment) {
	}

	bool GroupInfo::operator <(const GroupInfo & rhs) const {
		return name < rhs.name;
	}

	bool GroupInfo::operator ==(const ustring & nm) const {
		return this->name == nm;
	}

	///======================================================================================= WinGroups
	WinGroups::WinGroups(bool autocache) {
		if (autocache)
			cache();
	}

	bool WinGroups::cache(const ustring & dom) {
		// Cache all groups.
		const DWORD dwLevel = 1, dwPrefMaxLen = MAX_PREFERRED_LENGTH;
		DWORD dwEntriesRead = 0, dwTotalEntries = 0;
		ULONG_PTR dwResumeHandle = 0;
		NET_API_STATUS nStatus;
		clear();
		m_dom = dom;
		do {
			PLOCALGROUP_INFO_1 info = nullptr;
			nStatus = ::NetLocalGroupEnum(m_dom.c_str(), dwLevel, (PBYTE*)&info, dwPrefMaxLen,
			                              &dwEntriesRead, &dwTotalEntries, &dwResumeHandle);
			if (NERR_Success == nStatus || ERROR_MORE_DATA == nStatus) {
				PLOCALGROUP_INFO_1 ptr = info;
				for (DWORD i = 0; i < dwEntriesRead && ptr; ++i, ++ptr) {
					push_back(GroupInfo(ptr));
				}
			}
			if (info)
				::NetApiBufferFree(info);
		} while (ERROR_MORE_DATA == nStatus);
		return (NERR_Success == nStatus);
	}

	bool WinGroups::cache_by_user(const ustring & name, const ustring & dom) {
		// Cache groups that contains USER "name".
		const DWORD dwLevel = 0, dwPrefMaxLen = MAX_PREFERRED_LENGTH, dwFlags = LG_INCLUDE_INDIRECT;
		DWORD dwEntriesRead = 0, dwTotalEntries = 0;
		NET_API_STATUS nStatus;
		clear();
		m_dom = dom;
		LPGROUP_USERS_INFO_0 info = nullptr;
		nStatus = ::NetUserGetLocalGroups(m_dom.c_str(), name.c_str(), dwLevel, dwFlags, (PBYTE*)&info,
		                                  dwPrefMaxLen, &dwEntriesRead, &dwTotalEntries);
		if (nStatus == NERR_Success || nStatus == ERROR_MORE_DATA) {
			for (DWORD i = 0; i < dwEntriesRead; ++i) {
				GroupInfo gtmp;
				gtmp.name = info[i].grui0_name;
				gtmp.comm = Group::get_comm(info[i].grui0_name);
				push_back(gtmp);
			}
			::NetApiBufferFree(info);
		}
		return (NERR_Success == nStatus);
	}

	WinGroups::iterator WinGroups::find(const ustring & name) {
		return std::find(begin(), end(), name);
	}

	void WinGroups::add(const ustring & name) {
		Group::add(name);
		GroupInfo grtmp;
		grtmp.name = name;
		push_back(grtmp);
	}

	void WinGroups::del(const ustring & name) {
		iterator it = find(name);
		if (it != end())
			del(it);
	}

	void WinGroups::del(iterator it) {
		Group::del(it->name);
		erase(it);
	}

	void WinGroups::rename(const ustring & name, const ustring & new_name) {
		iterator it = find(name);
		if (it != end())
			rename(it, new_name);
	}

	void WinGroups::rename(iterator it, const ustring & new_name) {
		Group::set_name(it->name, new_name);
		it->name = new_name;
	}

}
