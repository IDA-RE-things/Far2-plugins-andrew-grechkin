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

///=================================================================================== WmiConnection
class WmiConnection {
public:
	WmiConnection(PCWSTR srv = nullptr, PCWSTR user = nullptr, PCWSTR pass = nullptr) {
		if (!srv || !srv[0])
			srv = L".";				// Empty srv means local computer

		if (user && !user[0])
			user = pass = nullptr;	// Empty username means default security

		if (NORM_M_PREFIX(srv) || REV_M_PREFIX(srv))
			srv += 2 * sizeofe(srv);

		CheckCom(::CoInitializeSecurity(0, -1, 0, 0, RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE, 0, EOAC_NONE, 0));

		ComObject<IWbemLocator>	pIWbemLocator;
		CheckCom(::CoCreateInstance(CLSID_WbemLocator, nullptr, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (PVOID*) &pIWbemLocator));

		WCHAR	Namespace[MAX_PATH];
		::_snwprintf(Namespace, sizeofa(Namespace), L"\\\\%s\\root\\cimv2", srv);

		CheckWmi(pIWbemLocator->ConnectServer(Namespace, BStr(user), BStr(pass), nullptr, 0, nullptr, nullptr, &m_svc));
	}

	int				ExecMethod(PCWSTR clname, const BStr &object, PCWSTR method, PCWSTR wsParamName = nullptr, DWORD dwParam = nullptr) const {
		ComObject<IWbemClassObject>	obj(get_object(clname));

		ComObject<IWbemClassObject>	pInSignature;
		ComObject<IWbemClassObject> pInParams;
		if (wsParamName) {
			CheckWmi(obj->GetMethod(method, 0, &pInSignature, nullptr));
			CheckWmi(pInSignature->SpawnInstance(0, &pInParams));
			Variant var(dwParam);
			CheckWmi(pInParams->Put(wsParamName, 0, &var, 0));
		}

		ComObject<IWbemClassObject> OutParams;
		CheckWmi(m_svc->ExecMethod(object, BStr(method), 0, nullptr, pInParams, &OutParams, 0));

		Variant var;
		CheckWmi(OutParams->Get(L"ReturnValue", 0, &var, nullptr, nullptr));
		if (var.is_int())
			return var.lVal;
		return	-1;
	}

	operator		bool() const {
		return	m_svc;
	}

	IWbemServices*	GetIWbemServices() const {
		return m_svc;
	}

	IWbemServices*	operator->() const {
		return	m_svc;
	}

	template<typename Functor>
	void			Enum(PCWSTR clname, Functor Func, PVOID data = nullptr) {
		ComObject<IEnumWbemClassObject>	spEnum;

		// создаем перечислитель
		CheckWmi(m_svc->CreateInstanceEnum(BStr(clname), WBEM_FLAG_SHALLOW | WBEM_FLAG_FORWARD_ONLY, nullptr, &spEnum));

		ComObject<IWbemClassObject>	obj;
		HRESULT ret = WBEM_S_NO_ERROR;
		ULONG ulCount = 0;
		while (ret == WBEM_S_NO_ERROR) {
			ret = spEnum->Next(WBEM_INFINITE, 1, &obj, &ulCount);
			if (!ulCount || !SUCCEEDED(ret) || !Func(*this, obj, data))
				break;
		}
	}

	template<typename Functor>
	bool			Exec(PCWSTR clname, Functor &Func, PVOID data = nullptr) {
		ComObject<IWbemClassObject>	obj;
		CheckWmi(m_svc->GetObject(BStr(clname), WBEM_FLAG_DIRECT_READ, nullptr, &obj, nullptr));
		return	Func(obj, data);
	}

	ComObject<IWbemClassObject>	get_object(const BStr &clname) const {
		ComObject<IWbemClassObject>	obj;
		CheckWmi(m_svc->GetObject(clname, WBEM_FLAG_DIRECT_READ, nullptr, &obj, nullptr));
		return	obj;
	}
	ComObject<IWbemClassObject>	get_object(const BStr &clname, PCWSTR method) const {
		ComObject<IWbemClassObject>	obj;
		CheckWmi(m_svc->ExecMethod(clname, BStr(method), 0, nullptr, nullptr, &obj, nullptr));
		return	obj;
	}
	ComObject<IWbemClassObject>	get_method(const BStr &clname, PCWSTR method) const {
		ComObject<IWbemClassObject> obj = get_object(clname);
		ComObject<IWbemClassObject> pInSignature;
		CheckWmi(obj->GetMethod(method, 0, &pInSignature, nullptr));
		return	pInSignature;
	}

	Variant		GetParam(const BStr &path, PCWSTR param) const {
		ComObject<IWbemClassObject> obj = get_object(path);
		Variant	Result;
		CheckWmi(obj->Get(param, 0, &Result, nullptr, nullptr));
		return	Result;
	}
	Variant		GetParam(const BStr &path, PCWSTR method, PCWSTR param) const {
		ComObject<IWbemClassObject> obj = get_object(path, method);
		Variant	Result;
		CheckWmi(obj->Get(param, 0, &Result, nullptr, nullptr));
		return	Result;
	}
private:
	ComObject<IWbemServices> m_svc;
};

