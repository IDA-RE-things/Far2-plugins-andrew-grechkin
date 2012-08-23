#ifndef WIN_NET_DACL_HPP
#define WIN_NET_DACL_HPP

#include <libbase/std.hpp>
#include <libext/sid.hpp>
#include <libext/trustee.hpp>

#include <aclapi.h>

namespace Ext {

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

		static void parse(PACL acl);

	private:
		PACL m_dacl;
	};

}

#endif
