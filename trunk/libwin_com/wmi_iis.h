#ifndef _WIN_WMI_IIS_H_
#define _WIN_WMI_IIS_H_

#include "wmi.h"

#include <vector>

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

///============================================================================== WmiIisProcessModel
class WmiIisProcessModel: public WmiBase {
public:
	WmiIisProcessModel(const WmiConnection &conn, const ComObject<IWbemClassObject> &obj):
		WmiBase(conn, obj) {
	}

	AutoUTF user() const;

	AutoUTF pass() const;

	void user(const AutoUTF &name, const AutoUTF &pass = AutoUTF());
};

///=================================================================================== WmiIisAppPool
class WmiIisAppPool: public WmiBase {
public:
	static void Create(const WmiConnection &conn, const AutoUTF &name, bool enabled = true);

	static WmiEnum Enum(const WmiConnection &conn);

	static WmiEnum EnumLike(const WmiConnection &conn, const AutoUTF &like);

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

	WmiIisProcessModel model() const;

	void model(const WmiIisProcessModel &in);

private:
	BStr Path(PCWSTR name) const;
};

///=================================================================================== WmiIisBinding
class WmiIisBinding: public WmiBase {
public:
	WmiIisBinding(const WmiConnection &conn, const AutoUTF &info, const AutoUTF &prot = AutoUTF(L"http"));

	WmiIisBinding(const WmiConnection &conn, IWbemClassObject* obj):
		WmiBase(conn, (obj->AddRef(), ComObject<IWbemClassObject>(obj))) {
		info();
		update();
	}

	WmiIisBinding(const WmiConnection &conn, const ComObject<IWbemClassObject> &obj):
		WmiBase(conn, obj) {
		info();
		update();
	}

	AutoUTF info() const;

	AutoUTF protocol() const;

	AutoUTF ip() const;

	AutoUTF port() const;

	AutoUTF name() const;

	void info(const AutoUTF &in);

	void protocol(const AutoUTF &in);

	void ip(const AutoUTF &in);

	void port(const AutoUTF &in);

	void name(const AutoUTF &in);

	operator IWbemClassObject*() const {
		return m_obj;
	}

private:
	void update() const;

	mutable AutoUTF m_info;
	mutable AutoUTF m_prot;
	mutable AutoUTF m_ip;
	mutable AutoUTF m_port;
	mutable AutoUTF m_name;
};

///================================================================================== WmiIisBindings
class WmiIisBindings: public std::vector< std::pair<AutoUTF, AutoUTF> > {
public:
	typedef std::vector< std::pair<AutoUTF, AutoUTF> > class_type;
	typedef class_type::value_type value_type;
	typedef class_type::iterator iterator;

public:
	WmiIisBindings() {
	}

	WmiIisBindings(const Variant &var);

	void add(const AutoUTF &info, const AutoUTF &prot = AutoUTF(L"http"));

	void add(const AutoUTF &ip, const AutoUTF &port, const AutoUTF &name, const AutoUTF &prot = AutoUTF(L"http"));

	void del(const AutoUTF &info, const AutoUTF &prot = AutoUTF(L"http"));

	void del(const AutoUTF &ip, const AutoUTF &port, const AutoUTF &name, const AutoUTF &prot = AutoUTF(L"http"));

	WmiIisBindings operator-(const WmiIisBindings &rhs) const;
};

///======================================================================================= WmiIisLog
class WmiIisSiteLog: public WmiBase {
public:
	WmiIisSiteLog(const WmiConnection &conn, const ComObject<IWbemClassObject> &obj):
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

///=========================================================================== WmiSectionInformation
class WmiSectionInformation: public WmiBase {
public:
	WmiSectionInformation(const WmiConnection &conn, const ComObject<IWbemClassObject> &obj):
		WmiBase(conn, obj) {
	}

	AutoUTF override() const;

	void override(bool in);

//	bool is_locked() const;
//
//	void locked(bool in);
};

///=================================================================================== WmiIisSection
class WmiIisSection: public WmiBase {
public:
	WmiIisSection(const WmiConnection &conn, const BStr &path):
		WmiBase(conn, path) {
	}

	WmiIisSection(const WmiConnection &conn, const ComObject<IWbemClassObject> &obj):
		WmiBase(conn, obj) {
	}

	AutoUTF name() const;

	void revert(PCWSTR name);

	ComObject<IWbemClassObject> info() const;

	void info(const WmiSectionInformation &in);
};

///==================================================================================== WmiIisAccess
class WmiIisAccess: public WmiIisSection {
public:
	WmiIisAccess(const WmiConnection &conn, PCWSTR path = nullptr):
		WmiIisSection(conn, Path(path)) {
	}

