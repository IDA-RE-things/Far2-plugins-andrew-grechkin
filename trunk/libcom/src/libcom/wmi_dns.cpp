#include <libcom/wmi_dns.hpp>

namespace Com {
const ustring& parent() {
	static ustring ret(L"@");
	return ret;
}

///==================================================================================== WmiDnsServer
WmiObject WmiDnsServer::CreateZone(const ustring &zone, DWORD type, const std::vector<ustring> &ip, const ustring &email, bool integr) const {
	WmiObject in_params(conn().get_in_params(L"MicrosoftDNS_Zone", L"CreateZone"));

	WmiObject::put_param(in_params, L"ZoneName", zone);
	WmiObject::put_param(in_params, L"ZoneType", type);
	WmiObject::put_param(in_params, L"DsIntegrated", integr);

	if (!ip.empty()) {
		WmiObject::put_param(in_params, L"IpAddr", Variant(&ip[0], ip.size()));
	}

	if (!email.empty()) {
		WmiObject::put_param(in_params, L"AdminEmailName", email);
	}

	return conn().get_object(conn().exec_method_get_param(L"MicrosoftDNS_Zone", L"CreateZone", in_params, L"RR").bstrVal);
}

ustring WmiDnsServer::name() const {
	return get_param(L"Name").as_str();
}

BStr WmiDnsServer::Path(PCWSTR name) const {
	WCHAR	path[MAX_PATH];
	::_snwprintf(path, sizeofa(path), L"MicrosoftDNS_Server.Name=\"%s\"", name);
	return BStr(path);
}

///====================================================================================== WmiDnsBase
ustring WmiDnsBase::server() const {
	return get_param(L"DnsServerName").as_str();
}

ustring WmiDnsBase::name() const {
	return get_param(L"Name").as_str();
}

ustring WmiDnsBase::container() const {
	return get_param(L"ContainerName").as_str();
}

///====================================================================================== WmiDnsZone
void WmiDnsZone::Save() const {
	exec_method(L"WriteBackZone");
	exec_method(L"ReloadZone");
}

ustring WmiDnsZone::file() const {
	return get_param(L"DataFile").as_str();
}

void WmiDnsZone::CreateRecord(const ustring &txt) {
	WmiObject in_params = conn().get_in_params(L"MicrosoftDNS_ResourceRecord", L"CreateInstanceFromTextRepresentation");

	in_params.Put(L"DnsServerName", WmiObject::get_server(m_obj));
	in_params.Put(L"ContainerName", get_param(L"ContainerName"));
	in_params.Put(L"TextRepresentation", txt);

	conn().exec_method(L"MicrosoftDNS_ResourceRecord", L"CreateInstanceFromTextRepresentation", in_params);
}

void WmiDnsZone::CreateRecordA(const ustring &name, const ustring &ip) {
	WmiObject in_params = conn().get_in_params(L"MicrosoftDNS_AType", L"CreateInstanceFromPropertyData");

	in_params.Put(L"DnsServerName", WmiObject::get_server(m_obj));
	in_params.Put(L"ContainerName", get_param(L"ContainerName"));

	in_params.Put(L"OwnerName", (name.empty() || name == parent()) ? get_param(L"ContainerName").as_str() : name);
	in_params.Put(L"IPAddress", ip);

	conn().exec_method(L"MicrosoftDNS_AType", L"CreateInstanceFromPropertyData", in_params);
}

void WmiDnsZone::CreateRecordAAAA(const ustring &name, const ustring &ip) {
	WmiObject in_params = conn().get_in_params(L"MicrosoftDNS_AAAAType", L"CreateInstanceFromPropertyData");

	in_params.Put(L"DnsServerName", WmiObject::get_server(m_obj));
	in_params.Put(L"ContainerName", get_param(L"ContainerName"));

	in_params.Put(L"OwnerName", (name.empty() || name == parent()) ? get_param(L"ContainerName").as_str() : name);
	in_params.Put(L"IPv6Address", ip);

	conn().exec_method(L"MicrosoftDNS_AAAAType", L"CreateInstanceFromPropertyData", in_params);
}

void WmiDnsZone::CreateRecordCNAME(const ustring &name, const ustring &prim) {
	WmiObject in_params = conn().get_in_params(L"MicrosoftDNS_CNAMEType", L"CreateInstanceFromPropertyData");

	in_params.Put(L"DnsServerName", WmiObject::get_server(m_obj));
	in_params.Put(L"ContainerName", get_param(L"ContainerName"));
	in_params.Put(L"OwnerName", (name.empty() || name == parent()) ? get_param(L"ContainerName").as_str() : name);
	in_params.Put(L"PrimaryName", prim);

	conn().exec_method(L"MicrosoftDNS_CNAMEType", L"CreateInstanceFromPropertyData", in_params);
}

void WmiDnsZone::CreateRecordMX(const ustring &name, size_t pri, const ustring &exchange) {
	WmiObject in_params = conn().get_in_params(L"MicrosoftDNS_MXType", L"CreateInstanceFromPropertyData");

	in_params.Put(L"DnsServerName", WmiObject::get_server(m_obj));
	in_params.Put(L"ContainerName", get_param(L"ContainerName"));
	in_params.Put(L"OwnerName", (name.empty() || name == parent()) ? get_param(L"ContainerName").as_str() : name);
	in_params.Put(L"Preference", (uint16_t)pri);
	in_params.Put(L"MailExchange", exchange);

	conn().exec_method(L"MicrosoftDNS_MXType", L"CreateInstanceFromPropertyData", in_params);
}

void WmiDnsZone::CreateRecordNS(const ustring &name, const ustring &host) {
	WmiObject in_params = conn().get_in_params(L"MicrosoftDNS_NSType", L"CreateInstanceFromPropertyData");

	in_params.Put(L"DnsServerName", WmiObject::get_server(m_obj));
	in_params.Put(L"ContainerName", get_param(L"ContainerName"));
	in_params.Put(L"OwnerName", (name.empty() || name == parent()) ? get_param(L"ContainerName").as_str() : name);
	in_params.Put(L"NSHost", host);

	conn().exec_method(L"MicrosoftDNS_NSType", L"CreateInstanceFromPropertyData", in_params);
}

void WmiDnsZone::CreateRecordPTR(const ustring &name, const ustring &dom) {
	WmiObject in_params = conn().get_in_params(L"MicrosoftDNS_PTRType", L"CreateInstanceFromPropertyData");

	in_params.Put(L"DnsServerName", WmiObject::get_server(m_obj));
	in_params.Put(L"ContainerName", get_param(L"ContainerName"));
	in_params.Put(L"OwnerName", (name.empty() || name == parent()) ? get_param(L"ContainerName").as_str() : name);
	in_params.Put(L"PTRDomainName", dom);

	conn().exec_method(L"MicrosoftDNS_PTRType", L"CreateInstanceFromPropertyData", in_params);
}

void WmiDnsZone::CreateRecordSRV(const ustring &name, size_t prio, size_t weight, size_t port, const ustring &dom) {
	WmiObject in_params = conn().get_in_params(L"MicrosoftDNS_SRVType", L"CreateInstanceFromPropertyData");

	in_params.Put(L"DnsServerName", WmiObject::get_server(m_obj));
	in_params.Put(L"ContainerName", get_param(L"ContainerName"));
	in_params.Put(L"OwnerName", (name.empty() || name == parent()) ? get_param(L"ContainerName").as_str() : name);
	in_params.Put(L"Priority", (uint16_t)prio);
	in_params.Put(L"Weight", (uint16_t)weight);
	in_params.Put(L"Port", (uint16_t)port);
	in_params.Put(L"DomainName", dom);

	conn().exec_method(L"MicrosoftDNS_SRVType", L"CreateInstanceFromPropertyData", in_params);
}

void WmiDnsZone::CreateRecordTXT(const ustring &name, const ustring &txt) {
	WmiObject in_params = conn().get_in_params(L"MicrosoftDNS_TXTType", L"CreateInstanceFromPropertyData");

	in_params.Put(L"DnsServerName", WmiObject::get_server(m_obj));
	in_params.Put(L"ContainerName", get_param(L"ContainerName"));
	in_params.Put(L"OwnerName", (name.empty() || name == parent()) ? get_param(L"ContainerName").as_str() : name);
	in_params.Put(L"DescriptiveText", txt);

	conn().exec_method(L"MicrosoftDNS_TXTType", L"CreateInstanceFromPropertyData", in_params);
}

BStr WmiDnsZone::Path(PCWSTR srv, PCWSTR name) const {
	WCHAR	path[MAX_PATH];
	::_snwprintf(path, sizeofa(path), L"MicrosoftDNS_Zone.DnsServerName=\"%s\",ContainerName=\"%s\",Name=\"%s\"", srv, name, name);
	return BStr(path);
}

///==================================================================================== WmiDnsRecord
ustring WmiDnsRecord::domain() const {
	return get_param(L"DomainName").as_str();
}

ustring WmiDnsRecord::name() const {
	return get_param(L"OwnerName").as_str();
}

ustring WmiDnsRecord::data() const {
	return get_param(L"RecordData").as_str();
}

ustring WmiDnsRecord::text() const {
	return get_param(L"TextRepresentation").as_str();
}

ustring WmiDnsRecord::type() const {
	static ustring md(L"MicrosoftDNS_");
	static ustring tp(L"Type");
	ustring cls(WmiObject::get_class(m_obj));
	ustring::size_type pos = cls.find(tp);
	if (pos != ustring::npos)
		cls.erase(pos);
	if (cls.find(md) != ustring::npos)
		return cls.substr(md.size());
	return cls;
}

int WmiDnsRecord::ttl() const {
	return get_param(L"TTL").as_uint();
}

BStr WmiDnsRecord::Path(const WmiConnection &conn, PCWSTR srv, PCWSTR zone, PCWSTR text) const {
	WmiObject in_params = conn.get_in_params(L"MicrosoftDNS_ResourceRecord", L"GetObjectByTextRepresentation");

	in_params.Put(L"DnsServerName", srv);
	in_params.Put(L"ContainerName", zone);
	in_params.Put(L"TextRepresentation", text);

	return BStr(conn.exec_method_get_param(L"MicrosoftDNS_ResourceRecord", L"GetObjectByTextRepresentation", in_params, L"RR").bstrVal);
}

///=================================================================================== WmiDnsRecordA
ustring WmiDnsRecordA::ip() const {
	return get_param(L"IPAddress").as_str();
}

void WmiDnsRecordA::Modify(const ustring &ip) {
	WmiObject in_params(WmiObject::get_in_params(conn().get_object_class(m_obj), L"Modify"));

	in_params.Put(L"IPAddress", ip);

	m_obj = conn().get_object(exec_method_get_param(L"Modify", in_params, L"RR").bstrVal);
}

BStr WmiDnsRecordA::Path(PCWSTR srv, PCWSTR zone, PCWSTR name) const {
	WCHAR	path[MAX_PATH];
	::_snwprintf(path, sizeofa(path), L"MicrosoftDNS_AType.DnsServerName=\"%s\",ContainerName=\"%s\",OwnerName=\"%s\"", srv, zone, name);
	return BStr(path);
}

///================================================================================ WmiDnsRecordAAAA
ustring WmiDnsRecordAAAA::ip() const {
	return get_param(L"IPv6Address").as_str();
}

void WmiDnsRecordAAAA::Modify(const ustring &ip) {
	WmiObject in_params = WmiObject::get_in_params(conn().get_object_class(m_obj), L"Modify");

	in_params.Put(L"IPv6Address", ip);

	m_obj = conn().get_object(exec_method_get_param(L"Modify", in_params, L"RR").bstrVal);
}

BStr WmiDnsRecordAAAA::Path(PCWSTR srv, PCWSTR zone, PCWSTR name) const {
	WCHAR	path[MAX_PATH];
	::_snwprintf(path, sizeofa(path), L"MicrosoftDNS_AAAAType.DnsServerName=\"%s\",ContainerName=\"%s\",OwnerName=\"%s\"", srv, zone, name);
	return BStr(path);
}

///================================================================================== WmiDnsRecordMX
ustring WmiDnsRecordMX::addr() const {
	return get_param(L"MailExchange").as_str();
}

int WmiDnsRecordMX::priority() const {
	return get_param(L"Preference").as_uint();
}

BStr WmiDnsRecordMX::Path(PCWSTR name) const {
	WCHAR	path[MAX_PATH];
	::_snwprintf(path, sizeofa(path), L"MicrosoftDNS_MXType.OwnerName='%s'", name);
	return BStr(path);
}

///================================================================================== WmiDnsRecordNS
ustring WmiDnsRecordNS::host() const {
	return get_param(L"NSHost").as_str();
}

BStr WmiDnsRecordNS::Path(PCWSTR name) const {
	WCHAR	path[MAX_PATH];
	::_snwprintf(path, sizeofa(path), L"MicrosoftDNS_NSType.OwnerName='%s'", name);
	return BStr(path);
}

///================================================================================= WmiDnsRecordSRV
ustring WmiDnsRecordSRV::addr() const {
	return get_param(L"SRVDomainName").as_str();
}

int WmiDnsRecordSRV::priority() const {
	return get_param(L"Priority").as_uint();
}

int WmiDnsRecordSRV::weight() const {
	return get_param(L"Weight").as_uint();
}

int WmiDnsRecordSRV::port() const {
	return get_param(L"Port").as_uint();
}

void WmiDnsRecordSRV::ModifyPort(size_t in) {
	WmiObject in_params(WmiObject::get_in_params(conn().get_object_class(m_obj), L"Modify"));

	in_params.Put(L"Port", (uint16_t)in);

	m_obj = conn().get_object(exec_method_get_param(L"Modify", in_params, L"RR").bstrVal);
}

void WmiDnsRecordSRV::ModifyPriority(size_t in) {
	WmiObject in_params(WmiObject::get_in_params(conn().get_object_class(m_obj), L"Modify"));

	in_params.Put(L"Priority", (uint16_t)in);

	m_obj = conn().get_object(exec_method_get_param(L"Modify", in_params, L"RR").bstrVal);
}

void WmiDnsRecordSRV::ModifyWeight(size_t in) {
	WmiObject in_params = WmiObject::get_in_params(conn().get_object_class(m_obj), L"Modify");

	in_params.Put(L"Weight", (uint16_t)in);

	m_obj = conn().get_object(exec_method_get_param(L"Modify", in_params, L"RR").bstrVal);
}

BStr WmiDnsRecordSRV::Path(PCWSTR srv, PCWSTR zone, PCWSTR name) const {
	WCHAR	path[MAX_PATH];
	::_snwprintf(path, sizeofa(path), L"MicrosoftDNS_SRVType.DnsServerName=\"%s\",ContainerName=\"%s\",OwnerName=\"%s\"", srv, zone, name);
	return BStr(path);
}
}
