#ifndef _LIBCOM_DNS_HPP_
#define _LIBCOM_DNS_HPP_

#include <libcom/wmi.hpp>

#include <vector>

/// Управление Microsoft DNS server через WMI

///==================================================================================== WmiDnsServer
class WmiDnsServer: public WmiBase {
public:
	WmiDnsServer(const WmiConnection & conn, PCWSTR name = L"."):
		WmiBase(conn, Path(name)) {
	}

	WmiDnsServer(const WmiConnection & conn, const WmiObject &obj):
		WmiBase(conn, obj) {
	}

	WmiObject CreateZone(const ustring & zone, DWORD type = 0,
	                     const std::vector<ustring> & ip = std::vector<ustring>(),
	                     const ustring & email = ustring(), bool integr = false) const;

	ustring name() const;

private:
	BStr Path(PCWSTR name) const;
};

///====================================================================================== WmiDnsZone
class WmiDnsBase: public WmiBase {
public:
	WmiDnsBase(const WmiConnection & conn, const BStr & path):
		WmiBase(conn, path) {
	}

	WmiDnsBase(const WmiConnection & conn, const WmiObject & obj):
		WmiBase(conn, obj) {
	}

	ustring server() const;

	ustring name() const;

	ustring container() const;
};

///====================================================================================== WmiDnsZone
class WmiDnsZone: public WmiDnsBase {
public:
	WmiDnsZone(const WmiConnection & conn, PCWSTR srv, PCWSTR name):
		WmiDnsBase(conn, Path(srv, name)) {
	}

	WmiDnsZone(const WmiConnection & conn, const WmiObject & obj):
		WmiDnsBase(conn, obj) {
	}

	void Save() const;

	ustring file() const;

	void CreateRecord(const ustring & txt);

	void CreateRecordA(const ustring & name, const ustring & ip);

	void CreateRecordAAAA(const ustring & name, const ustring & ip);

	void CreateRecordCNAME(const ustring & name, const ustring & prim);

	void CreateRecordMX(const ustring & name, size_t pri, const ustring & exchange);

	void CreateRecordNS(const ustring & name, const ustring & host);

	void CreateRecordPTR(const ustring & name, const ustring & dom);

	void CreateRecordSRV(const ustring & name, size_t prio, size_t weight, size_t port, const ustring & dom);

	void CreateRecordTXT(const ustring & name, const ustring & txt);

private:
	BStr Path(PCWSTR srv, PCWSTR name) const;
};

///==================================================================================== WmiDnsRecord
class WmiDnsRecord: public WmiDnsBase {
public:
	WmiDnsRecord(const WmiConnection & conn, const BStr & path):
		WmiDnsBase(conn, path) {
	}

	WmiDnsRecord(const WmiConnection & conn, PCWSTR srv, PCWSTR zone, PCWSTR text):
		WmiDnsBase(conn, Path(conn, srv, zone, text)) {
	}

	WmiDnsRecord(const WmiConnection & conn, const WmiObject & obj):
		WmiDnsBase(conn, obj) {
	}

	ustring domain() const;

	ustring name() const;

	ustring data() const;

	ustring text() const;

	ustring type() const;

	int ttl() const;

private:
	BStr Path(const WmiConnection & conn, PCWSTR srv, PCWSTR zone, PCWSTR text) const;
};

///=================================================================================== WmiDnsRecordA
class WmiDnsRecordA: public WmiDnsRecord {
public:
	WmiDnsRecordA(const WmiConnection & conn, PCWSTR srv, PCWSTR zone, PCWSTR name):
		WmiDnsRecord(conn, Path(srv, zone, name)) {
	}

	WmiDnsRecordA(const WmiConnection & conn, const WmiObject & obj):
		WmiDnsRecord(conn, obj) {
	}

	ustring ip() const;

	void Modify(const ustring & ip);

private:
	BStr Path(PCWSTR srv, PCWSTR zone, PCWSTR name) const;
};

///================================================================================ WmiDnsRecordAAAA
class WmiDnsRecordAAAA: public WmiDnsRecord {
public:
	WmiDnsRecordAAAA(const WmiConnection & conn, PCWSTR srv, PCWSTR zone, PCWSTR name):
		WmiDnsRecord(conn, Path(srv, zone, name)) {
	}

	WmiDnsRecordAAAA(const WmiConnection & conn, const WmiObject & obj):
		WmiDnsRecord(conn, obj) {
	}

	ustring ip() const;

	void Modify(const ustring & ip);

private:
	BStr Path(PCWSTR srv, PCWSTR zone, PCWSTR name) const;
};

///================================================================================== WmiDnsRecordNS
class WmiDnsRecordNS: public WmiDnsRecord {
public:
	WmiDnsRecordNS(const WmiConnection & conn, PCWSTR name):
		WmiDnsRecord(conn, Path(name)) {
	}

	WmiDnsRecordNS(const WmiConnection & conn, const WmiObject & obj):
		WmiDnsRecord(conn, obj) {
	}

	ustring host() const;

private:
	BStr Path(PCWSTR name) const;
};

///================================================================================== WmiDnsRecordMX
class WmiDnsRecordMX: public WmiDnsRecord {
public:
	WmiDnsRecordMX(const WmiConnection & conn, PCWSTR name):
		WmiDnsRecord(conn, Path(name)) {
	}

	WmiDnsRecordMX(const WmiConnection & conn, const WmiObject & obj):
		WmiDnsRecord(conn, obj) {
	}

	ustring addr() const;

	int priority() const;

private:
	BStr Path(PCWSTR name) const;
};

///================================================================================= WmiDnsRecordSRV
class WmiDnsRecordSRV: public WmiDnsRecord {
public:
	WmiDnsRecordSRV(const WmiConnection & conn, PCWSTR srv, PCWSTR zone, PCWSTR name):
		WmiDnsRecord(conn, Path(srv, zone, name)) {
	}

	WmiDnsRecordSRV(const WmiConnection & conn, const WmiObject & obj):
		WmiDnsRecord(conn, obj) {
	}

	ustring addr() const;

	int priority() const;

	int weight() const;

	int port() const;

	void ModifyPort(size_t in);

	void ModifyPriority(size_t in);

	void ModifyWeight(size_t in);

private:
	BStr Path(PCWSTR srv, PCWSTR zone, PCWSTR name) const;
};

#endif
