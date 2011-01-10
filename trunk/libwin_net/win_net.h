/**
	win_net

	@classes	()
	@author		© 2009 Andrew Grechkin
	@link		()
**/

#ifndef WIN_NET_HPP
#define WIN_NET_HPP

#include <winsock2.h>

#include <libwin_def/win_def.h>

//#include <tr1/memory>
//using std::tr1::shared_ptr;

#include <vector>

#include "exception.h"
#include "c_map.h"

#include <sys/types.h>
#include <aclapi.h>
#include <ntsecapi.h>

extern "C" {
	int __cdecl snprintf(char* s, size_t n, const char*  format, ...);
}

///▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒
///========================================================================================== WinNet
namespace	WinNet {
	AutoUTF 	GetCompName(COMPUTER_NAME_FORMAT cnf = ComputerNameNetBIOS);

	inline bool SetCompName(const AutoUTF &in, COMPUTER_NAME_FORMAT cnf) {
		return	::SetComputerNameExW(cnf, in.c_str()) != 0;
	}
}

///▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒ net_auth
void	PassSave(PCWSTR name, PCWSTR pass);
inline void	PassSave(const AutoUTF &name, const AutoUTF &pass) {
	PassSave(name.c_str(), pass.c_str());
}

void	PassDel(PCWSTR name);
inline void	PassDel(const AutoUTF &name) {
	PassDel(name.c_str());
}

AutoUTF	PassRead(PCWSTR name);
inline AutoUTF	PassRead(const AutoUTF &name) {
	return 	PassRead(name.c_str());
}

void	PassList();

///▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒ net_exec
///============================================================================================ Exec
namespace	Exec {
	extern DWORD	TIMEOUT;
	extern DWORD	TIMEOUT_DX;
	void	Run(const AutoUTF &cmd);
	int		Run(const AutoUTF &cmd, astring &out);
	int		Run(const AutoUTF &cmd, astring &out, const astring &in);
	int		RunWait(const AutoUTF &cmd, DWORD wait = TIMEOUT);
	void	RunAsUser(const AutoUTF &cmd, HANDLE token);
	void	RunAsUser(const AutoUTF &cmd, const AutoUTF &user, const AutoUTF &pass);
	int		RunAsUser(const AutoUTF &cmd, astring &out, const astring &in, const AutoUTF &user, const AutoUTF &pass);
	HANDLE	Logon(const AutoUTF &name, const AutoUTF &pass, DWORD type, const AutoUTF &dom = AutoUTF());
	void	Impersonate(HANDLE hToken);
	HANDLE	Impersonate(const AutoUTF &name, const AutoUTF &pass, DWORD type = LOGON32_LOGON_BATCH, const AutoUTF &dom = AutoUTF());
}

///========================================================================================== WinJob
struct		WinJob {
	~WinJob();
	WinJob();
	WinJob(const AutoUTF &name);
	void	SetTimeLimit(size_t seconds);
	void	SetUiLimit();
	void	AddProcess(HANDLE hProc);
	void	RunAsUser(const AutoUTF &cmd, HANDLE hToken);
	int		RunAsUser(const AutoUTF &cmd, astring &out, const astring &in, HANDLE hToken);
private:
	static DWORD	TIMEOUT_DX;
	HANDLE m_job;
};

///▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒ net_rc
///================================================================================ RemoteConnection
struct		RemoteConnection {
	~RemoteConnection() {
		Close();
	}

	RemoteConnection(PCWSTR host = nullptr, PCWSTR user = nullptr, PCWSTR pass = nullptr): m_conn(false) {
		Open(host, user, pass);
	}

	void		Open(PCWSTR host, PCWSTR user = nullptr, PCWSTR pass = nullptr);

	void		Close();

	PCWSTR		host() const {
		return	m_host.c_str();
	}

private:
	ustring	m_host;
	bool	m_conn;
};

