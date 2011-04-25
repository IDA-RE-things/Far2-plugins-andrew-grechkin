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

WmiEnum WmiIisAppPool::EnumLike(const WmiConnection &conn, const AutoUTF &like) {
	WCHAR query[MAX_PATH];
	::_snwprintf(query, sizeofa(query), L"SELECT * FROM ApplicationPool WHERE Name LIKE \"%s\"", like.c_str());
	return WmiEnum(conn.Query(query));
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
	put_param(m_obj, L"AutoStart", true);
	Save();
}

void WmiIisAppPool::disable() {
	put_param(m_obj, L"AutoStart", false);
	Save();
}

void WmiIisAppPool::classic(bool in) {
	put_param(m_obj, L"ManagedPipelineMode", (DWORD)in);
	Save();
}

void WmiIisAppPool::support_x32(bool in) {
	put_param(m_obj, L"Enable32BitAppOnWin64", in);
	Save();
}

void WmiIisAppPool::version(PCWSTR in) {
	put_param(m_obj, L"ManagedRuntimeVersion", in);
	Save();
}

BStr WmiIisAppPool::Path(PCWSTR name) const {
	WCHAR	path[MAX_PATH];
	::_snwprintf(path, sizeofa(path), L"ApplicationPool.Name=\"%s\"", name);
	return BStr(path);
}

