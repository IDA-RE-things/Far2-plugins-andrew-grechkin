#include <libcom/variant.hpp>
#include <libext/exception.hpp>
#include <libbase/pcstr.hpp>


namespace Com {

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
		bstrVal = ::SysAllocStringLen(val, Base::get_str_len(val));
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
			LONG ind[] = {(LONG)i};
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

	Variant::Variant(const base_type & in) {
		::VariantInit(this);
		CheckCom(::VariantCopy(this, (VARIANTARG*)&in));
	}

	const Variant::this_type & Variant::operator =(const base_type & rhs) {
		if (this != &rhs)
			this_type(rhs).swap(*this);
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

	Variant::pointer Variant::ref() {
		::VariantClear(this);
		return this;
	}

	void Variant::swap(this_type & rhs) {
		VARIANT & a(*this), & b(rhs);
		using std::swap;
		swap(a, b);
	}

}
