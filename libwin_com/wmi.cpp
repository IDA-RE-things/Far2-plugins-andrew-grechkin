/**
 * wmi
 * @classes		()
 * @author		Copyright © 2011 Andrew Grechkin
 * @link		(ole32, oleaut32, uuid)
**/

#include "wmi.h"

EXTERN_C const	CLSID CLSID_WbemLocator;
EXTERN_C const	IID IID_IWbemLocator;
//EXTERN_C const CLSID CLSID_WbemRefresher;
//EXTERN_C const	CLSID CLSID_WbemLocator = {0x4590f811, 0x1d3a, 0x11d0, {0x89, 0x1f, 0x00, 0xaa, 0x00, 0x4b, 0x2e, 0x24}};
//EXTERN_C const	IID IID_IWbemLocator = {0xdc12a687, 0x737f, 0x11cf, {0x88, 0x4d, 0x00, 0xaa, 0x00, 0x4b, 0x2e, 0x24}};

void get_param(Variant &out, const ComObject<IWbemClassObject> &obj, PCWSTR param) {
	CheckWmi(obj->Get(param, 0, &out, 0, 0));
}

BStr get_class(const ComObject<IWbemClassObject> &obj) {
	Variant ret;
	::get_param(ret, obj, L"__CLASS");
	return ret.bstrVal;
}

BStr get_path(const ComObject<IWbemClassObject> &obj) {
	Variant ret;
	::get_param(ret, obj, L"__RELPATH");
	return ret.bstrVal;
}

///=================================================================================== WmiConnection
WmiConnection::WmiConnection(PCWSTR srv, PCWSTR user, PCWSTR pass) {
	//CheckCom(::CoInitializeSecurity(0, -1, 0, 0, RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE, 0, EOAC_NONE, 0));

	ComObject<IWbemLocator>	pIWbemLocator;
	CheckCom(::CoCreateInstance(CLSID_WbemLocator, nullptr, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (PVOID*) &pIWbemLocator));

	if (!srv || !srv[0])
		srv = L".";				// Empty srv means local computer

	if (user && !user[0])
		user = pass = nullptr;	// Empty username means default security

	if (NORM_M_PREFIX(srv) || REV_M_PREFIX(srv))
		srv += 2 * sizeofe(srv);

	WCHAR	Namespace[MAX_PATH];
	::_snwprintf(Namespace, sizeofa(Namespace), L"\\\\%s\\root\\cimv2", srv);

	CheckWmi(pIWbemLocator->ConnectServer(Namespace, BStr(user), BStr(pass), nullptr, 0, nullptr, nullptr, &m_svc));
	CheckCom(::CoSetProxyBlanket(m_svc, RPC_C_AUTHN_DEFAULT, RPC_C_AUTHN_DEFAULT,
								 NULL, RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_STATIC_CLOAKING));
}

ComObject<IEnumWbemClassObject>	WmiConnection::Query(PCWSTR query, ssize_t flags) const {
	ComObject<IEnumWbemClassObject> ewco;
	CheckWmi(m_svc->ExecQuery((OLECHAR*)L"WQL", (OLECHAR*)query, flags, nullptr, &ewco));
	return ewco;
}

ComObject<IEnumWbemClassObject>	WmiConnection::Enum(PCWSTR path, ssize_t flags) const {
	ComObject<IEnumWbemClassObject>	ewco;
	CheckWmi(m_svc->CreateInstanceEnum((BSTR)path, flags, nullptr, &ewco));
	return ewco;
}

ComObject<IWbemClassObject>	WmiConnection::get_object(PCWSTR path) const {
	ComObject<IWbemClassObject>	obj;
	CheckWmi(m_svc->GetObject((BSTR)path, WBEM_FLAG_DIRECT_READ, nullptr, &obj, nullptr));
	return	obj;
}

ComObject<IWbemClassObject>	WmiConnection::get_object(PCWSTR path, PCWSTR method) const {
	ComObject<IWbemClassObject>	obj;
	CheckWmi(m_svc->ExecMethod((BSTR)path, (BSTR)method, 0, nullptr, nullptr, &obj, nullptr));
	return	obj;
}

