#ifndef _WIN_WMI_DNS_H_
#define _WIN_WMI_DNS_H_
#include "wmi.h"

/// Управление Microsoft DNS server через WMI

///==================================================================================== WmiDnsServer
class WmiDnsServer: public WmiBase {
public:
	WmiDnsServer(const WmiConnection &conn, PCWSTR name):
		WmiBase(conn, Path(name)) {
	}

	WmiDnsServer(const WmiConnection &conn, const ComObject<IWbemClassObject> &obj):
		WmiBase(conn, obj) {
	}

	ComObject<IWbemClassObject> CreateZone(const AutoUTF &zone, DWORD type = 0,
	                                       std::vector<AutoUTF> ip = std::vector<AutoUTF>(),
	                                       const AutoUTF &email = AutoUTF(), bool integr = false);

	AutoUTF name() const;

private:
	BStr Path(PCWSTR name) const;
};

///====================================================================================== WmiDnsZone
class WmiDnsZone: public WmiBase {
public:
	WmiDnsZone(const WmiConnection &conn, PCWSTR srv, PCWSTR name):
		WmiBase(conn, Path(srv, name)) {
	}

	WmiDnsZone(const WmiConnection &conn, const ComObject<IWbemClassObject> &obj):
		WmiBase(conn, obj) {
	}

	AutoUTF server() const;

	AutoUTF name() const;

	AutoUTF file() const;

	void Create(const AutoUTF &txt);

	void CreateA(const AutoUTF &name, const AutoUTF &ip);

	void CreateCNAME(const AutoUTF &name, const AutoUTF &prim);

	void CreateMX(const AutoUTF &name, size_t pri, const AutoUTF &exchange);

	void CreateNS(const AutoUTF &name, const AutoUTF &host);

	void CreatePTR(const AutoUTF &name, const AutoUTF &dom);

	void CreateSRV(const AutoUTF &name, size_t prio, size_t weight, size_t port, const AutoUTF &dom);

	void CreateTXT(const AutoUTF &name, const AutoUTF &txt);

private:
	BStr Path(PCWSTR srv, PCWSTR name) const;
};

///==================================================================================== WmiDnsRecord
class WmiDnsRecord: public WmiBase {
public:
	WmiDnsRecord(const WmiConnection &conn, const BStr &path):
		WmiBase(conn, path) {
	}

	WmiDnsRecord(const WmiConnection &conn, const ComObject<IWbemClassObject> &obj):
		WmiBase(conn, obj) {
	}

	AutoUTF domain() const;

	AutoUTF name() const;

	AutoUTF text() const;

	int ttl() const;
};

///=================================================================================== WmiDnsRecordA
class WmiDnsRecordA: public WmiDnsRecord {
public:
	WmiDnsRecordA(const WmiConnection &conn, PCWSTR name):
		WmiDnsRecord(conn, Path(name)) {
	}

	WmiDnsRecordA(const WmiConnection &conn, const ComObject<IWbemClassObject> &obj):
		WmiDnsRecord(conn, obj) {
	}

	AutoUTF ip() const;

	void Modify(const AutoUTF &ip);

private:
	BStr Path(PCWSTR name) const;
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

	AutoUTF exchange() const;

	int priority() const;

private:
	BStr Path(PCWSTR name) const;
};

#endif
