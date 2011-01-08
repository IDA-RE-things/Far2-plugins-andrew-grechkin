/**
 * win_com
 * @classes		()
 * @author		Copyright © 2011 Andrew Grechkin
 * @link		(ole32, oleaut32, uuid)
**/

#ifndef WIN_COM_HPP
#define WIN_COM_HPP

#include <win_net.h>

///========================================================================================== WinCom
/// Класс инициализации COM singletone (объекты создавать запрещено, нужно использовать фукцию init)
struct WinCOM: private Uncopyable {
	~WinCOM() {
		::CoUninitialize();
	}

	static WinCOM &init() {
		static WinCOM com;
		return com;
	}

private:
	WinCOM() {
		CheckCom(::CoInitializeEx(nullptr, COINIT_MULTITHREADED));
	}
};

class ComBase: public IUnknown {
protected:
	ULONG ref_cnt;
public:
	ComBase() :
			ref_cnt(0) {
	}
	virtual ~ComBase() {
	}
};

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
	explicit ComObject(pointer param) :
			m_obj(param) {
		if (m_obj)
			m_obj->AddRef();
	}
	ComObject(const class_type &param) :
			m_obj(param.m_obj) {
		if (m_obj)
			m_obj->AddRef();
	}

	void Release() {
		if (m_obj) {
			m_obj->Release();
			m_obj = nullptr;
		}
	}

	operator bool() const {
		return m_obj != nullptr;
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

	class_type& operator=(pointer rhs) {
		class_type tmp(rhs);
		swap(tmp);
		return *this;
	}
	class_type& operator=(const class_type &rhs) {
		if (this != &rhs) {
			class_type tmp(rhs);
			swap(tmp);
		}
		return *this;
	}

	void attach(pointer &param) {
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

///========================================================================================= Variant
struct	Variant: public VARIANT {
	typedef Variant class_type;
	typedef class_type *pointer;

	~Variant() {
		::VariantClear(this);
	}

	Variant() {
		::VariantInit(this);
	}

	Variant(PCWSTR val) {
		::VariantInit(this);
		vt = VT_BSTR;
		bstrVal = ::SysAllocStringLen(val, Len(val));
		if (bstrVal == nullptr) {
			vt = VT_ERROR;
			CheckCom(E_OUTOFMEMORY);
		}
	}

	Variant(const AutoUTF &val) {
		::VariantInit(this);
		vt = VT_BSTR;
		bstrVal = ::SysAllocStringLen(val.c_str(), val.size());
		if (bstrVal == nullptr) {
			vt = VT_ERROR;
			CheckCom(E_OUTOFMEMORY);
		}
	}

	Variant(bool val) {
		::VariantInit(this);
		vt = VT_BOOL;
		boolVal = val ? VARIANT_TRUE : VARIANT_FALSE;
	}

	Variant(DWORD in) {
		::VariantInit(this);
		vt = VT_I4;//VT_UINT;
		lVal = in;
	}
	Variant(int64_t in) {
		::VariantInit(this);
		vt = VT_I8;
		llVal = in;
	}
	Variant(uint64_t in) {
		::VariantInit(this);
		vt = VT_UI8;
		ullVal = in;
	}

	Variant(const Variant &in) {
		::VariantInit(this);
		CheckCom(::VariantCopy(this, (VARIANTARG*)&in));
	}

	const Variant&	operator=(const Variant &in) {
		if (this != &in) {
			::VariantCopy(this, (VARIANTARG*)&in);
		}
		return	*this;
	}

	bool is_empty() const {
		return	vt == VT_EMPTY;
	}
	bool is_null() const {
		return	vt == VT_NULL;
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

	HRESULT Type(DWORD type, DWORD flag = 0) {
		return	::VariantChangeType(this, this, flag, type);
	}

	VARTYPE Type() const {
		return	vt;
	}

	bool	as_bool() const {
		if (vt != VT_BOOL) {
			throw ApiError(E_INVALIDARG, Num2Str(vt), THROW_PLACE);
		}
		return boolVal == VARIANT_TRUE;
	}
	int64_t	as_int() const {
		switch (vt) {
			case VT_I1:
				return cVal;
			case VT_I2:
				return iVal;
			case VT_I4:
				return lVal;
			case VT_INT:
				return intVal;
			case VT_I8:
				return llVal;
			case VT_UI1:
				return bVal;
			case VT_UI2:
				return uiVal;
			case VT_UI4:
				return ulVal;
			case VT_UINT:
				return uintVal;
			case VT_UI8:
				return ullVal;
		}
		throw ApiError(E_INVALIDARG, Num2Str(vt), THROW_PLACE);
		return 0;
	}
	uint64_t as_uint() const {
		return as_int();
	}
	AutoUTF	as_str() const {
		switch (vt) {
			case VT_BSTR:
				return AutoUTF(bstrVal, ::SysStringLen(bstrVal));
		}
		throw ApiError(E_INVALIDARG, Num2Str(vt), THROW_PLACE);
		return AutoUTF();
	}

	operator		VARIANT() const {
		return	*this;
	}
};

class	PropVariant: public PROPVARIANT {
	typedef PropVariant class_type;
	typedef class_type *pointer;
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
	void detach(pointer var) {
		if (var->vt != VT_EMPTY)
			CheckCom(::PropVariantClear(var));
		*var = *this;
		vt = VT_EMPTY;
	}

	PropVariant(const class_type &var) {
		CheckCom(::PropVariantCopy(this, &var));
	}
	PropVariant(PCWSTR val) {
		vt = VT_BSTR;
		bstrVal = ::SysAllocStringLen(val, Len(val));
		if (bstrVal == nullptr) {
			vt = VT_ERROR;
			CheckCom(E_OUTOFMEMORY);
		}
	}
	PropVariant(const AutoUTF &val) {
		vt = VT_BSTR;
		bstrVal = ::SysAllocStringLen(val.c_str(), val.size());
		if (bstrVal == nullptr) {
			vt = VT_ERROR;
			CheckCom(E_OUTOFMEMORY);
		}
	}
	PropVariant(bool val) {
		vt = VT_BOOL;
		boolVal = val ? VARIANT_TRUE : VARIANT_FALSE;
	}
	PropVariant(uint32_t val) {
		vt = VT_UI4;
		ulVal = val;
	}
	PropVariant(uint64_t val) {
		vt = VT_UI8;
		uhVal.QuadPart = val;
	}
	PropVariant(const FILETIME &val) {
		vt = VT_FILETIME;
		filetime = val;
	}

	class_type& operator=(const class_type &rhs) {
		if (this != &rhs) {
			class_type tmp(rhs);
			swap(tmp);
		}
		return *this;
	}
	class_type& operator=(PCWSTR rhs) {
		class_type tmp(rhs);
		swap(tmp);
		return *this;
	}
	class_type& operator=(const AutoUTF &rhs) {
		class_type tmp(rhs);
		swap(tmp);
		return *this;
	}
	PropVariant& operator=(bool rhs) {
		if (vt != VT_BOOL) {
			clear();
			vt = VT_BOOL;
		}
		boolVal = rhs ? VARIANT_TRUE : VARIANT_FALSE;
		return *this;
	}
	PropVariant& operator=(uint32_t rhs) {
		if (vt != VT_UI4) {
			clear();
			vt = VT_UI4;
		}
		ulVal = rhs;
		return *this;
	}
	PropVariant& operator=(uint64_t rhs) {
		if (vt != VT_UI8) {
			clear();
			vt = VT_UI8;
		}
		uhVal.QuadPart = rhs;
		return *this;
	}
	PropVariant& operator=(const FILETIME &rhs) {
		if (vt != VT_FILETIME) {
			clear();
			vt = VT_FILETIME;
		}
		filetime = rhs;
		return *this;
	}

	bool is_empty() const {
		return	vt == VT_EMPTY;
	}

	bool is_null() const {
		return	vt == VT_NULL;
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

	bool is_bool() const {
		return vt == VT_BOOL;
	}

	bool is_time() const {
		return vt == VT_FILETIME;
	}

	size_t get_int_size() const {
		switch (vt) {
			case VT_UI1:
				return sizeof(bVal);
			case VT_UI2:
				return sizeof(uiVal);
			case VT_UI4:
				return sizeof(ulVal);
			case VT_UINT:
				return sizeof(uintVal);
			case VT_UI8:
				return sizeof(uhVal);
			case VT_I1:
				return sizeof(cVal);
			case VT_I2:
				return sizeof(iVal);
			case VT_I4:
				return sizeof(lVal);
			case VT_INT:
				return sizeof(intVal);
			case VT_I8:
				return sizeof(hVal);
			default:
				CheckCom(E_INVALIDARG);
		}
		return 0;
	}

	FILETIME as_time() const {
		if (vt != VT_FILETIME) {
			CheckCom(E_INVALIDARG);
		}
		return filetime;
	}

	bool as_bool() const {
		if (vt != VT_BOOL) {
			CheckCom(E_INVALIDARG);
		}
		return boolVal == VARIANT_TRUE;
	}

	AutoUTF			as_str() const {
		switch (vt) {
			case VT_BSTR:
				return AutoUTF(bstrVal, ::SysStringLen(bstrVal));
			case VT_LPWSTR:
				return AutoUTF(pwszVal);
		}
		throw ApiError(E_INVALIDARG, Num2Str(vt), THROW_PLACE);
		return AutoUTF();
	}

	int64_t			as_int() const {
		switch (vt) {
			case VT_I1:
				return cVal;
			case VT_I2:
				return iVal;
			case VT_I4:
				return lVal;
			case VT_INT:
				return intVal;
			case VT_I8:
				return hVal.QuadPart;
			case VT_UI1:
				return bVal;
			case VT_UI2:
				return uiVal;
			case VT_UI4:
				return ulVal;
			case VT_UINT:
				return uintVal;
			case VT_UI8:
				return uhVal.QuadPart;
		}
		throw ApiError(E_INVALIDARG, Num2Str(vt), THROW_PLACE);
		return 0;
	}

	uint64_t		as_uint() const {
		return as_int();
	}

	void swap(class_type &rhs) {
		using std::swap;
		swap(*this, rhs);
	}
private:
	void clear() {
		if (vt != VT_EMPTY)
			CheckCom(::PropVariantClear(this));
	}
};

class	BStr {
	typedef BStr class_type;
	typedef class_type *pointer;
public:
	~BStr() {
		cleanup();
	}

	BStr() :
		m_str(nullptr) {
	}
	BStr(const class_type &val):
		m_str(nullptr) {
		if (val.m_str) {
			m_str = ::SysAllocStringLen(val.m_str, val.size());
			if (!m_str)
				CheckCom(E_OUTOFMEMORY);
		}
	}
	BStr(PCWSTR val):
		m_str(nullptr) {
		if (val) {
			m_str = ::SysAllocString(val);
			if (!m_str)
				CheckCom(E_OUTOFMEMORY);
		}
	}
	BStr(const AutoUTF& val) {
		m_str = ::SysAllocStringLen(val.c_str(), val.size());
		if (!m_str)
			CheckCom(E_OUTOFMEMORY);
	}

	class_type& operator=(PCWSTR val) {
		if (!::SysReAllocString(&m_str, val))
			CheckCom(E_OUTOFMEMORY);
		return *this;
	}

	class_type& operator=(const AutoUTF& val) {
		if (!::SysReAllocStringLen(&m_str, val.c_str(), val.size()))
			CheckCom(E_OUTOFMEMORY);
		return *this;
	}

	class_type& operator=(const class_type& val) {
		if (this != &val) {
			if (!::SysReAllocStringLen(&m_str, val.m_str, val.size()))
				CheckCom(E_OUTOFMEMORY);
		}
		return *this;
	}

	size_t size() const {
		return ::SysStringLen(m_str);
	}

	BSTR* operator&() {
		cleanup();
		return (BSTR*)&m_str;
	}
	operator BSTR() const {
		return	m_str;
	}
	operator PCWSTR() const {
		return m_str;
	}

	void attach(BSTR &str) {
		m_str = str;
		str = nullptr;
	}
	void detach(BSTR &str) {
		str = m_str;
		m_str = nullptr;
	}
	void swap(class_type &rhs) {
		using std::swap;
		swap(m_str, rhs.m_str);
	}

private:
	void cleanup() {
		if (m_str) {
			::SysFreeString(m_str);
			m_str = nullptr;
		}
	}
	BSTR m_str;
};

struct WinGUID: public GUID {
	WinGUID() {
	}

	WinGUID(PCWSTR str) {
		init(str);
	}

	WinGUID(const AutoUTF &str) {
		init(str);
	}

	WinGUID(const PropVariant &prop) {
		init(prop);
	}

	void init(PCWSTR str) {
		CheckApiError(::CLSIDFromString((PWSTR)str, this));
	}

	void init(const AutoUTF &str) {
		CheckApiError(::CLSIDFromString((PWSTR)str.c_str(), this));
	}

	void init(const PropVariant &prop) {
		if (prop.vt == VT_BSTR) {
			size_t len = ::SysStringByteLen(prop.bstrVal);
			if (len == sizeof(*this)) {
				WinMem::Copy(this, prop.bstrVal, len);
				return;
			}
		}
		CheckApiError(E_FAIL);
	}

	AutoUTF as_str() const {
		return as_str(*this);
	}

	static AutoUTF	generate() {
		GUID guid;
		HRESULT hr = ::CoCreateGuid(&guid);
		if (SUCCEEDED(hr)) {
			WCHAR szGUID[40];
			if (::StringFromGUID2(guid, szGUID, sizeofa(szGUID)))
				return szGUID;
		}
		return AutoUTF();
	}

	static AutoUTF as_str(const GUID &guid) {
		OLECHAR* bstr;
		CheckApiError(::StringFromCLSID(guid, &bstr));
		AutoUTF ret(bstr);
		::CoTaskMemFree(bstr);
		return ret;
	}
};

inline AutoUTF as_str(const GUID &guid) {
	return WinGUID::as_str(guid);
}

#endif // WIN_COM_HPP
