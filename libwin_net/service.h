/**
	win_svc

	@classes	()
	@author		© 2009 Andrew Grechkin
	@link		()
**/

#ifndef WIN_SVC_HPP
#define WIN_SVC_HPP

#include "win_net.h"

///▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒ net_svc
///========================================================================================== WinScm
class WinScm {
public:
	static SC_HANDLE Open(ACCESS_MASK acc = SC_MANAGER_CONNECT, RemoteConnection *conn = nullptr);

	static void Close(SC_HANDLE &in);

public:
	~WinScm() {
		Close(m_hndl);
	}

	//WinScm(ACCESS_MASK acc = SC_MANAGER_CONNECT | SC_MANAGER_ENUMERATE_SERVICE, RemoteConnection *conn = nullptr): m_hndl(nullptr), m_mask(acc), m_conn(conn) {
	WinScm(ACCESS_MASK acc, RemoteConnection *conn = nullptr):
		m_hndl(Open(acc, conn)) {
	}

	void Close() {
		Close(m_hndl);
	}

	void Reopen(ACCESS_MASK acc = SC_MANAGER_CONNECT | SC_MANAGER_ENUMERATE_SERVICE, RemoteConnection *conn = nullptr);

	operator SC_HANDLE() const {
		return m_hndl;
	}

	void Create(PCWSTR name, PCWSTR path, DWORD StartType = SERVICE_DEMAND_START, PCWSTR disp = nullptr);

private:
	SC_HANDLE			m_hndl;
};

///========================================================================================== WinSvc
class WinSvc {
public:
	~WinSvc() {
		Close(m_hndl);
	}

	WinSvc(PCWSTR name, ACCESS_MASK access, RemoteConnection *conn = nullptr);

	WinSvc(PCWSTR name, ACCESS_MASK access, const WinScm &scm);

	void QueryConfig(auto_buf<LPQUERY_SERVICE_CONFIGW> &buf) const;
	void QueryConfig2(auto_buf<PBYTE> &buf, DWORD level) const;

//	template<typename Functor>
//	void WaitForState(DWORD state, DWORD dwTimeout, Functor &func, PVOID param = nullptr) const {
//		DWORD	dwStartTime = ::GetTickCount();
//		DWORD	dwBytesNeeded;
//		SERVICE_STATUS_PROCESS ssp = {0};
//		while (true) {
//			CheckApi(::QueryServiceStatusEx(m_hndl, SC_STATUS_PROCESS_INFO, (PBYTE)&ssp, sizeof(ssp), &dwBytesNeeded));
//			if (ssp.dwCurrentState == state)
//				break;
//			if (::GetTickCount() - dwStartTime > dwTimeout)
//				throw	ApiError(WAIT_TIMEOUT);
//			func(state, ::GetTickCount() - dwStartTime, param);
//		}
//	}

	void WaitForState(DWORD state, DWORD dwTimeout) const;

	bool Start();
	bool Stop();
	void Continue();
	void Pause();

	void Del();

	void SetStartup(DWORD type);
	void SetLogon(const ustring &user, const ustring &pass = L"", bool desk = false);

	void GetStatus(SERVICE_STATUS_PROCESS &info) const;
	DWORD GetState() const;
	DWORD GetType() const;
	ustring GetUser() const;

	operator SC_HANDLE() const {
		return m_hndl;
	}

private:
	SC_HANDLE Open(SC_HANDLE scm, PCWSTR name, ACCESS_MASK acc);
	void Close(SC_HANDLE &hndl);

	SC_HANDLE m_hndl;
};

///====================================================================================== WinService
namespace	WinService {
	ustring	ParseState(DWORD in);
	ustring	ParseState(const ustring &name);
	void	WaitForState(const ustring &name, DWORD state, DWORD dwTimeout = 10000);
//	DWORD	WaitForState(const WinSvcHnd &sch, DWORD state, DWORD dwTimeout = 10000);

	void	Del(const ustring &name);
	void	Start(const ustring &name);
	void	Stop(const ustring &name);

	void	Auto(const ustring &name);
	void	Manual(const ustring &name);
	void	Disable(const ustring &name);

