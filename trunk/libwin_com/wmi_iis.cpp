#include "wmi_iis.h"
#include <libwin_net/exception.h>

///==================================================================================== WmiIisServer
ustring WmiIisServer::name() const {
	return get_param(L"Name").as_str();
}

BStr WmiIisServer::Path(PCWSTR name) const {
	WCHAR	path[MAX_PATH];
	::_snwprintf(path, sizeofa(path), L"Server.Name=\"%s\"", name);
	return BStr(path);
}

///============================================================================== WmiIisProcessModel
ustring WmiIisProcessModel::user() const {
	return get_param(L"UserName").as_str();
}

ustring WmiIisProcessModel::pass() const {
	return get_param(L"Password").as_str();
}

void WmiIisProcessModel::user(const ustring &name, const ustring &pass) {
	if (name.empty()) {
		try {
			WmiObject::put_param(m_obj, L"IdentityType", DWORD(4)); // ApplicationPoolIdentity
		} catch (...) {
			WmiObject::put_param(m_obj, L"IdentityType", DWORD(2)); // network service
		}
		WmiObject::put_param(m_obj, L"Password", L"");
	} else {
		WmiObject::put_param(m_obj, L"IdentityType", DWORD(3));
		WmiObject::put_param(m_obj, L"Password", pass);
	}
	WmiObject::put_param(m_obj, L"UserName", name);
}

///=================================================================================== WmiIisAppPool
void WmiIisAppPool::Create(const WmiConnection &conn, const ustring &name, bool enabled) {
	WmiObject in_params = conn.get_in_params(L"ApplicationPool", L"Create");

	WmiObject::put_param(in_params, L"Name", name);
	WmiObject::put_param(in_params, L"AutoStart", enabled);

	conn.exec_method(L"ApplicationPool", L"Create", in_params);
}

WmiEnum WmiIisAppPool::Enum(const WmiConnection &conn) {
	return WmiEnum(conn.Enum(L"ApplicationPool"));
}

WmiEnum WmiIisAppPool::EnumLike(const WmiConnection &conn, const ustring &like) {
	WCHAR query[MAX_PATH];
	::_snwprintf(query, sizeofa(query), L"SELECT * FROM ApplicationPool WHERE Name LIKE \"%s\"", like.c_str());
	return WmiEnum(conn.Query(query));
}

ustring WmiIisAppPool::name() const {
	return get_param(L"Name").as_str();
}

ustring WmiIisAppPool::id() const {
	return exec_method_get_param(L"GetApplicationPoolSid").as_str();
}

bool WmiIisAppPool::is_enabled() const {
	return get_param(L"AutoStart").as_bool();
}

bool WmiIisAppPool::is_classic() const {
	return get_param(L"ManagedPipelineMode").as_int();
}

ustring WmiIisAppPool::version() const {
	return get_param(L"ManagedRuntimeVersion").as_str();
}

void WmiIisAppPool::enable() {
	WmiObject::put_param(m_obj, L"AutoStart", true);
}

void WmiIisAppPool::disable() {
	WmiObject::put_param(m_obj, L"AutoStart", false);
}

void WmiIisAppPool::classic(bool in) {
	WmiObject::put_param(m_obj, L"ManagedPipelineMode", (DWORD)in);
}

void WmiIisAppPool::support_x32(bool in) {
	WmiObject::put_param(m_obj, L"Enable32BitAppOnWin64", in);
}

void WmiIisAppPool::version(PCWSTR in) {
	WmiObject::put_param(m_obj, L"ManagedRuntimeVersion", in);
}

WmiIisProcessModel WmiIisAppPool::model() const {
	Variant val(get_param(L"ProcessModel"));
	WmiObject ret((IWbemClassObject*)val.ppunkVal);
	ret->AddRef();
	return WmiIisProcessModel(conn(), ret);
}

void WmiIisAppPool::model(const WmiIisProcessModel &in) {
	WmiObject::put_param(m_obj, L"ProcessModel", Variant((IUnknown*)in));
}

BStr WmiIisAppPool::Path(PCWSTR name) const {
	WCHAR	path[MAX_PATH];
	::_snwprintf(path, sizeofa(path), L"ApplicationPool.Name=\"%s\"", name);
	return BStr(path);
}

