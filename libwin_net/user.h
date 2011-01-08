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
namespace	NetUser {
	bool	IsExist(const AutoUTF &name, const AutoUTF &dom = L"");
	bool	IsDisabled(const AutoUTF &name, const AutoUTF &dom = L"");
	bool	IsExpired(const AutoUTF &name, const AutoUTF &dom = L"");

	AutoUTF	GetComm(const AutoUTF &name, const AutoUTF &dom = L"");
	AutoUTF	GetDesc(const AutoUTF &name, const AutoUTF &dom = L"");
	AutoUTF	GetFName(const AutoUTF &name, const AutoUTF &dom = L"");
	AutoUTF	GetHome(const AutoUTF &name, const AutoUTF &dom = L"");
	AutoUTF	GetParams(const AutoUTF &name, const AutoUTF &dom = L"");
	AutoUTF	GetProfile(const AutoUTF &name, const AutoUTF &dom = L"");
	AutoUTF	GetScript(const AutoUTF &name, const AutoUTF &dom = L"");
	AutoUTF	GetWorkstations(const AutoUTF &name, const AutoUTF &dom = L"");
	DWORD	GetFlags(const AutoUTF &name, const AutoUTF &dom = L"");
	DWORD	GetUID(const AutoUTF &name, const AutoUTF &dom = L"");

	void	Add(const AutoUTF &name, const AutoUTF &pass = L"", const AutoUTF &dom = L"");
	void	Del(const AutoUTF &name, const AutoUTF &dom = L"");
	void	Disable(const AutoUTF &name, const AutoUTF &dom = L"");
	void	Enable(const AutoUTF &name, const AutoUTF &dom = L"");

	void	SetExpire(const AutoUTF &name, bool in, const AutoUTF &dom = L"");
	void	SetName(const AutoUTF &name, const AutoUTF &in, const AutoUTF &dom = L"");
	void	SetPass(const AutoUTF &name, const AutoUTF &in, const AutoUTF &dom = L"");
	void	SetDesc(const AutoUTF &name, const AutoUTF &in, const AutoUTF &dom = L"");
	void	SetFName(const AutoUTF &name, const AutoUTF &in, const AutoUTF &dom = L"");
	void	SetComm(const AutoUTF &name, const AutoUTF &in, const AutoUTF &dom = L"");
	void	SetProfile(const AutoUTF &name, const AutoUTF &in, const AutoUTF &dom = L"");
	void	SetHome(const AutoUTF &name, const AutoUTF &in, const AutoUTF &dom = L"");
	void	SetScript(const AutoUTF &name, const AutoUTF &in, const AutoUTF &dom = L"");
	void	SetFlags(const AutoUTF &name, DWORD in, bool value = true, const AutoUTF &dom = L"");
}

///======================================================================================== SysUsers
struct		UserInfo {
	AutoUTF	desc;
	AutoUTF	fname;
	AutoUTF	comm;
	AutoUTF	prof;
	AutoUTF	home;
	AutoUTF	script;
	DWORD	priv;	// priv USER_PRIV_GUEST = 0, USER_PRIV_USER = 1, USER_PRIV_ADMIN = 2
	DWORD	flags;

	UserInfo();
	UserInfo(const AutoUTF &name, const AutoUTF &dom = L"");
};

class		SysUsers : public MapContainer<AutoUTF, UserInfo> {
	AutoUTF	gr;
	AutoUTF dom;
public:
	SysUsers(bool autocache = true);
	bool	Cache(const AutoUTF &dom = L"");
	bool	CacheByPriv(DWORD priv, const AutoUTF &dom = L"");
	bool	CacheByGroup(const AutoUTF &name, const AutoUTF &dom = L"");
	bool	CacheByGid(const AutoUTF &gid, const AutoUTF &dom = L"");

	bool	IsAdmin() const;
	bool	IsDisabled() const;

	void	Add(const AutoUTF &name, const AutoUTF &pass = L"");
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

///▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒ net_group
///======================================================================================== NetGroup
namespace	NetGroup {
	bool	IsExist(const AutoUTF &name, const AutoUTF &dom = AutoUTF());
	bool	IsMember(const AutoUTF &name, const AutoUTF &in, const AutoUTF &dom = AutoUTF());

	void	Add(const AutoUTF &name, const AutoUTF &comm = AutoUTF(), const AutoUTF &dom = AutoUTF());
	void	Del(const AutoUTF &name, const AutoUTF &dom = AutoUTF());
	void	Rename(const AutoUTF &name, const AutoUTF &in, const AutoUTF &dom = AutoUTF());

	void	AddMember(const AutoUTF &name, const Sid &user, const AutoUTF &dom = AutoUTF());
	void	AddMemberGid(const SidString &gid, const Sid &user, const AutoUTF &dom = AutoUTF());
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
	bool	Cache(const AutoUTF &dom = L"");
	bool	CacheByUser(const AutoUTF &name, const AutoUTF &dom = L"");

	void	Add(const AutoUTF &name);
	void	Del();
	void	SetName(const AutoUTF &in);
	void	SetComm(const AutoUTF &in);

	AutoUTF	GetName() const;
	AutoUTF	GetComm() const;
};

#endif // WIN_USER_HPP
