/**
	win_user

	@classes	()
	@author		© 2009 Andrew Grechkin
	@link		()
**/

#ifndef WIN_USER_HPP
#define WIN_USER_HPP

#include "win_net.h"

///▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒ net_user
///========================================================================================= NetUser
namespace NetUser {
	bool	IsExist(const ustring &name, const ustring &dom = ustring());
	bool	IsDisabled(const ustring &name, const ustring &dom = ustring());
	bool	IsExpired(const ustring &name, const ustring &dom = ustring());

	ustring	GetComm(const ustring &name, const ustring &dom = ustring());
	ustring	GetDesc(const ustring &name, const ustring &dom = ustring());
	ustring	GetFName(const ustring &name, const ustring &dom = ustring());
	ustring	GetHome(const ustring &name, const ustring &dom = ustring());
	ustring	GetParams(const ustring &name, const ustring &dom = ustring());
	ustring	GetProfile(const ustring &name, const ustring &dom = ustring());
	ustring	GetScript(const ustring &name, const ustring &dom = ustring());
	ustring	GetWorkstations(const ustring &name, const ustring &dom = ustring());
	DWORD	GetFlags(const ustring &name, const ustring &dom = ustring());
	DWORD	GetUID(const ustring &name, const ustring &dom = ustring());

	void	Add(const ustring &name, const ustring &pass = ustring(), const ustring &dom = ustring());
	void	Del(const ustring &name, const ustring &dom = ustring());
	void	Disable(const ustring &name, const ustring &dom = ustring());
	void	Enable(const ustring &name, const ustring &dom = ustring());

	void	SetExpire(const ustring &name, bool in, const ustring &dom = ustring());
	void	SetName(const ustring &name, const ustring &in, const ustring &dom = ustring());
	void	SetPass(const ustring &name, const ustring &in, const ustring &dom = ustring());
	void	SetDesc(const ustring &name, const ustring &in, const ustring &dom = ustring());
	void	SetFName(const ustring &name, const ustring &in, const ustring &dom = ustring());
	void	SetComm(const ustring &name, const ustring &in, const ustring &dom = ustring());
	void	SetProfile(const ustring &name, const ustring &in, const ustring &dom = ustring());
	void	SetHome(const ustring &name, const ustring &in, const ustring &dom = ustring());
	void	SetScript(const ustring &name, const ustring &in, const ustring &dom = ustring());
	void	SetFlags(const ustring &name, DWORD in, bool value = true, const ustring &dom = ustring());
}

///======================================================================================== UserInfo
struct UserInfo {
	ustring	name;
	ustring	desc;
	ustring	fname;
	ustring	comm;
	ustring	prof;
	ustring	home;
	ustring	script;
	DWORD	priv;	// priv USER_PRIV_GUEST = 0, USER_PRIV_USER = 1, USER_PRIV_ADMIN = 2
	DWORD	flags;

//	UserInfo();
	explicit UserInfo(const ustring &n);
	explicit UserInfo(PVOID info);

	bool operator<(const UserInfo &rhs) const;

	bool operator==(const ustring &nm) const;

	bool is_admin() const;

	bool is_disabled() const;
};

///======================================================================================== WinUsers
class WinUsers: private std::vector<UserInfo> {
public:
	typedef UserInfo value_type;
	typedef std::vector<UserInfo> class_type;

	typedef class_type::iterator iterator;
	typedef class_type::const_iterator const_iterator;
	using class_type::begin;
	using class_type::end;
	using class_type::size;

public:
	WinUsers(bool autocache = true);
	bool	cache(const ustring &dom = ustring());
	bool	cache_by_priv(DWORD priv, const ustring &dom = ustring());
	bool	cache_by_group(const ustring &name, const ustring &dom = ustring());
	bool	cache_by_gid(const ustring &gid, const ustring &dom = ustring());

	iterator find(const ustring &name);

	void	add(const ustring &name, const ustring &pass = ustring());
	void	del(const ustring &name);
	void	del(iterator it);
	void	rename(const ustring &name, const ustring &new_name);
	void	rename(iterator it, const ustring &new_name);

private:
	ustring	m_group;
	ustring	m_dom;
};

class WinUsersByPriv: public WinUsers {
public:
	WinUsersByPriv(DWORD priv, const ustring &dom = ustring());
};

class WinUsersByGroup: public WinUsers {
public:
	WinUsersByGroup(const ustring &group, const ustring &dom = ustring());
};

class WinUsersByGid: public WinUsers {
public:
	WinUsersByGid(const ustring &gid, const ustring &dom = ustring());
};

///▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒ net_group
///======================================================================================== NetGroup
namespace NetGroup {
	bool	IsExist(const ustring &name, const ustring &dom = ustring());
	bool	IsMember(const ustring &name, const ustring &in, const ustring &dom = ustring());

	void	Add(const ustring &name, const ustring &comm = ustring(), const ustring &dom = ustring());
	void	Del(const ustring &name, const ustring &dom = ustring());
	void	Rename(const ustring &name, const ustring &in, const ustring &dom = ustring());

	void	AddMember(const ustring &name, const Sid &user, const ustring &dom = ustring());
	void	AddMember(const SidString &gid, const Sid &user, const ustring &dom = ustring());
	void	DelMember(const ustring &name, const Sid &user, const ustring &dom = ustring());

	ustring	GetComm(const ustring &name, const ustring &dom = ustring());
	void	SetComm(const ustring &name, const ustring &in, const ustring &dom = ustring());
}

///======================================================================================= GroupInfo
struct GroupInfo {
	ustring	name;
	ustring	comm;

	GroupInfo() : comm(L"") {
	}

	explicit GroupInfo(PVOID info);

	bool operator<(const GroupInfo &rhs) const;

	bool operator==(const ustring &nm) const;
};

///======================================================================================= WinGroups
class WinGroups: private std::vector<GroupInfo> {
public:
	typedef GroupInfo value_type;
	typedef std::vector<GroupInfo> class_type;
	typedef class_type::iterator iterator;
	typedef class_type::const_iterator const_iterator;
	using class_type::begin;
	using class_type::end;
	using class_type::size;

public:
	WinGroups(bool autocache = true);
	bool	cache(const ustring &dom = ustring());
	bool	cache_by_user(const ustring &name, const ustring &dom = ustring());

	iterator find(const ustring &name);

	void	add(const ustring &name);
	void	del(const ustring &name);
	void	del(iterator it);
	void	rename(const ustring &name, const ustring &new_name);
	void	rename(iterator it, const ustring &new_name);
private:
	ustring	m_dom;
};

#endif // WIN_USER_HPP
