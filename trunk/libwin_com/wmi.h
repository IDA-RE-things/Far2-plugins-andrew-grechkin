/**
 * wmi
 * @classes		()
 * @author		Copyright © 2011 Andrew Grechkin
 * @link		(ole32, oleaut32, uuid)
**/

#ifndef WIN_COM_WMI_HPP
#define WIN_COM_WMI_HPP

#include "win_com.h"
//#include <objbase.h>
#include <wbemidl.h>

void get_param(Variant &out, const ComObject<IWbemClassObject> &obj, PCWSTR param = L"ReturnValue");

BStr get_class(const ComObject<IWbemClassObject> &obj);

BStr get_path(const ComObject<IWbemClassObject> &obj);

///=================================================================================== WmiConnection
class WmiConnection {
public:
	WmiConnection(PCWSTR srv = nullptr, PCWSTR user = nullptr, PCWSTR pass = nullptr);

	operator		bool() const {
		return	m_svc;
	}

	IWbemServices*	GetIWbemServices() const {
		return m_svc;
	}

	IWbemServices*	operator->() const {
		return	m_svc;
	}

	ComObject<IEnumWbemClassObject>	Query(PCWSTR query, ssize_t flags = WBEM_FLAG_FORWARD_ONLY) const;

	ComObject<IEnumWbemClassObject>	Enum(PCWSTR path, ssize_t flags = WBEM_FLAG_SHALLOW | WBEM_FLAG_FORWARD_ONLY) const;

	template<typename Functor>
	void			QueryExec(PCWSTR query, Functor Func, PVOID data = nullptr) {
		ComObject<IEnumWbemClassObject>	ewco = Query(query);

		ComObject<IWbemClassObject>	obj;
		HRESULT ret = WBEM_S_NO_ERROR;
		ULONG ulCount = 0;
		while (ret == WBEM_S_NO_ERROR) {
			ret = ewco->Next(WBEM_INFINITE, 1, &obj, &ulCount);
			if (!ulCount || !SUCCEEDED(ret) || !Func(*this, obj, data))
				break;
		}
	}

	template<typename Functor>
	void			EnumExec(PCWSTR clname, Functor Func, PVOID data = nullptr) {
		ComObject<IEnumWbemClassObject>	ewco = Enum(clname);

		ComObject<IWbemClassObject>	obj;
		HRESULT ret = WBEM_S_NO_ERROR;
		ULONG ulCount = 0;
		while (ret == WBEM_S_NO_ERROR) {
			ret = ewco->Next(WBEM_INFINITE, 1, &obj, &ulCount);
			if (!ulCount || !SUCCEEDED(ret) || !Func(*this, obj, data))
				break;
		}
	}

	template<typename Functor>
	bool			Exec(PCWSTR clname, Functor &Func, PVOID data = nullptr) {
		ComObject<IWbemClassObject>	obj;
		CheckWmi(m_svc->GetObject((BSTR)clname, WBEM_FLAG_DIRECT_READ, nullptr, &obj, nullptr));
		return	Func(obj, data);
	}

	ComObject<IWbemClassObject>	get_object(PCWSTR path) const;
	ComObject<IWbemClassObject>	get_object(PCWSTR path, PCWSTR method) const;
	ComObject<IWbemClassObject>	get_method(PCWSTR path, PCWSTR method) const;

//	Variant		GetParam(PCWSTR path, PCWSTR param) const {
//		ComObject<IWbemClassObject> obj = get_object(path);
//		Variant	Result;
//		CheckWmi(obj->Get(param, 0, &Result, nullptr, nullptr));
//		return	Result;
//	}
//	Variant		GetParam(PCWSTR path, PCWSTR method, PCWSTR param) const {
//		ComObject<IWbemClassObject> obj = get_object(path, method);
//		Variant	Result;
//		CheckWmi(obj->Get(param, 0, &Result, nullptr, nullptr));
//		return	Result;
//	}

private:
	ComObject<IWbemServices> m_svc;
};

///========================================================================================= WmiBase
class WmiBase {
public:
	virtual ~WmiBase();

