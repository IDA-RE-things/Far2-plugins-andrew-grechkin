#ifndef WIN_NET_DACL_HPP
#define WIN_NET_DACL_HPP

#include "win_net.h"
#include "sid.h"

#include <aclapi.h>

///▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒ net_dacl
class Trustee: public TRUSTEEW {
public:
	Trustee(PCWSTR name);

	Trustee(PSID sid);
};

///======================================================================================= ExpAccess
class ExpAccess: public EXPLICIT_ACCESSW {
public:
	ExpAccess(PCWSTR name, ACCESS_MASK acc, ACCESS_MODE mode, DWORD inh = SUB_CONTAINERS_AND_OBJECTS_INHERIT);

	ustring get_name() const;

	ustring get_fullname() const;

	Sid		get_sid() const;
};

class ExpAccessArray {
public:
	~ExpAccessArray();

	ExpAccessArray(PACL acl);

	ExpAccess& operator[](int i) const {
		return m_eacc[i];
	}

	size_t count() const {
		return m_cnt;
	}

private:
	ExpAccess *m_eacc;
	ULONG m_cnt;
};

ACCESS_MASK eff_rights(const PSECURITY_DESCRIPTOR psd, PSID sid);

size_t		access2mode(ACCESS_MASK acc);

ACCESS_MASK mode2access(size_t mode);

///========================================================================================= WinDacl
class WinDacl {
public:
	~WinDacl();
	WinDacl(size_t size);
	WinDacl(PACL acl);
	WinDacl(PSECURITY_DESCRIPTOR sd);
	WinDacl(const ustring &name, SE_OBJECT_TYPE type = SE_FILE_OBJECT);

	operator	PACL() const {
		return m_dacl;
	}

	PACL* operator&();

	void Add(const ExpAccess &acc);
	void Set(PCWSTR name, ACCESS_MASK acc);
	void Revoke(PCWSTR name);
	void Grant(PCWSTR name, ACCESS_MASK acc);
	void Deny(PCWSTR name, ACCESS_MASK acc);

	void	SetTo(DWORD flag, const ustring &name, SE_OBJECT_TYPE type = SE_FILE_OBJECT) const {
		WinDacl::set(name.c_str(), m_dacl, flag, type);
	}

	size_t	count() const {
		return count(m_dacl);
	}

	size_t	size() const {
		return size(m_dacl);
	}

	void	detach(PACL &acl);
	void	swap(WinDacl &rhs);

	static ustring	Parse(PACL acl);

	static bool		is_valid(PACL in) {
		return ::IsValidAcl(in);
	}
	static void		get_info(PACL acl, ACL_SIZE_INFORMATION &out);
	static size_t	count(PACL acl);
	static size_t	used_bytes(PACL acl);
	static size_t	free_bytes(PACL acl);
	static size_t	size(PACL acl);
	static PVOID	get_ace(PACL acl, size_t index);

	static void del_inherited_aces(PACL acl);

	static void set(PCWSTR path, PACL dacl, DWORD flag, SE_OBJECT_TYPE type = SE_FILE_OBJECT);
	static void set_inherit(PCWSTR path, PACL dacl, SE_OBJECT_TYPE type = SE_FILE_OBJECT);
	static void set_protect(PCWSTR path, PACL dacl, SE_OBJECT_TYPE type = SE_FILE_OBJECT);
	static void set_protect_copy(PCWSTR path, PACL dacl, SE_OBJECT_TYPE type = SE_FILE_OBJECT);

	static void inherit(const ustring &path, SE_OBJECT_TYPE type = SE_FILE_OBJECT);
	static void protect(const ustring &path, SE_OBJECT_TYPE type = SE_FILE_OBJECT);
	static void protect_copy(const ustring &path, SE_OBJECT_TYPE type = SE_FILE_OBJECT);

	static PACL create(size_t size);

private:
	WinDacl(): m_dacl(nullptr) {
	}
	void	Init(PACL acl);
	void	Init(PSECURITY_DESCRIPTOR sd);

	PACL	m_dacl;
};

#endif
