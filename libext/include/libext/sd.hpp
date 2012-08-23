#ifndef WIN_NET_SD_HPP
#define WIN_NET_SD_HPP

#include <libbase/std.hpp>
#include <libbase/logger.hpp>
#include <libext/sid.hpp>
#include <libext/trustee.hpp>

#include <sys/types.h>
#include <aclapi.h>

#ifndef S_IXUSR
#define S_IFDIR 0x4000
#define S_IRUSR 0x0100
#define S_IWUSR 0x0080
#define S_IXUSR 0x0040
#define S_IRGRP 0x0020
#define S_IWGRP 0x0010
#define S_IXGRP 0x0008
#define S_IROTH 0x0004
#define S_IWOTH 0x0002
#define S_IXOTH 0x0001
#endif

namespace Ext {

	///▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒ net_SD
	const DWORD ALL_SD_INFO = OWNER_SECURITY_INFORMATION |
		GROUP_SECURITY_INFORMATION |
		DACL_SECURITY_INFORMATION |
		SACL_SECURITY_INFORMATION |
		LABEL_SECURITY_INFORMATION;

	///=============================================================================== sddl_string_t
	struct sddl_string_t {
		sddl_string_t(const ustring & str);

		ustring as_str() const;

	private:
		ustring m_str;
	};

	ustring as_str(const sddl_string_t & sddl_str);

	///======================================================================================= WinSD
	/// Security descriptor (Дескриптор защиты)
	/// Version		- версия SD (revision)
	/// Flags		- флаги состояния
	/// Owner SID	- sid владельца
	/// Group SID	- sid группы (не используется вендой, лишь для совместимости с POSIX)
	/// DACL		- список записей контроля доступа
	/// SACL		- список записей аудита
	struct WinSD: private Base::Uncopyable {
		typedef WinSD this_class;

		virtual ~WinSD();

		WinSD(const trustee_t & owner, const trustee_t & group, const ExpAccessArray * dacl, const ExpAccessArray * sacl = nullptr);
		WinSD(PSECURITY_DESCRIPTOR sd);

		WinSD(WinSD && rhs);
		WinSD & operator = (WinSD && rhs);
		void swap(WinSD & rhs);

		operator PSECURITY_DESCRIPTOR() const {
			return m_sd;
		}
		PSECURITY_DESCRIPTOR descriptor() const {
			return m_sd;
		}

		bool is_protected() const {
			return is_protected(m_sd);
		}
		bool is_selfrelative() const {
			return is_selfrelative(m_sd);
		}

		DWORD size() const {
			return size(m_sd);
		}

		WORD get_control() const {
			return get_control(m_sd);
		}
		PSID get_owner() const {
			return get_owner(m_sd);
		}
		PSID get_group() const {
			return get_group(m_sd);
		}
		PACL get_dacl() const {
			return get_dacl(m_sd);
		}
		PACL get_sacl() const {
			return get_sacl(m_sd);
		}

		void set_control(WORD flag, bool s) {
			set_control(m_sd, flag, s);
		}
		void set_owner(PSID pSid, bool deflt = false) {
			set_owner(m_sd, pSid, deflt);
		}
		void set_group(PSID pSid, bool deflt = false) {
			set_group(m_sd, pSid, deflt);
		}
		void set_dacl(PACL acl) {
			set_dacl(m_sd, acl);
		}
		void set_sacl(PACL acl) {
			set_sacl(m_sd, acl);
		}
		void set_protect(bool pr) {
			set_control(SE_DACL_PROTECTED, pr);
		}

		ustring	as_sddl(SECURITY_INFORMATION in = ALL_SD_INFO) const {
			return as_sddl(m_sd, in);
		}

		static bool is_valid(PSECURITY_DESCRIPTOR sd);
		static bool is_protected(PSECURITY_DESCRIPTOR sd);
		static bool is_selfrelative(PSECURITY_DESCRIPTOR sd);

		static size_t size(PSECURITY_DESCRIPTOR sd);

		static WORD get_control(PSECURITY_DESCRIPTOR sd);
		static PSID get_owner(PSECURITY_DESCRIPTOR sd);
		static PSID get_group(PSECURITY_DESCRIPTOR sd);
		static PACL get_dacl(PSECURITY_DESCRIPTOR sd);
		static PACL get_sacl(PSECURITY_DESCRIPTOR sd);
		static size_t get_dacl_size(PSECURITY_DESCRIPTOR sd);
		static size_t get_sacl_size(PSECURITY_DESCRIPTOR sd);

		static void set_control(PSECURITY_DESCRIPTOR sd, WORD flag, bool s);
		static void set_owner(PSECURITY_DESCRIPTOR sd, PSID pSid, bool deflt = false);
		static void set_group(PSECURITY_DESCRIPTOR sd, PSID pSid, bool deflt = false);
		static void set_dacl(PSECURITY_DESCRIPTOR sd, PACL acl);
		static void set_sacl(PSECURITY_DESCRIPTOR sd, PACL acl);

		static ustring	as_sddl(PSECURITY_DESCRIPTOR sd, SECURITY_INFORMATION in = ALL_SD_INFO);

		static void parse(PSECURITY_DESCRIPTOR sd);

