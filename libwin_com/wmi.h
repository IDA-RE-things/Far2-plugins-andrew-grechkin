/**
 * wmi
 * @classes		()
 * @author		Copyright © 2011 Andrew Grechkin
 * @link		(ole32, oleaut32, uuid)
**/

#ifndef WIN_COM_WMI_HPP
#define WIN_COM_WMI_HPP

#include "win_com.h"
//#include <objbase.h>
#include <wbemidl.h>

#include <vector>

void get_param(Variant &out, const ComObject<IWbemClassObject> &obj, PCWSTR param = L"ReturnValue");

Variant get_class(const ComObject<IWbemClassObject> &obj);

Variant get_path(const ComObject<IWbemClassObject> &obj);

Variant get_server(const ComObject<IWbemClassObject> &obj);

///=================================================================================== WmiConnection
class WmiConnection {
public:
	WmiConnection(PCWSTR srv = nullptr, PCWSTR namesp = L"cimv2");

	WmiConnection(PCWSTR srv, PCWSTR namesp, PCWSTR user, PCWSTR pass);

	operator		bool() const {
		return	m_svc;
	}

	IWbemServices*	GetIWbemServices() const {
		return m_svc;
	}

	IWbemServices*	operator->() const {
		return	m_svc;
	}

	ComObject<IEnumWbemClassObject>	Query(PCWSTR query, ssize_t flags = WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY) const;

	ComObject<IEnumWbemClassObject>	Enum(PCWSTR path, ssize_t flags = WBEM_FLAG_SHALLOW | WBEM_FLAG_FORWARD_ONLY) const;

	template<typename Functor>
	void			QueryExec(PCWSTR query, Functor Func, PVOID data = nullptr) {
		ComObject<IEnumWbemClassObject>	ewco = Query(query);

		ComObject<IWbemClassObject>	obj;
		HRESULT ret = WBEM_S_NO_ERROR;
		ULONG ulCount = 0;
		while (ret == WBEM_S_NO_ERROR) {
			ret = ewco->Next(WBEM_INFINITE, 1, &obj, &ulCount);
			if (!ulCount || !SUCCEEDED(ret) || !Func(*this, obj, data))
				break;
		}
	}

	template<typename Functor>
	void			EnumExec(PCWSTR clname, Functor Func, PVOID data = nullptr) {
		ComObject<IEnumWbemClassObject>	ewco = Enum(clname);

		ComObject<IWbemClassObject>	obj;
		HRESULT ret = WBEM_S_NO_ERROR;
		ULONG ulCount = 0;
		while (ret == WBEM_S_NO_ERROR) {
			ret = ewco->Next(WBEM_INFINITE, 1, &obj, &ulCount);
			if (!ulCount || !SUCCEEDED(ret) || !Func(*this, obj, data))
				break;
		}
	}

	template<typename Functor>
	bool			Exec(PCWSTR clname, Functor &Func, PVOID data = nullptr) {
		ComObject<IWbemClassObject>	obj;
		CheckWmi(m_svc->GetObject((BSTR)clname, WBEM_FLAG_DIRECT_READ, nullptr, &obj, nullptr));
		return	Func(obj, data);
	}

	ComObject<IWbemClassObject>	get_object(PCWSTR path) const;
	ComObject<IWbemClassObject>	get_object(PCWSTR path, PCWSTR method) const;
	ComObject<IWbemClassObject>	get_method(PCWSTR path, PCWSTR method) const;

	Variant		GetParam(PCWSTR path, PCWSTR param) const {
		ComObject<IWbemClassObject> obj = get_object(path);
		Variant	Result;
		CheckWmi(obj->Get(param, 0, &Result, nullptr, nullptr));
		return	Result;
	}

	Variant		GetParam(PCWSTR path, PCWSTR method, PCWSTR param) const {
		ComObject<IWbemClassObject> obj = get_object(path, method);
		Variant	Result;
		CheckWmi(obj->Get(param, 0, &Result, nullptr, nullptr));
		return	Result;
	}

private:
	void Init(PCWSTR srv, PCWSTR namesp, PCWSTR user, PCWSTR pass);

	ComObject<IWbemServices> m_svc;
};

///========================================================================================= WmiBase
class WmiBase {
public:
	virtual ~WmiBase();

	WmiBase(const WmiConnection &conn, const BStr &path);

	WmiBase(const WmiConnection &conn, const ComObject<IWbemClassObject> &obj);

	Variant 	get_param(PCWSTR param) const;

	Variant 	get_param(PCWSTR method, PCWSTR param) const;

	void Delete();

//	template<typename Functor>
//	bool	exec(Functor Func, PVOID data = nullptr) const {
//		return	Func(m_conn, m_obj, data);
//	}

protected:
	void 	exec_method(PCWSTR method) const;

	Variant	exec_method_get_param(PCWSTR method, PCWSTR param, ComObject<IWbemClassObject> in = ComObject<IWbemClassObject>()) const;

	Variant	exec_method_in(PCWSTR method, PCWSTR param = nullptr, DWORD value = 0) const;

	void refresh();

	const WmiConnection &m_conn;
	BStr				m_path;
	ComObject<IWbemClassObject> m_obj;
private:
};

///====================================================================================== WMIProcess
class WmiProcess: public WmiBase {
public:
	WmiProcess(const WmiConnection &conn, DWORD id):
			WmiBase(conn, Path(id)) {
	}

	WmiProcess(const WmiConnection &conn, const ComObject<IWbemClassObject> &obj):
			WmiBase(conn, obj) {
	}

	template<typename Functor>
	bool  exec(Functor Func, PCVOID data = nullptr) const {
		return	Func(*this, data);
	}

	int	attach_debugger() const;

	int	terminate() const;

	int	set_priority(DWORD pri);

	AutoUTF	get_owner() const;

	AutoUTF	get_owner_dom() const;

	AutoUTF	get_owner_sid() const;

private:
	BStr Path(DWORD id) const;
};

///==================================================================================== WmiProcessor
class WmiProcessor: public WmiBase {
public:
	WmiProcessor(const WmiConnection &conn, DWORD id):
			WmiBase(conn, Path(id)) {
	}

	WmiProcessor(const WmiConnection &conn, const ComObject<IWbemClassObject> &obj):
			WmiBase(conn, obj) {
	}

	template<typename Functor>
	bool  exec(Functor Func, PCVOID data = nullptr) const {
		return	Func(*this, data);
	}

private:
	BStr Path(DWORD id) const;
};

///======================================================================================= WmiSystem
class WmiSystem: public WmiBase {
public:
	WmiSystem(const WmiConnection &conn, PCWSTR name):
			WmiBase(conn, Path(name)) {
	}

	WmiSystem(const WmiConnection &conn, const ComObject<IWbemClassObject> &obj):
			WmiBase(conn, obj) {
	}

	template<typename Functor>
	bool  exec(Functor Func, PCVOID data = nullptr) const {
		return	Func(*this, data);
	}

private:
	BStr Path(PCWSTR name) const;
};

#endif