ComObject<IWbemClassObject>	WmiConnection::get_method(PCWSTR path, PCWSTR method) const {
	ComObject<IWbemClassObject> obj = get_object(path);
	ComObject<IWbemClassObject> pInSignature;
	CheckWmi(obj->GetMethod(method, 0, &pInSignature, nullptr));
	return	pInSignature;
}

///=========================================================================================s WmiBase
WmiBase::~WmiBase() {
}

WmiBase::WmiBase(const WmiConnection &conn, const BStr &path):
		m_conn(conn),
		m_path(path),
		m_obj(m_conn.get_object(m_path)) {
}

WmiBase::WmiBase(const WmiConnection &conn, const ComObject<IWbemClassObject> &obj):
		m_conn(conn),
		m_path(::get_path(obj)),
		m_obj(obj) {
}

Variant WmiBase::get_param(PCWSTR param) const {
	Variant	ret;
	CheckWmi(m_obj->Get(param, 0, &ret, nullptr, nullptr));
	return ret;
}

Variant WmiBase::get_param(PCWSTR method, PCWSTR param) const {
	return	exec_method_get_param(method, param);
}

void 	WmiBase::exec_method(PCWSTR method) const {
	CheckWmi(m_conn->ExecMethod(m_path, (BSTR)method, 0, nullptr, nullptr, nullptr, nullptr));
}

Variant	WmiBase::exec_method_get_param(PCWSTR method, PCWSTR param, ComObject<IWbemClassObject> in) const {
	ComObject<IWbemClassObject> out;
	CheckWmi(m_conn->ExecMethod(m_path, (BSTR)method, 0, nullptr, in, &out, nullptr));

	Variant var;
	CheckWmi(out->Get(param, 0, &var, nullptr, nullptr));
	return	var;
}

Variant	WmiBase::exec_method_in(PCWSTR method, PCWSTR param, DWORD value) const {
	ComObject<IWbemClassObject>	pInSignature;
	ComObject<IWbemClassObject> pInParams;
	if (param) {
		ComObject<IWbemClassObject>	obj(m_conn.get_object(get_class(m_obj)));
		CheckWmi(obj->GetMethod(method, 0, &pInSignature, nullptr));
		CheckWmi(pInSignature->SpawnInstance(0, &pInParams));
		Variant var(value);
		CheckWmi(pInParams->Put(param, 0, &var, 0));
	}

	return exec_method_get_param(method, L"ReturnValue", pInParams);
}

void WmiBase::refresh() {
	m_obj = m_conn.get_object(m_path);
}

///====================================================================================== WMIProcess
int	WmiProcess::attach_debugger() const {
	return exec_method_in(L"AttachDebugger").as_int();
}

int	WmiProcess::terminate() const {
	return	exec_method_in(L"Terminate", L"Reason", 0xffffffff).as_int();
}

int	WmiProcess::set_priority(DWORD pri) {
	int ret = exec_method_in(L"SetPriority", L"Priority", pri).as_int();
	refresh();
	return ret;
}

AutoUTF	WmiProcess::get_owner() const {
	return	exec_method_get_param(L"GetOwner", L"User").as_str();
}

AutoUTF	WmiProcess::get_owner_dom() const {
	return	exec_method_get_param(L"GetOwner", L"Domain").as_str();
}

AutoUTF	WmiProcess::get_owner_sid() const {
	return	exec_method_get_param(L"GetOwnerSid", L"Sid").as_str();
}

BStr WmiProcess::Path(DWORD id) const {
	WCHAR	path[MAX_PATH];
	::_snwprintf(path, sizeofa(path), L"Win32_Process.Handle=%d", id);

	return BStr(path);
}

///==================================================================================== WmiProcessor
BStr WmiProcessor::Path(DWORD id) const {
	WCHAR	path[MAX_PATH];
	::_snwprintf(path, sizeofa(path), L"Win32_Processor.DeviceID='CPU%d'", id);

	return BStr(path);
}

///======================================================================================= WmiSystem
BStr WmiSystem::Path(PCWSTR name) const {
	WCHAR	path[MAX_PATH];
	::_snwprintf(path, sizeofa(path), L"Win32_ComputerSystem", name);

	return BStr(path);
}

///=============================================================================== WmiNetworkAdapter
void WmiNetworkAdapter::Disable() const {
	WmiBase::exec_method(L"Disable");
}