///================================================================================== DinamicLibrary
class DynamicLibrary: private Uncopyable {
public:
	~DynamicLibrary() {
		::FreeLibrary(m_hnd);
	}
	DynamicLibrary(PCWSTR path) :
			m_hnd(CheckHandle(::LoadLibraryW(path))) {
	}

	operator HMODULE() const {
		return m_hnd;
	}

	HMODULE handle() const {
		return m_hnd;
	}

	FARPROC get_function_nothrow(PCSTR name) const {
		return ::GetProcAddress(m_hnd, name);
	}

	FARPROC get_function(PCSTR name) const {
		return CheckPointer(::GetProcAddress(m_hnd, name));
	}

//	static uint64_t get_version(PCWSTR path) {
//		DWORD handle;
//		DWORD size = ::GetFileVersionInfoSizeW(path, &handle);
//		if (size) {
//			BYTE buf[size];
//			if (::GetFileVersionInfoW(path, handle, size, buf)) {
//				VS_FIXEDFILEINFO* fixed_file_info;
//				UINT len;
//				if (::VerQueryValueW(buf, PATH_SEPARATOR, (PVOID*)&fixed_file_info, &len)) {
//					return HighLow64(fixed_file_info->dwFileVersionMS, fixed_file_info->dwFileVersionLS);
//				}
//			}
//		}
//		return 0;
//	}

private:
	HMODULE m_hnd;
//	AutoUTF m_path;
//	uint64_t m_ver;
};

///▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒ net_sid
///============================================================================================= Sid
/// Security Identifier (Идентификатор безопасности) -
/// структура данных переменной длины, которая идентифицирует учетную запись пользователя, группы,
/// домена или компьютера
#ifndef PSIDFromPACE
#define PSIDFromPACE(pACE)((PSID)(&((pACE)->SidStart)))
#endif

class Sid {
	typedef Sid class_type;
public:
	typedef PSID value_type;
	typedef size_t size_type;

	explicit Sid(WELL_KNOWN_SID_TYPE wns);

	explicit Sid(value_type rhs):
		m_psid(class_type::size(rhs)) {
		copy(rhs);
	}

	Sid(const class_type &rhs):
		m_psid(rhs.size()) {
		copy(rhs);
	}

	Sid(PCWSTR name, PCWSTR srv = nullptr) {
		init(name, srv);
	}

	Sid(const AutoUTF &name, PCWSTR srv = nullptr) {
		init(name.c_str(), srv);
	}

	class_type& operator=(value_type rhs);

	class_type& operator=(const class_type &rhs);

	bool operator==(value_type rhs) const;

	bool operator==(const class_type &rhs) const {
		return operator==(rhs.m_psid.data());
	}

	bool operator!=(value_type rhs) const {
		return !operator==(rhs);
	}

	bool operator!=(const class_type &rhs) const {
		return !operator==(rhs.m_psid.data());
	}

	size_type size() const {
		return class_type::size(m_psid);
	}

	bool is_valid() const {
		return class_type::is_valid(m_psid);
	}

	AutoUTF str() const {
		return class_type::str(m_psid);
	}

	AutoUTF name() const {
		return class_type::name(m_psid);
	}
	AutoUTF full_name() const {
		return class_type::full_name(m_psid);
	}
	AutoUTF domain() const {
		return class_type::domain(m_psid);
	}


	void copy_to(value_type out, size_t size) const {
		CheckApi(::CopySid(size, out, m_psid));
	}


	operator value_type() const {
		return m_psid.data();
	}

	void swap(class_type &rhs) {
		m_psid.swap(rhs.m_psid);
	}

	static bool is_valid(value_type in) {
		return in && ::IsValidSid(in);
	}
	static void check(value_type in);
	static size_type size(value_type in);
	static size_type sub_authority_count(value_type in);
	static size_type rid(value_type in);

	// PSID to sid string
	static AutoUTF str(value_type in);

