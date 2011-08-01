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

	WmiIisServer(const WmiConnection &conn, const WmiObject &obj):
		WmiBase(conn, obj) {
	}

	ustring name() const;

private:
	BStr Path(PCWSTR name) const;
};

///==================================================================================== WmiIisAppDom
class WmiIisAppDom: public WmiBase {
public:
	WmiIisAppDom(const WmiConnection &conn, PCWSTR name):
		WmiBase(conn, Path(name)) {
	}

	WmiIisAppDom(const WmiConnection &conn, const WmiObject &obj):
		WmiBase(conn, obj) {
	}

	ustring name() const;

	size_t id() const;

	void unload();

private:
	BStr Path(PCWSTR name) const;
};

///============================================================================== WmiIisProcessModel
class WmiIisProcessModel: public WmiBase {
public:
	WmiIisProcessModel(const WmiConnection &conn, const WmiObject &obj):
		WmiBase(conn, obj) {
	}

	ustring user() const;

	ustring pass() const;

	void user(const ustring &name, const ustring &pass = ustring());
};

///=================================================================================== WmiIisAppPool
class WmiIisAppPool: public WmiBase {
public:
	static void Create(const WmiConnection &conn, const ustring &name, bool enabled = true);

	static WmiEnum Enum(const WmiConnection &conn);

	static WmiEnum EnumLike(const WmiConnection &conn, const ustring &like);

public:
	WmiIisAppPool(const WmiConnection &conn, PCWSTR name):
		WmiBase(conn, Path(name)) {
	}

	WmiIisAppPool(const WmiConnection &conn, const WmiObject &obj):
		WmiBase(conn, obj) {
	}

	ustring name() const;

	ustring id() const;

	bool is_enabled() const;

	bool is_classic() const;

	ustring version() const;

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
	WmiIisBinding(const WmiConnection &conn, const ustring &info, const ustring &prot = ustring(L"http"));

	WmiIisBinding(const WmiConnection &conn, IWbemClassObject* obj):
		WmiBase(conn, (obj->AddRef(), WmiObject(obj))) {
		info();
		update();
	}

	WmiIisBinding(const WmiConnection &conn, const WmiObject &obj):
		WmiBase(conn, obj) {
		info();
		update();
	}

	ustring info() const;

	ustring protocol() const;

	ustring ip() const;

	ustring port() const;

	ustring name() const;

	void info(const ustring &in);

	void protocol(const ustring &in);

	void ip(const ustring &in);

	void port(const ustring &in);

	void name(const ustring &in);

	operator IWbemClassObject*() const {
		return m_obj;
	}

private:
	void update() const;

	mutable ustring m_info;
	mutable ustring m_prot;
	mutable ustring m_ip;
	mutable ustring m_port;
	mutable ustring m_name;
};

///================================================================================== WmiIisBindings
class WmiIisBindings: public std::vector< std::pair<ustring, ustring> > {
public:
	typedef std::vector< std::pair<ustring, ustring> > class_type;
	typedef class_type::value_type value_type;
	typedef class_type::iterator iterator;

public:
	WmiIisBindings() {
	}

	WmiIisBindings(const Variant &var);

	void add(const ustring &info, const ustring &prot = ustring(L"http"));

	void add(const ustring &ip, const ustring &port, const ustring &name, const ustring &prot = ustring(L"http"));

	void del(const ustring &info, const ustring &prot = ustring(L"http"));

	void del(const ustring &ip, const ustring &port, const ustring &name, const ustring &prot = ustring(L"http"));

	WmiIisBindings operator-(const WmiIisBindings &rhs) const;
};

///======================================================================================= WmiIisLog
class WmiIisSiteLog: public WmiBase {
public:
	WmiIisSiteLog(const WmiConnection &conn, const WmiObject &obj):
		WmiBase(conn, obj) {
	}

	ustring directory() const;

	size_t format() const;

	size_t period() const;

	size_t size() const;

	size_t flags() const;

	bool is_rollover() const;

	bool is_enabled() const;

	void directory(const ustring &in);

	void enable();

	void disable();
};

///=============================================================================== WmiIisApplication
class WmiIisApplication: public WmiBase {
public:
	static void Create(const WmiConnection &conn, const ustring &name, const ustring &app_path, const ustring &phis_path);

