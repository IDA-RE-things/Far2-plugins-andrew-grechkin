#ifndef _WIN_WMI_IIS_H_
#define _WIN_WMI_IIS_H_

#include "wmi.h"

///==================================================================================== WmiIisServer
class WmiIisServer: public WmiBase {
public:
	WmiIisServer(const WmiConnection &conn, PCWSTR name = L"."):
		WmiBase(conn, Path(name)) {
	}

	WmiIisServer(const WmiConnection &conn, const ComObject<IWbemClassObject> &obj):
		WmiBase(conn, obj) {
	}

	AutoUTF name() const;

private:
	BStr Path(PCWSTR name) const;
};

///==================================================================================== WmiIisAppDom
class WmiIisAppDom: public WmiBase {
public:
	WmiIisAppDom(const WmiConnection &conn, PCWSTR name):
		WmiBase(conn, Path(name)) {
	}

	WmiIisAppDom(const WmiConnection &conn, const ComObject<IWbemClassObject> &obj):
		WmiBase(conn, obj) {
	}

	AutoUTF name() const;

	size_t id() const;

	void unload();

private:
	BStr Path(PCWSTR name) const;
};

///=================================================================================== WmiIisAppPool
class WmiIisAppPool: public WmiBase {
public:
	static void Create(const WmiConnection &conn, const AutoUTF &name, bool enabled = true);

	static WmiEnum Enum(const WmiConnection &conn);

public:
	WmiIisAppPool(const WmiConnection &conn, PCWSTR name):
		WmiBase(conn, Path(name)) {
	}

	WmiIisAppPool(const WmiConnection &conn, const ComObject<IWbemClassObject> &obj):
		WmiBase(conn, obj) {
	}

	AutoUTF name() const;

	AutoUTF id() const;

	bool is_enabled() const;

	bool is_classic() const;

	AutoUTF version() const;

	void enable();

	void disable();

	void classic(bool in);

	void support_x32(bool in);

	void version(PCWSTR in);

private:
	BStr Path(PCWSTR name) const;
};

///=================================================================================== WmiIisBinding
class WmiIisBinding: public WmiBase {
public:
	WmiIisBinding(const WmiConnection &conn, const AutoUTF &info, const AutoUTF &prot = AutoUTF(L"http"));

	WmiIisBinding(const WmiConnection &conn, IWbemClassObject* obj):
		WmiBase(conn, (obj->AddRef(), ComObject<IWbemClassObject>(obj))) {
	}

	WmiIisBinding(const WmiConnection &conn, const ComObject<IWbemClassObject> &obj):
		WmiBase(conn, obj) {
	}

	AutoUTF info() const;

	AutoUTF protocol() const;

	void info(const AutoUTF &in);

	void protocol(const AutoUTF &in);
};

///======================================================================================= WmiIisLog
class WmiIisLog: public WmiBase {
public:
	WmiIisLog(const WmiConnection &conn, const ComObject<IWbemClassObject> &obj):
		WmiBase(conn, obj) {
	}

	AutoUTF directory() const;

	size_t format() const;

	size_t period() const;

	size_t size() const;

	size_t flags() const;

	bool is_rollover() const;

	bool is_enabled() const;

	void directory(const AutoUTF &in);

	void enable();

	void disable();
};

///=============================================================================== WmiIisApplication
class WmiIisApplication: public WmiBase {
public:
	static void Create(const WmiConnection &conn, const AutoUTF &name, const AutoUTF &app_path, const AutoUTF &phis_path);

	static WmiEnum Enum(const WmiConnection &conn);

public:
	WmiIisApplication(const WmiConnection &conn, PCWSTR name, PCWSTR path = L"/"):
		WmiBase(conn, Path(name, path)) {
	}

	WmiIisApplication(const WmiConnection &conn, const ComObject<IWbemClassObject> &obj):
		WmiBase(conn, obj) {
	}

	AutoUTF name() const;

	AutoUTF pool() const;

	AutoUTF path() const;

	AutoUTF protocols() const;

	void pool(const AutoUTF &in);

	void protocols(const AutoUTF &in);

private:
	BStr Path(PCWSTR name, PCWSTR path) const;
};

///=================================================================================== WmiIisVirtDir
class WmiIisVirtDir: public WmiBase {
public:
	static void Create(const WmiConnection &conn, const AutoUTF &name, const AutoUTF &vd_path, const AutoUTF &phis_path, const AutoUTF &app_path = AutoUTF(L"/"));

	static WmiEnum Enum(const WmiConnection &conn);

public:
	WmiIisVirtDir(const WmiConnection &conn, PCWSTR name, PCWSTR path = L"/", PCWSTR apppath = L"/"):
		WmiBase(conn, Path(name, path, apppath)) {
	}

	WmiIisVirtDir(const WmiConnection &conn, const ComObject<IWbemClassObject> &obj):
		WmiBase(conn, obj) {
	}

	AutoUTF name() const;

	AutoUTF directory() const;

	AutoUTF app_path() const;

	AutoUTF path() const;

	void directory(const AutoUTF &in);

private:
	BStr Path(PCWSTR name, PCWSTR path, PCWSTR apppath = L"/") const;
};

///====================================================================================== WmiIisSite
class WmiIisSite: public WmiBase {
public:
	static void Create(const WmiConnection &conn, const AutoUTF &name, const AutoUTF &ip, const AutoUTF &path);

	static WmiEnum Enum(const WmiConnection &conn);

public:
	WmiIisSite(const WmiConnection &conn, PCWSTR name):
		WmiBase(conn, Path(name)) {
	}

	WmiIisSite(const WmiConnection &conn, const ComObject<IWbemClassObject> &obj):
		WmiBase(conn, obj) {
	}

	AutoUTF name() const;

	size_t id() const;

	Variant bindings() const;

	ComObject<IWbemClassObject> log() const;

	bool is_enabled() const;

	void enable();

	void disable();

private:
	BStr Path(PCWSTR name) const;
};

#endif