	WmiIisAccess(const WmiConnection &conn, const ComObject<IWbemClassObject> &obj):
		WmiIisSection(conn, obj) {
	}

	size_t flags() const;

	void flags(DWORD acc);

private:
	BStr Path(PCWSTR path) const;
};

///============================================================================ WmiIisAuthentication
class WmiIisAuthentication: public WmiIisSection {
public:
	WmiIisAuthentication(const WmiConnection &conn, PCWSTR path = nullptr):
		WmiIisSection(conn, Path(path)) {
	}

	WmiIisAuthentication(const WmiConnection &conn, const ComObject<IWbemClassObject> &obj):
		WmiIisSection(conn, obj) {
	}

private:
	BStr Path(PCWSTR path) const;
};

///============================================================================= WmiIisAuthorization
class WmiIisAuthorization: public WmiIisSection {
public:
	WmiIisAuthorization(const WmiConnection &conn, PCWSTR path = nullptr):
		WmiIisSection(conn, Path(path)) {
	}

	WmiIisAuthorization(const WmiConnection &conn, const ComObject<IWbemClassObject> &obj):
		WmiIisSection(conn, obj) {
	}

	Variant rules() const;

private:
	BStr Path(PCWSTR path) const;
};

///=========================================================================== WmiIisDefaultDocument
class WmiIisDefaultDocument: public WmiIisSection {
public:
	WmiIisDefaultDocument(const WmiConnection &conn, PCWSTR path = nullptr):
		WmiIisSection(conn, Path(path)) {
	}

	WmiIisDefaultDocument(const WmiConnection &conn, const ComObject<IWbemClassObject> &obj):
		WmiIisSection(conn, obj) {
	}

	bool list(std::vector<AutoUTF> &out) const;

	bool add(const AutoUTF &in);

	bool del(const AutoUTF &in);

	void clear();

private:
	BStr Path(PCWSTR path) const;
};

///================================================================================== WmiIisHandlers
class WmiIisHandlers: public WmiIisSection {
public:
	WmiIisHandlers(const WmiConnection &conn, PCWSTR path = nullptr):
		WmiIisSection(conn, Path(path)) {
	}

	WmiIisHandlers(const WmiConnection &conn, const ComObject<IWbemClassObject> &obj):
		WmiIisSection(conn, obj) {
	}

	size_t access() const;

	void access(DWORD acc);

private:
	BStr Path(PCWSTR path) const;
};

///============================================================================= WmiIsapiCgiRestrict
class WmiIsapiCgiRestrict: public WmiIisSection {
public:
	WmiIsapiCgiRestrict(const WmiConnection &conn, PCWSTR name = nullptr):
		WmiIisSection(conn, Path(name)) {
	}

	WmiIsapiCgiRestrict(const WmiConnection &conn, const ComObject<IWbemClassObject> &obj):
		WmiIisSection(conn, obj) {
	}

	bool is_not_listed_cgis_allowed() const;

	bool is_not_listed_isapis_allowed() const;

	void not_listed_cgis_allowed(bool in);

	void not_listed_isapis_allowed(bool in);

private:
	BStr Path(PCWSTR name) const;
};

///==================================================================================== WmiIisAccess
class WmiIisLog: public WmiIisSection {
public:
	enum LogMode {
		Site = 0,
		CentralBinary,
		CentralW3C,
	};

public:
	WmiIisLog(const WmiConnection &conn, PCWSTR name = nullptr):
		WmiIisSection(conn, Path(name)) {
	}

	WmiIisLog(const WmiConnection &conn, const ComObject<IWbemClassObject> &obj):
		WmiIisSection(conn, obj) {
	}

	ComObject<IWbemClassObject> CentralW3CLogFile() const;

	LogMode mode() const;

	void mode(LogMode in);

private:
	BStr Path(PCWSTR path) const;
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

	void bindings(const WmiIisBindings &in);

	void add_binding(const AutoUTF &ip, const AutoUTF &port, const AutoUTF &name, const AutoUTF &prot = AutoUTF(L"http"));

	void del_binding(const AutoUTF &ip, const AutoUTF &port, const AutoUTF &name, const AutoUTF &prot = AutoUTF(L"http"));

	ComObject<IWbemClassObject> log() const;

	void log(const WmiIisSiteLog &in);

	ComObject<IWbemClassObject> get_section(PCWSTR name) const;

	bool is_enabled() const;

	void enable();

	void disable();

	void start();

	void stop();

private:
	BStr Path(PCWSTR name) const;
};

#endif
