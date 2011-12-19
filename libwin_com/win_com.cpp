/**
 * win_com
 * @classes		()
 * @author		Copyright © 2011 Andrew Grechkin
 * @link		(ole32, oleaut32, uuid)
**/

#include "win_com.h"
#include <libwin_net/exception.h>

///========================================================================================== WinCom
WinCOM::~WinCOM() {
	::CoUninitialize();
}

WinCOM::WinCOM() {
	CheckCom(::CoInitializeEx(nullptr, COINIT_MULTITHREADED));
	CheckCom(::CoInitializeSecurity(
		nullptr,
		-1,		// COM negotiates service
		nullptr,// Authentication services
		nullptr,// Reserved
		RPC_C_AUTHN_LEVEL_PKT_PRIVACY,// authentication
		RPC_C_IMP_LEVEL_IMPERSONATE,// Impersonation
		nullptr,// Authentication info
		EOAC_STATIC_CLOAKING,// Additional capabilities
		nullptr// Reserved
	));
}

WinCOM& WinCOM::init() {
	static WinCOM com;
	return com;
}

///====================================================================================== UnknownImp
UnknownImp::~UnknownImp() {
}

UnknownImp::UnknownImp() :
	m_ref_cnt(1) {
}

ULONG WINAPI UnknownImp::AddRef() {
	return ++m_ref_cnt;
}

ULONG WINAPI UnknownImp::Release() {
	if (--m_ref_cnt == 0) {
		delete this;
		return 0;
	}
	return m_ref_cnt;
}

HRESULT WINAPI UnknownImp::QueryInterface(REFIID riid, void ** ppvObject) {
	if (riid == IID_IUnknown) {
		*ppvObject = static_cast<IUnknown*>(this);
		AddRef();
		return S_OK;
	}
	*ppvObject = nullptr;
	return E_NOINTERFACE;
}

///========================================================================================= Variant
Variant::~Variant() {
	::VariantClear(this);
}

Variant::Variant() {
	::VariantInit(this);
}

Variant::Variant(IUnknown * val) {
	::VariantInit(this);
	vt = VT_UNKNOWN;
	punkVal = val;
	punkVal->AddRef();
}

Variant::Variant(PCWSTR val) {
	::VariantInit(this);
	bstrVal = ::SysAllocStringLen(val, Len(val));
	if (bstrVal == nullptr) {
		vt = VT_ERROR;
		CheckCom(E_OUTOFMEMORY);
	} else {
		vt = VT_BSTR;
	}
}

Variant::Variant(PCWSTR val[], size_t cnt) {
	parray = CheckPointer(::SafeArrayCreateVector(VT_BSTR, 0, cnt));
	vt = VT_ARRAY | VT_BSTR;
	BSTR *data = (BSTR*)parray->pvData;
	for (size_t i = 0; i < cnt; ++i) {
		data[i] = ::SysAllocString(val[i]);
	}
}

Variant::Variant(size_t val[], size_t cnt, VARTYPE type) {
	parray = CheckPointer(::SafeArrayCreateVector(type, 0, cnt));
	vt = VT_ARRAY | type;
	for (size_t i = 0; i < cnt; ++i) {
		LONG ind[] = {i};
		::SafeArrayPutElement(parray, ind, &val[i]);
	}
}

Variant::Variant(const ustring & val) {
	::VariantInit(this);
	bstrVal = ::SysAllocStringLen(val.c_str(), val.size());
	if (bstrVal == nullptr) {
		vt = VT_ERROR;
		CheckCom(E_OUTOFMEMORY);
	} else {
		vt = VT_BSTR;
	}
}

Variant::Variant(const ustring val[], size_t cnt) {
	parray = CheckPointer(::SafeArrayCreateVector(VT_BSTR, 0, cnt));
	vt = VT_ARRAY | VT_BSTR;
	BSTR *data = (BSTR*)parray->pvData;
	for (size_t i = 0; i < cnt; ++i) {
		data[i] = ::SysAllocStringLen(val[i].c_str(), val[i].size());
	}
}

Variant::Variant(bool val) {
	::VariantInit(this);
	vt = VT_BOOL;
	boolVal = val ? VARIANT_TRUE : VARIANT_FALSE;
}

Variant::Variant(DWORD in) {
	::VariantInit(this);
	vt = VT_I4;//VT_UINT;
	lVal = in;
}

Variant::Variant(int64_t in) {
	::VariantInit(this);
	vt = VT_I8;
	llVal = in;
}

Variant::Variant(uint64_t in) {
	::VariantInit(this);
	vt = VT_UI8;
	ullVal = in;
}

Variant::Variant(uint16_t in) {
	::VariantInit(this);
	vt = VT_I2;
	iVal = in;
}

Variant::Variant(const Variant & in) {
	::VariantInit(this);
	CheckCom(::VariantCopy(this, (VARIANTARG*)&in));
}

const Variant&	Variant::operator=(const Variant & in) {
	if (this != &in) {
		::VariantClear(this);
		CheckCom(::VariantCopy(this, (VARIANTARG*)&in));
	}
	return *this;
}