///======================================================================================= WmiIisSiteLog
ustring WmiIisSiteLog::directory() const {
	return get_param(L"Directory").as_str();
}

size_t WmiIisSiteLog::format() const {
	return get_param(L"LogFormat").as_uint();
}

size_t WmiIisSiteLog::period() const {
	return get_param(L"Period").as_uint();
}

size_t WmiIisSiteLog::size() const {
	return get_param(L"TruncateSize").as_uint();
}

size_t WmiIisSiteLog::flags() const {
	return get_param(L"LogExtFileFlags").as_uint();
}

bool WmiIisSiteLog::is_rollover() const {
	return get_param(L"LocalTimeRollover").as_bool();
}

bool WmiIisSiteLog::is_enabled() const {
	return get_param(L"Enabled").as_bool();
}

void WmiIisSiteLog::directory(const ustring &in) {
	m_obj.Put(L"Directory", in);
}

///=================================================================================== WmiIisBinding
WmiIisBinding::WmiIisBinding(const WmiConnection &conn, const ustring &info, const ustring &prot):
	WmiBase(conn, conn.get_object(L"BindingElement")) {
	m_info = TrimOut(info);
	m_prot = TrimOut(prot);
	m_obj.Put(L"BindingInformation", m_info);
	m_obj.Put(L"Protocol", m_prot);
	update();
}

ustring WmiIisBinding::info() const {
	if (m_info.empty()) {
		m_info = get_param(L"BindingInformation").as_str();
		update();
	}
	return m_info;
}

ustring WmiIisBinding::protocol() const {
	if (m_prot.empty()) {
		m_prot = get_param(L"Protocol").as_str();
	}
	return m_prot;
}

ustring WmiIisBinding::ip() const {
	return m_ip;
}

ustring WmiIisBinding::port() const {
	return m_port;
}

ustring WmiIisBinding::name() const {
	return m_name;
}

void WmiIisBinding::info(const ustring &in) {
	m_obj.Put(L"BindingInformation", in);
	m_info = in;
	update();
}

void WmiIisBinding::protocol(const ustring &in) {
	m_prot = in;
	m_obj.Put(L"Protocol", m_prot);
}

void WmiIisBinding::ip(const ustring &in) {
	ustring info = in + L":" + m_port + L":" + m_name;
	m_obj.Put(L"BindingInformation", info);
	m_info = info;
	m_ip = in;
}

void WmiIisBinding::port(const ustring &in) {
	ustring info = m_ip + L":" + in + L":" + m_name;
	m_obj.Put(L"BindingInformation", info);
	m_info = info;
	m_port = in;
}

void WmiIisBinding::name(const ustring &in) {
	ustring info = m_ip + L":" + m_port + L":" + in;
	m_obj.Put(L"BindingInformation", info);
	m_info = info;
	m_name = in;
}

void WmiIisBinding::update() const {
	m_name = m_info;
	m_ip = CutWord(m_name, L":");
	m_port = CutWord(m_name, L":");
}

///================================================================================== WmiIisBindings
WmiIisBindings::WmiIisBindings(const Variant &var) {
	SafeArray arr(var);
	for (size_t i = 0; i < arr.size(); ++i) {
		push_back(value_type(
			WmiObject::get_param(arr.at<IWbemClassObject*>(i), L"BindingInformation").as_str(),
			WmiObject::get_param(arr.at<IWbemClassObject*>(i), L"Protocol").as_str()));
	}
}

void WmiIisBindings::add(const ustring &info, const ustring &prot) {
	push_back(value_type(info, prot));
}

void WmiIisBindings::add(const ustring &ip, const ustring &port, const ustring &name, const ustring &prot) {
	add(ip + L":" + port + L":" + name, prot);
}

void WmiIisBindings::del(const ustring &info, const ustring &prot) {
	iterator it;
	if ((it = std::find(begin(), end(), value_type(info, prot))) != end())
		erase(it);
}

void WmiIisBindings::del(const ustring &ip, const ustring &port, const ustring &name, const ustring &prot) {
	del(ip + L":" + port + L":" + name, prot);
}