///=========================================================================================s WmiBase
class WmiBase {
public:
	virtual ~WmiBase() {
	}

	WmiBase(const WmiConnection &conn, const BStr &path):
		m_conn(conn),
		m_path(path),
		m_obj(m_conn.get_object(m_path)) {
	}

	WmiBase(const WmiConnection &conn, const ComObject<IWbemClassObject> &obj):
		m_conn(conn),
		m_obj(obj) {
	}

	Variant 	get_param(PCWSTR param) const {
		Variant	ret;
		CheckWmi(m_obj->Get(param, 0, &ret, nullptr, nullptr));
		return ret;
	}

	Variant 	get_param(PCWSTR method, PCWSTR param) const {
		return	exec_method_out(method, param);
	}

protected:
	void 		exec_method(PCWSTR method) const {
		CheckWmi(m_conn->ExecMethod(m_path, (BSTR)method, 0, nullptr, nullptr, nullptr, nullptr));
	}

	Variant 	exec_method_out(PCWSTR method, PCWSTR param) const {
		ComObject<IWbemClassObject>	out;
		CheckWmi(m_conn->ExecMethod(m_path, (BSTR)method, 0, nullptr, nullptr, &out, nullptr));

		Variant var;
		CheckWmi(out->Get(param, 0, &var, nullptr, nullptr));
		return	var;
	}

	int 	exec_method_in(PCWSTR method, PCWSTR classname = nullptr, PCWSTR param = nullptr, DWORD value = 0) const {
		ComObject<IWbemClassObject>	pInSignature;
		ComObject<IWbemClassObject> pInParams;
		if (param) {
			ComObject<IWbemClassObject>	obj(m_conn.get_object(classname));
			CheckWmi(obj->GetMethod(method, 0, &pInSignature, nullptr));
			CheckWmi(pInSignature->SpawnInstance(0, &pInParams));
			Variant var(value);
			CheckWmi(pInParams->Put(param, 0, &var, 0));
		}

		ComObject<IWbemClassObject> OutParams;
		CheckWmi(m_conn->ExecMethod(m_path, BStr(method), 0, nullptr, pInParams, &OutParams, 0));

		Variant var;
		CheckWmi(OutParams->Get(L"ReturnValue", 0, &var, nullptr, nullptr));

		if (var.is_int())
			return var.lVal;
		return	-1;
	}

	void refresh() {
		m_obj = m_conn.get_object(m_path);
	}

private:
	const WmiConnection &m_conn;
	BStr				m_path;
	ComObject<IWbemClassObject> m_obj;
};

///====================================================================================== WMIProcess
class WmiProcess: public WmiBase {
public:
	WmiProcess(const WmiConnection &conn, DWORD id):
		WmiBase(conn, Path(id)) {
	}

	int		attach_debugger() const {
		return WmiBase::exec_method_in(L"AttachDebugger");
	}

	int		terminate() const {
		return	exec_method_in(L"Terminate", L"Win32_Process", L"Reason", 0xffffffff);
	}

	int	set_priority(DWORD pri) {
		int ret = exec_method_in(L"SetPriority", L"Win32_Process", L"Priority", pri);
		refresh();
		return ret;
	}

	AutoUTF	get_owner() const {
		return	WmiBase::exec_method_out(L"GetOwner", L"User").as_str();
	}

	AutoUTF	get_owner_dom() const {
		return	WmiBase::exec_method_out(L"GetOwner", L"Domain").as_str();
	}

	AutoUTF	get_owner_sid() const {
		return	WmiBase::exec_method_out(L"GetOwnerSid", L"Sid").as_str();
	}

	template<typename Functor>
	bool	exec(Functor Func, PVOID data = nullptr) const {
		return	Func(*this, data);
	}

private:
	BStr Path(DWORD id) const {
		WCHAR	path[MAX_PATH];
		::_snwprintf(path, sizeofa(path), L"Win32_Process.Handle=%d", id);

		return BStr(path);
	}
};

///==================================================================================== WmiProcessor
class WmiProcessor: public WmiBase {
public:
	WmiProcessor(const WmiConnection &conn, DWORD id):
		WmiBase(conn, Path(id)) {
	}

	template<typename Functor>
	bool			exec(Functor Func, PCVOID data = nullptr) const {
		return	Func(*this, data);
	}

private:
	BStr Path(DWORD id) const {
		WCHAR	path[MAX_PATH];
		::_snwprintf(path, sizeofa(path), L"Win32_Processor.DeviceID='CPU%d'", id);

		return BStr(path);
	}
};

///======================================================================================= WmiSystem
class WmiSystem: public WmiBase {
public:
	WmiSystem(const WmiConnection &conn, PCWSTR name):
		WmiBase(conn, Path(name)) {
	}

	template<typename Functor>
	bool			exec(Functor Func, PCVOID data = nullptr) const {
		return	Func(*this, data);
	}

private:
	BStr Path(PCWSTR name) const {
		WCHAR	path[MAX_PATH];
		::_snwprintf(path, sizeofa(path), L"Win32_ComputerSystem", name);

		return BStr(path);
	}
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
