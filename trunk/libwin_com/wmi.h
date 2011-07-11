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

class WmiObject: public ComObject<IWbemClassObject> {
public:
	typedef IWbemClassObject value_type;
	typedef IWbemClassObject* pointer;

	static Variant get_param(const value_type* obj, PCWSTR param = L"ReturnValue");
	static AutoUTF get_class(const value_type* obj);
	static AutoUTF get_path(const value_type* obj);
	static AutoUTF get_server(const value_type* obj);

	static WmiObject spawn_instance(const value_type *obj);
	static WmiObject clone(const value_type *obj);
	static WmiObject get_in_params(const value_type *obj, PCWSTR method);
	static void put_param(const value_type *obj, PCWSTR name, const Variant &val);

public:
	WmiObject();
	WmiObject(const pointer p);
	WmiObject(const Variant &param);

	void Put(PCWSTR name, const Variant &val);

	Variant Get(PCWSTR name) const;
	WmiObject SpawnInstance() const;
	WmiObject Clone() const;

private:
};

///========================================================================================= WmiEnum
class WmiEnum: public ComObject<IEnumWbemClassObject> {
public:
	WmiEnum() {
	}
	WmiEnum(const IEnumWbemClassObject *en):
		ComObject<IEnumWbemClassObject>(en),
		m_end(false) {
	}

	operator bool() const {
		return (bool)*this;
	}

	void Begin();

	bool Next();

	bool Next(WmiObject &obj);

	bool End();

	WmiObject Elem() const;

private:
	WmiObject m_element;
	bool m_end;
};

///=================================================================================== WmiConnection
class WmiConnection {
public:
	WmiConnection(PCWSTR srv = nullptr, PCWSTR namesp = L"cimv2", PCWSTR user = nullptr, PCWSTR pass = nullptr);

	operator bool() const {
		return m_svc;
	}

//	IWbemServices*	GetIWbemServices() const {
//		return m_svc;
//	}
//
	IWbemServices*	operator->() const {
		return m_svc;
	}

	WmiEnum	Query(PCWSTR query, ssize_t flags = WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY) const;

	WmiEnum	Enum(PCWSTR path, ssize_t flags = WBEM_FLAG_SHALLOW | WBEM_FLAG_FORWARD_ONLY) const;

	template<typename Functor>
	void	QueryExec(PCWSTR query, Functor Func, PVOID data = nullptr) {
		WmiEnum	ewco(Query(query));

		WmiObject	obj;
		ULONG count = 0;
		while (ewco && SUCCEEDED(ewco->Next(WBEM_INFINITE, 1, &obj, &count)) && count) {
			if (!Func(*this, obj, data))
				break;
		}
	}

	template<typename Functor>
	void	EnumExec(PCWSTR clname, Functor Func, PVOID data = nullptr) {
		WmiEnum	ewco(Enum(clname));

		WmiObject	obj;
		ULONG count = 0;
		while (ewco && SUCCEEDED(ewco->Next(WBEM_INFINITE, 1, &obj, &count)) && count) {
			if (!Func(*this, obj, data))
				break;
		}
	}

	template<typename Functor>
	bool	Exec(PCWSTR clname, Functor &Func, PVOID data = nullptr) {
		WmiObject	obj;
		CheckWmi(m_svc->GetObject((BSTR)clname, WBEM_FLAG_DIRECT_READ, nullptr, &obj, nullptr));
		return Func(obj, data);
	}

	void DeleteInstance(PCWSTR path);

	void DeleteInstance(const WmiObject &obj);

	void CreateInstance(const WmiObject &obj) const;

	void UpdateInstance(const WmiObject &obj) const;

	WmiObject get_object_class(const WmiObject &obj) const;

	WmiObject get_object(PCWSTR path) const;

	WmiObject get_in_params(PCWSTR path, PCWSTR method) const;

	void exec_method(PCWSTR path, PCWSTR method, const WmiObject &in_params) const;

	Variant	exec_method_get_param(PCWSTR path, PCWSTR method, const WmiObject &in_params, PCWSTR ret_par) const;

private:
	ComObject<IWbemServices> m_svc;
};

///========================================================================================= WmiBase
class WmiBase {
public:
	virtual ~WmiBase();

	WmiBase(const WmiConnection &conn, const BStr &path);

	WmiBase(const WmiConnection &conn, const WmiObject &obj);

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
	WmiObject	exec_method(PCWSTR method) const;

	WmiObject	exec_method(PCWSTR method, const WmiObject &in_params) const;

	WmiObject	exec_method(PCWSTR method, PCWSTR param, const Variant &val) const;

	Variant	exec_method_get_param(PCWSTR method, PCWSTR ret_par = L"ReturnValue") const;

	Variant	exec_method_get_param(PCWSTR method, const WmiObject &in_params, PCWSTR ret_par = L"ReturnValue") const;

	Variant	exec_method_get_param(PCWSTR method, PCWSTR param, const Variant &val, PCWSTR ret_par = L"ReturnValue") const;

	void refresh();

	WmiObject m_obj;
	BStr				m_path;
};

///====================================================================================== WMIProcess
class WmiProcess: public WmiBase {
public:
	WmiProcess(const WmiConnection &conn, DWORD id):
		WmiBase(conn, Path(id)) {
	}

	WmiProcess(const WmiConnection &conn, const WmiObject &obj):
		WmiBase(conn, obj) {
	}

	template<typename Functor>
	bool  exec(Functor Func, PCVOID data = nullptr) const {
		return Func(*this, data);
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

	WmiProcessor(const WmiConnection &conn, const WmiObject &obj):
		WmiBase(conn, obj) {
	}

	template<typename Functor>
	bool  exec(Functor Func, PCVOID data = nullptr) const {
		return Func(*this, data);
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

	WmiSystem(const WmiConnection &conn, const WmiObject &obj):
			WmiBase(conn, obj) {
	}

	template<typename Functor>
	bool  exec(Functor Func, PCVOID data = nullptr) const {
		return Func(*this, data);
	}

private:
	BStr Path(PCWSTR name) const;
};

#endif