	// name to sid string
	static AutoUTF str(const AutoUTF &name, PCWSTR srv = nullptr);
	// PSID to name
	static void name(value_type pSID, AutoUTF &name, AutoUTF &dom, PCWSTR srv = nullptr);
	static AutoUTF name(value_type pSID, PCWSTR srv = nullptr);
	static AutoUTF full_name(value_type pSID, PCWSTR srv = nullptr);
	static AutoUTF domain(value_type pSID, PCWSTR srv = nullptr);

protected:
	Sid() {
	}

private:
	void copy(value_type in);
	void init(PCWSTR name, PCWSTR srv = nullptr);

	auto_buf<value_type> m_psid;
};

class SidString: public Sid {
public:
	SidString(PCWSTR str) {
		init(str);
	}

	SidString(const AutoUTF &str) {
		init(str.c_str());
	}

private:
	void init(PCWSTR str);
};

bool IsUserAdmin();


AutoUTF	GetUser(HANDLE hToken);

///======================================================================================= WinPolicy
namespace	WinPolicy {
	void		InitLsaString(LSA_UNICODE_STRING &lsaString, const AutoUTF &in);
	LSA_HANDLE	GetPolicyHandle(const AutoUTF &dom = L"");
	NTSTATUS	AccountRightAdd(const AutoUTF &name, const AutoUTF &right, const AutoUTF &dom = L"");
	NTSTATUS	AccountRightDel(const AutoUTF &name, const AutoUTF &right, const AutoUTF &dom = L"");

	bool		GetTokenUser(HANDLE	hToken, AutoUTF &name);
}

///▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒ net_log
///========================================================================================== WinLog
class		WinLog {
	HANDLE	m_hndl;

	WinLog();
public:
	~WinLog() {
		::DeregisterEventSource(m_hndl);
	}
	WinLog(PCWSTR name): m_hndl(::RegisterEventSourceW(nullptr, name)) {
		CheckApi(m_hndl != nullptr);
	}
//	void		Write(DWORD Event, WORD Count, LPCWSTR *Strings) {
//		PSID user = nullptr;
//		HANDLE token;
//		PTOKEN_USER token_user = nullptr;
//		if (OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, FALSE, &token)) {
//			token_user = (PTOKEN_USER)DefaultTokenInformation(token, TokenUser);
//			if (token_user)
//				user = token_user->User.Sid;
//			CloseHandle(token);
//		}
//		ReportEventW(m_hndl, EVENTLOG_ERROR_TYPE, 0, Event, user, Count, 0, Strings, nullptr);
//		free(token_user);
//	}

	static void		Register(PCWSTR name, PCWSTR path = nullptr) {
		WCHAR	fullpath[MAX_PATH_LEN];
		WCHAR	key[MAX_PATH_LEN];
		if (!path || Empty(path)) {
			CheckApi(::GetModuleFileNameW(0, fullpath, sizeofa(fullpath)));
		} else {
			Copy(fullpath, path, sizeofa(fullpath));
		}
		HKEY	hKey = nullptr;
		Copy(key, L"SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\", sizeofa(key));
		Cat(key, name, sizeofa(key));
		CheckApi(::RegCreateKeyW(HKEY_LOCAL_MACHINE, key, &hKey) == ERROR_SUCCESS);
		// Add the Event ID message-file name to the subkey.
		::RegSetValueExW(hKey, L"EventMessageFile", 0, REG_EXPAND_SZ, (LPBYTE)fullpath, (DWORD)((Len(fullpath) + 1)*sizeof(WCHAR)));
		// Set the supported types flags.
		DWORD dwData = EVENTLOG_ERROR_TYPE | EVENTLOG_WARNING_TYPE | EVENTLOG_INFORMATION_TYPE;
		::RegSetValueExW(hKey, L"TypesSupported", 0, REG_DWORD, (LPBYTE)&dwData, sizeof(dwData));
		::RegCloseKey(hKey);
	}
};

///▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒ net_SD
inline void	SetOwner(HANDLE handle, PSID owner, SE_OBJECT_TYPE type = SE_FILE_OBJECT) {
	CheckApiError(::SetSecurityInfo(handle, type, OWNER_SECURITY_INFORMATION, owner, nullptr, nullptr, nullptr));
}
inline void	SetGroup(HANDLE handle, PSID owner, SE_OBJECT_TYPE type = SE_FILE_OBJECT) {
	CheckApiError(::SetSecurityInfo(handle, type, GROUP_SECURITY_INFORMATION, nullptr, owner, nullptr, nullptr));
}
inline void	SetDacl(HANDLE handle, PACL pACL, SE_OBJECT_TYPE type = SE_FILE_OBJECT) {
	CheckApiError(::SetSecurityInfo(handle, type, DACL_SECURITY_INFORMATION, nullptr, nullptr, pACL, nullptr));
}
inline void	SetSacl(HANDLE handle, PACL pACL, SE_OBJECT_TYPE type = SE_FILE_OBJECT) {
	CheckApiError(::SetSecurityInfo(handle, type, SACL_SECURITY_INFORMATION, nullptr, nullptr, nullptr, pACL));
}

///=========================================================================================== WinSD
/// Security descriptor (Дескриптор защиты)
/// Version		- версия SD (revision)
/// Flags		- флаги состояния
/// Owner SID	- sid владельца
/// Group SID	- sid группы (не используется вендой, лишь для совместимости с POSIX)
/// DACL		- список записей контроля доступа
/// SACL		- список записей аудита
const	DWORD ALL_SD_INFO = OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION;

class	WinSD {
protected:
	PSECURITY_DESCRIPTOR	m_pSD;

