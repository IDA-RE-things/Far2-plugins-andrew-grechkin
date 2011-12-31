#ifndef WIN_NET_SD_HPP
#define WIN_NET_SD_HPP

#include "win_net.h"
#include "sid.h"

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

///▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒ net_SD
const DWORD ALL_SD_INFO = OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION;

ustring	GetOwner(HANDLE hnd, SE_OBJECT_TYPE type = SE_FILE_OBJECT);
ustring	GetOwner(const ustring &path, SE_OBJECT_TYPE type = SE_FILE_OBJECT);

ustring	GetGroup(HANDLE hnd, SE_OBJECT_TYPE type = SE_FILE_OBJECT);
ustring	GetGroup(const ustring &path, SE_OBJECT_TYPE type = SE_FILE_OBJECT);

void	SetOwner(HANDLE handle, PSID owner, SE_OBJECT_TYPE type = SE_FILE_OBJECT);
void	SetOwner(const ustring &path, PSID owner, SE_OBJECT_TYPE type = SE_FILE_OBJECT);
void	SetOwnerSD(const ustring &name, PSECURITY_DESCRIPTOR sd, SE_OBJECT_TYPE type = SE_FILE_OBJECT);

void	SetGroup(HANDLE handle, PSID owner, SE_OBJECT_TYPE type = SE_FILE_OBJECT);
void	SetGroup(const ustring &path, PSID owner, SE_OBJECT_TYPE type = SE_FILE_OBJECT);
void	SetGroupSD(const ustring &name, PSECURITY_DESCRIPTOR sd, SE_OBJECT_TYPE type = SE_FILE_OBJECT);

void	SetDacl(HANDLE handle, PACL acl, SE_OBJECT_TYPE type = SE_FILE_OBJECT);
void	SetDacl(const ustring &path, PACL pacl, SE_OBJECT_TYPE type = SE_FILE_OBJECT);
void	SetDacl(const ustring &name, PSECURITY_DESCRIPTOR sd, SE_OBJECT_TYPE type = SE_FILE_OBJECT);

void	SetSacl(HANDLE handle, PACL acl, SE_OBJECT_TYPE type = SE_FILE_OBJECT);
void	SetSacl(const ustring &path, PACL pacl, SE_OBJECT_TYPE type = SE_FILE_OBJECT);

void	SetSecurity(HANDLE hnd, PSECURITY_DESCRIPTOR sd, SE_OBJECT_TYPE type = SE_FILE_OBJECT);
void	SetSecurity(const ustring &path, PSECURITY_DESCRIPTOR sd, SE_OBJECT_TYPE type = SE_FILE_OBJECT);
void	SetSecurity(const ustring &path, const ustring &sddl, SE_OBJECT_TYPE type = SE_FILE_OBJECT);
void	SetSecurity(const ustring &path, const Sid &uid, const Sid &gid, mode_t mode, bool protect = false, SE_OBJECT_TYPE type = SE_FILE_OBJECT);

ustring	Mode2Sddl(const ustring &name, const ustring &group, mode_t mode);
ustring	MakeSDDL(const ustring &name, const ustring &group, mode_t mode, bool protect = false);

void SetOwnerRecur(const ustring & path, PSID owner, SE_OBJECT_TYPE type = SE_FILE_OBJECT);

///=========================================================================================== WinSD
/// Security descriptor (Дескриптор защиты)
/// Version		- версия SD (revision)
/// Flags		- флаги состояния
/// Owner SID	- sid владельца
/// Group SID	- sid группы (не используется вендой, лишь для совместимости с POSIX)
/// DACL		- список записей контроля доступа
/// SACL		- список записей аудита
class	WinSD {
public:
	virtual ~WinSD() = 0;

	operator	PSECURITY_DESCRIPTOR() const {
		return m_sd;
	}
	PSECURITY_DESCRIPTOR descriptor() const {
		return m_sd;
	}

	bool	IsProtected() const {
		return is_protected(m_sd);
	}
	bool	IsSelfRelative() const {
		return is_selfrelative(m_sd);
	}
	DWORD	Size() const {
		return size(m_sd);
	}

