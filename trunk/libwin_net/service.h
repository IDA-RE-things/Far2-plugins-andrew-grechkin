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
		return	m_hndl;
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
	void SetLogon(const AutoUTF &user, const AutoUTF &pass = L"", bool desk = false);

	void GetStatus(SERVICE_STATUS_PROCESS &info) const;
	DWORD GetState() const;
	DWORD GetType() const;
	AutoUTF GetUser() const;

	operator SC_HANDLE() const {
		return	m_hndl;
	}

private:
	SC_HANDLE Open(SC_HANDLE scm, PCWSTR name, ACCESS_MASK acc);
	void Close(SC_HANDLE &hndl);

	SC_HANDLE m_hndl;
};

///====================================================================================== WinService
namespace	WinService {
	AutoUTF	ParseState(DWORD in);
	AutoUTF	ParseState(const AutoUTF &name);
	void	WaitForState(const AutoUTF &name, DWORD state, DWORD dwTimeout = 10000);
//	DWORD	WaitForState(const WinSvcHnd &sch, DWORD state, DWORD dwTimeout = 10000);

	void	Del(const AutoUTF &name);
	void	Start(const AutoUTF &name);
	void	Stop(const AutoUTF &name);

	void	Auto(const AutoUTF &name);
	void	Manual(const AutoUTF &name);
	void	Disable(const AutoUTF &name);

	bool	IsExist(const AutoUTF &name);
	bool	IsRunning(const AutoUTF &name);
	bool	IsStarting(const AutoUTF &name);
	bool	IsStopping(const AutoUTF &name);
	bool	IsStopped(const AutoUTF &name);

	DWORD	GetStartType(const AutoUTF &name);
	bool	IsAuto(const AutoUTF &name);
	bool	IsManual(const AutoUTF &name);
	bool	IsDisabled(const AutoUTF &name);

	void	GetStatus(SC_HANDLE sch, SERVICE_STATUS_PROCESS &ssp);
	void	GetStatus(const AutoUTF &name, SERVICE_STATUS_PROCESS &ssp);
	DWORD	GetState(const AutoUTF &name);
	AutoUTF	GetDesc(const AutoUTF &name);
	AutoUTF	GetDName(const AutoUTF &name);
	AutoUTF	GetPath(const AutoUTF &name);

//	DWORD	SetConf(const AutoUTF &name, SvcConf &conf);
	void	SetDesc(const AutoUTF &name, const AutoUTF &in);
	void	SetDName(const AutoUTF &name, const AutoUTF &in);
	void	SetPath(const AutoUTF &name, const AutoUTF &in);
};

///========================================================================================== struct
struct s_ServiceInfo: public _SERVICE_STATUS {
	AutoUTF		name;				// AN C0
	AutoUTF		dname;				// N
	AutoUTF		path;				// C3
	AutoUTF		descr;				// Z
	AutoUTF		OrderGroup;			// C5
	AutoUTF		ServiceStartName;	// C6
	mstring		Dependencies;		// LN

	DWORD		StartType;			// C2
	DWORD		ErrorControl;
	DWORD		TagId;

	s_ServiceInfo() {
		WinMem::Zero(this, sizeof(_SERVICE_STATUS));
		StartType = ErrorControl = TagId = 0;
	}
	s_ServiceInfo(const AutoUTF &n, const SERVICE_STATUS &sp): _SERVICE_STATUS(sp), name(n) {
		StartType = ErrorControl = TagId = 0;
	}
};

///===================================================================================== WinServices
class WinServices : public MapContainer<AutoUTF, s_ServiceInfo> {
	RemoteConnection	*m_conn;
	DWORD				m_type;
public:
	~WinServices() {
		Clear();
	}
	WinServices(RemoteConnection *conn = nullptr, bool autocache = true): m_conn(conn) {
		m_type = SERVICE_WIN32 | SERVICE_INTERACTIVE_PROCESS;
		if (autocache)
			Cache();
	}
	bool				Cache();
	bool				CacheByName(const AutoUTF &in);
	bool				CacheByState(DWORD state = SERVICE_STATE_ALL);
	bool				CacheByType(DWORD state = SERVICE_STATE_ALL);

