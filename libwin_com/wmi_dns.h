#ifndef _WIN_WMI_DNS_H_
#define _WIN_WMI_DNS_H_

#include "wmi.h"

#include <vector>

/// Управление Microsoft DNS server через WMI

///==================================================================================== WmiDnsServer
class WmiDnsServer: public WmiBase {
public:
	WmiDnsServer(const WmiConnection &conn, PCWSTR name = L"."):
		WmiBase(conn, Path(name)) {
	}

	WmiDnsServer(const WmiConnection &conn, const ComObject<IWbemClassObject> &obj):
		WmiBase(conn, obj) {
	}

	ComObject<IWbemClassObject> CreateZone(const AutoUTF &zone, DWORD type = 0,
	                                       const std::vector<AutoUTF> &ip = std::vector<AutoUTF>(),
	                                       const AutoUTF &email = AutoUTF(), bool integr = false) const;

	AutoUTF name() const;

private:
	BStr Path(PCWSTR name) const;
};

///====================================================================================== WmiDnsZone
class WmiDnsBase: public WmiBase {
public:
	WmiDnsBase(const WmiConnection &conn, const BStr &path):
		WmiBase(conn, path) {
	}

	WmiDnsBase(const WmiConnection &conn, const ComObject<IWbemClassObject> &obj):
		WmiBase(conn, obj) {
	}

	AutoUTF server() const;

	AutoUTF name() const;

	AutoUTF container() const;
};

///====================================================================================== WmiDnsZone
class WmiDnsZone: public WmiDnsBase {
public:
	WmiDnsZone(const WmiConnection &conn, PCWSTR srv, PCWSTR name):
		WmiDnsBase(conn, Path(srv, name)) {
	}

	WmiDnsZone(const WmiConnection &conn, const ComObject<IWbemClassObject> &obj):
		WmiDnsBase(conn, obj) {
	}

	void Save() const;

	AutoUTF file() const;

	void CreateRecord(const AutoUTF &txt);

	void CreateRecordA(const AutoUTF &name, const AutoUTF &ip);

	void CreateRecordAAAA(const AutoUTF &name, const AutoUTF &ip);

	void CreateRecordCNAME(const AutoUTF &name, const AutoUTF &prim);

	void CreateRecordMX(const AutoUTF &name, size_t pri, const AutoUTF &exchange);

	void CreateRecordNS(const AutoUTF &name, const AutoUTF &host);

	void CreateRecordPTR(const AutoUTF &name, const AutoUTF &dom);

	void CreateRecordSRV(const AutoUTF &name, size_t prio, size_t weight, size_t port, const AutoUTF &dom);

	void CreateRecordTXT(const AutoUTF &name, const AutoUTF &txt);

private:
	BStr Path(PCWSTR srv, PCWSTR name) const;
};

///==================================================================================== WmiDnsRecord
class WmiDnsRecord: public WmiDnsBase {
public:
	WmiDnsRecord(const WmiConnection &conn, const BStr &path):
		WmiDnsBase(conn, path) {
	}

	WmiDnsRecord(const WmiConnection &conn, PCWSTR srv, PCWSTR zone, PCWSTR text):
		WmiDnsBase(conn, Path(conn, srv, zone, text)) {
	}

	WmiDnsRecord(const WmiConnection &conn, const ComObject<IWbemClassObject> &obj):
		WmiDnsBase(conn, obj) {
	}

	AutoUTF domain() const;

	AutoUTF name() const;

	AutoUTF data() const;

	AutoUTF text() const;

	AutoUTF type() const;

	int ttl() const;
private:
	BStr Path(const WmiConnection &conn, PCWSTR srv, PCWSTR zone, PCWSTR text) const;
};

///=================================================================================== WmiDnsRecordA
class WmiDnsRecordA: public WmiDnsRecord {
public:
	WmiDnsRecordA(const WmiConnection &conn, PCWSTR srv, PCWSTR zone, PCWSTR name):
		WmiDnsRecord(conn, Path(srv, zone, name)) {
	}

	WmiDnsRecordA(const WmiConnection &conn, const ComObject<IWbemClassObject> &obj):
		WmiDnsRecord(conn, obj) {
	}

	AutoUTF ip() const;

	void Modify(const AutoUTF &ip);

private:
	BStr Path(PCWSTR srv, PCWSTR zone, PCWSTR name) const;
};

///================================================================================ WmiDnsRecordAAAA
class WmiDnsRecordAAAA: public WmiDnsRecord {
public:
	WmiDnsRecordAAAA(const WmiConnection &conn, PCWSTR srv, PCWSTR zone, PCWSTR name):
		WmiDnsRecord(conn, Path(srv, zone, name)) {
	}

	WmiDnsRecordAAAA(const WmiConnection &conn, const ComObject<IWbemClassObject> &obj):
		WmiDnsRecord(conn, obj) {
	}

	AutoUTF ip() const;

	void Modify(const AutoUTF &ip);

private:
	BStr Path(PCWSTR srv, PCWSTR zone, PCWSTR name) const;
};

///================================================================================== WmiDnsRecordNS
class WmiDnsRecordNS: public WmiDnsRecord {
public:
	WmiDnsRecordNS(const WmiConnection &conn, PCWSTR name):
		WmiDnsRecord(conn, Path(name)) {
	}

	WmiDnsRecordNS(const WmiConnection &conn, const ComObject<IWbemClassObject> &obj):
		WmiDnsRecord(conn, obj) {
	}

	AutoUTF host() const;

private:
	BStr Path(PCWSTR name) const;
};

///================================================================================== WmiDnsRecordMX
class WmiDnsRecordMX: public WmiDnsRecord {
public:
	WmiDnsRecordMX(const WmiConnection &conn, PCWSTR name):
		WmiDnsRecord(conn, Path(name)) {
	}

	WmiDnsRecordMX(const WmiConnection &conn, const ComObject<IWbemClassObject> &obj):
		WmiDnsRecord(conn, obj) {
	}

	AutoUTF addr() const;

	int priority() const;

private:
	BStr Path(PCWSTR name) const;
};

///================================================================================= WmiDnsRecordSRV
class WmiDnsRecordSRV: public WmiDnsRecord {
public:
	WmiDnsRecordSRV(const WmiConnection &conn, PCWSTR srv, PCWSTR zone, PCWSTR name):
		WmiDnsRecord(conn, Path(srv, zone, name)) {
	}

	WmiDnsRecordSRV(const WmiConnection &conn, const ComObject<IWbemClassObject> &obj):
		WmiDnsRecord(conn, obj) {
	}

	AutoUTF addr() const;

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