void WmiNetworkAdapter::Enable() const {
	WmiBase::exec_method(L"Enable");
}

BStr WmiNetworkAdapter::Path(DWORD id) const {
	WCHAR	path[MAX_PATH];
	::_snwprintf(path, sizeofa(path), L"Win32_NetworkAdapter.DeviceID=\"%d\"", id);
	return BStr(path);
}

///=========================================================================== WmiNetworkAdapterConf
size_t WmiNetworkAdapterConf::EnableDHCP() const {
	return exec_method_get_param(L"EnableDHCP", L"ReturnValue").as_uint();
}

size_t WmiNetworkAdapterConf::EnableStatic(const Variant &ip, const Variant &mask) const {
	ComObject<IWbemClassObject>	obj(m_conn.get_object(get_class(m_obj)));

	ComObject<IWbemClassObject>	pInSignature;
	CheckWmi(obj->GetMethod(L"EnableStatic", 0, &pInSignature, nullptr));

	ComObject<IWbemClassObject> pInParams;
	CheckWmi(pInSignature->SpawnInstance(0, &pInParams));

	CheckWmi(pInParams->Put(L"IPAddress", 0, (VARIANT*)&ip, 0));
	CheckWmi(pInParams->Put(L"SubnetMask", 0, (VARIANT*)&mask, 0));

	return exec_method_get_param(L"EnableStatic", L"ReturnValue", pInParams).as_uint();
}

size_t WmiNetworkAdapterConf::SetGateways(const Variant &ip) const {
	ComObject<IWbemClassObject>	obj(m_conn.get_object(get_class(m_obj)));

	ComObject<IWbemClassObject>	pInSignature;
	CheckWmi(obj->GetMethod(L"SetGateways", 0, &pInSignature, nullptr));

	ComObject<IWbemClassObject> pInParams;
	CheckWmi(pInSignature->SpawnInstance(0, &pInParams));

	CheckWmi(pInParams->Put(L"DefaultIPGateway", 0, (VARIANT*)&ip, 0));
	//		CheckWmi(pInParams->Put(L"GatewayCostMetric", 0, (VARIANT*)&metric, 0));

	return exec_method_get_param(L"SetGateways", L"ReturnValue", pInParams).as_uint();
}

size_t WmiNetworkAdapterConf::SetDNSServerSearchOrder(const Variant &ip) const {
	ComObject<IWbemClassObject>	obj(m_conn.get_object(get_class(m_obj)));

	ComObject<IWbemClassObject>	pInSignature;
	CheckWmi(obj->GetMethod(L"SetDNSServerSearchOrder", 0, &pInSignature, nullptr));

	ComObject<IWbemClassObject> pInParams;
	CheckWmi(pInSignature->SpawnInstance(0, &pInParams));

	CheckWmi(pInParams->Put(L"DNSServerSearchOrder", 0, (VARIANT*)&ip, 0));

	return exec_method_get_param(L"SetDNSServerSearchOrder", L"ReturnValue", pInParams).as_uint();
}

size_t WmiNetworkAdapterConf::ReleaseDHCPLease() const {
	return exec_method_get_param(L"ReleaseDHCPLease", L"ReturnValue").as_uint();
}

size_t WmiNetworkAdapterConf::RenewDHCPLease() const {
	return exec_method_get_param(L"RenewDHCPLease", L"ReturnValue").as_uint();
}

BStr WmiNetworkAdapterConf::Path(DWORD index) const {
	WCHAR	path[MAX_PATH];
	::_snwprintf(path, sizeofa(path), L"Win32_NetworkAdapterConfiguration.index=%d", index);
	return BStr(path);
}















///==================================================================================== WMIRefresher
WMIRefresher::WMIRefresher(const WmiConnection &conn, PCWSTR in): m_conn(conn), m_refr(nullptr), m_confrefr(nullptr), m_enum(nullptr) {
	err(::CoCreateInstance(CLSID_WbemRefresher, nullptr, CLSCTX_INPROC_SERVER, IID_IWbemRefresher, (PVOID*) & m_refr));
	if (SUCCEEDED(err())) {
		err(m_refr->QueryInterface(IID_IWbemConfigureRefresher, (PVOID*)&m_confrefr));
		if (SUCCEEDED(err())) {
			long                    lID = 0;
			err(m_confrefr->AddEnum(m_conn.GetIWbemServices(), in, 0, nullptr, &m_enum, &lID));
			CloseConfig();
		}
	}
}