	static WmiEnum Enum(const WmiConnection &conn);

public:
	WmiIisApplication(const WmiConnection &conn, PCWSTR name, PCWSTR path = L"/"):
		WmiBase(conn, Path(name, path)) {
	}

	WmiIisApplication(const WmiConnection &conn, const WmiObject &obj):
		WmiBase(conn, obj) {
	}

	ustring name() const;

	ustring pool() const;

	ustring path() const;

	ustring protocols() const;

	void pool(const ustring &in);

	void protocols(const ustring &in);

private:
	BStr Path(PCWSTR name, PCWSTR path) const;
};

///=================================================================================== WmiIisVirtDir
class WmiIisVirtDir: public WmiBase {
public:
	static void Create(const WmiConnection &conn, const ustring &name, const ustring &vd_path, const ustring &phis_path, const ustring &app_path = ustring(L"/"));

	static WmiEnum Enum(const WmiConnection &conn);

public:
	WmiIisVirtDir(const WmiConnection &conn, PCWSTR name, PCWSTR path = L"/", PCWSTR apppath = L"/"):
		WmiBase(conn, Path(name, path, apppath)) {
	}

	WmiIisVirtDir(const WmiConnection &conn, const WmiObject &obj):
		WmiBase(conn, obj) {
	}

	ustring name() const;

	ustring directory() const;

	ustring app_path() const;

	ustring path() const;

	void directory(const ustring &in);

private:
	BStr Path(PCWSTR name, PCWSTR path, PCWSTR apppath = L"/") const;
};

///=========================================================================== WmiSectionInformation
class WmiSectionInformation: public WmiBase {
public:
	WmiSectionInformation(const WmiConnection &conn, const WmiObject &obj):
		WmiBase(conn, obj) {
	}

	ustring override() const;

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

	WmiIisSection(const WmiConnection &conn, const WmiObject &obj):
		WmiBase(conn, obj) {
	}

	ustring name() const;

	void revert(PCWSTR name);

	WmiObject info() const;

	void info(const WmiSectionInformation &in);
};

///==================================================================================== WmiIisAccess
class WmiIisAccess: public WmiIisSection {
public:
	WmiIisAccess(const WmiConnection &conn, PCWSTR path = nullptr):
		WmiIisSection(conn, Path(path)) {
	}

	WmiIisAccess(const WmiConnection &conn, const WmiObject &obj):
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

	WmiIisAuthentication(const WmiConnection &conn, const WmiObject &obj):
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

	WmiIisAuthorization(const WmiConnection &conn, const WmiObject &obj):
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

	WmiIisDefaultDocument(const WmiConnection &conn, const WmiObject &obj):
		WmiIisSection(conn, obj) {
	}

	bool list(std::vector<ustring> &out) const;

	bool add(const ustring &in);

	bool del(const ustring &in);

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

	WmiIisHandlers(const WmiConnection &conn, const WmiObject &obj):
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

	WmiIsapiCgiRestrict(const WmiConnection &conn, const WmiObject &obj):
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

	WmiIisLog(const WmiConnection &conn, const WmiObject &obj):
		WmiIisSection(conn, obj) {
	}

	WmiObject CentralW3CLogFile() const;

	LogMode mode() const;

	void mode(LogMode in);

private:
	BStr Path(PCWSTR path) const;
};

///====================================================================================== WmiIisSite
class WmiIisSite: public WmiBase {
public:
	static void Create(const WmiConnection &conn, const ustring &name, const ustring &ip, const ustring &path);

	static WmiEnum Enum(const WmiConnection &conn);

public:
	WmiIisSite(const WmiConnection &conn, PCWSTR name):
		WmiBase(conn, Path(name)) {
	}


	WmiIisSite(const WmiConnection &conn, const WmiObject &obj):
		WmiBase(conn, obj) {
	}

	ustring name() const;

	size_t id() const;

	Variant bindings() const;

	void bindings(const WmiIisBindings &in);

	void add_binding(const ustring &ip, const ustring &port, const ustring &name, const ustring &prot = ustring(L"http"));

	void del_binding(const ustring &ip, const ustring &port, const ustring &name, const ustring &prot = ustring(L"http"));

	WmiObject log() const;

	void log(const WmiIisSiteLog &in);

	WmiObject get_section(PCWSTR name) const;

	bool is_enabled() const;

	void enable();

	void disable();

	void start();

	void stop();

private:
	BStr Path(PCWSTR name) const;
};

#endif
