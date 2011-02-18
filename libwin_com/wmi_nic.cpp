#include "wmi_nic.h"

///=============================================================================== WmiNetworkAdapter
void WmiNetworkAdapter::Disable() const {
	WmiBase::exec_method(L"Disable");
}

void WmiNetworkAdapter::Enable() const {
	WmiBase::exec_method(L"Enable");
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

size_t WmiNetworkAdapterConf::EnableStatic(const Variant &ip, const Variant &mask) const {
	ComObject<IWbemClassObject>	obj(m_conn.get_object(get_class(m_obj).bstrVal));

	ComObject<IWbemClassObject>	pInSignature;
	CheckWmi(obj->GetMethod(L"EnableStatic", 0, &pInSignature, nullptr));

	ComObject<IWbemClassObject> pInParams;
	CheckWmi(pInSignature->SpawnInstance(0, &pInParams));

	CheckWmi(pInParams->Put(L"IPAddress", 0, (VARIANT*)&ip, 0));
	CheckWmi(pInParams->Put(L"SubnetMask", 0, (VARIANT*)&mask, 0));

	return exec_method_get_param(L"EnableStatic", L"ReturnValue", pInParams).as_uint();
}

size_t WmiNetworkAdapterConf::SetGateways(const Variant &ip) const {
	ComObject<IWbemClassObject>	obj(m_conn.get_object(get_class(m_obj).bstrVal));

	ComObject<IWbemClassObject>	pInSignature;
	CheckWmi(obj->GetMethod(L"SetGateways", 0, &pInSignature, nullptr));

	ComObject<IWbemClassObject> pInParams;
	CheckWmi(pInSignature->SpawnInstance(0, &pInParams));

	CheckWmi(pInParams->Put(L"DefaultIPGateway", 0, (VARIANT*)&ip, 0));
	//		CheckWmi(pInParams->Put(L"GatewayCostMetric", 0, (VARIANT*)&metric, 0));

	return exec_method_get_param(L"SetGateways", L"ReturnValue", pInParams).as_uint();
}

size_t WmiNetworkAdapterConf::SetDNSServerSearchOrder(const Variant &ip) const {
	ComObject<IWbemClassObject>	obj(m_conn.get_object(get_class(m_obj).bstrVal));

	ComObject<IWbemClassObject>	pInSignature;
	CheckWmi(obj->GetMethod(L"SetDNSServerSearchOrder", 0, &pInSignature, nullptr));

	ComObject<IWbemClassObject> pInParams;
	CheckWmi(pInSignature->SpawnInstance(0, &pInParams));

	CheckWmi(pInParams->Put(L"DNSServerSearchOrder", 0, (VARIANT*)&ip, 0));

	return exec_method_get_param(L"SetDNSServerSearchOrder", L"ReturnValue", pInParams).as_uint();
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