	WmiBase(const WmiConnection &conn, const BStr &path);

	WmiBase(const WmiConnection &conn, const ComObject<IWbemClassObject> &obj);

	Variant 	get_param(PCWSTR param) const;

	Variant 	get_param(PCWSTR method, PCWSTR param) const;

//	template<typename Functor>
//	bool	exec(Functor Func, PVOID data = nullptr) const {
//		return	Func(m_conn, m_obj, data);
//	}

protected:
	void 		exec_method(PCWSTR method) const;

	Variant	exec_method_get_param(PCWSTR method, PCWSTR param, ComObject<IWbemClassObject> in = ComObject<IWbemClassObject>()) const;

	Variant	exec_method_in(PCWSTR method, PCWSTR param = nullptr, DWORD value = 0) const;


	void refresh();

	const WmiConnection &m_conn;
	BStr				m_path;
	ComObject<IWbemClassObject> m_obj;
private:
};

///====================================================================================== WMIProcess
class WmiProcess: public WmiBase {
public:
	WmiProcess(const WmiConnection &conn, DWORD id):
			WmiBase(conn, Path(id)) {
	}

	WmiProcess(const WmiConnection &conn, const ComObject<IWbemClassObject> &obj):
			WmiBase(conn, obj) {
	}

	template<typename Functor>
	bool  exec(Functor Func, PCVOID data = nullptr) const {
		return	Func(*this, data);
	}

	int	attach_debugger() const;

	int	terminate() const;

	int	set_priority(DWORD pri);

	AutoUTF	get_owner() const;

	AutoUTF	get_owner_dom() const;

	AutoUTF	get_owner_sid() const;

private:
	BStr Path(DWORD id) const;
};

///==================================================================================== WmiProcessor
class WmiProcessor: public WmiBase {
public:
	WmiProcessor(const WmiConnection &conn, DWORD id):
			WmiBase(conn, Path(id)) {
	}

	WmiProcessor(const WmiConnection &conn, const ComObject<IWbemClassObject> &obj):
			WmiBase(conn, obj) {
	}

	template<typename Functor>
	bool  exec(Functor Func, PCVOID data = nullptr) const {
		return	Func(*this, data);
	}

private:
	BStr Path(DWORD id) const;
};

///======================================================================================= WmiSystem
class WmiSystem: public WmiBase {
public:
	WmiSystem(const WmiConnection &conn, PCWSTR name):
			WmiBase(conn, Path(name)) {
	}

	WmiSystem(const WmiConnection &conn, const ComObject<IWbemClassObject> &obj):
			WmiBase(conn, obj) {
	}

	template<typename Functor>
	bool  exec(Functor Func, PCVOID data = nullptr) const {
		return	Func(*this, data);
	}

private:
	BStr Path(PCWSTR name) const;
};

///=============================================================================== WmiNetworkAdapter
class WmiNetworkAdapter: public WmiBase {
public:
	WmiNetworkAdapter(const WmiConnection &conn, DWORD id):
			WmiBase(conn, Path(id)) {
	}

	WmiNetworkAdapter(const WmiConnection &conn, const ComObject<IWbemClassObject> &obj):
			WmiBase(conn, obj) {
	}

	template<typename Functor>
	bool exec(Functor Func, PCVOID data = nullptr) const {
		return	Func(*this, data);
	}

	void Disable() const;

	void Enable() const;

private:
	BStr Path(DWORD id) const;
};

///=========================================================================== WmiNetworkAdapterConf
class WmiNetworkAdapterConf: public WmiBase {
public:
	WmiNetworkAdapterConf(const WmiConnection &conn, DWORD index):
			WmiBase(conn, Path(index)) {
	}

	WmiNetworkAdapterConf(const WmiConnection &conn, const ComObject<IWbemClassObject> &obj):
			WmiBase(conn, obj) {
	}

	template<typename Functor>
	bool exec(Functor Func, PCVOID data = nullptr) const {
		return	Func(*this, data);
	}