WmiIisBindings WmiIisBindings::operator-(const WmiIisBindings &rhs) const {
	WmiIisBindings ret;
	std::set_difference(begin(), end(),rhs.begin(), rhs.end(), std::back_inserter(ret));
	return ret;
}

///=============================================================================== WmiIisApplication
void WmiIisApplication::Create(const WmiConnection &conn, const ustring &name, const ustring &app_path, const ustring &phis_path) {
	WmiObject in_params = conn.get_in_params(L"Application", L"Create");
	in_params.Put(L"ApplicationPath", app_path);
	in_params.Put(L"SiteName", name);
	in_params.Put(L"PhysicalPath", phis_path);
	conn.exec_method(L"Application", L"Create", in_params);
}

WmiEnum WmiIisApplication::Enum(const WmiConnection &conn) {
	return WmiEnum(conn.Enum(L"Application"));
}

ustring WmiIisApplication::name() const {
	return get_param(L"SiteName").as_str();
}

ustring WmiIisApplication::pool() const {
	return get_param(L"ApplicationPool").as_str();
}

ustring WmiIisApplication::path() const {
	return get_param(L"Path").as_str();
}

ustring WmiIisApplication::protocols() const {
	return get_param(L"EnabledProtocols").as_str();
}

void WmiIisApplication::pool(const ustring &in) {
	m_obj.Put(L"ApplicationPool", in);
	Save();
}

void WmiIisApplication::protocols(const ustring &in) {
	m_obj.Put(L"EnabledProtocols", in);
	Save();
}

BStr WmiIisApplication::Path(PCWSTR name, PCWSTR path) const {
	WCHAR	tmp[MAX_PATH];
	::_snwprintf(tmp, sizeofa(tmp), L"Application.Path=\"%s\",SiteName=\"%s\"", path, name);
	return BStr(tmp);
}

///=================================================================================== WmiIisVirtDir
void WmiIisVirtDir::Create(const WmiConnection &conn, const ustring &name, const ustring &vd_path, const ustring &phis_path, const ustring &app_path) {
	WmiObject in_params = conn.get_in_params(L"VirtualDirectory", L"Create");
	in_params.Put(L"VirtualDirectoryPath", vd_path);
	in_params.Put(L"ApplicationPath", app_path);
	in_params.Put(L"PhysicalPath", phis_path);
	in_params.Put(L"SiteName", name);
	conn.exec_method(L"VirtualDirectory", L"Create", in_params);
}

WmiEnum WmiIisVirtDir::Enum(const WmiConnection &conn) {
	return WmiEnum(conn.Enum(L"VirtualDirectory"));
}

ustring WmiIisVirtDir::name() const {
	return get_param(L"SiteName").as_str();
}

ustring WmiIisVirtDir::directory() const {
	return get_param(L"PhysicalPath").as_str();
}

ustring WmiIisVirtDir::app_path() const {
	return get_param(L"ApplicationPath").as_str();
}

ustring WmiIisVirtDir::path() const {
	return get_param(L"Path").as_str();
}

void WmiIisVirtDir::directory(const ustring &in) {
	m_obj.Put(L"PhysicalPath", in);
	Save();
}

BStr WmiIisVirtDir::Path(PCWSTR name, PCWSTR path, PCWSTR apppath) const {
	WCHAR	tmp[MAX_PATH];
	::_snwprintf(tmp, sizeofa(tmp), L"VirtualDirectory.ApplicationPath=\"%s\",Path=\"%s\",SiteName=\"%s\"", apppath, path, name);
	return BStr(tmp);
}

///=========================================================================== WmiSectionInformation
ustring WmiSectionInformation::override() const {
	return get_param(L"EffectiveOverrideMode").as_str();
}

void WmiSectionInformation::override(bool in) {
	m_obj.Put(L"OverrideMode", in ? L"Allow" : L"Deny");
}

//bool WmiSectionInformation::is_locked() const {
//	return get_param(L"IsLocked").as_bool();
//}
//
//void WmiSectionInformation::locked(bool in) {
//	put_param(m_obj, L"LockItem", in);
//	conn().update(m_obj);
//}