	protected:
		static PSECURITY_DESCRIPTOR alloc(size_t size);
		static void free(PSECURITY_DESCRIPTOR & in);

		PSECURITY_DESCRIPTOR	m_sd;

		WinSD():
			m_sd(nullptr)
		{
			LogTrace();
		}
	};

	///===================================================================================== WinSDDL
	/// Security descriptor by SDDL
	struct WinSDDL: public WinSD {
		~WinSDDL();
		WinSDDL(PCWSTR sddl);
	};

	///==================================================================================== WinAbsSD
	/// Absolute Security descriptor
	struct WinAbsSD: public WinSD {
		virtual ~WinAbsSD();
		WinAbsSD();
		WinAbsSD(const WinSD & sd) {
			Init((PSECURITY_DESCRIPTOR)sd);
		}
		WinAbsSD(PSECURITY_DESCRIPTOR sd) {
			Init(sd);
		}
		WinAbsSD(const ustring &usr, const ustring &grp, bool prot = true);
		WinAbsSD(const ustring &usr, const ustring &grp, mode_t mode, bool prot = true);
		WinAbsSD(mode_t mode, bool prot = true);
		WinAbsSD(PSID ow, PSID gr, PACL dacl, bool prot = true);

	private:
		void Init(PSECURITY_DESCRIPTOR sd);

		PSID	m_owner;
		PSID	m_group;
		PACL	m_dacl;
		PACL	m_sacl;
	};

	///====================================================================================== WinSDH
	/// Security descriptor by handle
	struct WinSDH: public WinSD {
		~WinSDH();
		WinSDH(HANDLE handle, SE_OBJECT_TYPE type = SE_FILE_OBJECT):
			m_hnd(handle),
			m_type(type)
		{
			Get();
		}
		HANDLE	hnd() const {
			return m_hnd;
		}
		void	Get();
		void	Set() const;

	private:
		HANDLE			m_hnd;
		SE_OBJECT_TYPE	m_type;
	};

	///====================================================================================== WinSDW
	/// Security descriptor by name
	struct WinSDW: public WinSD {
		~WinSDW();
		WinSDW(const ustring & name, SE_OBJECT_TYPE type = SE_FILE_OBJECT):
			m_name(name),
			m_type(type)
		{
			Get();
		}
		ustring	name() const {
			return m_name;
		}
		void	Get();
		void	Set() const;
		void	Set(PCWSTR path) const;

	private:
		ustring			m_name;
		SE_OBJECT_TYPE	m_type;
	};

	///======================================================================================== util
	ustring	get_owner(HANDLE hnd, SE_OBJECT_TYPE type = SE_FILE_OBJECT);
	ustring	get_owner(const ustring & path, SE_OBJECT_TYPE type = SE_FILE_OBJECT);

	ustring	get_group(HANDLE hnd, SE_OBJECT_TYPE type = SE_FILE_OBJECT);
	ustring	get_group(const ustring & path, SE_OBJECT_TYPE type = SE_FILE_OBJECT);

	void	set_owner(HANDLE handle, PSID owner, SE_OBJECT_TYPE type = SE_FILE_OBJECT);
	void	set_owner(PCWSTR path, PSID owner, SE_OBJECT_TYPE type = SE_FILE_OBJECT);

	void	set_group(HANDLE handle, PSID owner, SE_OBJECT_TYPE type = SE_FILE_OBJECT);
	void	set_group(PCWSTR path, PSID owner, SE_OBJECT_TYPE type = SE_FILE_OBJECT);

	void	set_dacl(HANDLE handle, PACL acl, SE_OBJECT_TYPE type = SE_FILE_OBJECT);
	void	set_dacl(PCWSTR path, PACL pacl, SE_OBJECT_TYPE type = SE_FILE_OBJECT);
	void	set_dacl(PCWSTR path, PSECURITY_DESCRIPTOR sd, SE_OBJECT_TYPE type = SE_FILE_OBJECT);

	void	set_sacl(HANDLE handle, PACL acl, SE_OBJECT_TYPE type = SE_FILE_OBJECT);
	void	set_sacl(PCWSTR path, PACL pacl, SE_OBJECT_TYPE type = SE_FILE_OBJECT);

	void	set_security(HANDLE hnd, PSECURITY_DESCRIPTOR sd, SE_OBJECT_TYPE type = SE_FILE_OBJECT);
	void	set_security(PCWSTR path, PSECURITY_DESCRIPTOR sd, SE_OBJECT_TYPE type = SE_FILE_OBJECT);
//	void	SetSecurity(const ustring &path, const ustring &sddl, SE_OBJECT_TYPE type = SE_FILE_OBJECT);
//	void	SetSecurity(const ustring &path, const Sid &uid, const Sid &gid, mode_t mode, bool protect = false, SE_OBJECT_TYPE type = SE_FILE_OBJECT);

	ustring	Mode2Sddl(const ustring & owner, const ustring & group, mode_t mode);
	ustring	MakeSDDL(const ustring &name, const ustring &group, mode_t mode, bool protect = false);

	void SetOwnerRecur(const ustring & path, PSID owner, SE_OBJECT_TYPE type = SE_FILE_OBJECT);

}

#endif