///================================================================================== WMIPerfObjects
bool		WMIPerfObjects::Read() {
	Close();
	DWORD	size = 0;
	err(m_refr.Enum()->GetObjects(0L, m_cnt, apEnumAccess, &size));
	if (size && err() == (DWORD)WBEM_E_BUFFER_TOO_SMALL) {
		if ((apEnumAccess = new IWbemObjectAccess*[size])) {
			m_cnt = size;
			WinMem::Zero(apEnumAccess, m_cnt * sizeof(IWbemObjectAccess*));
			if (SUCCEEDED(err(m_refr.Enum()->GetObjects(0L,	m_cnt, apEnumAccess, &size)))) {
				return	true;
			}
			Close();
		}
	}
	return	false;
}

bool		WMIPerfObjects::GetDWORD(size_t index, PCWSTR name, DWORD &out) const {
	long	hnd = 0;
	apEnumAccess[index]->Lock(0);
	if (SUCCEEDED(apEnumAccess[index]->GetPropertyHandle(name, nullptr, &hnd))) {
		if (SUCCEEDED(apEnumAccess[index]->ReadDWORD(hnd, &out))) {
			apEnumAccess[index]->Unlock(0);
			return	true;
		}
	}
	apEnumAccess[index]->Unlock(0);
	return	false;
}

bool		WMIPerfObjects::GetQWORD(size_t index, PCWSTR name, uint64_t &out) const {
	long	hnd = 0;
	apEnumAccess[index]->Lock(0);
	if (SUCCEEDED(apEnumAccess[index]->GetPropertyHandle(name, nullptr, &hnd))) {
		if (SUCCEEDED(apEnumAccess[index]->ReadQWORD(hnd, &out))) {
			apEnumAccess[index]->Unlock(0);
			return	true;
		}
	}
	apEnumAccess[index]->Unlock(0);
	return	false;
}

bool		WMIPerfObjects::GetStr(size_t index, PCWSTR name, AutoUTF &out) const {
	long	hnd = 0;
	long	size = 0;
	DWORD	hr = 0;
	apEnumAccess[index]->Lock(0);
	if (SUCCEEDED(apEnumAccess[index]->GetPropertyHandle(name, nullptr, &hnd))) {
		if (SUCCEEDED(hr = apEnumAccess[index]->ReadPropertyValue(hnd, out.capacity() * sizeof(WCHAR), &size, (PBYTE)out.c_str()))) {
			apEnumAccess[index]->Unlock(0);
			return	true;
		} else {
			if (hr == WBEM_E_BUFFER_TOO_SMALL) {
				out.reserve(size / sizeof(WCHAR));
				if (SUCCEEDED(apEnumAccess[index]->ReadPropertyValue(hnd, out.capacity()*sizeof(WCHAR), &size, (PBYTE)out.c_str()))) {
					apEnumAccess[index]->Unlock(0);
					return	true;
				}
			}
		}
	}
	apEnumAccess[index]->Unlock(0);
	return	false;
}



///========================================================================================= WMIPerf