	WORD	Control() const {
		return get_control(m_sd);
	}
	void	Control(WORD flag, bool s) {
		set_control(m_sd, flag, s);
	}
	ustring	Owner() const {
		return Sid::name(get_owner(m_sd));
	}
	void	SetOwner(PSID pSid, bool deflt = false) {
		set_owner(m_sd, pSid, deflt);
	}
	ustring	Group() const {
		return Sid::name(get_group(m_sd));
	}
	void	SetGroup(PSID pSid, bool deflt = false) {
		set_group(m_sd, pSid, deflt);
	}
	PACL	Dacl() const {
		return get_dacl(m_sd);
	}
	void	SetDacl(PACL dacl) {
		set_dacl(m_sd, dacl);
	}
	void	MakeSelfRelative();
	void	Protect(bool pr) {
		Control(SE_DACL_PROTECTED, pr);
	}

	ustring	as_sddl(SECURITY_INFORMATION in = ALL_SD_INFO) const {
		return as_sddl(m_sd, in);
	}

	static void Free(PSECURITY_DESCRIPTOR &in);

	static bool is_valid(PSECURITY_DESCRIPTOR sd) {
		return sd && ::IsValidSecurityDescriptor(sd);
	}
	static bool is_protected(PSECURITY_DESCRIPTOR sd);
	static bool is_selfrelative(PSECURITY_DESCRIPTOR sd);

	static WORD get_control(PSECURITY_DESCRIPTOR sd);
	static size_t size(PSECURITY_DESCRIPTOR sd);

	static PSID get_owner(PSECURITY_DESCRIPTOR sd);
	static PSID get_group(PSECURITY_DESCRIPTOR sd);
	static PACL get_dacl(PSECURITY_DESCRIPTOR sd);

	static void set_control(PSECURITY_DESCRIPTOR sd, WORD flag, bool s);
	static void set_owner(PSECURITY_DESCRIPTOR sd, PSID pSid, bool deflt = false);
	static void set_group(PSECURITY_DESCRIPTOR sd, PSID pSid, bool deflt = false);
	static void set_dacl(PSECURITY_DESCRIPTOR sd, PACL acl);
	static void set_sacl(PSECURITY_DESCRIPTOR sd, PACL acl);

	static ustring	as_sddl(PSECURITY_DESCRIPTOR sd, SECURITY_INFORMATION in = ALL_SD_INFO);
	static ustring	Parse(PSECURITY_DESCRIPTOR sd);

protected:
	PSECURITY_DESCRIPTOR	m_sd;

	WinSD(): m_sd(nullptr) {
	}
};

/// Security descriptor by SDDL
class	WinSDDL: public WinSD {
public:
	~WinSDDL();
	WinSDDL(const ustring &in);
};

/// Absolute Security descriptor
class	WinAbsSD: public WinSD {
public:
	~WinAbsSD();
	WinAbsSD();
	WinAbsSD(const WinSD &sd) {
		Init((PSECURITY_DESCRIPTOR)sd);
	}
	WinAbsSD(PSECURITY_DESCRIPTOR sd) {
		Init(sd);
	}
	WinAbsSD(const ustring &usr, const ustring &grp, bool protect = true);
	WinAbsSD(const ustring &usr, const ustring &grp, mode_t mode, bool protect = true);
	WinAbsSD(mode_t mode, bool protect = true);
	WinAbsSD(PSID ow, PSID gr, PACL dacl, bool protect = true);
private:
	void	Init(PSECURITY_DESCRIPTOR sd);

	PSID	m_owner;
	PSID	m_group;
	PACL	m_dacl;
	PACL	m_sacl;
};

/// Security descriptor by handle
class	WinSDH: public WinSD {
public:
	~WinSDH();
	WinSDH(HANDLE handle, SE_OBJECT_TYPE type = SE_FILE_OBJECT): m_hnd(handle), m_type(type) {
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

/// Security descriptor by name
class	WinSDW: public WinSD {
public:
	~WinSDW();
	WinSDW(const ustring &name, SE_OBJECT_TYPE type = SE_FILE_OBJECT): m_name(name), m_type(type) {
		Get();
	}
	ustring	name() const {
		return m_name;
	}
	void	Get();
	void	Set() const;
	void	Set(const ustring &path) const;
private:
	ustring			m_name;
	SE_OBJECT_TYPE	m_type;
};

#endif