	bool				services() const {
		return m_type == (SERVICE_WIN32 | SERVICE_INTERACTIVE_PROCESS);
	}
	bool				drivers() const {
		return m_type == (SERVICE_ADAPTER | SERVICE_DRIVER);
	}
	DWORD				type() const {
		return	m_type;
	}
	DWORD				state() const {
		return	Value().dwCurrentState;
	}
	void				services(bool st) {
		if (st)
			m_type = SERVICE_WIN32 | SERVICE_INTERACTIVE_PROCESS;
	}
	void				drivers(bool st) {
		if (st)
			m_type = SERVICE_ADAPTER | SERVICE_DRIVER;
	}

	bool				Add(const AutoUTF &name, const AutoUTF &path);
	bool				Del();

	bool				Start() const;
	bool				Stop() const;
	bool				Restart() const;

	bool				IsAuto() const;
	bool				IsRunning() const;

	AutoUTF				GetName() const;
	AutoUTF				GetDName() const;
	AutoUTF				GetPath() const;
};

void InstallService(PCWSTR name, PCWSTR path, DWORD StartType = SERVICE_DEMAND_START, PCWSTR dispname = nullptr);
void UninstallService(PCWSTR name);

///==================================================================================== WinServices1
struct ServiceInfo {
	AutoUTF		Name;				// AN C0
	AutoUTF		DName;				// N
	AutoUTF		Path;				// C3
	AutoUTF		Descr;				// Z
	AutoUTF		OrderGroup;			// C5
	AutoUTF		ServiceStartName;	// C6
	mstring		Dependencies;		// LN

	DWORD		StartType;			// C2
	DWORD		ErrorControl;
	DWORD		TagId;
    SERVICE_STATUS	Status;

    ServiceInfo(const WinScm &scm, const ENUM_SERVICE_STATUSW &st);

    ServiceInfo(const AutoUTF &nm):
    	Name(nm) {
	}

	bool operator<(const ServiceInfo &rhs) const;

	bool operator==(const AutoUTF &nm) const;
	//			try {
	//				WinSvc	svc(svc_stat[i].lpServiceName, SERVICE_QUERY_CONFIG, scm);
	//				svc.QueryConfig(svc_conf);
	//				info.path = svc_conf->lpBinaryPathName;
	//				info.OrderGroup = svc_conf->lpLoadOrderGroup;
	//				info.Dependencies = svc_conf->lpDependencies;
	//				info.ServiceStartName = svc_conf->lpServiceStartName;
	//				info.StartType = svc_conf->dwStartType;
	//				info.ErrorControl = svc_conf->dwErrorControl;
	//				info.TagId = svc_conf->dwTagId;
	//				svc.QueryConfig2(buf2, SERVICE_CONFIG_DESCRIPTION);
	//				LPSERVICE_DESCRIPTIONW ff = (LPSERVICE_DESCRIPTIONW)buf2.data();
	//				if (ff->lpDescription)
	//					info.descr = ff->lpDescription;
	//			} catch (WinError &e) {
	//				//	e.show();
	//			}
};

class WinServices1: private std::vector<ServiceInfo> {
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
	WinServices1(RemoteConnection *conn = nullptr, bool autocache = true):
		m_conn(conn),
		m_type(type_svc) {
		if (autocache)
			cache();
	}

	bool	cache() {
		return cache_by_type(m_type);
	}
	bool	cache_by_name(const AutoUTF &in);
	bool	cache_by_state(DWORD state = SERVICE_STATE_ALL);
	bool	cache_by_type(DWORD type = type_svc);

	bool	services() const {
		return m_type == type_svc;
	}
	bool	drivers() const {
		return m_type == type_drv;
	}

	iterator find(const AutoUTF &name);

	void	add(const AutoUTF &name, const AutoUTF &path);
	void	del(const AutoUTF &name);
	void	del(iterator it);

private:
	RemoteConnection *m_conn;
	DWORD			 m_type;
};

#endif
