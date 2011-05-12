/**
 * wmi
 * @classes		()
 * @author		Copyright © 2011 Andrew Grechkin
 * @link		(ole32, oleaut32, uuid)
**/

#ifndef WIN_COM_WMI_HPP
#define WIN_COM_WMI_HPP

#include "win_com.h"

#include <wbemidl.h>

Variant get_param(IWbemClassObject* obj, PCWSTR param = L"ReturnValue");

AutoUTF get_class(const ComObject<IWbemClassObject> &obj);

AutoUTF get_path(const ComObject<IWbemClassObject> &obj);

AutoUTF get_server(const ComObject<IWbemClassObject> &obj);

ComObject<IWbemClassObject> spawn_instance(const ComObject<IWbemClassObject> &obj);

ComObject<IWbemClassObject> clone(const ComObject<IWbemClassObject> &obj);

ComObject<IWbemClassObject>	get_in_params(const ComObject<IWbemClassObject> &obj, PCWSTR method);

void put_param(ComObject<IWbemClassObject> &obj, PCWSTR name, const Variant &val);

///========================================================================================= WmiEnum
class WmiEnum {
public:
	WmiEnum(ComObject<IEnumWbemClassObject> en):
		m_enum(en),
		m_end(false) {
	}

	operator bool() const {
		return (bool)m_enum;
	}

	void Begin();

	bool Next();

	bool Next(ComObject<IWbemClassObject> &obj);

	bool End();

	ComObject<IWbemClassObject> Elem() const;

private:
	ComObject<IEnumWbemClassObject>	m_enum;
	ComObject<IWbemClassObject> m_element;
	bool m_end;
};

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
	void	QueryExec(PCWSTR query, Functor Func, PVOID data = nullptr) {
		ComObject<IEnumWbemClassObject>	ewco = Query(query);

		ComObject<IWbemClassObject>	obj;
		ULONG count = 0;
		while (ewco && SUCCEEDED(ewco->Next(WBEM_INFINITE, 1, &obj, &count)) && count) {
			if (!Func(*this, obj, data))
				break;
		}
	}

	template<typename Functor>
	void	EnumExec(PCWSTR clname, Functor Func, PVOID data = nullptr) {
		ComObject<IEnumWbemClassObject>	ewco = Enum(clname);

		ComObject<IWbemClassObject>	obj;
		ULONG count = 0;
		while (ewco && SUCCEEDED(ewco->Next(WBEM_INFINITE, 1, &obj, &count)) && count) {
			if (!Func(*this, obj, data))
				break;
		}
	}

	template<typename Functor>
	bool	Exec(PCWSTR clname, Functor &Func, PVOID data = nullptr) {
		ComObject<IWbemClassObject>	obj;
		CheckWmi(m_svc->GetObject((BSTR)clname, WBEM_FLAG_DIRECT_READ, nullptr, &obj, nullptr));
		return	Func(obj, data);
	}

	void	del(PCWSTR path);

	void	create(const ComObject<IWbemClassObject> &obj) const;

	void	update(const ComObject<IWbemClassObject> &obj) const;

	ComObject<IWbemClassObject>	get_object_class(const ComObject<IWbemClassObject> &obj) const;

	ComObject<IWbemClassObject>	get_object(PCWSTR path) const;

	ComObject<IWbemClassObject>	get_in_params(PCWSTR path, PCWSTR method) const;

	void	exec_method(PCWSTR path, PCWSTR method, const ComObject<IWbemClassObject> &in_params) const;

	Variant	exec_method_get_param(PCWSTR path, PCWSTR method, const ComObject<IWbemClassObject> &in_params, PCWSTR ret_par) const;

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

	void Delete();

	void Save() const;

	AutoUTF rel_path() const;

	const WmiConnection& conn() const {
		return m_conn;
	}

	operator IUnknown*() const {
		return (IWbemClassObject*)m_obj;
	}

private:
	const WmiConnection &m_conn;

protected:
	ComObject<IWbemClassObject>	exec_method(PCWSTR method) const;

	ComObject<IWbemClassObject>	exec_method(PCWSTR method, const ComObject<IWbemClassObject> &in_params) const;

	ComObject<IWbemClassObject>	exec_method(PCWSTR method, PCWSTR param, const Variant &val) const;

	Variant	exec_method_get_param(PCWSTR method, PCWSTR ret_par = L"ReturnValue") const;

	Variant	exec_method_get_param(PCWSTR method, const ComObject<IWbemClassObject> &in_params, PCWSTR ret_par = L"ReturnValue") const;

	Variant	exec_method_get_param(PCWSTR method, PCWSTR param, const Variant &val, PCWSTR ret_par = L"ReturnValue") const;

	void refresh();

	ComObject<IWbemClassObject> m_obj;
	BStr				m_path;
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
