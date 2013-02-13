#include <libcom/wmi.hpp>
#include <libext/exception.hpp>

namespace Com {
Variant WmiObject::get_param(const pointer obj, PCWSTR param) {
	Variant ret;
	CheckWmi(const_cast<pointer>(obj)->Get(param, 0, &ret, 0, 0));
	return ret;
}

ustring WmiObject::get_class(const pointer obj) {
	return WmiObject::get_param(obj, L"__CLASS").as_str();
}

ustring WmiObject::get_path(const pointer obj) {
	return WmiObject::get_param(obj, L"__RELPATH").as_str();
}

ustring WmiObject::get_server(const pointer obj) {
	return WmiObject::get_param(obj, L"__SERVER").as_str();
}

WmiObject WmiObject::spawn_instance(const pointer obj) {
	WmiObject ret;
	CheckWmi(const_cast<pointer>(obj)->SpawnInstance(0, &ret));
	return ret;
}

WmiObject WmiObject::clone(const pointer obj) {
	WmiObject ret;
	CheckWmi(const_cast<pointer>(obj)->Clone(&ret));
	return ret;
}

WmiObject WmiObject::get_in_params(const pointer obj, PCWSTR method) {
	WmiObject in_params;
	CheckWmi(const_cast<pointer>(obj)->GetMethod(method, 0, &in_params, nullptr));
	return in_params;
}

void WmiObject::put_param(const pointer obj, PCWSTR name, const Variant &val) {
	CheckWmi(const_cast<pointer>(obj)->Put(name, 0, (VARIANT*)&val, 0));
}

WmiObject::WmiObject() {
}

WmiObject::WmiObject(const pointer p):
	ComObject<IWbemClassObject>(p) {
}

WmiObject::WmiObject(const Variant & param):
	ComObject<IWbemClassObject>(param) {
}

void WmiObject::Put(PCWSTR name, const Variant &val) {
	put_param(*this, name, val);
}

Variant WmiObject::Get(PCWSTR name) const {
	return get_param(*this, name);
}

WmiObject WmiObject::SpawnInstance() const {
	return spawn_instance(*this);
}

WmiObject WmiObject::Clone() const {
	return clone(*this);
}

///========================================================================================= WmiEnum
void WmiEnum::Begin() {
	(*this)->Reset();
	m_end = false;
	Next();
}

bool WmiEnum::Next() {
	ULONG ret = 0;
	m_end = !((*this)->Next(WBEM_INFINITE, 1, &m_element, &ret) == WBEM_S_NO_ERROR && ret);
	return !m_end;
}

bool WmiEnum::Next(WmiObject & obj) {
	if (Next()) {
		obj = m_element;
		return true;
	}
	return false;
}

bool WmiEnum::End() {
	return m_end;
}

WmiObject WmiEnum::Elem() const {
	return m_element;
}

///=================================================================================== WmiConnection
WmiConnection::WmiConnection(PCWSTR srv, PCWSTR namesp, PCWSTR user, PCWSTR pass) {
	ComObject<IWbemLocator>	wbemLocator;
	CheckCom(::CoCreateInstance(CLSID_WbemLocator, nullptr, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (PVOID*) &wbemLocator));

	if (!srv || !srv[0])
		srv = L".";				// Empty srv means local computer

	if (NORM_M_PREFIX(srv) || REV_M_PREFIX(srv))
		srv += 2 * sizeofe(srv);

	if (user && !user[0])
		user = pass = nullptr;	// Empty username means default security

	wchar_t	Namespace[MAX_PATH];
	::_snwprintf(Namespace, sizeofa(Namespace), L"\\\\%s\\root\\%s", srv, namesp);

	CheckCom(wbemLocator->ConnectServer(Namespace, BStr(user), BStr(pass), nullptr, 0, nullptr, nullptr, &m_svc));
	CheckCom(::CoSetProxyBlanket(m_svc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE,
								 nullptr, RPC_C_AUTHN_LEVEL_PKT_PRIVACY/*RPC_C_AUTHN_LEVEL_DEFAULT*/, RPC_C_IMP_LEVEL_IMPERSONATE, nullptr,
								 EOAC_STATIC_CLOAKING));
//	CheckCom(::CoSetProxyBlanket(m_svc, RPC_C_AUTHN_WINNT,
//	RPC_C_AUTHZ_NONE, nullptr, RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
//	RPC_C_IMP_LEVEL_IMPERSONATE, nullptr, EOAC_NONE));
}

WmiEnum	WmiConnection::Query(PCWSTR query, ssize_t flags) const {
	WmiEnum ewco;
	CheckWmi(m_svc->ExecQuery((OLECHAR*)L"WQL", (OLECHAR*)query, flags, nullptr, &ewco));
	return ewco;
}

WmiEnum	WmiConnection::Enum(PCWSTR path, ssize_t flags) const {
	WmiEnum	ewco;
	CheckWmi(m_svc->CreateInstanceEnum((BSTR)path, flags, nullptr, &ewco));
	return ewco;
}

void WmiConnection::DeleteInstance(PCWSTR path) {
	CheckWmi(m_svc->DeleteInstance((BSTR)path, 0, nullptr, nullptr));
}

void WmiConnection::DeleteInstance(const WmiObject &obj) {
	DeleteInstance(WmiObject::get_path(obj).c_str());
}

void WmiConnection::CreateInstance(const WmiObject &obj) const {
	CheckWmi(m_svc->PutInstance(obj, WBEM_FLAG_CREATE_ONLY, nullptr, nullptr));
}

void WmiConnection::UpdateInstance(const WmiObject &obj) const {
	CheckCom(m_svc->PutInstance(obj, WBEM_FLAG_UPDATE_ONLY, nullptr, nullptr));
}

WmiObject WmiConnection::get_object_class(const WmiObject &obj) const {
	WmiObject	ret;
	CheckWmi(m_svc->GetObject((BSTR)WmiObject::get_class(obj).c_str(), WBEM_FLAG_DIRECT_READ, nullptr, &ret, nullptr));
	return ret;
}

WmiObject WmiConnection::get_object(PCWSTR clname) const {
	WmiObject ret;
	CheckWmi(m_svc->GetObject((BSTR)clname, WBEM_FLAG_DIRECT_READ, nullptr, &ret, nullptr));
	return ret;
}

void	WmiConnection::exec_method(PCWSTR path, PCWSTR method, const WmiObject &in_params) const {
	CheckWmi(m_svc->ExecMethod((BSTR)path, (BSTR)method, 0, nullptr, in_params, nullptr, nullptr));
}

Variant	WmiConnection::exec_method_get_param(PCWSTR path, PCWSTR method, const WmiObject &in_params, PCWSTR ret_par) const {
	WmiObject out_params;
	CheckWmi(m_svc->ExecMethod((BSTR)path, (BSTR)method, 0, nullptr, in_params, &out_params, nullptr));
	return WmiObject::get_param(out_params, ret_par);
}

WmiObject	WmiConnection::get_in_params(PCWSTR path, PCWSTR method) const {
	return WmiObject::get_in_params(get_object(path), method);
}

///=========================================================================================s WmiBase
WmiBase::~WmiBase() {
}

WmiBase::WmiBase(const WmiConnection & conn, const BStr & path):
	m_conn(conn),
	m_obj(m_conn.get_object(path)),
	m_path(path) {
}

WmiBase::WmiBase(const WmiConnection & conn, const WmiObject & obj):
	m_conn(conn),
	m_obj(obj),
	m_path(WmiObject::get_path(obj))	{
}

void WmiBase::Delete() {
	CheckWmi(m_conn->DeleteInstance(m_path, 0, nullptr, nullptr));
}

void WmiBase::Save() const {
	m_conn.UpdateInstance(m_obj);
}

ustring WmiBase::rel_path() const {
	return get_param(L"__RELPATH").as_str();
}

Variant WmiBase::get_param(PCWSTR param) const {
	return WmiObject::get_param(m_obj, param);
}

WmiObject WmiBase::exec_method(PCWSTR method) const {
	WmiObject out_params;
	CheckCom(m_conn->ExecMethod(m_path, (BSTR)method, 0, nullptr, nullptr, &out_params, nullptr));
	return out_params;
}

WmiObject WmiBase::exec_method(PCWSTR method, const WmiObject &in_params) const {
	WmiObject out_params;
	CheckWmi(m_conn->ExecMethod(m_path, (BSTR)method, 0, nullptr, in_params, &out_params, nullptr));
	return out_params;
}

WmiObject WmiBase::exec_method(PCWSTR method, PCWSTR param, const Variant &val) const {
	WmiObject in_params(WmiObject::get_in_params(m_conn.get_object(WmiObject::get_class(m_obj).c_str()), method));
	WmiObject::put_param(in_params, param, val);
	return exec_method(method, in_params);
}

Variant	WmiBase::exec_method_get_param(PCWSTR method, PCWSTR ret_par) const {
	return WmiObject::get_param(exec_method(method), ret_par);
}

Variant	WmiBase::exec_method_get_param(PCWSTR method, const WmiObject &in_params, PCWSTR ret_par) const {
	return WmiObject::get_param(exec_method(method, in_params), ret_par);
}

Variant	WmiBase::exec_method_get_param(PCWSTR method, PCWSTR param, const Variant &val, PCWSTR ret_par) const {
	return WmiObject::get_param(exec_method(method, param, val), ret_par);
}

void WmiBase::refresh() {
	m_obj = m_conn.get_object(m_path);
}

///====================================================================================== WMIProcess
int	WmiProcess::attach_debugger() const {
	return exec_method_get_param(L"AttachDebugger").as_int();
}

int	WmiProcess::terminate() const {
	return exec_method_get_param(L"Terminate", L"Reason", (DWORD)0xffffffff).as_int();
}

int	WmiProcess::set_priority(DWORD pri) {
	int ret = exec_method_get_param(L"SetPriority", L"Priority", pri).as_int();
	refresh();
	return ret;
}

ustring	WmiProcess::get_owner() const {
	return exec_method_get_param(L"GetOwner", L"User").as_str();
}

ustring	WmiProcess::get_owner_dom() const {
	return exec_method_get_param(L"GetOwner", L"Domain").as_str();
}

ustring	WmiProcess::get_owner_sid() const {
	return exec_method_get_param(L"GetOwnerSid", L"Sid").as_str();
}

BStr WmiProcess::Path(DWORD id) const {
	wchar_t	path[MAX_PATH];
	::_snwprintf(path, sizeofa(path), L"Win32_Process.Handle=%d", id);

	return BStr(path);
}

///==================================================================================== WmiProcessor
BStr WmiProcessor::Path(DWORD id) const {
	wchar_t	path[MAX_PATH];
	::_snwprintf(path, sizeofa(path), L"Win32_Processor.DeviceID='CPU%d'", id);

	return BStr(path);
}

///======================================================================================= WmiSystem
BStr WmiSystem::Path(PCWSTR name) const {
	wchar_t	path[MAX_PATH];
	::_snwprintf(path, sizeofa(path), L"Win32_ComputerSystem", name);

	return BStr(path);
}

}