void Variant::change_type(DWORD type, DWORD flag) {
	CheckCom(::VariantChangeType(this, this, flag, type));
}

HRESULT Variant::change_type_nt(VARTYPE type, DWORD flag) throw() {
	return ::VariantChangeType(this, this, flag, type);
}

bool	Variant::as_bool() const {
	if (vt != VT_BOOL) {
		CheckApiError(E_INVALIDARG);
	}
	return boolVal == VARIANT_TRUE;
}

int64_t	Variant::as_int() const {
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
	CheckApiError(E_INVALIDARG);
	return 0;
}

uint64_t Variant::as_uint() const {
	return as_int();
}

ustring	Variant::as_str() const {
	switch (vt) {
		case VT_BSTR:
			return ustring(bstrVal, ::SysStringLen(bstrVal));
	}
	CheckApiError(E_INVALIDARG);
	return ustring();
}

ustring	Variant::as_str() {
	if (is_null() || is_empty())
		return ustring();
	else if (!is_str()) {
		change_type(VT_BSTR);
	}
	return bstrVal;
}

SafeArray::~SafeArray() {
	::SafeArrayUnlock(m_ptr);
}

SafeArray::SafeArray(VARTYPE type, size_t size):
	m_ptr(CheckPointer(::SafeArrayCreateVector(type, 0, size))) {
	::SafeArrayLock(m_ptr);
}

SafeArray::SafeArray(SAFEARRAY * ptr):
	m_ptr(CheckPointer(ptr)) {
	::SafeArrayLock(m_ptr);
}

SafeArray::SafeArray(const Variant &var):
	m_ptr(CheckPointer(var.parray)) {
	::SafeArrayLock(m_ptr);
}

size_t SafeArray::dims() const {
	return m_ptr->cDims;
}

size_t SafeArray::size() const {
	return m_ptr->rgsabound[0].cElements;
}
///===================================================================================== PropVariant
PropVariant::PropVariant(PCWSTR val) {
//	printf(L"PropVariant::PropVariant(PCWSTR val)\n");
	PropVariantInit(this);
	bstrVal = ::SysAllocString(val);
	vt = VT_BSTR;
}

PropVariant::PropVariant(const ustring & val) {
//	printf(L"PropVariant::PropVariant(const ustring &val)\n");
	PropVariantInit(this);
	bstrVal = ::SysAllocStringLen(val.c_str(), val.size());
	vt = VT_BSTR;
}

PropVariant::PropVariant(bool val) {
	vt = VT_BOOL;
	boolVal = val ? VARIANT_TRUE : VARIANT_FALSE;
}

PropVariant::PropVariant(uint32_t val) {
	vt = VT_UI4;
	ulVal = val;
}

PropVariant::PropVariant(uint64_t val) {
	vt = VT_UI8;
	uhVal.QuadPart = val;
}

PropVariant::PropVariant(const FILETIME & val) {
	vt = VT_FILETIME;
	filetime = val;
}

PropVariant::PropVariant(const this_type & var) {
//	printf(L"PropVariant::PropVariant(const class_type &var)\n");
	CheckCom(::PropVariantCopy(this, &var));
}

PropVariant& PropVariant::operator=(const this_type &rhs) {
	if (this != &rhs) {
		this_type tmp(rhs);
		swap(tmp);
	}
	return *this;
}

PropVariant& PropVariant::operator=(PCWSTR rhs) {
//	printf(L"PropVariant& PropVariant::operator=(PCWSTR rhs)\n");
	this_type tmp(rhs);
	swap(tmp);
	return *this;
}

PropVariant& PropVariant::operator=(const ustring &rhs) {
//	printf(L"PropVariant& PropVariant::operator=(const ustring &rhs)\n");
	this_type tmp(rhs);
	swap(tmp);
	return *this;
}

PropVariant& PropVariant::operator=(bool rhs) {
	if (vt != VT_BOOL) {
		clean();
		vt = VT_BOOL;
	}
	boolVal = rhs ? VARIANT_TRUE : VARIANT_FALSE;
	return *this;
}

PropVariant& PropVariant::operator=(uint32_t rhs) {
	if (vt != VT_UI4) {
		clean();
		vt = VT_UI4;
	}
	ulVal = rhs;
	return *this;
}

PropVariant& PropVariant::operator=(uint64_t rhs) {
	if (vt != VT_UI8) {
		clean();
		vt = VT_UI8;
	}
	uhVal.QuadPart = rhs;
	return *this;
}

PropVariant& PropVariant::operator=(const FILETIME &rhs) {
	if (vt != VT_FILETIME) {
		clean();
		vt = VT_FILETIME;
	}
	filetime = rhs;
	return *this;
}

PropVariant::pointer PropVariant::ref() {
	clean();
	return this;
}

void PropVariant::detach(pointer var) {
	if (var->vt != VT_EMPTY)
		CheckCom(::PropVariantClear(var));
	*var = *this;
	vt = VT_EMPTY;
}