	size_t EnableDHCP() const;

	size_t EnableStatic(const Variant &ip, const Variant &mask) const;

	size_t SetGateways(const Variant &ip) const;

	size_t SetDNSServerSearchOrder(const Variant &ip) const;

	size_t ReleaseDHCPLease() const;

	size_t RenewDHCPLease() const;

private:
	BStr Path(DWORD index) const;
};














///==================================================================================== WMIRefresher
class		WMIRefresher: public WinErrorCheck {
	const WmiConnection		&m_conn;
	IWbemRefresher			*m_refr;
	IWbemConfigureRefresher *m_confrefr;
	IWbemHiPerfEnum			*m_enum;

	bool		CloseConfig() {
		if (m_confrefr) {
			m_confrefr->Release();
			m_confrefr = nullptr;
			return	true;
		}
		return	false;
	}
	bool		CloseRefresher() {
		if (m_refr) {
			m_refr->Release();
			m_refr = nullptr;
			return	true;
		}
		return	false;
	}
public:
	~WMIRefresher() {
		CloseConfig();
		CloseRefresher();
	}
	WMIRefresher(const WmiConnection &conn, PCWSTR in);
	bool		IsOK() const {
		return SUCCEEDED(err());
	}
	void		Refresh() const {
		m_refr->Refresh(0L);
	}
	IWbemHiPerfEnum*	Enum() const {
		return	m_enum;
	}

};

///========================================================================================= WMIPerf
class		WMIPerf: public WinErrorCheck {
	WMIRefresher*	m_refr[2];
public:
	~WMIPerf() {
		delete m_refr[0];
		delete m_refr[1];
	}
	WMIPerf(const WmiConnection &conn, PCWSTR in) {
		m_refr[0] = new WMIRefresher(conn, in);
		m_refr[1] = new WMIRefresher(conn, in);
	}
	void		Refresh() {
		std::swap(m_refr[0], m_refr[1]);
		m_refr[1]->Refresh();
	}
};

///================================================================================== WMIPerfObjects
class		WMIPerfObjects: public WinErrorCheck {
	const WMIRefresher &m_refr;
	IWbemObjectAccess	**apEnumAccess;
	DWORD				m_cnt;

	bool		Read();
	bool		Close() {
		delete[] apEnumAccess;
		apEnumAccess = nullptr;
		m_cnt	= 0;
		return	true;
	}
public:
	~WMIPerfObjects() {
		Close();
	}
	WMIPerfObjects(const WMIRefresher &refr): m_refr(refr), apEnumAccess(nullptr), m_cnt(0) {
		Refresh();
	}

	void		Refresh() {
		m_refr.Refresh();
		Read();
	}

	size_t		size() const {
		return	m_cnt;
	}
	bool		GetDWORD(size_t index, PCWSTR name, DWORD &out) const;
	bool		GetQWORD(size_t index, PCWSTR name, uint64_t &out) const;
	bool		GetStr(size_t index, PCWSTR name, AutoUTF &out) const;
};





#define NCOUNTERS 22
#define MAX_USERNAME_LENGTH 128

extern struct _Counters {
		PCWSTR	Name;
//	DWORD	idName;
//	DWORD	idCol;
	} Counters[NCOUNTERS];


// A wrapper class to provide auto-closing of registry key
class		RegKey {
	HKEY	hKey;
public:
	~RegKey() {
		if (hKey)
			::RegCloseKey(hKey);
	}
	RegKey(HKEY hParent, PCWSTR pKey, DWORD flags = KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS) {
		DWORD	rc;
		if ((rc = ::RegOpenKeyExW(hParent, pKey, 0, flags, &hKey)) != ERROR_SUCCESS) {
			::SetLastError(rc);
			hKey = nullptr;
		}
	}
	operator			HKEY() const {
		return	hKey;
	}
};

struct		PerfLib {
	WCHAR	szSubKey[1024];
	DWORD	dwProcessIdTitle;
	DWORD	dwPriorityTitle;
	DWORD	dwThreadTitle;
	DWORD	dwCreatingPIDTitle;
	DWORD	dwElapsedTitle;
	DWORD	dwCounterTitles[sizeofa(Counters)];
	DWORD	CounterTypes[sizeofa(Counters)];
};