	WinSD(): m_pSD(nullptr) {
	}

public:
	virtual	~WinSD() = 0;

	operator	PSECURITY_DESCRIPTOR() const {
		return	m_pSD;
	}

	bool	IsValid() const {
		return	IsValid(m_pSD);
	}
	bool	IsProtected() const {
		return	IsProtected(m_pSD);
	}
	bool	IsSelfRelative() const {
		return	IsSelfRelative(m_pSD);
	}
	DWORD	Size() const {
		return	Size(m_pSD);
	}

	WORD	Control() const {
		return	GetControl(m_pSD);
	}
	void	Control(WORD flag, bool s) {
		SetControl(m_pSD, flag, s);
	}
	AutoUTF	Owner() const {
		return	Sid::name(GetOwner(m_pSD));
	}
	void	SetOwner(PSID pSid, bool deflt = false) {
		SetOwner(m_pSD, pSid, deflt);
	}
	AutoUTF	Group() const {
		return	Sid::name(GetGroup(m_pSD));
	}
	void	SetGroup(PSID pSid, bool deflt = false) {
		SetGroup(m_pSD, pSid, deflt);
	}
	PACL	Dacl() const {
		return	GetDacl(m_pSD);
	}
	void	SetDacl(PACL dacl) {
		SetDacl(m_pSD, dacl);
	}
	void	MakeSelfRelative();
	void	Protect(bool pr) {
		Control(SE_DACL_PROTECTED, pr);
	}

	AutoUTF	AsSddl(SECURITY_INFORMATION in = ALL_SD_INFO) const {
		return	AsSddl(m_pSD, in);
	}
	AutoUTF	AsSddlOwner() const {
		return	AsSddl(OWNER_SECURITY_INFORMATION);
	}
	AutoUTF	AsSddlDACL() const {
		return	AsSddl(DACL_SECURITY_INFORMATION);
	}

	AutoUTF	Parse() const {
		return	Parse(m_pSD);
	}

	static void	Free(PSECURITY_DESCRIPTOR &in) {
		if (in) {
			::LocalFree(in);
			in = nullptr;
		}
	}
	static bool	IsValid(PSECURITY_DESCRIPTOR pSD) {
		return	pSD && ::IsValidSecurityDescriptor(pSD);
	}
	static bool	IsProtected(PSECURITY_DESCRIPTOR pSD) {
		WORD	control = GetControl(pSD);
		return	WinFlag::Check(control, (WORD)SE_DACL_PROTECTED);
	}
	static bool	IsSelfRelative(PSECURITY_DESCRIPTOR pSD) {
		WORD	control = GetControl(pSD);
		return	WinFlag::Check(control, (WORD)SE_SELF_RELATIVE);
	}

