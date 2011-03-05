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

///====================================================================================== WmiIisSite
class WmiIisSite: public WmiBase {
public:
	WmiIisSite(const WmiConnection &conn, PCWSTR name):
		WmiBase(conn, Path(name)) {
	}

	WmiIisSite(const WmiConnection &conn, const ComObject<IWbemClassObject> &obj):
		WmiBase(conn, obj) {
	}

	AutoUTF name() const;

	size_t id() const;

	bool is_enabled() const;

	void enable();

	void disable();

private:
	BStr Path(PCWSTR name) const;
};

#endif