struct		ProcessPerfData {
	DWORD       dwProcessId;
	DWORD       dwProcessPriority;
	DWORD       dwThreads;
	DWORD       dwCreatingPID;
	DWORD       dwElapsedTime;
	LONGLONG    qwCounters[NCOUNTERS];
	LONGLONG    qwResults[NCOUNTERS];

	TCHAR       ProcessName[MAX_PATH];
	TCHAR       FullPath[MAX_PATH];
	TCHAR       Owner[MAX_USERNAME_LENGTH];
	TCHAR       CommandLine[MAX_USERNAME_LENGTH];
	FILETIME    ftCreation;
	DWORD       dwGDIObjects, dwUSERObjects;
	BOOL        bProcessIsWow64;
};

class DebugToken {
	// Debug thread token
	static volatile HANDLE hDebugToken;

	// Saved impersonation token
	HANDLE hSavedToken;

	bool saved;
	bool enabled;

public:
	DebugToken(): hSavedToken(nullptr), saved(false), enabled(false) {}
	~DebugToken() {
		Revert();
	}

	bool Enable();
	bool Revert();

	static bool CreateToken();
	static void CloseToken();
};

class PerfThread {
	HANDLE	hThread;
	HANDLE	hEvtBreak, hEvtRefresh, hEvtRefreshDone;
	DWORD	dwThreadId;
//	Array<ProcessPerfData> *pData;

	DWORD	dwLastTickCount;
	bool	bOK;
	HKEY	hHKLM, hPerf;
	DWORD	dwRefreshMsec, dwLastRefreshTicks;
	TCHAR	HostName[64];
	HANDLE	hMutex;
	WmiConnection	WMI;

	PerfLib	pf;
	bool	bUpdated;
	bool	bConnectAttempted;

	void	Refresh();
	void	RefreshWMIData();

//	Plist&	PlistPlugin;

	static DWORD WINAPI ThreadProc(LPVOID lpParm);
	DWORD WINAPI ThreadProc();

public:
	~PerfThread();
	PerfThread(PCWSTR hostname = nullptr, PCWSTR pUser = nullptr, PCWSTR pPasw = nullptr);

//	void				GetProcessData(ProcessPerfData* &pd, DWORD &nProc) const {
//		if (!pData) {
//			nProc = 0;
//			pd = 0;
//			return;
//		}
//		pd = *pData;
//		nProc = pData->length();
//	}
	ProcessPerfData* GetProcessData(DWORD dwPid, DWORD dwThreads) const;
	const PerfLib* GetPerfLib() const {
		return &pf;
	}
	void AsyncReread() {
		SetEvent(hEvtRefresh);
	}
	void SyncReread();
	void SmartReread() {
		if (dwLastRefreshTicks > 1000) AsyncReread();
		else SyncReread();
	}
	bool IsOK() const {
		return bOK;
	}
	LPCTSTR GetHostName() const {
		return HostName;
	}
	bool Updated() {
		bool bRet = bUpdated;
		bUpdated = false;
		return bRet;
	}
	bool IsWMIConnected() {
		return WMI;
	}
	static void GetProcessOwnerInfo(DWORD dwPid, TCHAR* pUser, TCHAR* UserSid, TCHAR* pDomain, int& nSession);
#ifndef UNICODE
	bool IsLocal() {
		return HostName[0] == '\0';
	}
#endif
	TCHAR UserName[64];
	TCHAR Password[64];

	friend class MutexLock;
};







class		MutexLock {
	HANDLE	m_handle;
public:
	MutexLock(PerfThread* pth): m_handle((pth) ? pth->hMutex : 0) {
		if (m_handle)
			::WaitForSingleObject(m_handle, INFINITE);
	}
	~MutexLock() {
		if (m_handle)
			::ReleaseMutex(m_handle);
	}
};


#endif
