/**
 * win_com
 * @classes		()
 * @author		Copyright © 2011 Andrew Grechkin
 * @link		(ole32, oleaut32, uuid)
**/

#ifndef WIN_COM_HPP
#define WIN_COM_HPP

#include <libwin_def/std.h>
#include <libwin_def/str.h>
//#include <ole2.h>
//#include <wtypes.h>

///========================================================================================== WinCom
/// Класс инициализации COM singleton (объекты создавать запрещено, нужно использовать фукцию init)
struct WinCOM: private Uncopyable {
	~WinCOM();

	static WinCOM &init();

private:
	WinCOM();
};

template <typename Type>
struct CoMem: private Uncopyable {
	~CoMem() {
		clean();
	}

	CoMem():
		m_ptr(nullptr) {
	}

	operator Type() const {
		return m_ptr;
	}

	Type* operator&() {
		clean();
		return &m_ptr;
	}

	Type operator->() const {
		return m_ptr;
	}

	void reserve(size_t size) {
		PVOID tmp(::CoTaskMemRealloc(m_ptr, size));
		if (tmp)
			m_ptr = tmp;
	}

private:
	void clean() {
		if (m_ptr)
			::CoTaskMemFree(m_ptr);
	}

	Type m_ptr;
};

///====================================================================================== UnknownImp
struct UnknownImp: public IUnknown {
	virtual ~UnknownImp();

	UnknownImp();

	virtual ULONG WINAPI AddRef();

	virtual ULONG WINAPI Release();

	virtual HRESULT WINAPI QueryInterface(REFIID riid, void ** ppvObject);

private:
	ULONG m_ref_cnt;
};

///========================================================================================= Variant
class Variant: public VARIANT {
	typedef VARIANT base_type;
	typedef Variant this_type;
	typedef this_type * pointer;

public:
	~Variant();
	Variant();
	Variant(IUnknown * val);
	Variant(PCWSTR val);
	Variant(PCWSTR val[], size_t cnt);
	Variant(size_t val[], size_t cnt, VARTYPE type);
	Variant(const ustring &val);
	Variant(const ustring val[], size_t cnt);
	Variant(bool val);
	Variant(DWORD in);
	Variant(int64_t in);
	Variant(uint64_t in);
	Variant(uint16_t in);

	Variant(const base_type & in);
	const this_type & operator =(const base_type & in);

	bool is_empty() const {
		return vt == VT_EMPTY;
	}
	bool is_null() const {
		return vt == VT_NULL;
	}
	bool is_bool() const {
		return vt == VT_BOOL;
	}
	bool is_int() const {
		return vt == VT_I1 || vt == VT_I2 || vt == VT_I4 || vt == VT_INT || vt == VT_I8;
	}
	bool is_uint() const {
		return vt == VT_UI1 || vt == VT_UI2 || vt == VT_UI4 || vt == VT_UINT || vt == VT_UI8;
	}
	bool is_str() const {
		return vt == VT_BSTR;
	}
	bool is_array() const {
		return (vt & VT_ARRAY) == VT_ARRAY;
	}

	VARTYPE Type() const {
		return vt;
	}

	void change_type(DWORD type, DWORD flag = 0);

	HRESULT change_type_nt(VARTYPE type, DWORD flag = 0) throw();

	bool as_bool() const;
	int64_t as_int() const;
	uint64_t as_uint() const;
	ustring as_str() const;
	ustring as_str();

	pointer ref();

	void swap(this_type & rhs);
};

///===================================================================================== PropVariant
class PropVariant: public PROPVARIANT {
	typedef PROPVARIANT base_type;
	typedef PropVariant this_type;
	typedef PROPVARIANT * pointer;

public:
	~PropVariant();

	PropVariant();
	PropVariant(PCWSTR val);
	PropVariant(const ustring & val);
	PropVariant(bool val);
	PropVariant(uint32_t val);
	PropVariant(uint64_t val);
	PropVariant(const FILETIME & val);

	PropVariant(const base_type & var);
	this_type & operator =(const base_type & rhs);

	this_type & operator =(PCWSTR rhs);
	this_type & operator =(const ustring & rhs);
	this_type & operator =(bool rhs);
	this_type & operator =(uint32_t rhs);
	this_type & operator =(uint64_t rhs);
	this_type & operator =(const FILETIME & rhs);

	bool is_empty() const {
		return vt == VT_EMPTY;
	}
	bool is_null() const {
		return vt == VT_NULL;
	}
	bool is_bool() const {
		return vt == VT_BOOL;
	}
	bool is_int() const {
		return vt == VT_I1 || vt == VT_I2 || vt == VT_I4 || vt == VT_INT || vt == VT_I8;
	}
	bool is_uint() const {
		return vt == VT_UI1 || vt == VT_UI2 || vt == VT_UI4 || vt == VT_UINT || vt == VT_UI8;
	}
	bool is_str() const {
		return vt == VT_BSTR || vt == VT_LPWSTR;
	}
	bool is_time() const {
		return vt == VT_FILETIME;
	}

	size_t get_int_size() const;