	bool	IsExist(const ustring &name);
	bool	IsRunning(const ustring &name);
	bool	IsStarting(const ustring &name);
	bool	IsStopping(const ustring &name);
	bool	IsStopped(const ustring &name);

	DWORD	GetStartType(const ustring &name);
	bool	IsAuto(const ustring &name);
	bool	IsManual(const ustring &name);
	bool	IsDisabled(const ustring &name);

	void	GetStatus(SC_HANDLE sch, SERVICE_STATUS_PROCESS &ssp);
	void	GetStatus(const ustring &name, SERVICE_STATUS_PROCESS &ssp);
	DWORD	GetState(const ustring &name);
	ustring	GetDesc(const ustring &name);
	ustring	GetDName(const ustring &name);
	ustring	GetPath(const ustring &name);

//	DWORD	SetConf(const ustring &name, SvcConf &conf);
	void	SetDesc(const ustring &name, const ustring &in);
	void	SetDName(const ustring &name, const ustring &in);
	void	SetPath(const ustring &name, const ustring &in);
};

///========================================================================================== struct
struct s_ServiceInfo: public _SERVICE_STATUS {
	ustring		name;				// AN C0
	ustring		dname;				// N
	ustring		path;				// C3
	ustring		descr;				// Z
	ustring		OrderGroup;			// C5
	ustring		ServiceStartName;	// C6
	mstring		Dependencies;		// LN

	DWORD		StartType;			// C2
	DWORD		ErrorControl;
	DWORD		TagId;

	s_ServiceInfo() {
		WinMem::Zero(this, sizeof(_SERVICE_STATUS));
		StartType = ErrorControl = TagId = 0;
	}
	s_ServiceInfo(const ustring &n, const SERVICE_STATUS &sp): _SERVICE_STATUS(sp), name(n) {
		StartType = ErrorControl = TagId = 0;
	}
};

void InstallService(PCWSTR name, PCWSTR path, DWORD StartType = SERVICE_DEMAND_START, PCWSTR dispname = nullptr);
void UninstallService(PCWSTR name);

///==================================================================================== WinServices
struct ServiceInfo {
	ustring		Name;				// AN C0
	ustring		DName;				// N
	ustring		Path;				// C3
	ustring		Descr;				// Z
	ustring		OrderGroup;			// C5
	ustring		ServiceStartName;	// C6
	mstring		Dependencies;		// LN

	DWORD		StartType;			// C2
	DWORD		ErrorControl;
	DWORD		TagId;
    SERVICE_STATUS	Status;

	ServiceInfo(const WinScm &scm, const ENUM_SERVICE_STATUSW &st);

	ServiceInfo(const ustring &nm):
		Name(nm) {
	}

	bool operator<(const ServiceInfo &rhs) const;

	bool operator==(const ustring &nm) const;
};

class WinServices: private std::vector<ServiceInfo> {
public:
	typedef ServiceInfo value_type;
	typedef std::vector<ServiceInfo> class_type;

	typedef class_type::iterator iterator;
	typedef class_type::const_iterator const_iterator;
	using class_type::begin;
	using class_type::end;
	using class_type::size;

	static const DWORD type_svc = SERVICE_WIN32 | SERVICE_INTERACTIVE_PROCESS;
	static const DWORD type_drv = SERVICE_ADAPTER | SERVICE_DRIVER;

public:
	WinServices(RemoteConnection *conn = nullptr, bool autocache = true):
		m_conn(conn),
		m_type(type_svc) {
		if (autocache)
			cache();
	}

	bool	cache() {
		return cache_by_type(m_type);
	}
	bool	cache_by_name(const ustring &in);
	bool	cache_by_state(DWORD state = SERVICE_STATE_ALL);
	bool	cache_by_type(DWORD type = type_svc);

	bool	services() const {
		return m_type == type_svc;
	}
	bool	drivers() const {
		return m_type == type_drv;
	}

	DWORD	type() const {
		return m_type;
	}

	iterator find(const ustring &name);

	void	add(const ustring &name, const ustring &path);
	void	del(const ustring &name);
	void	del(iterator it);

private:
	RemoteConnection *m_conn;
	DWORD			 m_type;
};

#endif