_Counters Counters[] = {
	{L"% Processor Time"},//        MProcessorTime     , MColProcessorTime     },
	{L"% Privileged Time"},//       MPrivilegedTime    , MColPrivilegedTime    },
	{L"% User Time"},//             MUserTime          , MColUserTime          },
	{L"Handle Count"},//            MHandleCount       , MColHandleCount       },
	{L"Page File Bytes"},//         MPageFileBytes     , MColPageFileBytes     },
	{L"Page File Bytes Peak"},//    MPageFileBytesPeak , MColPageFileBytesPeak },
	{L"Working Set"},//             MWorkingSet        , MColWorkingSet        },
	{L"Working Set Peak"},//        MWorkingSetPeak    , MColWorkingSetPeak    },
	{L"Pool Nonpaged Bytes"},//     MPoolNonpagedBytes , MColPoolNonpagedBytes },
	{L"Pool Paged Bytes"},//        MPoolPagedBytes    , MColPoolPagedBytes    },
	{L"Private Bytes"},//           MPrivateBytes      , MColPrivateBytes      },
	{L"Page Faults/sec"},//         MPageFaults        , MColPageFaults        },
	{L"Virtual Bytes"},//           MVirtualBytes      , MColVirtualBytes      },
	{L"Virtual Bytes Peak"},//      MVirtualBytesPeak  , MColVirtualBytesPeak  },
	{L"IO Data Bytes/sec"},//       MIODataBytes       , MColIODataBytes       },
	{L"IO Read Bytes/sec"},//       MIOReadBytes       , MColIOReadBytes       },
	{L"IO Write Bytes/sec"},//      MIOWriteBytes      , MColIOWriteBytes      },
	{L"IO Other Bytes/sec"},//      MIOOtherBytes      , MColIOOtherBytes      },
	{L"IO Data Operations/sec"},//  MIODataOperations  , MColIODataOperations  },
	{L"IO Read Operations/sec"},//  MIOReadOperations  , MColIOReadOperations  },
	{L"IO Write Operations/sec"},// MIOWriteOperations , MColIOWriteOperations },
	{L"IO Other Operations/sec"},// MIOOtherOperations , MColIOOtherOperations },
};

static int getcounter(PWSTR p) {
	PWSTR	p2;
	for (p2 = p - 2; iswdigit(*p2); p2--)
		;
	return _wtoi(p2 + 1);
}

PerfThread::PerfThread(PCWSTR hostname, PCWSTR pUser, PCWSTR pPasw) {
	memset(this, 0, sizeof(*this));
	dwRefreshMsec = 500;
	if (pUser && *pUser) {
		lstrcpyn(UserName, pUser, sizeofa(UserName));
		if (pPasw)
			lstrcpyn(Password, pPasw, sizeofa(Password));
	}

	hMutex = CreateMutex(0, FALSE, 0);
	MutexLock	l(this);

	DWORD rc;
	if (hostname) {
		lstrcpyn(HostName, hostname, sizeofa(HostName));
		if ((rc = RegConnectRegistry(hostname, HKEY_LOCAL_MACHINE, &hHKLM)) != ERROR_SUCCESS ||
				(rc = RegConnectRegistry(hostname, HKEY_PERFORMANCE_DATA, &hPerf)) != ERROR_SUCCESS) {
			::SetLastError(rc);
			return;
		}
	} else {
		hHKLM = HKEY_LOCAL_MACHINE;
		hPerf = HKEY_PERFORMANCE_DATA;
	}

	LANGID lid = MAKELANGID(LANG_ENGLISH, SUBLANG_NEUTRAL);

	_snwprintf(pf.szSubKey, sizeofa(pf.szSubKey), L"Software\\Microsoft\\Windows NT\\CurrentVersion\\Perflib\\%03X", lid);
//	FSF.sprintf(pf.szSubKey, _T("Software\\Microsoft\\Windows NT\\CurrentVersion\\Perflib\\%03X"), lid);

	RegKey hKeyNames(hHKLM, pf.szSubKey, KEY_READ);
	if (!hKeyNames)
		return;

	// Get the buffer size for the counter names
	DWORD dwType, dwSize;
	if ((rc = ::RegQueryValueEx(hKeyNames, L"Counters", 0, &dwType, nullptr, &dwSize))
			!= ERROR_SUCCESS) {
		SetLastError(rc);
		return;
	}

	// Allocate the counter names buffer
	TCHAR* buf = new TCHAR [dwSize];

	// read the counter names from the registry
	if ((rc = RegQueryValueEx(hKeyNames, L"Counters", 0, &dwType, (BYTE*)buf, &dwSize))) {
		SetLastError(rc);
		return;
	}

	// now loop thru the counter names looking for the following counters:
	//      1.  "Process"           process name
	//      2.  "ID Process"        process id
	// the buffer contains multiple null terminated strings and then
	// finally null terminated at the end.  the strings are in pairs of
	// counter number and counter name.

	for (TCHAR* p = buf; *p; p += lstrlen(p) + 1) {
		if (Cmpi(p, L"Process") == 0)
			_itow(getcounter(p), pf.szSubKey, 10);
		else if (!pf.dwProcessIdTitle && Cmpi(p, L"ID Process") == 0)
			pf.dwProcessIdTitle = getcounter(p);
		else if (!pf.dwPriorityTitle && Cmpi(p, L"Priority Base") == 0)
			pf.dwPriorityTitle = getcounter(p);
		else if (!pf.dwThreadTitle && Cmpi(p, L"Thread Count") == 0)
			pf.dwThreadTitle = getcounter(p);
		else if (!pf.dwCreatingPIDTitle && Cmpi(p, L"Creating Process ID") == 0)
			pf.dwCreatingPIDTitle = getcounter(p);
		else if (!pf.dwElapsedTitle && Cmpi(p, L"Elapsed Time") == 0)
			pf.dwElapsedTitle = getcounter(p);
		else
			for (int i = 0; i < NCOUNTERS; i++)
				if (!pf.dwCounterTitles[i] && Cmpi(p, Counters[i].Name) == 0)
					pf.dwCounterTitles[i] = getcounter(p);
	}
	delete buf;

	hEvtBreak = CreateEvent(0, 0, 0, 0);
	hEvtRefresh = CreateEvent(0, 0, 0, 0);
	hEvtRefreshDone = CreateEvent(0, 0, 0, 0);

	Refresh();

//	hThread = CreateThread(0, 0, ThreadProc, this, 0, &dwThreadId);
	bOK = true;
}
PerfThread::~PerfThread() {
	::SetEvent(hEvtBreak);
	::WaitForSingleObject(hThread, INFINITE);
//	delete pData;
	::CloseHandle(hEvtRefreshDone);
	::CloseHandle(hEvtRefresh);
	::CloseHandle(hEvtBreak);
	::CloseHandle(hMutex);
	::CloseHandle(hThread);
	if (hHKLM)
		::RegCloseKey(hHKLM);
	if (hPerf)
		::RegCloseKey(hPerf);
}

