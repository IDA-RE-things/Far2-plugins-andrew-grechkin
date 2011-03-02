/**
 * wmi
 * @classes		()
 * @author		Copyright © 2011 Andrew Grechkin
 * @link		(ole32, oleaut32, uuid)
**/

#include "wmi.h"

Variant get_param(const ComObject<IWbemClassObject> &obj, PCWSTR param) {
	Variant ret;
	CheckWmi(obj->Get(param, 0, &ret, 0, 0));
	return ret;
}

AutoUTF get_class(const ComObject<IWbemClassObject> &obj) {
	return get_param(obj, L"__CLASS").as_str();
}

AutoUTF get_path(const ComObject<IWbemClassObject> &obj) {
	return get_param(obj, L"__RELPATH").as_str();
}

AutoUTF get_server(const ComObject<IWbemClassObject> &obj) {
	return get_param(obj, L"__SERVER").as_str();
}

ComObject<IWbemClassObject> spawn_instance(const ComObject<IWbemClassObject> &obj) {
	ComObject<IWbemClassObject> ret;
	CheckWmi(obj->SpawnInstance(0, &ret));
	return ret;
}

ComObject<IWbemClassObject> clone(const ComObject<IWbemClassObject> &obj) {
	ComObject<IWbemClassObject> ret;
	CheckWmi(obj->Clone(&ret));
	return ret;
}

ComObject<IWbemClassObject>	get_in_params(const ComObject<IWbemClassObject> &obj, PCWSTR method) {
	ComObject<IWbemClassObject> in_params;
	CheckWmi(obj->GetMethod(method, 0, &in_params, nullptr));
	return	in_params;
}

void put_param(ComObject<IWbemClassObject> &obj, PCWSTR name, const Variant &val) {
	CheckWmi(obj->Put(name, 0, (VARIANT*)&val, 0));
}

///=================================================================================== WmiConnection
void WmiConnection::Init(PCWSTR srv, PCWSTR namesp, PCWSTR user, PCWSTR pass) {
	//CheckCom(::CoInitializeSecurity(0, -1, 0, 0, RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE, 0, EOAC_NONE, 0));

	ComObject<IWbemLocator>	wbemLocator;
	CheckCom(::CoCreateInstance(CLSID_WbemLocator, nullptr, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (PVOID*) &wbemLocator));

	if (!srv || !srv[0])
		srv = L".";				// Empty srv means local computer

	if (NORM_M_PREFIX(srv) || REV_M_PREFIX(srv))
		srv += 2 * sizeofe(srv);

	if (user && !user[0])
		user = pass = nullptr;	// Empty username means default security

	WCHAR	Namespace[MAX_PATH];
	::_snwprintf(Namespace, sizeofa(Namespace), L"\\\\%s\\root\\%s", srv, namesp);

	CheckCom(wbemLocator->ConnectServer(Namespace, BStr(user), BStr(pass), nullptr, 0, nullptr, nullptr, &m_svc));
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

void	WmiConnection::del(PCWSTR path) {
	CheckWmi(m_svc->DeleteInstance((BSTR)path, 0, nullptr, nullptr));
}

ComObject<IWbemClassObject>	WmiConnection::get_object_class(const ComObject<IWbemClassObject> &obj) const {
	ComObject<IWbemClassObject>	ret;
	CheckWmi(m_svc->GetObject((BSTR)get_class(obj).c_str(), WBEM_FLAG_DIRECT_READ, nullptr, &ret, nullptr));
	return	ret;
}

ComObject<IWbemClassObject>	WmiConnection::get_object(PCWSTR path) const {
	ComObject<IWbemClassObject>	ret;
	CheckWmi(m_svc->GetObject((BSTR)path, WBEM_FLAG_DIRECT_READ, nullptr, &ret, nullptr));
	return	ret;
}

void	WmiConnection::exec_method(PCWSTR path, PCWSTR method, const ComObject<IWbemClassObject> &in_params) const {
	CheckWmi(m_svc->ExecMethod((BSTR)path, (BSTR)method, 0, nullptr, in_params, nullptr, nullptr));
}

Variant	WmiConnection::exec_method_get_param(PCWSTR path, PCWSTR method, PCWSTR param, const ComObject<IWbemClassObject> &in_params) const {
	ComObject<IWbemClassObject> out_params;
	CheckWmi(m_svc->ExecMethod((BSTR)path, (BSTR)method, 0, nullptr, in_params, &out_params, nullptr));
	return	::get_param(out_params, param);
}

ComObject<IWbemClassObject>	WmiConnection::get_in_params(PCWSTR path, PCWSTR method) const {
	return	::get_in_params(get_object(path), method);
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

void WmiBase::Delete() {
	CheckWmi(m_conn->DeleteInstance(m_path, 0, nullptr, nullptr));
}

Variant WmiBase::get_param(PCWSTR param) const {
	return	::get_param(m_obj, param);
}

void 	WmiBase::exec_method(PCWSTR method) const {
	CheckWmi(m_conn->ExecMethod(m_path, (BSTR)method, 0, nullptr, nullptr, nullptr, nullptr));
}

void 	WmiBase::exec_method(PCWSTR method, const ComObject<IWbemClassObject> &in_params) const {
	CheckWmi(m_conn->ExecMethod(m_path, (BSTR)method, 0, nullptr, in_params, nullptr, nullptr));
}

Variant	WmiBase::exec_method_in(PCWSTR method, PCWSTR param, const Variant &val) const {
//	ComObject<IWbemClassObject>	pInSignature;
//	ComObject<IWbemClassObject> pInParams;
	ComObject<IWbemClassObject> in_params;

	ComObject<IWbemClassObject>	obj(m_conn.get_object(get_class(m_obj).c_str()));
	CheckWmi(obj->GetMethod(method, 0, &in_params, nullptr));
//	CheckWmi(pInSignature->SpawnInstance(0, &pInParams));

	CheckWmi(in_params->Put(param, 0, (VARIANT*)&val, 0));

	return exec_method_get_param(method, L"ReturnValue", in_params);
}

Variant	WmiBase::exec_method_get_param(PCWSTR method, PCWSTR param) const {
	ComObject<IWbemClassObject> out_params;
	CheckWmi(m_conn->ExecMethod(m_path, (BSTR)method, 0, nullptr, nullptr, &out_params, nullptr));
	return	::get_param(out_params, param);
}

Variant	WmiBase::exec_method_get_param(PCWSTR method, PCWSTR param, const ComObject<IWbemClassObject> &in_params) const {
	return m_conn.exec_method_get_param(m_path, method, param, in_params);
}

void WmiBase::refresh() {
	m_obj = m_conn.get_object(m_path);
}

///====================================================================================== WMIProcess
int	WmiProcess::attach_debugger() const {
	return exec_method_get_param(L"AttachDebugger").as_int();
}

int	WmiProcess::terminate() const {
	return	exec_method_in(L"Terminate", L"Reason", (DWORD)0xffffffff).as_int();
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