///=================================================================================== WmiIisSection
ustring WmiIisSection::name() const {
	return get_param(L"Path").as_str();
}

void WmiIisSection::revert(PCWSTR name) {
	exec_method(L"RevertToParent", L"PropertyName", name);
}

WmiObject WmiIisSection::info() const {
	printf(L"info 1\n");
	Variant val(get_param(L"SectionInformation"));
	printf(L"info 2\n");
	WmiObject ret((IWbemClassObject*)val.ppunkVal);
	printf(L"info 3\n");
	ret->AddRef();
	printf(L"info 4\n");
	return ret;
}

void WmiIisSection::info(const WmiSectionInformation &in) {
	m_obj.Put(L"SectionInformation", Variant((IUnknown*)in));
	Save();
}

///==================================================================================== WmiIisAccess
size_t WmiIisAccess::flags() const {
	return get_param(L"SslFlags").as_uint();
}

void WmiIisAccess::flags(DWORD acc) {
	m_obj.Put(L"SslFlags", acc);
	Save();
}

BStr WmiIisAccess::Path(PCWSTR name) const {
	WCHAR	tmp[MAX_PATH];
	if (name)
		::_snwprintf(tmp, sizeofa(tmp), L"AccessSection.Location=\"\",Path=\"MACHINE/WEBROOT/APPHOST/%s\"", name);
	else
		::_snwprintf(tmp, sizeofa(tmp), L"AccessSection.Location=\"\",Path=\"MACHINE/WEBROOT/APPHOST\"");
	return BStr(tmp);
}

///============================================================================ WmiIisAuthentication
BStr WmiIisAuthentication::Path(PCWSTR name) const {
	WCHAR	tmp[MAX_PATH];
	if (name)
		::_snwprintf(tmp, sizeofa(tmp), L"AuthenticationSection.Location=\"\",Path=\"MACHINE/WEBROOT/APPHOST/%s\"", name);
	else
		::_snwprintf(tmp, sizeofa(tmp), L"AuthenticationSection.Location=\"\",Path=\"MACHINE/WEBROOT/APPHOST\"");
	return BStr(tmp);
}

///============================================================================= WmiIisAuthorization
Variant WmiIisAuthorization::rules() const {
	return get_param(L"Authorization");
}

BStr WmiIisAuthorization::Path(PCWSTR name) const {
	WCHAR	tmp[MAX_PATH];
	if (name)
		::_snwprintf(tmp, sizeofa(tmp), L"AuthorizationSection.Location=\"\",Path=\"MACHINE/WEBROOT/APPHOST/%s\"", name);
	else
		::_snwprintf(tmp, sizeofa(tmp), L"AuthorizationSection.Location=\"\",Path=\"MACHINE/WEBROOT/APPHOST\"");
	return BStr(tmp);
}

///============================================================================== WmiDefaultDocument
bool WmiIisDefaultDocument::list(std::vector<ustring> &out) const {
	try {
		Variant files(get_param(L"Files"));
		WmiObject ifiles(files.punkVal);
		Variant strings(WmiObject::get_param(ifiles, L"Files"));
		SafeArray arr(strings);
		out.clear();
		for (size_t i = 0; i < arr.size(); ++i) {
			WmiObject str(arr.at<IUnknown*>(i));
			out.push_back(WmiObject::get_param(str, L"Value").as_str());
		}
	} catch (...) {
		return false;
	}
	return true;
}

bool WmiIisDefaultDocument::add(const ustring &in) {
	try {
		WmiObject in_params(WmiObject::get_in_params(conn().get_object(WmiObject::get_class(m_obj).c_str()), L"Add"));
		in_params.Put(L"CollectionName", L"Files.Files");

		WmiObject	elem(conn().get_object(L"StringElement"));
		elem.Put(L"Value", in);
		in_params.Put(L"element", Variant((IUnknown*)elem));
		exec_method(L"Add", in_params);
	} catch (...) {
		return false;
	}
	return true;
}

