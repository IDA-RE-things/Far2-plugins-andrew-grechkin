#include "wmi_dns.h"

///==================================================================================== WmiDnsServer
ComObject<IWbemClassObject> WmiDnsServer::CreateZone(const AutoUTF &zone, DWORD type, std::vector<AutoUTF> ip, const AutoUTF &email, bool integr) {
	ComObject<IWbemClassObject> in_params;
	{
		ComObject<IWbemClassObject> zone_class = m_conn.get_object(L"MicrosoftDNS_Zone");
		CheckWmi(zone_class->GetMethod(L"CreateZone", 0, &in_params, nullptr));
	}
	Variant zname(zone);
	Variant ztype(type);
	Variant zds(integr);

	CheckWmi(in_params->Put(L"ZoneName", 0, &zname, 0));
	CheckWmi(in_params->Put(L"ZoneType", 0, &ztype, 0));
	CheckWmi(in_params->Put(L"DsIntegrated", 0, &zds, 0));
	if (!email.empty()) {
		Variant tmp(email);
		CheckWmi(in_params->Put(L"AdminEmailName", 0, &tmp, 0));
	}
	if (!ip.empty()) {
		Variant tmp(&ip[0], ip.size());
		CheckWmi(in_params->Put(L"IpAddr", 0, &tmp, 0));
	}

	ComObject<IWbemClassObject> out_params;
	CheckWmi(m_conn->ExecMethod((BSTR)L"MicrosoftDNS_Zone", (BSTR)L"CreateZone", 0, nullptr, in_params, &out_params, nullptr));

	Variant var;
	CheckWmi(out_params->Get(L"RR", 0, &var, nullptr, nullptr));
	return m_conn.get_object(var.as_str().c_str());
}

AutoUTF WmiDnsServer::name() const {
	return get_param(L"Name").as_str();
}

BStr WmiDnsServer::Path(PCWSTR name) const {
	WCHAR	path[MAX_PATH];
	::_snwprintf(path, sizeofa(path), L"MicrosoftDNS_Server.Name=\"%s\"", name);
	return BStr(path);
}

///====================================================================================== WmiDnsZone
AutoUTF WmiDnsZone::server() const {
	return get_param(L"DnsServerName").as_str();
}

AutoUTF WmiDnsZone::name() const {
	return get_param(L"Name").as_str();
}

AutoUTF WmiDnsZone::file() const {
	return get_param(L"DataFile").as_str();
}

void WmiDnsZone::Create(const AutoUTF &txt) {
	ComObject<IWbemClassObject> in_params;
	{
		ComObject<IWbemClassObject> record = m_conn.get_object(L"MicrosoftDNS_ResourceRecord");
		CheckWmi(record->GetMethod(L"CreateInstanceFromTextRepresentation", 0, &in_params, nullptr));
	}
	Variant rsrv(get_server(m_obj));
	CheckWmi(in_params->Put(L"DnsServerName", 0, &rsrv, 0));

	Variant rdom(get_param(L"ContainerName"));
	CheckWmi(in_params->Put(L"ContainerName", 0, &rdom, 0));

	Variant rtxt(txt);
	CheckWmi(in_params->Put(L"TextRepresentation", 0, &rtxt, 0));

	CheckWmi(m_conn->ExecMethod((BSTR)L"MicrosoftDNS_ResourceRecord", (BSTR)L"CreateInstanceFromTextRepresentation", 0, nullptr, in_params, nullptr, nullptr));
}

void WmiDnsZone::CreateA(const AutoUTF &name, const AutoUTF &ip) {
	ComObject<IWbemClassObject> in_params;
	{
		ComObject<IWbemClassObject> record = m_conn.get_object(L"MicrosoftDNS_AType");
		CheckWmi(record->GetMethod(L"CreateInstanceFromPropertyData", 0, &in_params, nullptr));
	}
	Variant rsrv(get_server(m_obj));
	CheckWmi(in_params->Put(L"DnsServerName", 0, &rsrv, 0));

	Variant rdom(get_param(L"ContainerName"));
	CheckWmi(in_params->Put(L"ContainerName", 0, &rdom, 0));

	Variant rname(name);
	CheckWmi(in_params->Put(L"OwnerName", 0, &rname, 0));

	Variant rip(ip);
	CheckWmi(in_params->Put(L"IPAddress", 0, &rip, 0));

	CheckWmi(m_conn->ExecMethod((BSTR)L"MicrosoftDNS_AType", (BSTR)L"CreateInstanceFromPropertyData", 0, nullptr, in_params, nullptr, nullptr));
}

