#include "wmi_iis.h"

///==================================================================================== WmiIisServer
AutoUTF WmiIisServer::name() const {
	return get_param(L"Name").as_str();
}

BStr WmiIisServer::Path(PCWSTR name) const {
	WCHAR	path[MAX_PATH];
	::_snwprintf(path, sizeofa(path), L"Server.Name=\"%s\"", name);
	return BStr(path);
}

///=================================================================================== WmiIisAppPool
void WmiIisAppPool::Create(const WmiConnection &conn, const AutoUTF &name, bool enabled) {
	ComObject<IWbemClassObject> in_params = conn.get_in_params(L"ApplicationPool", L"Create");

	put_param(in_params, L"Name", name);
	put_param(in_params, L"AutoStart", enabled);

	conn.exec_method(L"ApplicationPool", L"Create", in_params);
}

WmiEnum WmiIisAppPool::Enum(const WmiConnection &conn) {
	return WmiEnum(conn.Enum(L"ApplicationPool"));
}

AutoUTF WmiIisAppPool::name() const {
	return get_param(L"Name").as_str();
}

AutoUTF WmiIisAppPool::id() const {
	return exec_method_get_param(L"GetApplicationPoolSid").as_str();
}

bool WmiIisAppPool::is_enabled() const {
	return get_param(L"AutoStart").as_bool();
}

bool WmiIisAppPool::is_classic() const {
	return get_param(L"ManagedPipelineMode").as_int();
}

AutoUTF WmiIisAppPool::version() const {
	return get_param(L"ManagedRuntimeVersion").as_str();
}

void WmiIisAppPool::enable() {
	Variant tmp(true);
	CheckWmi(m_obj->Put(L"AutoStart", 0, &tmp, 0));
	conn().update(m_obj);
}

void WmiIisAppPool::disable() {
	Variant tmp(false);
	CheckWmi(m_obj->Put(L"AutoStart", 0, &tmp, 0));
	conn().update(m_obj);
}

void WmiIisAppPool::classic(bool in) {
	Variant tmp((DWORD)in);
	CheckWmi(m_obj->Put(L"ManagedPipelineMode", 0, &tmp, 0));
	conn().update(m_obj);
}

void WmiIisAppPool::support_x32(bool in) {
	Variant tmp(in);
	CheckWmi(m_obj->Put(L"Enable32BitAppOnWin64", 0, &tmp, 0));
	conn().update(m_obj);
}

void WmiIisAppPool::version(PCWSTR in) {
	Variant tmp(in);
	CheckWmi(m_obj->Put(L"ManagedRuntimeVersion", 0, &tmp, 0));
	conn().update(m_obj);
}

BStr WmiIisAppPool::Path(PCWSTR name) const {
	WCHAR	path[MAX_PATH];
	::_snwprintf(path, sizeofa(path), L"ApplicationPool.Name=\"%s\"", name);
	return BStr(path);
}

///======================================================================================= WmiIisLog
AutoUTF WmiIisLog::directory() const {
	return get_param(L"Directory").as_str();
}

size_t WmiIisLog::format() const {
	return get_param(L"LogFormat").as_uint();
}

size_t WmiIisLog::period() const {
	return get_param(L"Period").as_uint();
}

size_t WmiIisLog::size() const {
	return get_param(L"TruncateSize").as_uint();
}

size_t WmiIisLog::flags() const {
	return get_param(L"LogExtFileFlags").as_uint();
}

bool WmiIisLog::is_rollover() const {
	return get_param(L"LocalTimeRollover").as_bool();
}

bool WmiIisLog::is_enabled() const {
	return get_param(L"Enabled").as_bool();
}

void WmiIisLog::directory(const AutoUTF &in) {
	put_param(m_obj, L"Directory", in);
}

///=================================================================================== WmiIisBinding
WmiIisBinding::WmiIisBinding(const WmiConnection &conn, const AutoUTF &info, const AutoUTF &prot):
	WmiBase(conn, conn.get_object(L"BindingElement")) {
	m_info = TrimOut(info);
	m_prot = TrimOut(prot);
	put_param(m_obj, L"BindingInformation", m_info);
	put_param(m_obj, L"Protocol", m_prot);
	update();
}