	static DWORD	Size(PSECURITY_DESCRIPTOR pSD) {
		CheckApi(IsValid(pSD));
		return	::GetSecurityDescriptorLength(pSD);
	}
	static WORD		GetControl(PSECURITY_DESCRIPTOR pSD) {
		WORD	Result = 0;
		DWORD	Revision;
		CheckApi(::GetSecurityDescriptorControl(pSD, &Result, &Revision));
		return	Result;
	}
	static PSID		GetOwner(PSECURITY_DESCRIPTOR pSD) {
		PSID	psid;
		BOOL	bTmp;
		CheckApi(::GetSecurityDescriptorOwner(pSD, &psid, &bTmp));
		return	psid;
	}
	static PSID		GetGroup(PSECURITY_DESCRIPTOR pSD) {
		PSID	psid;
		BOOL	bTmp;
		CheckApi(::GetSecurityDescriptorGroup(pSD, &psid, &bTmp));
		return	psid;
	}
	static PACL		GetDacl(PSECURITY_DESCRIPTOR pSD) {
		BOOL	bDaclPresent   = false;
		BOOL	bDaclDefaulted = false;
		PACL	Result = nullptr;
		CheckApi(::GetSecurityDescriptorDacl(pSD, &bDaclPresent, &Result, &bDaclDefaulted));
		CheckApi(bDaclPresent);
		return	Result;
	}

	static void	SetControl(PSECURITY_DESCRIPTOR pSD, WORD flag, bool s) {
		CheckApi(::SetSecurityDescriptorControl(pSD, flag, s ? flag : 0));
	}
	static void	SetOwner(PSECURITY_DESCRIPTOR pSD, PSID pSid, bool deflt = false) {
		CheckApi(::SetSecurityDescriptorOwner(pSD, pSid, deflt));
	}
	static void	SetGroup(PSECURITY_DESCRIPTOR pSD, PSID pSid, bool deflt = false) {
		CheckApi(::SetSecurityDescriptorGroup(pSD, pSid, deflt));
	}
	static void	SetDacl(PSECURITY_DESCRIPTOR pSD, PACL pACL) {
		CheckApi(::SetSecurityDescriptorDacl(pSD, true, pACL, false));
	}
	static void	SetSacl(PSECURITY_DESCRIPTOR pSD, PACL pACL) {
		CheckApi(::SetSecurityDescriptorSacl(pSD, true, pACL, false));
	}

	static AutoUTF	AsSddl(PSECURITY_DESCRIPTOR pSD, SECURITY_INFORMATION in = ALL_SD_INFO);
	static AutoUTF	Parse(PSECURITY_DESCRIPTOR pSD);
};

/// Security descriptor by SDDL
class	WinSDDL: public WinSD {
public:
	WinSDDL(const AutoUTF &in);
};

/// Absolute Security descriptor
class	WinAbsSD: public WinSD {
	PSID	m_owner;
	PSID	m_group;
	PACL	m_dacl;
	PACL	m_sacl;