	HRESULT as_bool_nt(bool & val) const throw();
	HRESULT as_str_nt(ustring & val) const throw();

	bool as_bool() const;
	FILETIME as_time() const;
	ustring	as_str() const;
	int64_t	as_int() const;
	uint64_t as_uint() const;

	pointer ref();

	void detach(pointer var);

	void swap(this_type & rhs);
};

///======================================================================================= SafeArray
struct SafeArray {
	~SafeArray();

	SafeArray(VARTYPE type, size_t size);
	SafeArray(SAFEARRAY * ptr);
	SafeArray(const Variant & var);

	size_t dims() const;

	size_t size() const;

	template<typename Type>
	Type& at(size_t index) const {
		return *(((Type*)m_ptr->pvData) + index);
	}
	//	size_t array_size(size_t dim = 1) const {
	//		LONG lbound, ubound;
	//		CheckCom(::SafeArrayGetLBound(parray, dim, &lbound));
	//		CheckCom(::SafeArrayGetUBound(parray, dim, &ubound));
	//		return ubound - lbound + 1;
	//	}
	//	void at(size_t index, BSTR &out) const {
	//		LONG lbound;
	//		CheckCom(::SafeArrayGetLBound(parray, 1, &lbound));
	//		lbound += index;
	//		CheckCom(::SafeArrayGetElement(parray, &lbound, &out));
	//	}

	operator SAFEARRAY *() const {
		return m_ptr;
	}

private:
	SAFEARRAY * m_ptr;
};

///============================================================================================ BStr
class BStr {
	typedef BStr this_type;
	typedef this_type * pointer;

public:
	~BStr() {
		clean();
	}

	BStr():
		m_str(nullptr) {
	}
	BStr(PCWSTR val);
	BStr(const ustring & val);
	BStr(const this_type & val);

	this_type & operator =(PCWSTR val);
	this_type & operator =(const ustring & val);
	this_type & operator =(const this_type & val);

	size_t size() const;

	BSTR * operator &();
	operator BSTR() const {
		return m_str;
	}
	operator PCWSTR() const {
		return m_str;
	}
	operator bool() const {
		return m_str;
	}
	PCWSTR c_str() const {
		return m_str;
	}

	void attach(BSTR & str);
	void detach(BSTR & str);
	void swap(this_type & rhs);

private:
	void clean();
	BSTR m_str;
};

///========================================================================================= WinGUID
struct WinGUID: public GUID {
	WinGUID();

	WinGUID(PCWSTR str) {
		init(str);
	}
	WinGUID(const ustring & str) {
		init(str);
	}
	WinGUID(const PropVariant & prop) {
		init(prop);
	}

	void init(PCWSTR str);
	void init(const ustring & str);
	void init(const PropVariant & prop);

	ustring as_str() const {
		return WinGUID::as_str(*this);
	}

	static ustring as_str(const GUID & guid);
};

inline ustring as_str(const GUID &guid) {
	return WinGUID::as_str(guid);
}

///======================================================================================= ComObject
template<typename Interface>
class ComObject {
	typedef ComObject this_type;
	typedef Interface * pointer;

public:
	~ComObject() {
		Release();
	}

	ComObject() :
		m_obj(nullptr) {
	}

	explicit ComObject(const pointer param):
		m_obj(param) { // caller must not Release param
	}

	explicit ComObject(const Variant & param):
		m_obj((pointer)param.ppunkVal) {
		m_obj->AddRef();
	}

	ComObject(const this_type & param):
		m_obj(param.m_obj) {
		if (m_obj) {
			m_obj->AddRef();
		}
	}

	this_type & operator =(const pointer rhs) { // caller must not Release rhs
		if (m_obj != rhs) {
			this_type tmp(rhs);
			swap(tmp);
		}
		return *this;
	}
	this_type & operator =(const this_type & rhs) {
		if (m_obj != rhs.m_obj) {
			this_type tmp(rhs);
			swap(tmp);
		}
		return *this;
	}

	void Release() {
		if (m_obj) {
			m_obj->Release();
			m_obj = nullptr;
		}
	}

	operator bool() const {
		return m_obj;
	}
	operator pointer() const {
		return m_obj;
	}

	pointer * operator &() {
		Release();
		return &m_obj;
	}
	pointer operator ->() const {
		return m_obj;
	}

	bool operator ==(const pointer rhs) const {
		return m_obj == rhs;
	}
	bool operator ==(const this_type & rhs) const {
		return m_obj == rhs.m_obj;
	}
	bool operator !=(const pointer rhs) const {
		return m_obj != rhs;
	}
	bool operator !=(const this_type & rhs) const {
		return m_obj != rhs.m_obj;
	}

	void attach(pointer & param) {
		Release();
		m_obj = param;
		param = nullptr;
	}
	void detach(pointer & param) {
		param = m_obj;
		m_obj = nullptr;
	}

	void swap(this_type & rhs) {
		using std::swap;
		swap(m_obj, rhs.m_obj);
	}

private:
	pointer m_obj;
};

HRESULT ConvertBoolToHRESULT(bool result);

#endif
