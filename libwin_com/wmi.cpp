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

Variant get_class(const ComObject<IWbemClassObject> &obj) {
	Variant ret;
	::get_param(ret, obj, L"__CLASS");
	return ret;
}

Variant get_path(const ComObject<IWbemClassObject> &obj) {
	Variant ret;
	::get_param(ret, obj, L"__RELPATH");
	return ret;
}

Variant get_server(const ComObject<IWbemClassObject> &obj) {
	Variant ret;
	::get_param(ret, obj, L"__SERVER");
	return ret;
}

///=================================================================================== WmiConnection
void WmiConnection::Init(PCWSTR srv, PCWSTR namesp, PCWSTR user, PCWSTR pass) {
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
	::_snwprintf(Namespace, sizeofa(Namespace), L"\\\\%s\\root\\%s", srv, namesp);

	CheckCom(pIWbemLocator->ConnectServer(Namespace, BStr(user), BStr(pass), nullptr, 0, nullptr, nullptr, &m_svc));
	CheckCom(::CoSetProxyBlanket(m_svc, RPC_C_AUTHN_DEFAULT, RPC_C_AUTHN_DEFAULT,
								 NULL, RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_STATIC_CLOAKING));
//	CoSetProxyBlanket(pWbemServices, RPC_C_AUTHN_WINNT,
//	RPC_C_AUTHZ_NONE, NULL, RPC_C_AUTHN_LEVEL_CALL,
//	RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE))
}

WmiConnection::WmiConnection(PCWSTR srv, PCWSTR namesp) {
	Init(srv, namesp, nullptr, nullptr);
}

WmiConnection::WmiConnection(PCWSTR srv, PCWSTR namesp, PCWSTR user, PCWSTR pass) {
	Init(srv, namesp, user, pass);
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
		m_path(::get_path(obj).bstrVal),
		m_obj(obj) {
}

void WmiBase::Delete() {
	CheckWmi(m_conn->DeleteInstance(get_path(m_obj).bstrVal, 0, nullptr, nullptr));
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
		ComObject<IWbemClassObject>	obj(m_conn.get_object(get_class(m_obj).bstrVal));
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