void WmiDnsZone::CreateCNAME(const AutoUTF &name, const AutoUTF &prim) {
	ComObject<IWbemClassObject> in_params;
	{
		ComObject<IWbemClassObject> record = m_conn.get_object(L"MicrosoftDNS_CNAMEType");
		CheckWmi(record->GetMethod(L"CreateInstanceFromPropertyData", 0, &in_params, nullptr));
	}
	Variant rsrv(get_server(m_obj));
	CheckWmi(in_params->Put(L"DnsServerName", 0, &rsrv, 0));

	Variant rdom(get_param(L"ContainerName"));
	CheckWmi(in_params->Put(L"ContainerName", 0, &rdom, 0));

	Variant rname(name);
	CheckWmi(in_params->Put(L"OwnerName", 0, &rname, 0));

	Variant rpn(prim);
	CheckWmi(in_params->Put(L"PrimaryName", 0, &rpn, 0));

	CheckWmi(m_conn->ExecMethod((BSTR)L"MicrosoftDNS_CNAMEType", (BSTR)L"CreateInstanceFromPropertyData", 0, nullptr, in_params, nullptr, nullptr));
}

void WmiDnsZone::CreateMX(const AutoUTF &name, size_t pri, const AutoUTF &exchange) {
	ComObject<IWbemClassObject> in_params;
	{
		ComObject<IWbemClassObject> record = m_conn.get_object(L"MicrosoftDNS_MXType");
		CheckWmi(record->GetMethod(L"CreateInstanceFromPropertyData", 0, &in_params, nullptr));
	}
	Variant rsrv(get_server(m_obj));
	CheckWmi(in_params->Put(L"DnsServerName", 0, &rsrv, 0));

	Variant rdom(get_param(L"ContainerName"));
	CheckWmi(in_params->Put(L"ContainerName", 0, &rdom, 0));

	Variant rname(name);
	CheckWmi(in_params->Put(L"OwnerName", 0, &rname, 0));

	Variant rpri((uint16_t)pri);
	CheckWmi(in_params->Put(L"Preference", 0, &rpri, 0));

	Variant rexc(exchange);
	CheckWmi(in_params->Put(L"MailExchange", 0, &rexc, 0));

	CheckWmi(m_conn->ExecMethod((BSTR)L"MicrosoftDNS_MXType", (BSTR)L"CreateInstanceFromPropertyData", 0, nullptr, in_params, nullptr, nullptr));
}

void WmiDnsZone::CreateNS(const AutoUTF &name, const AutoUTF &host) {
	ComObject<IWbemClassObject> in_params;
	{
		ComObject<IWbemClassObject> record = m_conn.get_object(L"MicrosoftDNS_NSType");
		CheckWmi(record->GetMethod(L"CreateInstanceFromPropertyData", 0, &in_params, nullptr));
	}
	Variant rsrv(get_server(m_obj));
	CheckWmi(in_params->Put(L"DnsServerName", 0, &rsrv, 0));

	Variant rdom(get_param(L"ContainerName"));
	CheckWmi(in_params->Put(L"ContainerName", 0, &rdom, 0));

	Variant rname(name);
	CheckWmi(in_params->Put(L"OwnerName", 0, &rname, 0));

	Variant rhost(host);
	CheckWmi(in_params->Put(L"NSHost", 0, &rhost, 0));

	CheckWmi(m_conn->ExecMethod((BSTR)L"MicrosoftDNS_NSType", (BSTR)L"CreateInstanceFromPropertyData", 0, nullptr, in_params, nullptr, nullptr));
}

void WmiDnsZone::CreatePTR(const AutoUTF &name, const AutoUTF &dom) {
	ComObject<IWbemClassObject> in_params;
	{
		ComObject<IWbemClassObject> record = m_conn.get_object(L"MicrosoftDNS_TXTType");
		CheckWmi(record->GetMethod(L"CreateInstanceFromPropertyData", 0, &in_params, nullptr));
	}
	Variant rsrv(get_server(m_obj));
	CheckWmi(in_params->Put(L"DnsServerName", 0, &rsrv, 0));

	Variant rcont(get_param(L"ContainerName"));
	CheckWmi(in_params->Put(L"ContainerName", 0, &rcont, 0));

	Variant rname(name);
	CheckWmi(in_params->Put(L"OwnerName", 0, &rname, 0));

	Variant rdom(dom);
	CheckWmi(in_params->Put(L"PTRDomainName", 0, &rdom, 0));

	CheckWmi(m_conn->ExecMethod((BSTR)L"MicrosoftDNS_TXTType", (BSTR)L"CreateInstanceFromPropertyData", 0, nullptr, in_params, nullptr, nullptr));
}