AutoUTF WmiIisBinding::info() const {
	if (m_info.empty()) {
		m_info = get_param(L"BindingInformation").as_str();
		update();
	}
	return m_info;
}

AutoUTF WmiIisBinding::protocol() const {
	if (m_prot.empty()) {
		m_prot = get_param(L"Protocol").as_str();
	}
	return m_prot;
}

AutoUTF WmiIisBinding::ip() const {
	return m_ip;
}

AutoUTF WmiIisBinding::port() const {
	return m_port;
}

AutoUTF WmiIisBinding::name() const {
	return m_name;
}

void WmiIisBinding::info(const AutoUTF &in) {
	put_param(m_obj, L"BindingInformation", in);
	m_info = in;
	update();
}

void WmiIisBinding::protocol(const AutoUTF &in) {
	m_prot = in;
	put_param(m_obj, L"Protocol", m_prot);
}

void WmiIisBinding::ip(const AutoUTF &in) {
	AutoUTF info = in + L":" + m_port + L":" + m_name;
	put_param(m_obj, L"BindingInformation", info);
	m_info = info;
	m_ip = in;
}

void WmiIisBinding::port(const AutoUTF &in) {
	AutoUTF info = m_ip + L":" + in + L":" + m_name;
	put_param(m_obj, L"BindingInformation", info);
	m_info = info;
	m_port = in;
}

void WmiIisBinding::name(const AutoUTF &in) {
	AutoUTF info = m_ip + L":" + m_port + L":" + in;
	put_param(m_obj, L"BindingInformation", info);
	m_info = info;
	m_name = in;
}

void WmiIisBinding::update() const {
	m_name = m_info;
	m_ip = CutWord(m_name, L":");
	m_port = CutWord(m_name, L":");
}

///=============================================================================== WmiIisApplication
void WmiIisApplication::Create(const WmiConnection &conn, const AutoUTF &name, const AutoUTF &app_path, const AutoUTF &phis_path) {
	ComObject<IWbemClassObject> in_params = conn.get_in_params(L"Application", L"Create");
	put_param(in_params, L"ApplicationPath", app_path);
	put_param(in_params, L"SiteName", name);
	put_param(in_params, L"PhysicalPath", phis_path);
	conn.exec_method(L"Application", L"Create", in_params);
}

WmiEnum WmiIisApplication::Enum(const WmiConnection &conn) {
	return WmiEnum(conn.Enum(L"Application"));
}

AutoUTF WmiIisApplication::name() const {
	return get_param(L"SiteName").as_str();
}

AutoUTF WmiIisApplication::pool() const {
	return get_param(L"ApplicationPool").as_str();
}

AutoUTF WmiIisApplication::path() const {
	return get_param(L"Path").as_str();
}

AutoUTF WmiIisApplication::protocols() const {
	return get_param(L"EnabledProtocols").as_str();
}

void WmiIisApplication::pool(const AutoUTF &in) {
	put_param(m_obj, L"ApplicationPool", in);
	conn().update(m_obj);
}

void WmiIisApplication::protocols(const AutoUTF &in) {
	put_param(m_obj, L"EnabledProtocols", in);
	conn().update(m_obj);
}

BStr WmiIisApplication::Path(PCWSTR name, PCWSTR path) const {
	WCHAR	tmp[MAX_PATH];
	::_snwprintf(tmp, sizeofa(tmp), L"Application.Path=\"%s\",SiteName=\"%s\"", path, name);
	return BStr(tmp);
}

///=================================================================================== WmiIisVirtDir
void WmiIisVirtDir::Create(const WmiConnection &conn, const AutoUTF &name, const AutoUTF &vd_path, const AutoUTF &phis_path, const AutoUTF &app_path) {
	ComObject<IWbemClassObject> in_params = conn.get_in_params(L"VirtualDirectory", L"Create");
	put_param(in_params, L"VirtualDirectoryPath", vd_path);
	put_param(in_params, L"ApplicationPath", app_path);
	put_param(in_params, L"PhysicalPath", phis_path);
	put_param(in_params, L"SiteName", name);
	conn.exec_method(L"VirtualDirectory", L"Create", in_params);
}