	void	Init(PSECURITY_DESCRIPTOR sd);
public:
	~WinAbsSD();
	WinAbsSD();
	WinAbsSD(const WinSD &sd) {
		Init((PSECURITY_DESCRIPTOR)sd);
	}
	WinAbsSD(PSECURITY_DESCRIPTOR sd) {
		Init(sd);
	}
	WinAbsSD(PCWSTR usr, PCWSTR grp, mode_t mode);
};

void		SetOwner(const AutoUTF &path, PSID owner, bool setpriv, SE_OBJECT_TYPE type = SE_FILE_OBJECT);
inline void	SetOwner(const AutoUTF &path, PSID owner, SE_OBJECT_TYPE type = SE_FILE_OBJECT) {
	SetOwner(path, owner, false, type);
}
void		SetOwnerSD(const AutoUTF &name, PSECURITY_DESCRIPTOR pSD, SE_OBJECT_TYPE type = SE_FILE_OBJECT);

inline void	SetGroup(const AutoUTF &path, PSID owner, SE_OBJECT_TYPE type = SE_FILE_OBJECT) {
	CheckApiError(::SetNamedSecurityInfoW((PWSTR)path.c_str(), type, GROUP_SECURITY_INFORMATION, nullptr, owner, nullptr, nullptr));
}
void		SetGroupSD(const AutoUTF &name, PSECURITY_DESCRIPTOR pSD, SE_OBJECT_TYPE type = SE_FILE_OBJECT);

inline void	SetDacl(const AutoUTF &path, PACL pacl, SE_OBJECT_TYPE type = SE_FILE_OBJECT) {
	CheckApiError(::SetNamedSecurityInfoW((PWSTR)path.c_str(), type, DACL_SECURITY_INFORMATION, nullptr, nullptr, pacl, nullptr));
}
void		SetDacl(const AutoUTF &name, PSECURITY_DESCRIPTOR pSD, SE_OBJECT_TYPE type = SE_FILE_OBJECT);

inline void	SetSacl(const AutoUTF &path, PACL pacl, SE_OBJECT_TYPE type = SE_FILE_OBJECT) {
	CheckApiError(::SetNamedSecurityInfoW((PWSTR)path.c_str(), type, SACL_SECURITY_INFORMATION, nullptr, nullptr, nullptr, pacl));
}

void		SetSecurity(const AutoUTF &path, PSECURITY_DESCRIPTOR pSD, SE_OBJECT_TYPE type = SE_FILE_OBJECT);
inline void	SetSecurity(const AutoUTF &path, const AutoUTF &sddl, SE_OBJECT_TYPE type = SE_FILE_OBJECT) {
	WinSDDL	sd(sddl);
	SetSecurity(path, sd, type);
}
void		SetSecurity(HANDLE hnd, PSECURITY_DESCRIPTOR pSD, SE_OBJECT_TYPE type = SE_FILE_OBJECT);

AutoUTF		GetOwner(HANDLE hnd, SE_OBJECT_TYPE type = SE_FILE_OBJECT);
AutoUTF		GetOwner(const AutoUTF &path, SE_OBJECT_TYPE type = SE_FILE_OBJECT);
AutoUTF		GetGroup(HANDLE hnd, SE_OBJECT_TYPE type = SE_FILE_OBJECT);
AutoUTF		GetGroup(const AutoUTF &path, SE_OBJECT_TYPE type = SE_FILE_OBJECT);

AutoUTF		Mode2Sddl(const AutoUTF &name, const AutoUTF &group, mode_t mode, const AutoUTF dom = L"");
AutoUTF		MakeSDDL(const AutoUTF &name, const AutoUTF &group, mode_t mode, bool pr = false, const AutoUTF dom = L"");

/// Security descriptor by handle
class	WinSDH: public WinSD {
	HANDLE			m_hnd;
	SE_OBJECT_TYPE	m_type;
public:
	WinSDH(HANDLE handle, SE_OBJECT_TYPE type = SE_FILE_OBJECT): m_hnd(handle), m_type(type) {
		Get();
	}
	HANDLE	hnd() const {
		return	m_hnd;
	}
	void	Get() {
		Free(m_pSD);
		CheckApiError(::GetSecurityInfo(m_hnd, m_type,
										OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION |
										DACL_SECURITY_INFORMATION,
										nullptr, nullptr, nullptr, nullptr, &m_pSD));
	}
	void	Set() const {
		SetSecurity(m_hnd, m_pSD, m_type);
	}
};

/// Security descriptor by name
class	WinSDW: public WinSD {
	AutoUTF			m_name;
	SE_OBJECT_TYPE	m_type;
public:
	WinSDW(const AutoUTF &name, SE_OBJECT_TYPE type = SE_FILE_OBJECT): m_name(name), m_type(type) {
		Get();
	}
	AutoUTF	name() const {
		return	m_name;
	}
	void	Get() {
		Free(m_pSD);
		CheckApiError(::GetNamedSecurityInfoW((PWSTR)m_name.c_str(), m_type,
											  OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION |
											  DACL_SECURITY_INFORMATION,
											  nullptr, nullptr, nullptr, nullptr, &m_pSD));
	}
	void	Set() const {
		SetSecurity(m_name, m_pSD, m_type);
	}
	void	Set(const AutoUTF &path) const {
		SetSecurity(path, m_pSD, m_type);
	}
};

///▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒ net_dacl
///========================================================================================= WinDacl
class		WinDacl {
	PACL	m_PACL;
	bool	needDelete;

