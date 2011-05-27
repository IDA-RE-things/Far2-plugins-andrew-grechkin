#include "wmi_nic.h"

///=============================================================================== WmiNetworkAdapter
void WmiNetworkAdapter::Disable() const {
	exec_method(L"Disable");
}

void WmiNetworkAdapter::Enable() const {
	exec_method(L"Enable");
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

size_t WmiNetworkAdapterConf::EnableStatic(const std::vector<AutoUTF> &ip, const std::vector<AutoUTF> &mask) const {
	WmiObject in_params = WmiObject::get_in_params(conn().get_object_class(m_obj), L"EnableStatic");

	in_params.Put(L"IPAddress", Variant(&ip[0], ip.size()));
	in_params.Put(L"SubnetMask", Variant(&mask[0], mask.size()));

	return exec_method_get_param(L"EnableStatic", in_params).as_uint();
}

size_t WmiNetworkAdapterConf::SetGateways(const std::vector<AutoUTF> &ip) const {
	WmiObject in_params(WmiObject::get_in_params(conn().get_object_class(m_obj), L"SetGateways"));

	in_params.Put(L"DefaultIPGateway", Variant(&ip[0], ip.size()));
//	in_params.Put(L"GatewayCostMetric", Variant(&metric[0], metric.size()));

	return exec_method_get_param(L"SetGateways", in_params).as_uint();
}

size_t WmiNetworkAdapterConf::SetDNSServerSearchOrder(const std::vector<AutoUTF> &ip) const {
	WmiObject in_params(WmiObject::get_in_params(conn().get_object_class(m_obj), L"SetDNSServerSearchOrder"));

	in_params.Put(L"DNSServerSearchOrder", Variant(&ip[0], ip.size()));

	return exec_method_get_param(L"SetDNSServerSearchOrder", in_params).as_uint();
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