WmiEnum WmiIisVirtDir::Enum(const WmiConnection &conn) {
	return WmiEnum(conn.Enum(L"VirtualDirectory"));
}

AutoUTF WmiIisVirtDir::name() const {
	return get_param(L"SiteName").as_str();
}

AutoUTF WmiIisVirtDir::directory() const {
	return get_param(L"PhysicalPath").as_str();
}

AutoUTF WmiIisVirtDir::app_path() const {
	return get_param(L"ApplicationPath").as_str();
}

AutoUTF WmiIisVirtDir::path() const {
	return get_param(L"Path").as_str();
}

void WmiIisVirtDir::directory(const AutoUTF &in) {
	put_param(m_obj, L"PhysicalPath", in);
	conn().update(m_obj);
}

BStr WmiIisVirtDir::Path(PCWSTR name, PCWSTR path, PCWSTR apppath) const {
	WCHAR	tmp[MAX_PATH];
	::_snwprintf(tmp, sizeofa(tmp), L"VirtualDirectory.ApplicationPath=\"%s\",Path=\"%s\",SiteName=\"%s\"", apppath, path, name);
	return BStr(tmp);
}

///====================================================================================== WmiIisSite
void WmiIisSite::Create(const WmiConnection &conn, const AutoUTF &name, const AutoUTF &ip, const AutoUTF &path) {
	ComObject<IWbemClassObject> binding = conn.get_object(L"BindingElement");
	put_param(binding, L"BindingInformation", AutoUTF(ip + L":80:" + name));
	put_param(binding, L"Protocol", L"http");

	Variant tmp;
	tmp.parray = CheckPointer(::SafeArrayCreateVector(VT_UNKNOWN, 0, 1));
	tmp.vt = VT_ARRAY | VT_UNKNOWN;
	IWbemClassObject **data = (IWbemClassObject**)tmp.parray->pvData;
	for (size_t i = 0; i < 1; ++i) {
		binding.detach(data[i]);
	}

	ComObject<IWbemClassObject> in_params = conn.get_in_params(L"Site", L"Create");
	put_param(in_params, L"Name", name);
	put_param(in_params, L"Bindings", tmp);
	put_param(in_params, L"PhysicalPath", path);
	put_param(in_params, L"ServerAutoStart", true);
	conn.exec_method(L"Site", L"Create", in_params);
}

WmiEnum WmiIisSite::Enum(const WmiConnection &conn) {
	return WmiEnum(conn.Enum(L"Site"));
}

AutoUTF WmiIisSite::name() const {
	return get_param(L"Name").as_str();
}

size_t WmiIisSite::id() const {
	return get_param(L"Id").as_int();
}

Variant WmiIisSite::bindings() const {
	return get_param(L"Bindings");
}

ComObject<IWbemClassObject> WmiIisSite::log() const {
	Variant val(get_param(L"LogFile"));
	ComObject<IWbemClassObject> ret((IWbemClassObject*)val.ppunkVal);
	ret->AddRef();
	return ret;
}

bool WmiIisSite::is_enabled() const {
	return get_param(L"ServerAutoStart").as_bool();
}

void WmiIisSite::enable() {
	Variant tmp(true);
	CheckWmi(m_obj->Put(L"ServerAutoStart", 0, &tmp, 0));
	conn().update(m_obj);
}

void WmiIisSite::disable() {
	Variant tmp(false);
	CheckWmi(m_obj->Put(L"ServerAutoStart", 0, &tmp, 0));
	conn().update(m_obj);
}

BStr WmiIisSite::Path(PCWSTR name) const {
	WCHAR	path[MAX_PATH];
	::_snwprintf(path, sizeofa(path), L"Site.Name=\"%s\"", name);
	return BStr(path);
}