	void	Init(PACL pACL);
	void	Init(PSECURITY_DESCRIPTOR pSD);
public:
	~WinDacl() {
		if (needDelete)
			WinMem::Free(m_PACL);
	}
	WinDacl(size_t size);
	WinDacl(PACL pACL);
	WinDacl(PSECURITY_DESCRIPTOR pSD);
	WinDacl(const AutoUTF &name, SE_OBJECT_TYPE type = SE_FILE_OBJECT);

	operator	PACL() const {
		return	m_PACL;
	}

	bool	Valid() const {
		return	Valid(m_PACL);
	}
	ACL_SIZE_INFORMATION	GetAclInfo() const {
		ACL_SIZE_INFORMATION	info;
		GetAclInfo(m_PACL, info);
		return	info;
	}
	size_t	CountAces() const {
		return	CountAces(m_PACL);
	}
	size_t	GetUsed() const {
		return	GetUsed(m_PACL);
	}
	size_t	GetFree() const {
		return	GetFree(m_PACL);
	}
	size_t	GetSize() const {
		return	GetSize(m_PACL);
	}
	AutoUTF	Parse() const {
		return	Parse(m_PACL);
	}

	void	AddA(const Sid &sid);
	void	AddD(const Sid &sid);

	void	DelInheritedAces() {
		DelInheritedAces(m_PACL);
	}

	static bool		Valid(PACL in) {
		return	::IsValidAcl(in);
	}
	static void		GetAclInfo(PACL pACL, ACL_SIZE_INFORMATION &out);
	static void		DelInheritedAces(PACL pACL);
	static size_t	CountAces(PACL pACL);
	static size_t	CountAces(PACL pACL, size_t &sz, bool inh);
	static size_t	GetUsed(PACL pACL);
	static size_t	GetFree(PACL pACL);
	static size_t	GetSize(PACL pACL);
	static PVOID	GetAce(PACL pACL, size_t index);

	void			SetTo(DWORD flag, const AutoUTF &name, SE_OBJECT_TYPE type) const {
		SetTo(m_PACL, flag, name, type);
	}

	static AutoUTF	Parse(PACL pACL);

	static void		SetTo(PACL pACL, DWORD flag, const AutoUTF &path, SE_OBJECT_TYPE type = SE_FILE_OBJECT) {
		CheckApiError(::SetNamedSecurityInfoW((PWSTR)path.c_str(), type,
											  DACL_SECURITY_INFORMATION | flag, nullptr, nullptr, pACL, nullptr));
	}
	static void		Inherit(const AutoUTF &path, SE_OBJECT_TYPE type = SE_FILE_OBJECT);
	static void		Protect(const AutoUTF &path, bool copy, SE_OBJECT_TYPE type = SE_FILE_OBJECT);
};

///======================================================================================= WinAccess
struct		AccessInfo {
	AutoUTF	type;
	AutoUTF unix;
	DWORD	mask;
};

class		WinAccess : public MultiMapContainer<AutoUTF, AccessInfo> {
public:
	WinAccess(const WinSD &sd, bool autocache = true): pACL(sd.Dacl()) {
		if (autocache)
			Cache();
	}