void WmiDnsZone::CreateSRV(const AutoUTF &name, size_t prio, size_t weight, size_t port, const AutoUTF &dom) {
	ComObject<IWbemClassObject> in_params;
	{
		ComObject<IWbemClassObject> record = m_conn.get_object(L"MicrosoftDNS_SRVType");
		CheckWmi(record->GetMethod(L"CreateInstanceFromPropertyData", 0, &in_params, nullptr));
	}
	Variant rsrv(get_server(m_obj));
	CheckWmi(in_params->Put(L"DnsServerName", 0, &rsrv, 0));

	Variant rcont(get_param(L"ContainerName"));
	CheckWmi(in_params->Put(L"ContainerName", 0, &rcont, 0));

	Variant rname(name);
	CheckWmi(in_params->Put(L"OwnerName", 0, &rname, 0));

	Variant rprio((uint16_t)prio);
	CheckWmi(in_params->Put(L"Priority", 0, &rprio, 0));

	Variant rweight((uint16_t)weight);
	CheckWmi(in_params->Put(L"Weight", 0, &rweight, 0));

	Variant rport((uint16_t)port);
	CheckWmi(in_params->Put(L"Port", 0, &rport, 0));

	Variant rdom(dom);
	CheckWmi(in_params->Put(L"DomainName", 0, &rdom, 0));

	CheckWmi(m_conn->ExecMethod((BSTR)L"MicrosoftDNS_SRVType", (BSTR)L"CreateInstanceFromPropertyData", 0, nullptr, in_params, nullptr, nullptr));
}

void WmiDnsZone::CreateTXT(const AutoUTF &name, const AutoUTF &txt) {
	ComObject<IWbemClassObject> in_params;
	{
		ComObject<IWbemClassObject> record = m_conn.get_object(L"MicrosoftDNS_TXTType");
		CheckWmi(record->GetMethod(L"CreateInstanceFromPropertyData", 0, &in_params, nullptr));
	}
	Variant rsrv(get_server(m_obj));
	CheckWmi(in_params->Put(L"DnsServerName", 0, &rsrv, 0));

	Variant rdom(get_param(L"ContainerName"));
	CheckWmi(in_params->Put(L"ContainerName", 0, &rdom, 0));

	Variant rname(name);
	CheckWmi(in_params->Put(L"OwnerName", 0, &rname, 0));

	Variant rtxt(txt);
	CheckWmi(in_params->Put(L"DescriptiveText", 0, &rtxt, 0));

	CheckWmi(m_conn->ExecMethod((BSTR)L"MicrosoftDNS_TXTType", (BSTR)L"CreateInstanceFromPropertyData", 0, nullptr, in_params, nullptr, nullptr));
}

BStr WmiDnsZone::Path(PCWSTR srv, PCWSTR name) const {
	WCHAR	path[MAX_PATH];
	::_snwprintf(path, sizeofa(path), L"MicrosoftDNS_Zone.DnsServerName=\"%s\",ContainerName=\"%s\",Name=\"%s\"", srv, name, name);
	return BStr(path);
}

///==================================================================================== WmiDnsRecord
AutoUTF WmiDnsRecord::domain() const {
	return get_param(L"DomainName").as_str();
}

AutoUTF WmiDnsRecord::name() const {
	return get_param(L"OwnerName").as_str();
}

AutoUTF WmiDnsRecord::text() const {
	return get_param(L"TextRepresentation").as_str();
}

int WmiDnsRecord::ttl() const {
	return get_param(L"TTL").as_uint();
}

///=================================================================================== WmiDnsRecordA
AutoUTF WmiDnsRecordA::ip() const {
	return get_param(L"IPAddress").as_str();
}

void WmiDnsRecordA::Modify(const AutoUTF &ip) {
	ComObject<IWbemClassObject> in_params;
	{
		ComObject<IWbemClassObject> r_class = m_conn.get_object(get_class(m_obj).bstrVal);
		CheckWmi(r_class->GetMethod(L"Modify", 0, &in_params, nullptr));
	}
	Variant rip(ip);
	CheckWmi(in_params->Put(L"IPAddress", 0, &rip, 0));

	ComObject<IWbemClassObject> out_params;
	CheckWmi(m_conn->ExecMethod(get_path(m_obj).bstrVal, (BSTR)L"Modify", 0, nullptr, in_params, &out_params, nullptr));

	Variant var;
	CheckWmi(out_params->Get(L"RR", 0, &var, nullptr, nullptr));
	m_obj = m_conn.get_object(var.as_str().c_str());
}

BStr WmiDnsRecordA::Path(PCWSTR name) const {
	WCHAR	path[MAX_PATH];
	::_snwprintf(path, sizeofa(path), L"MicrosoftDNS_AType.OwnerName='%s'", name);
	return BStr(path);
}

///================================================================================== WmiDnsRecordNS
AutoUTF WmiDnsRecordNS::host() const {
	return get_param(L"NSHost").as_str();
}

BStr WmiDnsRecordNS::Path(PCWSTR name) const {
	WCHAR	path[MAX_PATH];
	::_snwprintf(path, sizeofa(path), L"MicrosoftDNS_NSType.OwnerName='%s'", name);
	return BStr(path);
}

///================================================================================== WmiDnsRecordMX
AutoUTF WmiDnsRecordMX::exchange() const {
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

