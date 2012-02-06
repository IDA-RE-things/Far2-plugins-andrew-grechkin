/**
	win_user

	@classes ()
	@author © 2012 Andrew Grechkin
	@link ()
**/

#ifndef WIN_USER_HPP
#define WIN_USER_HPP

#include "win_net.h"
#include <vector>

///▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒ net_user
///============================================================================================ User
namespace User {
	bool is_exist(const ustring & name, const ustring & dom = ustring());
	bool is_disabled(const ustring & name, const ustring & dom = ustring());
	bool is_expired(const ustring & name, const ustring & dom = ustring());

	ustring get_comm(const ustring & name, const ustring & dom = ustring());
	ustring get_desc(const ustring & name, const ustring & dom = ustring());
	ustring get_fname(const ustring & name, const ustring & dom = ustring());
	ustring get_home(const ustring & name, const ustring & dom = ustring());
	ustring get_params(const ustring & name, const ustring & dom = ustring());
	ustring get_profile(const ustring & name, const ustring & dom = ustring());
	ustring get_script(const ustring & name, const ustring & dom = ustring());
	ustring get_workstations(const ustring & name, const ustring & dom = ustring());
	DWORD get_flags(const ustring & name, const ustring & dom = ustring());
	DWORD get_uid(const ustring & name, const ustring & dom = ustring());

	void add(const ustring & name, const ustring & pass = ustring(), const ustring & dom = ustring());
	void del(const ustring & name, const ustring & dom = ustring());
	void disable(const ustring & name, const ustring & dom = ustring());
	void enable(const ustring & name, const ustring & dom = ustring());

	void set_expire(const ustring & name, bool in, const ustring & dom = ustring());
	void set_name(const ustring & name, const ustring & in, const ustring & dom = ustring());
	void set_pass(const ustring & name, const ustring & in, const ustring & dom = ustring());
	void set_desc(const ustring & name, const ustring & in, const ustring & dom = ustring());
	void set_fname(const ustring & name, const ustring & in, const ustring & dom = ustring());
	void set_comm(const ustring & name, const ustring & in, const ustring & dom = ustring());
	void set_profile(const ustring & name, const ustring & in, const ustring & dom = ustring());
	void set_home(const ustring & name, const ustring & in, const ustring & dom = ustring());
	void set_script(const ustring & name, const ustring & in, const ustring & dom = ustring());
	void set_flags(const ustring & name, DWORD in, bool value = true, const ustring & dom = ustring());
}

///======================================================================================== UserInfo
struct UserInfo {
	ustring name;
	ustring desc;
	ustring fname;
	ustring comm;
	ustring prof;
	ustring home;
	ustring script;
	DWORD priv; // priv USER_PRIV_GUEST = 0, USER_PRIV_USER = 1, USER_PRIV_ADMIN = 2
	DWORD flags;

	explicit UserInfo(const ustring & n);
	explicit UserInfo(PVOID info);

	bool operator <(const UserInfo & rhs) const;

	bool operator ==(const ustring & nm) const;

	bool is_admin() const;

	bool is_disabled() const;
};

///======================================================================================== WinUsers
struct WinUsers: private std::vector<UserInfo> {
	typedef std::vector<UserInfo> this_type;
	typedef UserInfo value_type;

	typedef this_type::iterator iterator;
	typedef this_type::const_iterator const_iterator;
	using this_type::begin;
	using this_type::end;
	using this_type::empty;
	using this_type::size;

public:
	WinUsers(bool autocache = true);
	bool cache(const ustring & dom = ustring());
	bool cache_by_priv(DWORD priv, const ustring & dom = ustring());
	bool cache_by_group(const ustring & name, const ustring & dom = ustring());
	bool cache_by_gid(const ustring & gid, const ustring & dom = ustring());

	iterator find(const ustring & name);

	void add(const ustring & name, const ustring & pass = ustring());
	void del(const ustring & name);
	void del(iterator it);
	void rename(const ustring & name, const ustring & new_name);
	void rename(iterator it, const ustring & new_name);

private:
	ustring m_group;
	ustring m_dom;
};

struct WinUsersByPriv: public WinUsers {
	WinUsersByPriv(DWORD priv, const ustring & dom = ustring());
};

struct WinUsersByGroup: public WinUsers {
	WinUsersByGroup(const ustring & group, const ustring & dom = ustring());
};

struct WinUsersByGid: public WinUsers {
	WinUsersByGid(const ustring & gid, const ustring & dom = ustring());
};

///▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒ net_group
///=========================================================================================== Group
class Sid;
class SidString;

namespace Group {
	bool is_exist(const ustring & name, const ustring & dom = ustring());
	bool is_member(const ustring & name, const ustring & in, const ustring & dom = ustring());

	void add(const ustring & name, const ustring & comm = ustring(), const ustring & dom = ustring());
	void del(const ustring & name, const ustring & dom = ustring());

	void add_member(const ustring & name, const Sid & user, const ustring & dom = ustring());
	void add_member(const SidString & gid, const Sid & user, const ustring & dom = ustring());
	void del_member(const ustring & name, const Sid & user, const ustring & dom = ustring());

	ustring get_comm(const ustring & name, const ustring & dom = ustring());
	void set_name(const ustring & name, const ustring & in, const ustring & dom = ustring());
	void set_comm(const ustring & name, const ustring & in, const ustring & dom = ustring());
}

///======================================================================================= GroupInfo
struct GroupInfo {
	ustring name;
	ustring comm;

	GroupInfo() {
	}

	explicit GroupInfo(PVOID info);

	bool operator <(const GroupInfo & rhs) const;

	bool operator ==(const ustring & nm) const;
};

///======================================================================================= WinGroups
struct WinGroups: private std::vector<GroupInfo> {
	typedef std::vector<GroupInfo> this_type;
	typedef GroupInfo value_type;
	typedef this_type::iterator iterator;
	typedef this_type::const_iterator const_iterator;
	using this_type::begin;
	using this_type::end;
	using this_type::empty;
	using this_type::size;

public:
	WinGroups(bool autocache = true);
	bool cache(const ustring & dom = ustring());
	bool cache_by_user(const ustring & name, const ustring & dom = ustring());

	iterator find(const ustring & name);

	void add(const ustring & name);
	void del(const ustring & name);
	void del(iterator it);
	void rename(const ustring & name, const ustring & new_name);
	void rename(iterator it, const ustring & new_name);

private:
	ustring m_dom;
};

#endif
