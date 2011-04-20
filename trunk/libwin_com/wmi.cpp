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

///========================================================================================= WmiEnum
void WmiEnum::Begin() {
	m_enum->Reset();
	m_end = false;
	Next();
}

bool WmiEnum::Next() {
	ULONG ret = 0;
	m_end = !(m_enum->Next(WBEM_INFINITE, 1, &m_element, &ret) == WBEM_S_NO_ERROR && ret);
	return !m_end;
}

bool WmiEnum::Next(ComObject<IWbemClassObject> &obj) {
	if (Next()) {
		obj = m_element;
		return true;
	}
	return false;
}

bool WmiEnum::End() {
	return m_end;
}

ComObject<IWbemClassObject> WmiEnum::Elem() const {
	return m_element;
}

///=================================================================================== WmiConnection
void WmiConnection::Init(PCWSTR srv, PCWSTR namesp, PCWSTR user, PCWSTR pass) {
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
	CheckCom(::CoSetProxyBlanket(m_svc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE,
								 nullptr, RPC_C_AUTHN_LEVEL_PKT_PRIVACY/*RPC_C_AUTHN_LEVEL_DEFAULT*/, RPC_C_IMP_LEVEL_IMPERSONATE, nullptr,
								 EOAC_STATIC_CLOAKING));
//	CheckCom(::CoSetProxyBlanket(m_svc, RPC_C_AUTHN_WINNT,
//	RPC_C_AUTHZ_NONE, nullptr, RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
//	RPC_C_IMP_LEVEL_IMPERSONATE, nullptr, EOAC_NONE));
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

void	WmiConnection::create(const ComObject<IWbemClassObject> &obj) const {
	CheckWmi(m_svc->PutInstance(obj, WBEM_FLAG_CREATE_ONLY, nullptr, nullptr));
}

void	WmiConnection::update(const ComObject<IWbemClassObject> &obj) const {
	CheckWmi(m_svc->PutInstance(obj, WBEM_FLAG_UPDATE_ONLY, nullptr, nullptr));
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

Variant	WmiConnection::exec_method_get_param(PCWSTR path, PCWSTR method, const ComObject<IWbemClassObject> &in_params, PCWSTR ret_par) const {
	ComObject<IWbemClassObject> out_params;
	CheckWmi(m_svc->ExecMethod((BSTR)path, (BSTR)method, 0, nullptr, in_params, &out_params, nullptr));
	return	::get_param(out_params, ret_par);
}

ComObject<IWbemClassObject>	WmiConnection::get_in_params(PCWSTR path, PCWSTR method) const {
	return	::get_in_params(get_object(path), method);
}

///=========================================================================================s WmiBase
WmiBase::~WmiBase() {
}

WmiBase::WmiBase(const WmiConnection &conn, const BStr &path):
	m_conn(conn),
	m_obj(m_conn.get_object(path)),
	m_path(path) {
}

WmiBase::WmiBase(const WmiConnection &conn, const ComObject<IWbemClassObject> &obj):
	m_conn(conn),
	m_obj(obj),
	m_path(::get_path(obj))	{
}

void WmiBase::Delete() {
	CheckWmi(m_conn->DeleteInstance(m_path, 0, nullptr, nullptr));
}

void WmiBase::Save() const {
	m_conn.update(m_obj);
}

AutoUTF WmiBase::rel_path() const {
	return get_param(L"__RELPATH").as_str();
}

Variant WmiBase::get_param(PCWSTR param) const {
	return	::get_param(m_obj, param);
}

ComObject<IWbemClassObject>	WmiBase::exec_method(PCWSTR method) const {
	ComObject<IWbemClassObject> out_params;
	CheckWmi(m_conn->ExecMethod(m_path, (BSTR)method, 0, nullptr, nullptr, &out_params, nullptr));
	return out_params;
}

ComObject<IWbemClassObject>	WmiBase::exec_method(PCWSTR method, const ComObject<IWbemClassObject> &in_params) const {
	ComObject<IWbemClassObject> out_params;
	CheckWmi(m_conn->ExecMethod(m_path, (BSTR)method, 0, nullptr, in_params, &out_params, nullptr));
	return out_params;
}

ComObject<IWbemClassObject>	WmiBase::exec_method(PCWSTR method, PCWSTR param, const Variant &val) const {
	ComObject<IWbemClassObject> in_params(get_in_params(m_conn.get_object(get_class(m_obj).c_str()), method));
	put_param(in_params, param, val);
	return exec_method(method, in_params);
}

Variant	WmiBase::exec_method_get_param(PCWSTR method, PCWSTR ret_par) const {
	return	::get_param(exec_method(method), ret_par);
}

Variant	WmiBase::exec_method_get_param(PCWSTR method, const ComObject<IWbemClassObject> &in_params, PCWSTR ret_par) const {
	return	::get_param(exec_method(method, in_params), ret_par);
}

Variant	WmiBase::exec_method_get_param(PCWSTR method, PCWSTR param, const Variant &val, PCWSTR ret_par) const {
	return	::get_param(exec_method(method, param, val), ret_par);
}

void WmiBase::refresh() {
	m_obj = m_conn.get_object(m_path);
}

///====================================================================================== WMIProcess
int	WmiProcess::attach_debugger() const {
	return exec_method_get_param(L"AttachDebugger").as_int();
}

int	WmiProcess::terminate() const {
	return	exec_method_get_param(L"Terminate", L"Reason", (DWORD)0xffffffff).as_int();
}

int	WmiProcess::set_priority(DWORD pri) {
	int ret = exec_method_get_param(L"SetPriority", L"Priority", pri).as_int();
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
