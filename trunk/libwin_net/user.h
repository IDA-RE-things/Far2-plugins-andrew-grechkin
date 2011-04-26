﻿/**
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
namespace	NetUser {
	bool	IsExist(const AutoUTF &name, const AutoUTF &dom = AutoUTF());
	bool	IsDisabled(const AutoUTF &name, const AutoUTF &dom = AutoUTF());
	bool	IsExpired(const AutoUTF &name, const AutoUTF &dom = AutoUTF());

	AutoUTF	GetComm(const AutoUTF &name, const AutoUTF &dom = AutoUTF());
	AutoUTF	GetDesc(const AutoUTF &name, const AutoUTF &dom = AutoUTF());
	AutoUTF	GetFName(const AutoUTF &name, const AutoUTF &dom = AutoUTF());
	AutoUTF	GetHome(const AutoUTF &name, const AutoUTF &dom = AutoUTF());
	AutoUTF	GetParams(const AutoUTF &name, const AutoUTF &dom = AutoUTF());
	AutoUTF	GetProfile(const AutoUTF &name, const AutoUTF &dom = AutoUTF());
	AutoUTF	GetScript(const AutoUTF &name, const AutoUTF &dom = AutoUTF());
	AutoUTF	GetWorkstations(const AutoUTF &name, const AutoUTF &dom = AutoUTF());
	DWORD	GetFlags(const AutoUTF &name, const AutoUTF &dom = AutoUTF());
	DWORD	GetUID(const AutoUTF &name, const AutoUTF &dom = AutoUTF());

	void	Add(const AutoUTF &name, const AutoUTF &pass = AutoUTF(), const AutoUTF &dom = AutoUTF());
	void	Del(const AutoUTF &name, const AutoUTF &dom = AutoUTF());
	void	Disable(const AutoUTF &name, const AutoUTF &dom = AutoUTF());
	void	Enable(const AutoUTF &name, const AutoUTF &dom = AutoUTF());

	void	SetExpire(const AutoUTF &name, bool in, const AutoUTF &dom = AutoUTF());
	void	SetName(const AutoUTF &name, const AutoUTF &in, const AutoUTF &dom = AutoUTF());
	void	SetPass(const AutoUTF &name, const AutoUTF &in, const AutoUTF &dom = AutoUTF());
	void	SetDesc(const AutoUTF &name, const AutoUTF &in, const AutoUTF &dom = AutoUTF());
	void	SetFName(const AutoUTF &name, const AutoUTF &in, const AutoUTF &dom = AutoUTF());
	void	SetComm(const AutoUTF &name, const AutoUTF &in, const AutoUTF &dom = AutoUTF());
	void	SetProfile(const AutoUTF &name, const AutoUTF &in, const AutoUTF &dom = AutoUTF());
	void	SetHome(const AutoUTF &name, const AutoUTF &in, const AutoUTF &dom = AutoUTF());
	void	SetScript(const AutoUTF &name, const AutoUTF &in, const AutoUTF &dom = AutoUTF());
	void	SetFlags(const AutoUTF &name, DWORD in, bool value = true, const AutoUTF &dom = AutoUTF());
}

///======================================================================================== SysUsers
struct		UserInfo {
	AutoUTF	name;
	AutoUTF	desc;
	AutoUTF	fname;
	AutoUTF	comm;
	AutoUTF	prof;
	AutoUTF	home;
	AutoUTF	script;
	DWORD	priv;	// priv USER_PRIV_GUEST = 0, USER_PRIV_USER = 1, USER_PRIV_ADMIN = 2
	DWORD	flags;

//	UserInfo();
	explicit UserInfo(const AutoUTF &n);
	explicit UserInfo(PVOID info);

	bool operator<(const UserInfo &rhs) const;

	bool operator==(const AutoUTF &nm) const;

	bool is_admin() const;

	bool is_disabled() const;
};

class		SysUsers : public MapContainer<AutoUTF, UserInfo> {
	AutoUTF	gr;
	AutoUTF dom;
public:
	SysUsers(bool autocache = true);
	bool	Cache(const AutoUTF &dom = AutoUTF());
	bool	CacheByPriv(DWORD priv, const AutoUTF &dom = AutoUTF());
	bool	CacheByGroup(const AutoUTF &name, const AutoUTF &dom = AutoUTF());
	bool	CacheByGid(const AutoUTF &gid, const AutoUTF &dom = AutoUTF());

	bool	IsAdmin() const;
	bool	IsDisabled() const;

	void	Add(const AutoUTF &name, const AutoUTF &pass = AutoUTF());
	void	Del();
	void	Del(const AutoUTF &name);

	void	Disable();
	void	Disable(const AutoUTF &name);
	void	Enable();
	void	Enable(const AutoUTF &name);

	void	SetName(const AutoUTF &in);
	void	SetPass(const AutoUTF &in);
	void	SetDesc(const AutoUTF &in);
	void	SetFName(const AutoUTF &in);
	void	SetComm(const AutoUTF &in);
	void	SetProfile(const AutoUTF &in);
	void	SetHome(const AutoUTF &in);
	void	SetScript(const AutoUTF &in);
	void	SetFlags(DWORD in, bool value);
	AutoUTF	GetName() const;
	AutoUTF	GetDesc() const;
	AutoUTF	GetFName() const;
	AutoUTF	GetComm() const;
	AutoUTF	GetProfile() const;
	AutoUTF	GetHome() const;
	AutoUTF	GetScript() const;
	DWORD	GetPriv() const;
	DWORD	GetFlags() const;
};

class	WinUsers : private std::vector<UserInfo> {
public:
	typedef UserInfo value_type;
	typedef std::vector<UserInfo>::iterator iterator;
	typedef std::vector<UserInfo>::const_iterator const_iterator;
	using std::vector<UserInfo>::begin;
	using std::vector<UserInfo>::end;

public:
	WinUsers(bool autocache = true);
	bool	Cache();
	bool	CacheByPriv(DWORD priv);
	bool	CacheByGroup(const AutoUTF &name);
	bool	CacheByGid(const AutoUTF &gid);

	void	Add(const AutoUTF &name, const AutoUTF &pass = AutoUTF());
	void	Del(const AutoUTF &name);
	void	Del(iterator it);
private:
	AutoUTF	gr;
};

///▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒ net_group
///======================================================================================== NetGroup
namespace	NetGroup {
	bool	IsExist(const AutoUTF &name, const AutoUTF &dom = AutoUTF());
	bool	IsMember(const AutoUTF &name, const AutoUTF &in, const AutoUTF &dom = AutoUTF());

	void	Add(const AutoUTF &name, const AutoUTF &comm = AutoUTF(), const AutoUTF &dom = AutoUTF());
	void	Del(const AutoUTF &name, const AutoUTF &dom = AutoUTF());
	void	Rename(const AutoUTF &name, const AutoUTF &in, const AutoUTF &dom = AutoUTF());

	void	AddMember(const AutoUTF &name, const Sid &user, const AutoUTF &dom = AutoUTF());
	void	AddMember(const SidString &gid, const Sid &user, const AutoUTF &dom = AutoUTF());
	void	DelMember(const AutoUTF &name, const Sid &user, const AutoUTF &dom = AutoUTF());

	AutoUTF	GetComm(const AutoUTF &name, const AutoUTF &dom = AutoUTF());
	void	SetComm(const AutoUTF &name, const AutoUTF &in, const AutoUTF &dom = AutoUTF());
}

///======================================================================================= SysGroups
struct		GroupInfo {
	AutoUTF	comm;
	GroupInfo() : comm(L"") {
	}
};

class		SysGroups : public MapContainer<AutoUTF, GroupInfo> {
	AutoUTF		dom;
public:
	SysGroups(bool autocache = true) {
		if (autocache)
			Cache();
	}
	bool	Cache(const AutoUTF &dom = AutoUTF());
	bool	CacheByUser(const AutoUTF &name, const AutoUTF &dom = AutoUTF());

	void	Add(const AutoUTF &name);
	void	Del();
	void	SetName(const AutoUTF &in);
	void	SetComm(const AutoUTF &in);

	AutoUTF	GetName() const;
	AutoUTF	GetComm() const;
};

#endif // WIN_USER_HPP