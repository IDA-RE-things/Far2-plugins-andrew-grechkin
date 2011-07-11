/**
 * win_com
 * @classes		()
 * @author		Copyright © 2011 Andrew Grechkin
 * @link		(ole32, oleaut32, uuid)
**/

#ifndef WIN_COM_HPP
#define WIN_COM_HPP

#include <libwin_def/win_def.h>
#include <libwin_net/exception.h>

///========================================================================================== WinCom
/// Класс инициализации COM singletone (объекты создавать запрещено, нужно использовать фукцию init)
struct WinCOM: private Uncopyable {
	~WinCOM();

	static WinCOM &init();

private:
	WinCOM();
};

class ComBase: public IUnknown {
protected:
	ULONG ref_cnt;
public:
	virtual ~ComBase() {
	}

	ComBase() :
		ref_cnt(1) {
	}
};

///========================================================================================= Variant
struct	Variant: public VARIANT {
	typedef Variant class_type;
	typedef class_type *pointer;

	~Variant();

	Variant();

	Variant(IUnknown* val);

	Variant(PCWSTR val);

	Variant(PCWSTR val[], size_t cnt);

	Variant(size_t val[], size_t cnt, VARTYPE type);

	Variant(const AutoUTF &val);

	Variant(const AutoUTF val[], size_t cnt);

	Variant(bool val);

	Variant(DWORD in);

	Variant(int64_t in);

	Variant(uint64_t in);

	Variant(uint16_t in);

	Variant(const Variant &in);
	const Variant&	operator=(const Variant &in);

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

	void Type(DWORD type, DWORD flag = 0);

	HRESULT try_change_type(DWORD type, DWORD flag = 0) {
		return ::VariantChangeType(this, this, flag, type);
	}

	bool	as_bool() const;
	int64_t	as_int() const;
	uint64_t as_uint() const;
	AutoUTF	as_str() const;
	AutoUTF	as_str();

	operator	VARIANT() const {
		return *this;
	}
};

template<typename Type>
struct SafeArray {
	~SafeArray() {
		::SafeArrayUnlock(m_ptr);
	}

	SafeArray(VARTYPE type, size_t size):
		m_ptr(CheckPointer(::SafeArrayCreateVector(type, 0, size))) {
		::SafeArrayLock(m_ptr);
	}

	SafeArray(SAFEARRAY ptr):
		m_ptr(ptr) {
		::SafeArrayLock(m_ptr);
	}

	SafeArray(const Variant &var):
		m_ptr(var.parray) {
		::SafeArrayLock(m_ptr);
	}

	size_t dims() const {
		return m_ptr->cDims;
	}

	size_t size() const {
		return m_ptr->rgsabound[0].cElements;
	}

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

	operator SAFEARRAY*() const {
		return m_ptr;
	}

private:
	SAFEARRAY *m_ptr;
};

///===================================================================================== PropVariant
class	PropVariant: public PROPVARIANT {
	typedef PropVariant class_type;
	typedef PROPVARIANT *pointer;
public:
	~PropVariant() {
		clear();
	}

	PropVariant() {
		PropVariantInit(this);
	}

	pointer ref() {
		clear();
		return this;
	}

	void detach(pointer var);

	PropVariant(const class_type &var);
	PropVariant(PCWSTR val);
	PropVariant(const AutoUTF &val);
	PropVariant(bool val);
	PropVariant(uint32_t val);
	PropVariant(uint64_t val);
	PropVariant(const FILETIME &val);

	class_type& operator=(const class_type &rhs);
	class_type& operator=(PCWSTR rhs);
	class_type& operator=(const AutoUTF &rhs);
	class_type& operator=(bool rhs);
	class_type& operator=(uint32_t rhs);
	class_type& operator=(uint64_t rhs);
	class_type& operator=(const FILETIME &rhs);

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

	HRESULT as_bool_nt(bool &val) const;
	HRESULT as_str_nt(AutoUTF &val) const;

	bool as_bool() const;
	FILETIME as_time() const;
	AutoUTF	as_str() const;
	int64_t	as_int() const;
	uint64_t as_uint() const;

	void swap(class_type &rhs);

private:
	void clear();
};

///============================================================================================ BStr
class	BStr {
	typedef BStr class_type;
	typedef class_type *pointer;
public:
	~BStr() {
		cleanup();
	}

	BStr():
		m_str(nullptr) {
	}
	BStr(const class_type &val);
	BStr(PCWSTR val);
	BStr(const AutoUTF& val);

	class_type& operator=(PCWSTR val);

	class_type& operator=(const AutoUTF& val);

	class_type& operator=(const class_type& val);

	size_t size() const;

	BSTR* operator&();
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

	void attach(BSTR &str);
	void detach(BSTR &str);
	void swap(class_type &rhs);

private:
	void cleanup();
	BSTR m_str;
};

///========================================================================================= WinGUID
struct WinGUID: public GUID {
	WinGUID();

	WinGUID(PCWSTR str) {
		init(str);
	}

	WinGUID(const AutoUTF &str) {
		init(str);
	}

	WinGUID(const PropVariant &prop) {
		init(prop);
	}

	void init(PCWSTR str);

	void init(const AutoUTF &str);

	void init(const PropVariant &prop);

	AutoUTF as_str() const {
		return WinGUID::as_str(*this);
	}

	static AutoUTF as_str(const GUID &guid);
};

inline AutoUTF as_str(const GUID &guid) {
	return WinGUID::as_str(guid);
}

///======================================================================================= ComObject
template<typename Interface>
class ComObject {
	typedef ComObject class_type;
	typedef Interface* pointer;
public:
	~ComObject() {
		Release();
	}

	ComObject() :
		m_obj(nullptr) {
	}
	explicit ComObject(const pointer param) :
		m_obj(param) { // caller must not Release param
	}
	ComObject(const class_type &param) :
		m_obj(param.m_obj) {
		if (m_obj) {
			m_obj->AddRef();
		}
	}
	explicit ComObject(const Variant &param) :
		m_obj((pointer)param.ppunkVal) {
		m_obj->AddRef();
	}

	class_type& operator=(pointer rhs) { // caller must not Release rhs
		if (m_obj != rhs) {
			class_type tmp(rhs);
			swap(tmp);
		}
		return *this;
	}
	class_type& operator=(const class_type &rhs) {
		if (m_obj != rhs.m_obj) {
			class_type tmp(rhs);
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

	pointer* operator&() {
		Release();
		return &m_obj;
	}
	pointer operator->() const {
		return m_obj;
	}

	bool operator==(pointer rhs) const {
		return m_obj == rhs;
	}
	bool operator==(const class_type &rhs) const {
		return m_obj == rhs.m_obj;
	}
	bool operator!=(pointer rhs) const {
		return m_obj != rhs;
	}
	bool operator!=(const class_type &rhs) const {
		return m_obj != rhs.m_obj;
	}

	void attach(pointer &param) {
		Release();
		m_obj = param;
		param = nullptr;
	}
	void detach(pointer &param) {
		param = m_obj;
		m_obj = nullptr;
	}
	void swap(class_type &rhs) {
		using std::swap;
		swap(m_obj, rhs.m_obj);
	}

private:
	pointer m_obj;
};

HRESULT ConvertBoolToHRESULT(bool result);

#endif