bool WmiIisDefaultDocument::del(const ustring &in) {
	try {
		WmiObject in_params(WmiObject::get_in_params(conn().get_object(WmiObject::get_class(m_obj).c_str()), L"Remove"));
		in_params.Put(L"CollectionName", L"Files.Files");

		WmiObject	elem(conn().get_object(L"StringElement"));
		elem.Put(L"Value", in);
		in_params.Put(L"element", Variant((IUnknown*)elem));
		exec_method(L"Remove", in_params);
	} catch (...) {
		return false;
	}
	return true;
}

void WmiIisDefaultDocument::clear() {
	exec_method(L"Clear", L"CollectionName", L"Files.Files");
}

BStr WmiIisDefaultDocument::Path(PCWSTR name) const {
	WCHAR	tmp[MAX_PATH];
	if (name)
		::_snwprintf(tmp, sizeofa(tmp), L"DefaultDocumentSection.Location=\"\",Path=\"MACHINE/WEBROOT/APPHOST/%s\"", name);
	else
		::_snwprintf(tmp, sizeofa(tmp), L"DefaultDocumentSection.Location=\"\",Path=\"MACHINE/WEBROOT/APPHOST\"");
	return BStr(tmp);
}

///================================================================================== WmiIisHandlers
size_t WmiIisHandlers::access() const {
	return get_param(L"AccessPolicy").as_uint();
}

void WmiIisHandlers::access(DWORD acc) {
	m_obj.Put(L"AccessPolicy", acc);
	Save();
}

BStr WmiIisHandlers::Path(PCWSTR name) const {
	WCHAR	tmp[MAX_PATH];
	if (name)
		::_snwprintf(tmp, sizeofa(tmp), L"HandlersSection.Location=\"\",Path=\"MACHINE/WEBROOT/APPHOST/%s\"", name);
	else
		::_snwprintf(tmp, sizeofa(tmp), L"HandlersSection.Location=\"\",Path=\"MACHINE/WEBROOT/APPHOST\"");
	return BStr(tmp);
}

///================================================================================ IsapiCgiRestrict
bool WmiIsapiCgiRestrict::is_not_listed_cgis_allowed() const {
	return get_param(L"NotListedCgisAllowed").as_bool();
}

bool WmiIsapiCgiRestrict::is_not_listed_isapis_allowed() const {
	return get_param(L"NotListedIsapisAllowed").as_bool();
}

void WmiIsapiCgiRestrict::not_listed_cgis_allowed(bool in) {
	m_obj.Put(L"NotListedCgisAllowed", in);
	Save();
}

void WmiIsapiCgiRestrict::not_listed_isapis_allowed(bool in) {
	m_obj.Put(L"NotListedIsapisAllowed", in);
	Save();
}

BStr WmiIsapiCgiRestrict::Path(PCWSTR name) const {
	WCHAR	tmp[MAX_PATH];
	if (name)
		::_snwprintf(tmp, sizeofa(tmp), L"IsapiCgiRestrictionSection.Location=\"\",Path=\"MACHINE/WEBROOT/APPHOST/%s\"", name);
	else
		::_snwprintf(tmp, sizeofa(tmp), L"IsapiCgiRestrictionSection.Location=\"\",Path=\"MACHINE/WEBROOT/APPHOST\"");
	return BStr(tmp);
}

///======================================================================================= WmiIisLog
WmiIisLog::LogMode WmiIisLog::mode() const {
	return (WmiIisLog::LogMode)get_param(L"CentralLogFileMode").as_uint();
}

WmiObject WmiIisLog::CentralW3CLogFile() const {
	Variant val(get_param(L"CentralW3CLogFile"));
	WmiObject ret((IWbemClassObject*)val.ppunkVal);
	ret->AddRef();
	return ret;
}

void WmiIisLog::mode(LogMode in) {
	m_obj.Put(L"CentralLogFileMode", (DWORD)in);
	Save();
}

BStr WmiIisLog::Path(PCWSTR name) const {
	WCHAR	tmp[MAX_PATH];
	if (name)
		::_snwprintf(tmp, sizeofa(tmp), L"LogSection.Location=\"\",Path=\"MACHINE/WEBROOT/APPHOST/%s\"", name);
	else
		::_snwprintf(tmp, sizeofa(tmp), L"LogSection.Location=\"\",Path=\"MACHINE/WEBROOT/APPHOST\"");
	return BStr(tmp);
}