ProcessPerfData* PerfThread::GetProcessData(DWORD /*dwPid*/, DWORD /*dwThreads*/) const {
//	for (DWORD i = 0; i < pData->length(); i++)
//		if ((*pData)[i].dwProcessId == dwPid && (dwPid || ((dwThreads > 5 && (*pData)[i].dwThreads > 5) ||
//						   (dwThreads <= 5 && (*pData)[i].dwThreads <= 5))))
//			return &(*pData)[i];
	return 0;
}
#define INITIAL_SIZE        51200
#define EXTEND_SIZE         25600

void			PerfThread::Refresh() {
	/*
	//	DebugToken	token;
		DWORD dwTicksBeforeRefresh = GetTickCount();

		// allocate the initial buffer for the performance data
		WinBuffer<PBYTE> buf(INITIAL_SIZE);
		PPERF_DATA_BLOCK pPerf;

		DWORD dwDeltaTickCount;
		while (1) {
			DWORD dwSize = buf.capacity(), dwType;
			dwDeltaTickCount = GetTickCount() - dwLastTickCount;
			DWORD rc;
			while ((rc = ::RegQueryValueEx(hPerf, pf.szSubKey, 0, &dwType, buf, &dwSize))
					== ERROR_LOCK_FAILED)  ; //Just retry

			pPerf = (PPERF_DATA_BLOCK)(BYTE*) buf;

			// check for success and valid perf data block signature
			if (rc == ERROR_SUCCESS &&
					!memcmp(pPerf->Signature, L"PERF", 8)) {
				break;
			}
			if (rc == ERROR_MORE_DATA)
				buf.reserve(buf.capacity() + EXTEND_SIZE);
			else if (rc < 0x100000) { // ??? sometimes we receive garbage in rc
				bOK = false;
				return;
			}
		}
		bool bDeltaValid = dwLastTickCount && dwDeltaTickCount;

		// set the perf_object_type pointer
		PPERF_OBJECT_TYPE pObj =
			(PPERF_OBJECT_TYPE)((DWORD_PTR)pPerf + pPerf->HeaderLength);

		// loop thru the performance counter definition records looking
		// for the process id counter and then save its offset
		PPERF_COUNTER_DEFINITION pCounterDef =
			(PPERF_COUNTER_DEFINITION)((DWORD_PTR)pObj + pObj->HeaderLength);

		if (!pf.CounterTypes[0] && !pf.CounterTypes[1]) {
			for (DWORD i = 0; i < (DWORD)pObj->NumCounters; i++)
				for (DWORD ii = 0; ii < NCOUNTERS; ii++)
					if (pf.dwCounterTitles[ii] && pCounterDef[i].CounterNameTitleIndex == pf.dwCounterTitles[ii])
						pf.CounterTypes[ii] = pCounterDef[i].CounterType;
		}

		DWORD dwProcessIdCounter = 0, dwPriorityCounter = 0, dwThreadCounter = 0, dwElapsedCounter = 0,
								   dwCreatingPIDCounter = 0, dwCounterOffsets[NCOUNTERS];

		memset(dwCounterOffsets, 0, sizeof(dwCounterOffsets));
		DWORD i;
		for (i = 0; i < (DWORD)pObj->NumCounters; i++) {
			if (pCounterDef->CounterNameTitleIndex == pf.dwProcessIdTitle)
				dwProcessIdCounter = pCounterDef->CounterOffset;
			else if (pCounterDef->CounterNameTitleIndex == pf.dwPriorityTitle)
				dwPriorityCounter = pCounterDef->CounterOffset;
			else if (pCounterDef->CounterNameTitleIndex == pf.dwThreadTitle)
				dwThreadCounter = pCounterDef->CounterOffset;
			else if (pCounterDef->CounterNameTitleIndex == pf.dwCreatingPIDTitle)
				dwCreatingPIDCounter = pCounterDef->CounterOffset;
			else if (pCounterDef->CounterNameTitleIndex == pf.dwElapsedTitle)
				dwElapsedCounter = pCounterDef->CounterOffset;
			else
				for (int ii = 0; ii < NCOUNTERS; ii++)
					if (pf.dwCounterTitles[ii] && pCounterDef->CounterNameTitleIndex == pf.dwCounterTitles[ii])
						dwCounterOffsets[ii] = pCounterDef->CounterOffset;
			pCounterDef++;
		}

		Array<ProcessPerfData> *pNewPData = new Array<ProcessPerfData>((DWORD)pObj->NumInstances);

		PPERF_INSTANCE_DEFINITION pInst =
			(PPERF_INSTANCE_DEFINITION)((DWORD_PTR)pObj + pObj->DefinitionLength);

		// loop thru the performance instance data extracting each process name
		// and process id
		//
		for (i = 0; i < (DWORD)pObj->NumInstances; i++) {

			ProcessPerfData& Task = (*pNewPData)[i];
			// get the process id
			PPERF_COUNTER_BLOCK pCounter = (PPERF_COUNTER_BLOCK)((DWORD_PTR)pInst + pInst->ByteLength);
			Task.bProcessIsWow64 = FALSE;
			Task.dwProcessId = *((LPDWORD)((DWORD_PTR)pCounter + dwProcessIdCounter));
			Task.dwProcessPriority = *((LPDWORD)((DWORD_PTR)pCounter + dwPriorityCounter));
			Task.dwThreads = dwThreadCounter ? *((LPDWORD)((DWORD_PTR)pCounter + dwThreadCounter)) : 0;
			Task.dwCreatingPID = dwCreatingPIDCounter ? *((LPDWORD)((DWORD_PTR)pCounter + dwCreatingPIDCounter)) : 0;
			if (pObj->PerfFreq.QuadPart && *((LONGLONG*)((DWORD_PTR)pCounter + dwElapsedCounter)))
				Task.dwElapsedTime = (DWORD)((pObj->PerfTime.QuadPart - *((LONGLONG*)((DWORD_PTR)pCounter + dwElapsedCounter))
											 ) / pObj->PerfFreq.QuadPart);
			else
				Task.dwElapsedTime = 0;

	// Store new qwCounters
			for (int ii = 0; ii < NCOUNTERS; ii++)
				if (dwCounterOffsets[ii]) {
					if ((pf.CounterTypes[ii]&0x300) == PERF_SIZE_LARGE)
						Task.qwCounters[ii] = *((LONGLONG*)((DWORD_PTR)pCounter + dwCounterOffsets[ii]));
					else // PERF_SIZE_DWORD
						Task.qwCounters[ii] = *((DWORD*)((DWORD_PTR)pCounter + dwCounterOffsets[ii]));
				}
			//memcpy(Task.qwResults, Task.qwCounters, sizeof(Task.qwResults));
			memset(Task.qwResults, 0, sizeof(Task.qwResults));

			ProcessPerfData* pOldTask = 0;

			if (pData) { // Use prev data if any

				//Get the pointer to the previous instance of this process
				pOldTask = GetProcessData(Task.dwProcessId, Task.dwThreads);

				//get the rest of the counters

				for (int ii = 0; ii < NCOUNTERS; ii++) {
					if (!pf.dwCounterTitles[ii])
						continue;

	// Fill qwResults
					if (bDeltaValid)
						switch (pf.CounterTypes[ii]) {
							case PERF_COUNTER_RAWCOUNT:
							case PERF_COUNTER_LARGE_RAWCOUNT:
								Task.qwResults[ii] = Task.qwCounters[ii];
								break;
							case PERF_100NSEC_TIMER:
								// 64-bit Timer in 100 nsec units. Display suffix: "%"
								Task.qwResults[ii] = !pOldTask ? 0 :
													 (*(LONGLONG*)((DWORD_PTR)pCounter + dwCounterOffsets[ii]) - pOldTask->qwCounters[ii])
													 / (dwDeltaTickCount * 100);
								break;
							case PERF_COUNTER_COUNTER:
							case PERF_COUNTER_BULK_COUNT:
								Task.qwResults[ii] = !pOldTask ? 0 :
													 (Task.qwCounters[ii] - pOldTask->qwCounters[ii])
													 * 1000 / dwDeltaTickCount;
								break;
						}
				}
			}//if pData

			if (pOldTask) { // copy process' data from pOldTask to Task
				//    lstrcpy(Task.ProcessName, pOldTask->ProcessName);
				//    lstrcpy(Task.FullPath, pOldTask->FullPath);
				//    lstrcpy(Task.CommandLine, pOldTask->CommandLine);
				//    Task.ftCreation = pOldTask->ftCreation;

				memcpy(Task.ProcessName, pOldTask->ProcessName, sizeof(Task) - offsetof(ProcessPerfData, ProcessName));
			} else {
				HANDLE hProcess = *HostName || Task.dwProcessId <= 8 ? 0 :
								  OpenProcessForced(&token, PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | READ_CONTROL, Task.dwProcessId);
				if (hProcess) {
					GetOpenProcessDataNT(hProcess, Task.ProcessName, ArraySize(Task.ProcessName),
										 Task.FullPath, ArraySize(Task.FullPath), Task.CommandLine, ArraySize(Task.CommandLine));
					FILETIME ftExit, ftKernel, ftUser;
					GetProcessTimes(hProcess, &Task.ftCreation, &ftExit, &ftKernel, &ftUser);

					SetLastError(0);
					Task.dwGDIObjects = pGetGuiResources(hProcess, 0);
					Task.dwUSERObjects = pGetGuiResources(hProcess, 1);

					BOOL wow64;
					if (pIsWow64Process(hProcess, &wow64) && wow64)
						Task.bProcessIsWow64 = TRUE;

					CloseHandle(hProcess);
				}
			}
			if (!*Task.ProcessName) { // if after all this it's still unfilled...
				// pointer to the process name
				// convert it to ascii
	#ifndef UNICODE
				if (WideCharToMultiByte(CP_OEMCP, 0, (LPCWSTR)((DWORD_PTR)pInst + pInst->NameOffset),
										-1, Task.ProcessName, sizeof(Task.ProcessName) - 5, 0, 0) == 0)
					lstrcpy(Task.ProcessName, _T("unknown"));
				else
	#else
				lstrcpyn(Task.ProcessName, (LPCWSTR)((DWORD_PTR)pInst + pInst->NameOffset),
						 ArraySize(Task.ProcessName) - 5);
	#endif
					if (Task.dwProcessId > 8)
						lstrcat(Task.ProcessName, _T(".exe"));
			}
			pInst = (PPERF_INSTANCE_DEFINITION)((DWORD_PTR)pCounter + pCounter->ByteLength);
		}
		dwLastTickCount += dwDeltaTickCount;
		{
			Lock l(this);
			delete pData;
			pData = pNewPData;
		}
	//    if(!PlistPlugin.PostUpdate())
		bUpdated = true;

		dwLastRefreshTicks = GetTickCount() - dwTicksBeforeRefresh;
		SetEvent(hEvtRefreshDone);
	*/
}
