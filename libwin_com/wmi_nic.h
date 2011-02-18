#ifndef _WIN_WMI_NIC_H_
#define _WIN_WMI_NIC_H_

#include "wmi.h"

///=============================================================================== WmiNetworkAdapter
class WmiNetworkAdapter: public WmiBase {
public:
	WmiNetworkAdapter(const WmiConnection &conn, DWORD id):
			WmiBase(conn, Path(id)) {
	}

	WmiNetworkAdapter(const WmiConnection &conn, const ComObject<IWbemClassObject> &obj):
			WmiBase(conn, obj) {
	}

	template<typename Functor>
	bool exec(Functor Func, PCVOID data = nullptr) const {
		return	Func(*this, data);
	}

	void Disable() const;

	void Enable() const;

private:
	BStr Path(DWORD id) const;
};

///=========================================================================== WmiNetworkAdapterConf
class WmiNetworkAdapterConf: public WmiBase {
public:
	WmiNetworkAdapterConf(const WmiConnection &conn, DWORD index):
			WmiBase(conn, Path(index)) {
	}

	WmiNetworkAdapterConf(const WmiConnection &conn, const ComObject<IWbemClassObject> &obj):
			WmiBase(conn, obj) {
	}

	template<typename Functor>
	bool exec(Functor Func, PCVOID data = nullptr) const {
		return	Func(*this, data);
	}

	size_t EnableDHCP() const;

	size_t EnableStatic(const Variant &ip, const Variant &mask) const;

	size_t SetGateways(const Variant &ip) const;

	size_t SetDNSServerSearchOrder(const Variant &ip) const;

	size_t ReleaseDHCPLease() const;

	size_t RenewDHCPLease() const;

private:
	BStr Path(DWORD index) const;
};

#endif
