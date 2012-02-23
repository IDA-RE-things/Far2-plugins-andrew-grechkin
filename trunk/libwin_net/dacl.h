#ifndef WIN_NET_DACL_HPP
#define WIN_NET_DACL_HPP

#include "win_net.h"
#include "sid.h"

#include <aclapi.h>

///▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒ net_dacl
struct trustee_t: public TRUSTEEW {
	trustee_t(PCWSTR name);

	trustee_t(PSID sid);
};

///======================================================================================= ExpAccess
struct ExpAccess: public EXPLICIT_ACCESSW {
	ExpAccess(PCWSTR name, ACCESS_MASK acc, ACCESS_MODE mode, DWORD inh = SUB_CONTAINERS_AND_OBJECTS_INHERIT);

	ustring get_name() const;

	Sid get_sid() const;
};

struct ExpAccessArray {
	~ExpAccessArray();

	ExpAccessArray(PACL acl);

	ExpAccess & operator [](size_t index) const;

	size_t size() const;

private:
	ExpAccess * m_eacc;
	ULONG m_size;
};

ACCESS_MASK eff_rights(const PSECURITY_DESCRIPTOR psd, PSID sid);

size_t access2mode(ACCESS_MASK acc);

ACCESS_MASK mode2access(size_t mode);

///========================================================================================= WinDacl
class WinDacl {
	typedef WinDacl this_class;

public:
	~WinDacl();

	WinDacl(size_t size);
	WinDacl(PACL acl);
	WinDacl(PSECURITY_DESCRIPTOR sd);
	WinDacl(const ustring & name, SE_OBJECT_TYPE type = SE_FILE_OBJECT);

	WinDacl(const this_class & rhs);
	this_class & operator =(const this_class & rhs);

	operator PACL() const;

	void set_entries(const ExpAccess & acc);
	void set_access(PCWSTR name, ACCESS_MASK acc);
	void revoke_access(PCWSTR name);
	void grant_access(PCWSTR name, ACCESS_MASK acc);
	void deny_access(PCWSTR name, ACCESS_MASK acc);

	void set_to(DWORD flag, const ustring & name, SE_OBJECT_TYPE type = SE_FILE_OBJECT) const;

	size_t count() const;

	size_t size() const;

	void attach(PACL & acl);
	void detach(PACL & acl);
	void swap(PACL & acl);
	void swap(this_class & rhs);

	void del_inherited_aces();

	static bool is_valid(PACL in);
	static void get_info(PACL acl, ACL_SIZE_INFORMATION & out);
	static size_t count(PACL acl);
	static size_t used_bytes(PACL acl);
	static size_t free_bytes(PACL acl);
	static size_t size(PACL acl);
	static PACCESS_ALLOWED_ACE get_ace(PACL acl, size_t index);

	static void del_inherited_aces(PACL acl);

	static void set(PCWSTR path, PACL dacl, DWORD flag, SE_OBJECT_TYPE type = SE_FILE_OBJECT);
	static void set_inherit(PCWSTR path, PACL dacl, SE_OBJECT_TYPE type = SE_FILE_OBJECT);
	static void set_protect(PCWSTR path, PACL dacl, SE_OBJECT_TYPE type = SE_FILE_OBJECT);
	static void set_protect_copy(PCWSTR path, PACL dacl, SE_OBJECT_TYPE type = SE_FILE_OBJECT);

	static void inherit(const ustring & path, SE_OBJECT_TYPE type = SE_FILE_OBJECT);
	static void protect(const ustring & path, SE_OBJECT_TYPE type = SE_FILE_OBJECT);
	static void protect_copy(const ustring & path, SE_OBJECT_TYPE type = SE_FILE_OBJECT);

	static PACL create(size_t size);
	static PACL copy(PACL acl);
	static PACL copy(PSECURITY_DESCRIPTOR sd);

private:
	PACL m_dacl;
};

#ifndef NDEBUG
ustring as_str(PACL acl);
#endif

#endif