///======================================================================================= WmiIisSiteLog
AutoUTF WmiIisSiteLog::directory() const {
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

void WmiIisSiteLog::directory(const AutoUTF &in) {
	put_param(m_obj, L"Directory", in);
}

WmiIisSiteLog::operator IUnknown*() const {
	return (IWbemClassObject*)m_obj;
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

///================================================================================== WmiIisBindings
WmiIisBindings::WmiIisBindings(const Variant &var) {
	SafeArray<IWbemClassObject*> arr(var);
	for (size_t i = 0; i < arr.size(); ++i) {
		push_back(value_type(::get_param(arr.at(i), L"BindingInformation").as_str(), ::get_param(arr.at(i), L"Protocol").as_str()));
	}
}

void WmiIisBindings::add(const AutoUTF &info, const AutoUTF &prot) {
	push_back(value_type(info, prot));
}

void WmiIisBindings::add(const AutoUTF &ip, const AutoUTF &port, const AutoUTF &name, const AutoUTF &prot) {
	add(ip + L":" + port + L":" + name, prot);
}

void WmiIisBindings::del(const AutoUTF &info, const AutoUTF &prot) {
	iterator it;
	if ((it = std::find(begin(), end(), value_type(info, prot))) != end())
		erase(it);
}

void WmiIisBindings::del(const AutoUTF &ip, const AutoUTF &port, const AutoUTF &name, const AutoUTF &prot) {
	del(ip + L":" + port + L":" + name, prot);
}

WmiIisBindings WmiIisBindings::operator-(const WmiIisBindings &rhs) const {
	WmiIisBindings ret;
	std::set_difference(begin(), end(),rhs.begin(), rhs.end(), std::back_inserter(ret));
	return ret;
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
	Save();
}

void WmiIisApplication::protocols(const AutoUTF &in) {
	put_param(m_obj, L"EnabledProtocols", in);
	Save();
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
	Save();
}

BStr WmiIisVirtDir::Path(PCWSTR name, PCWSTR path, PCWSTR apppath) const {
	WCHAR	tmp[MAX_PATH];
	::_snwprintf(tmp, sizeofa(tmp), L"VirtualDirectory.ApplicationPath=\"%s\",Path=\"%s\",SiteName=\"%s\"", apppath, path, name);
	return BStr(tmp);
}

///=========================================================================== WmiSectionInformation
AutoUTF WmiSectionInformation::override() const {
	return get_param(L"EffectiveOverrideMode").as_str();
}

void WmiSectionInformation::override(bool in) {
	put_param(m_obj, L"OverrideMode", in ? L"Allow" : L"Deny");
}

//bool WmiSectionInformation::is_locked() const {
//	return get_param(L"IsLocked").as_bool();
//}
//
//void WmiSectionInformation::locked(bool in) {
//	put_param(m_obj, L"LockItem", in);
//	conn().update(m_obj);
//}

WmiSectionInformation::operator IUnknown*() const {
	return (IWbemClassObject*)m_obj;
}

///=================================================================================== WmiIisSection
AutoUTF WmiIisSection::name() const {
	return get_param(L"Path").as_str();
}

void WmiIisSection::revert(PCWSTR name) {
	exec_method(L"RevertToParent", L"PropertyName", name);
}

ComObject<IWbemClassObject> WmiIisSection::info() const {
	Variant val(get_param(L"SectionInformation"));
	ComObject<IWbemClassObject> ret((IWbemClassObject*)val.ppunkVal);
	ret->AddRef();
	return ret;
}

void WmiIisSection::info(const WmiSectionInformation &in) {
	put_param(m_obj, L"SectionInformation", Variant((IUnknown*)in));
	Save();
}

///==================================================================================== WmiIisAccess
size_t WmiIisAccess::flags() const {
	return get_param(L"SslFlags").as_uint();
}

void WmiIisAccess::flags(DWORD acc) {
	put_param(m_obj, L"SslFlags", acc);
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
bool WmiIisDefaultDocument::list(std::vector<AutoUTF> &out) const {
	try {
		Variant files(get_param(L"Files"));
		ComObject<IWbemClassObject> ifiles(files.punkVal);
		Variant strings(::get_param(ifiles, L"Files"));
		SafeArray<IUnknown*> arr(strings);
		out.clear();
		for (size_t i = 0; i < arr.size(); ++i) {
			ComObject<IWbemClassObject> str(arr.at(i));
			out.push_back(::get_param(str, L"Value").as_str());
		}
	} catch (...) {
		return false;
	}
	return true;
}

bool WmiIisDefaultDocument::add(const AutoUTF &in) {
	try {
		ComObject<IWbemClassObject> in_params(get_in_params(conn().get_object(get_class(m_obj).c_str()), L"Add"));
		put_param(in_params, L"CollectionName", L"Files.Files");

		ComObject<IWbemClassObject>	elem(conn().get_object(L"StringElement"));
		put_param(elem, L"Value", in);
		put_param(in_params, L"element", Variant((IUnknown*)elem));
		exec_method(L"Add", in_params);
	} catch (...) {
		return false;
	}
	return true;
}

bool WmiIisDefaultDocument::del(const AutoUTF &in) {
	try {
		ComObject<IWbemClassObject> in_params(get_in_params(conn().get_object(get_class(m_obj).c_str()), L"Remove"));
		put_param(in_params, L"CollectionName", L"Files.Files");

		ComObject<IWbemClassObject>	elem(conn().get_object(L"StringElement"));
		put_param(elem, L"Value", in);
		put_param(in_params, L"element", Variant((IUnknown*)elem));
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
	put_param(m_obj, L"AccessPolicy", acc);
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
	put_param(m_obj, L"NotListedCgisAllowed", in);
	Save();
}

void WmiIsapiCgiRestrict::not_listed_isapis_allowed(bool in) {
	put_param(m_obj, L"NotListedIsapisAllowed", in);
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
size_t WmiIisLog::mode() const {
	return get_param(L"CentralLogFileMode").as_uint();
}

void WmiIisLog::mode(DWORD in) {
	put_param(m_obj, L"CentralLogFileMode", in);
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

void WmiIisSite::bindings(const WmiIisBindings &in) {
	std::vector<winstd::shared_ptr<WmiIisBinding> > binds;
	for (size_t i = 0; i < in.size(); ++i) {
		binds.push_back(winstd::shared_ptr<WmiIisBinding>(new WmiIisBinding(conn(), in[i].first, in[i].second)));
	}
	SafeArray<IWbemClassObject*> arr(VT_UNKNOWN, binds.size());
	for (size_t i = 0; i < arr.size(); ++i) {
		arr.at(i) = *binds[i];
	}
	Variant var;
	var.parray = arr;
	var.vt = VT_ARRAY | VT_UNKNOWN;
	put_param(m_obj, L"Bindings", var);
	Save();
}

void WmiIisSite::add_binding(const AutoUTF &ip, const AutoUTF &port, const AutoUTF &name, const AutoUTF &prot) {
	WmiIisBindings binds(bindings());
	binds.add(ip, port, name, prot);
	bindings(binds);
}

void WmiIisSite::del_binding(const AutoUTF &ip, const AutoUTF &port, const AutoUTF &name, const AutoUTF &prot) {
	WmiIisBindings binds(bindings());
	binds.del(ip, port, name, prot);
	bindings(binds);
}

ComObject<IWbemClassObject> WmiIisSite::log() const {
	Variant val(get_param(L"LogFile"));
	ComObject<IWbemClassObject> ret((IWbemClassObject*)val.ppunkVal);
	ret->AddRef();
	return ret;
}

void WmiIisSite::log(const WmiIisSiteLog &in) {
	put_param(m_obj, L"LogFile", Variant((IUnknown*)in));
	Save();
}

ComObject<IWbemClassObject> WmiIisSite::get_section(PCWSTR name) const {
	ComObject<IWbemClassObject> in_params = get_in_params(conn().get_object_class(m_obj), L"GetSection");
	put_param(in_params, L"SectionName", name);
	ComObject<IWbemClassObject> out_params(exec_method(L"GetSection", in_params));
	Variant val;
	CheckWmi(out_params->Get(L"Section", 0, &val, 0, 0));
	return ComObject<IWbemClassObject>(val);
}

bool WmiIisSite::is_enabled() const {
	return get_param(L"ServerAutoStart").as_bool();
}

void WmiIisSite::enable() {
	put_param(m_obj, L"ServerAutoStart", true);
	Save();
}

void WmiIisSite::disable() {
	put_param(m_obj, L"ServerAutoStart", false);
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