///====================================================================================== WmiIisSite
void WmiIisSite::Create(const WmiConnection &conn, const ustring &name, const ustring &ip, const ustring &path) {
	WmiObject binding = conn.get_object(L"BindingElement");
	binding.Put(L"BindingInformation", ustring(ip + L":80:" + name));
	binding.Put(L"Protocol", L"http");

	Variant tmp;
	tmp.parray = CheckPointer(::SafeArrayCreateVector(VT_UNKNOWN, 0, 1));
	tmp.vt = VT_ARRAY | VT_UNKNOWN;
	IWbemClassObject **data = (IWbemClassObject**)tmp.parray->pvData;
	for (size_t i = 0; i < 1; ++i) {
		binding.detach(data[i]);
	}

	WmiObject in_params = conn.get_in_params(L"Site", L"Create");
	in_params.Put(L"Name", name);
	in_params.Put(L"Bindings", tmp);
	in_params.Put(L"PhysicalPath", path);
	in_params.Put(L"ServerAutoStart", true);
	conn.exec_method(L"Site", L"Create", in_params);
}

WmiEnum WmiIisSite::Enum(const WmiConnection &conn) {
	return WmiEnum(conn.Enum(L"Site"));
}

ustring WmiIisSite::name() const {
	return get_param(L"Name").as_str();
}

size_t WmiIisSite::id() const {
	return get_param(L"Id").as_int();
}

Variant WmiIisSite::bindings() const {
	return get_param(L"Bindings");
}

void WmiIisSite::bindings(const WmiIisBindings &in) {
	std::vector<winstd::shared_ptr<WmiIisBinding> > binds;
	for (size_t i = 0; i < in.size(); ++i) {
		binds.push_back(winstd::shared_ptr<WmiIisBinding>(new WmiIisBinding(conn(), in[i].first, in[i].second)));
	}
	SafeArray arr(VT_UNKNOWN, binds.size());
	for (size_t i = 0; i < arr.size(); ++i) {
		arr.at<IWbemClassObject*>(i) = *binds[i];
	}
	Variant var;
	var.parray = arr;
	var.vt = VT_ARRAY | VT_UNKNOWN;
	m_obj.Put(L"Bindings", var);
	Save();
}

void WmiIisSite::add_binding(const ustring &ip, const ustring &port, const ustring &name, const ustring &prot) {
	WmiIisBindings binds(bindings());
	binds.add(ip, port, name, prot);
	bindings(binds);
}

void WmiIisSite::del_binding(const ustring &ip, const ustring &port, const ustring &name, const ustring &prot) {
	WmiIisBindings binds(bindings());
	binds.del(ip, port, name, prot);
	bindings(binds);
}

WmiObject WmiIisSite::log() const {
	Variant val(get_param(L"LogFile"));
	WmiObject ret((IWbemClassObject*)val.ppunkVal);
	ret->AddRef();
	return ret;
}

void WmiIisSite::log(const WmiIisSiteLog &in) {
	m_obj.Put(L"LogFile", Variant((IUnknown*)in));
	Save();
}

WmiObject WmiIisSite::get_section(PCWSTR name) const {
	WmiObject in_params(WmiObject::get_in_params(conn().get_object_class(m_obj), L"GetSection"));
	in_params.Put(L"SectionName", name);
	WmiObject out_params(exec_method(L"GetSection", in_params));
	Variant val;
	CheckWmi(out_params->Get(L"Section", 0, &val, 0, 0));
	return WmiObject(val);
}

bool WmiIisSite::is_enabled() const {
	return get_param(L"ServerAutoStart").as_bool();
}

void WmiIisSite::enable() {
	m_obj.Put(L"ServerAutoStart", true);
	Save();
}

void WmiIisSite::disable() {
	m_obj.Put(L"ServerAutoStart", false);
	Save();
}

void WmiIisSite::start() {
	exec_method(L"Start");
}

void WmiIisSite::stop() {
	exec_method(L"Stop");
}

BStr WmiIisSite::Path(PCWSTR name) const {
	WCHAR	path[MAX_PATH];
	::_snwprintf(path, sizeofa(path), L"Site.Name=\"%s\"", name);
	return BStr(path);
}
