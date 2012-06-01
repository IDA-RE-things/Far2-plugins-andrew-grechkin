/**
	win_svc

	@classes	()
	@author		© 2012 Andrew Grechkin
	@link		()
**/

#ifndef WIN_SVC_HPP
#define WIN_SVC_HPP

#include "win_net.h"
#include <libwin_def/mstring.h>

#include <vector>

///▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒ net_svc
///========================================================================================== WinScm
class WinSvc;
class RemoteConnection;

struct WinScm: private Uncopyable {
	~WinScm();

	WinScm(ACCESS_MASK acc = SC_MANAGER_CONNECT, RemoteConnection * conn = nullptr);

	operator SC_HANDLE() const {
		return m_hndl;
	}

	WinSvc create_service(PCWSTR name, PCWSTR path, DWORD StartType = SERVICE_DEMAND_START, PCWSTR disp = nullptr);

private:
	SC_HANDLE m_hndl;
};

///========================================================================================== WinSvc
struct WinSvc {
	~WinSvc();

	WinSvc(PCWSTR name, ACCESS_MASK access, RemoteConnection * conn = nullptr);

	WinSvc(PCWSTR name, ACCESS_MASK access, const WinScm & scm);

	void WaitForState(DWORD state, DWORD dwTimeout) const;

	void Start();
	void Stop();
	void Continue();
	void Pause();

	void Del();

	void set_startup(DWORD type);
	void set_logon(const ustring &user, const ustring &pass = L"", bool desk = false);

	void get_status(SERVICE_STATUS_PROCESS &info) const;
	DWORD get_state() const;
	DWORD get_type() const;
	ustring get_user() const;

	operator SC_HANDLE() const {
		return m_hndl;
	}

	static void Create(const ustring & name, const ustring & path, DWORD StartType = SERVICE_DEMAND_START, PCWSTR dispname = nullptr);
	static void Del(const ustring & name);
	static void Start(const ustring & name);
	static void Stop(const ustring & name);
	static void Restart(const ustring & name);
	static void Continue(const ustring & name);
	static void Pause(const ustring & name);

	static bool is_exist(const ustring &name);
	static bool is_running(const ustring &name);
	static bool is_starting(const ustring &name);
	static bool is_stopping(const ustring &name);
	static bool is_stopped(const ustring &name);

	static bool is_auto(const ustring &name);
	static bool is_manual(const ustring &name);
	static bool is_disabled(const ustring &name);

	static DWORD get_start_type(const ustring &name);
	static void get_status(SC_HANDLE sch, SERVICE_STATUS_PROCESS &ssp);
	static void get_status(const ustring &name, SERVICE_STATUS_PROCESS &ssp);
	static DWORD get_state(const ustring &name);

	static ustring get_desc(const ustring &name);
	static ustring get_dname(const ustring &name);
	static ustring get_path(const ustring &name);

	static void set_auto(const ustring &name);
	static void set_manual(const ustring &name);
	static void set_disable(const ustring &name);

	static void set_desc(const ustring &name, const ustring &in);
	static void set_dname(const ustring &name, const ustring &in);
	static void set_path(const ustring &name, const ustring &in);

private:
	WinSvc(SC_HANDLE svc):
		m_hndl(svc) {
	}

	auto_buf<LPQUERY_SERVICE_CONFIGW> QueryConfig() const;
	auto_buf<PBYTE> QueryConfig2(DWORD level) const;

	SC_HANDLE Open(SC_HANDLE scm, PCWSTR name, ACCESS_MASK acc);

	SC_HANDLE m_hndl;

	friend class WinScm;
	friend class ServiceInfo;
};

///===================================================================================== WinServices
struct ServiceInfo {
	ustring		Name;				// AN C0
	ustring		DName;				// N
	ustring		Path;				// C3
	ustring		Descr;				// Z
	ustring		OrderGroup;			// C5
	ustring		ServiceStartName;	// C6
	windef::mstring Dependencies;	// LN

	DWORD		StartType;			// C2
	DWORD		ErrorControl;
	DWORD		TagId;
    SERVICE_STATUS	Status;

	ServiceInfo(const WinScm &scm, const ENUM_SERVICE_STATUSW &st);

	ServiceInfo(const ustring &nm):
		Name(nm) {
	}

	bool operator<(const ServiceInfo & rhs) const;

	bool operator==(const ustring & nm) const;

	DWORD svc_type() const {
		return Status.dwServiceType;
	}

	DWORD svc_state() const {
		return Status.dwCurrentState;
	}

	DWORD start_type() const {
		return StartType;
	}

	DWORD error_control() const {
		return ErrorControl;
	}

	bool is_service() const {
		return svc_type() & (SERVICE_WIN32_OWN_PROCESS | SERVICE_WIN32_SHARE_PROCESS);
	}

	bool is_disabled() const {
		return start_type() == SERVICE_DISABLED;
	}
};

struct WinServices: private std::vector<ServiceInfo> {
	typedef ServiceInfo value_type;
	typedef std::vector<ServiceInfo> class_type;

	typedef class_type::iterator iterator;
	typedef class_type::const_iterator const_iterator;

	using class_type::begin;
	using class_type::end;
	using class_type::size;
	using class_type::empty;

	static const DWORD type_svc = SERVICE_WIN32 | SERVICE_INTERACTIVE_PROCESS;
	static const DWORD type_drv = SERVICE_ADAPTER | SERVICE_DRIVER;
	static const DWORD type_svc_op = SERVICE_WIN32_OWN_PROCESS | SERVICE_WIN32_SHARE_PROCESS;

public:
	WinServices(RemoteConnection * conn = nullptr, bool autocache = true);

	bool cache() {
		return cache_by_type(m_type);
	}
	bool cache_by_name(const ustring & in);
	bool cache_by_state(DWORD state = SERVICE_STATE_ALL);
	bool cache_by_type(DWORD type = type_svc);

	bool is_services() const {
		return m_type == type_svc;
	}

	bool is_drivers() const {
		return m_type == type_drv;
	}

	DWORD type() const {
		return m_type;
	}

	iterator find(const ustring & name);
	const_iterator find(const ustring & name) const;

	void	add(const ustring & name, const ustring & path);
	void	del(const ustring & name, PCWSTR msg = L"Unable to delete service");
	void	del(iterator it, PCWSTR msg = L"Unable to delete service");

private:
	RemoteConnection * m_conn;
	DWORD m_type;
};

#endif
