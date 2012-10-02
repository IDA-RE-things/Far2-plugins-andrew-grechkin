#ifndef _LIBCOM_NIC_HPP_
#define _LIBCOM_NIC_HPP_

#include <libcom/wmi.hpp>

#include <vector>
#include <map>

typedef std::map<ustring, ustring> IpAddresses;

namespace Com {
///=============================================================================== WmiNetworkAdapter
class WmiNetworkAdapter: public WmiBase {
public:
	static WmiEnum Enum(const WmiConnection &conn);

public:
	WmiNetworkAdapter(const WmiConnection &conn, DWORD id):
			WmiBase(conn, Path(id)) {
	}

	WmiNetworkAdapter(const WmiConnection &conn, const WmiObject &obj):
			WmiBase(conn, obj) {
	}

	template<typename Functor>
	bool exec(Functor Func, PCVOID data = nullptr) const {
		return Func(*this, data);
	}

	void Disable() const;

	void Enable() const;

private:
	BStr Path(DWORD id) const;
};

///=========================================================================== WmiNetworkAdapterConf
class WmiNetworkAdapterConf: public WmiBase {
public:
	static WmiEnum Enum(const WmiConnection &conn);

public:
	WmiNetworkAdapterConf(const WmiConnection &conn, DWORD index):
			WmiBase(conn, Path(index)) {
	}

	WmiNetworkAdapterConf(const WmiConnection &conn, const WmiObject &obj):
		WmiBase(conn, obj) {
	}

	template<typename Functor>
	bool exec(Functor Func, PCVOID data = nullptr) const {
		return Func(*this, data);
	}

	size_t InterfaceIndex() const;

	size_t EnableDHCP() const;

	size_t EnableStatic(const std::vector<ustring> &ip, const std::vector<ustring> &mask) const;

	size_t SetGateways(const std::vector<ustring> &ip) const;

	size_t SetDNSServerSearchOrder(const std::vector<ustring> &ip) const;

	size_t ReleaseDHCPLease() const;

	size_t RenewDHCPLease() const;

	IpAddresses GetIP() const;

	int IPv4Add(const ustring & ip);

	int IPv4AddGateway(const ustring & gw, size_t met = 0);

	int IPv4AddDns(const ustring & dns);

	int IPv6Add(const ustring & ip);

	int IPv6AddGateway(const ustring & gw);

	int IPv6AddDns(const ustring & dns);

	int IPv4Del(const ustring & ip);

	int IPv4DelGateway(const ustring & gw);

	int IPv4DelDns(const ustring & dns);

	int IPv6Del(const ustring & ip);

	int IPv6DelGateway(const ustring & gw);

	int IPv6DelDns(const ustring & dns);

	int IPv4DhcpEnable();

	int IPv6DhcpEnable();

private:
	BStr Path(DWORD index) const;
};
}
#endif