	bool			Cache();

private:
	PACL pACL;
};

///==================================================================================== WinSysTimers
struct		WinSysTimers {
	LARGE_INTEGER liKeBootTime;
	LARGE_INTEGER liKeSystemTime;
	LARGE_INTEGER liExpTimeZoneBias;
	ULONG uCurrentTimeZoneId;
	DWORD dwReserved;

	WinSysTimers();

	size_t	Uptime(size_t del = 1);

	AutoUTF	UptimeAsText();
};

///▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒ net_if
///=========================================================================================== WinIf
inline AutoUTF		MacAsStr(const PBYTE mac, size_t size) {
	WCHAR	buf[(size + 1) * 4];
	WinMem::Zero(buf, sizeof(buf));
	PWSTR	tmp = buf;
	for (size_t i = 0; i < size; ++i) {
		if (i == (size - 1))
			tmp += snprintf(tmp, sizeofa(buf) - i * 2, L"%.2X", mac[i]);
		else
			tmp += snprintf(tmp, sizeofa(buf) - i * 2, L"%.2X-", mac[i]);
	}
	return	AutoUTF(buf);
}
inline AutoUTF		IpAsStr(LPSOCKADDR addr, size_t	len) {
	WCHAR	buf[64];
	WinMem::Zero(buf, sizeof(buf));
	DWORD	size = sizeofa(buf);
	::WSAAddressToStringW(addr, len, nullptr, buf, &size);
	return	AutoUTF(buf);
}
inline AutoUTF		IpAsStr(SOCKET_ADDRESS pAddr) {
	return	IpAsStr(pAddr.lpSockaddr, pAddr.iSockaddrLength);
}

class	WinIp: public MapContainer<AutoUTF, SOCKET_ADDRESS> {
public:
	WinIp(bool autocache = true) {
		if (autocache)
			Cache();
	}
	bool		Cache();
};

///▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒ net_sock
///======================================================================================== WSockLib
class		WSockLib: private Uncopyable {
public:
	~WSockLib() {
		::WSACleanup();
	}

	static WSockLib &Init() {
		static WSockLib init;
		return	init;
	}

private:
	WSockLib() {
		CheckWSock(::WSAStartup(MAKEWORD(1, 2), &wsaData));
	}

	WSADATA	wsaData;
};

///=========================================================================================== WSock
class		WSock {
public:
	~WSock() {
		::closesocket(m_sock);
	}
	WSock(int fam = AF_INET): m_sock(INVALID_SOCKET), m_fam(fam) {
		WSockLib::Init();
		m_sock = ::socket(m_fam, SOCK_STREAM, 0);
		if (m_sock == INVALID_SOCKET)
			throw	WSockError("WinSock allocate socket error: ");
	}

	void		Connect(const AutoUTF &ip, DWORD port) {
		INT		size = 128;
		auto_buf<PSOCKADDR>	addr(size);
		INT		err = ::WSAStringToAddressW((PWSTR)ip.c_str(), m_fam, nullptr, addr, &size);
		if (err && err != WSAEFAULT)
			throw	WSockError("WinSock determine address error: ");
		if (err == WSAEFAULT) {
			addr.reserve(size);
			err = ::WSAStringToAddressW((PWSTR)ip.c_str(), m_fam, nullptr, addr, &size);
			if (err)
				throw	WSockError("WinSock determine address error: ");
		}
		if (m_fam == AF_INET || m_fam == AF_INET6) {
			sockaddr_in* tmp = (sockaddr_in*)addr.data();
			tmp->sin_port = htons(port);
		}
		if (::connect(m_sock, addr.data(), size))
			throw	WSockError("WinSock connect error: ");

	}

	void		Send(void* buf, size_t len) {
		::send(m_sock, (const char*)buf, len, 0);
	}

	operator	SOCKET() {
		return	m_sock;
	}

private:
	SOCKET	m_sock;
	int		m_fam;
};

#endif // WIN_NET_HPP