size_t PropVariant::get_int_size() const {
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
			break;
	}
	return 0;
}

HRESULT PropVariant::as_bool_nt(bool & val) const throw() {
	if (vt == VT_BOOL) {
		val = (boolVal == VARIANT_TRUE);
		return S_OK;
	}
	return E_INVALIDARG;
}

bool PropVariant::as_bool() const {
	if (vt != VT_BOOL) {
		CheckCom(E_INVALIDARG);
	}
	return boolVal == VARIANT_TRUE;
}

FILETIME PropVariant::as_time() const {
	if (vt != VT_FILETIME) {
		CheckCom(E_INVALIDARG);
	}
	return filetime;
}

HRESULT PropVariant::as_str_nt(ustring & val) const throw() {
	switch (vt) {
		case VT_BSTR:
			val.assign(bstrVal, ::SysStringLen(bstrVal));
			return S_OK;
		case VT_LPWSTR:
			val.assign(pwszVal);
			return S_OK;
	}
	return E_INVALIDARG;
}

ustring	PropVariant::as_str() const {
	ustring ret;
	CheckCom(as_str_nt(ret));
	return ret;
}

int64_t	PropVariant::as_int() const {
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
	CheckCom(E_INVALIDARG);
	return 0;
}

uint64_t PropVariant::as_uint() const {
	return as_int();
}

void PropVariant::swap(this_type &rhs) {
//	printf(L"void PropVariant::swap(class_type &rhs)\n");
	PROPVARIANT & a(*this), & b(rhs);
	using std::swap;
	swap(a, b);
}

void PropVariant::clean() {
	if (vt != VT_EMPTY)
		::PropVariantClear(this);
}

///============================================================================================ BStr
BStr::BStr(PCWSTR val):
	m_str(nullptr) {
	if (val) {
		m_str = ::SysAllocString(val);
		if (!m_str)
			CheckCom(E_OUTOFMEMORY);
	}
}

BStr::BStr(const ustring & val) {
	m_str = ::SysAllocStringLen(val.c_str(), val.size());
	if (!m_str)
		CheckCom(E_OUTOFMEMORY);
}

BStr::BStr(const this_type & val):
	m_str(nullptr) {
	if (val.m_str) {
		m_str = ::SysAllocStringLen(val.m_str, val.size());
		if (!m_str)
			CheckCom(E_OUTOFMEMORY);
	}
}

BStr& BStr::operator=(PCWSTR val) {
	if (!::SysReAllocString(&m_str, val))
		CheckCom(E_OUTOFMEMORY);
	return *this;
}

BStr& BStr::operator=(const ustring& val) {
	if (!::SysReAllocStringLen(&m_str, val.c_str(), val.size()))
		CheckCom(E_OUTOFMEMORY);
	return *this;
}

BStr& BStr::operator=(const this_type & val) {
	if (this != &val) {
		if (!::SysReAllocStringLen(&m_str, val.m_str, val.size()))
			CheckCom(E_OUTOFMEMORY);
	}
	return *this;
}

size_t BStr::size() const {
	if (!m_str)
		CheckCom(E_POINTER);
	return ::SysStringLen(m_str);
}

BSTR* BStr::operator&() {
	clean();
	return &m_str;
}

void BStr::attach(BSTR & str) {
	clean();
	m_str = str;
	str = nullptr;
}

void BStr::detach(BSTR & str) {
	str = m_str;
	m_str = nullptr;
}

void BStr::swap(this_type & rhs) {
	using std::swap;
	swap(m_str, rhs.m_str);
}

void BStr::clean() {
	if (m_str) {
		::SysFreeString(m_str);
		m_str = nullptr;
	}
}

///========================================================================================= WinGUID
WinGUID::WinGUID() {
	CheckCom(::CoCreateGuid(this));
}

void WinGUID::init(PCWSTR str) {
	CheckCom(::CLSIDFromString((PWSTR)str, this));
}

void WinGUID::init(const ustring & str) {
	CheckCom(::CLSIDFromString((PWSTR)str.c_str(), this));
}

void WinGUID::init(const PropVariant & prop) {
	if (prop.vt == VT_BSTR) {
		size_t len = ::SysStringByteLen(prop.bstrVal);
		if (len == sizeof(*this)) {
			WinMem::Copy(this, prop.bstrVal, len);
			return;
		}
	} else if (prop.is_str()) {
		init(prop.as_str());
	}
	CheckCom(E_FAIL);
}

ustring WinGUID::as_str(const GUID & guid) {
	WCHAR buf[64];
	CheckApi(::StringFromGUID2(guid, buf, sizeofa(buf)));
	return ustring(buf);
}

///=================================================================================================
HRESULT ConvertBoolToHRESULT(bool result) {
	if (result)
		return S_OK;
	DWORD lastError = ::GetLastError();
	if (lastError == 0)
		return E_FAIL;
	return HRESULT_FROM_WIN32(lastError);
}
