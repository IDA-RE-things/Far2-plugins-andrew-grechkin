#include <libcom/propvariant.hpp>
#include <libext/exception.hpp>
#include <libext/dll.hpp>


// Variant and PropVariant
//http://msdn.microsoft.com/en-us/library/windows/desktop/bb762286(v=VS.85).aspx

namespace {
	///================================================================================= propsys_dll
	struct propsys_dll: private Ext::DynamicLibrary {
		typedef HRESULT (WINAPI *FPropVariantToString)(const PROPVARIANT * const, PWSTR, UINT);
		typedef HRESULT (WINAPI *FPropVariantToInt64)(const PROPVARIANT * const, LONGLONG *);
		typedef HRESULT (WINAPI *FPropVariantToUInt64)(const PROPVARIANT * const, ULONGLONG *);
		typedef HRESULT (WINAPI *FPropVariantToBoolean)(const PROPVARIANT * const, BOOL *);

		DEFINE_FUNC(PropVariantToString);
		DEFINE_FUNC(PropVariantToInt64);
		DEFINE_FUNC(PropVariantToUInt64);
		DEFINE_FUNC(PropVariantToBoolean);

		static propsys_dll & inst() {
			static propsys_dll ret;
			return ret;
		}

	private:
		propsys_dll():
			DynamicLibrary(L"propsys.dll") {
			GET_DLL_FUNC(PropVariantToString);
			GET_DLL_FUNC(PropVariantToInt64);
			GET_DLL_FUNC(PropVariantToUInt64);
			GET_DLL_FUNC(PropVariantToBoolean);
		}
	};
}

namespace Com {

	PropVariant::~PropVariant() {
		::PropVariantClear(this);
	}

	PropVariant::PropVariant() {
		PropVariantInit(this);
	}

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

	PropVariant::PropVariant(const base_type & var) {
		CheckCom(::PropVariantCopy(this, &var));
	}

	PropVariant & PropVariant::operator =(const base_type & rhs) {
		if (this != &rhs)
			this_type(rhs).swap(*this);
		return *this;
	}

	PropVariant & PropVariant::operator =(PCWSTR rhs) {
		this_type(rhs).swap(*this);
		return *this;
	}

	PropVariant & PropVariant::operator =(const ustring & rhs) {
		this_type(rhs).swap(*this);
		return *this;
	}

	PropVariant & PropVariant::operator =(bool rhs) {
		this_type(rhs).swap(*this);
		return *this;
	}

	PropVariant & PropVariant::operator =(uint32_t rhs) {
		this_type(rhs).swap(*this);
		return *this;
	}

	PropVariant & PropVariant::operator =(uint64_t rhs) {
		this_type(rhs).swap(*this);
		return *this;
	}

	PropVariant & PropVariant::operator =(const FILETIME & rhs) {
		this_type(rhs).swap(*this);
		return *this;
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
	    BOOL ret = false;
	    CheckCom(propsys_dll::inst().PropVariantToBoolean(this, &ret));
	    return ret;
	}

	FILETIME PropVariant::as_time() const {
		if (vt != VT_FILETIME) {
			CheckCom(E_INVALIDARG);
		}
		return filetime;
	}

	HRESULT PropVariant::as_str_nt(ustring & val) const throw() {
		HRESULT err = S_OK;
		switch (vt) {
			case VT_BOOL:
				val.assign(boolVal == VARIANT_FALSE ? L"false" : L"true");
				break;
			default:
				wchar_t buf[MAX_PATH];
				err = propsys_dll::inst().PropVariantToString(this, buf, sizeof(buf));
				if (SUCCEEDED(err))
					val.assign(buf);
				break;
		}
		return err;
	}

	ustring	PropVariant::as_str() const {
		ustring ret;
		CheckCom(as_str_nt(ret));
		return ret;
	}

	int64_t	PropVariant::as_int() const {
		LONGLONG ret = 0ll;
		CheckCom(propsys_dll::inst().PropVariantToInt64(this, &ret));
		return ret;
	//	switch (vt) {
	//		case VT_I1:
	//			return cVal;
	//		case VT_I2:
	//			return iVal;
	//		case VT_I4:
	//			return lVal;
	//		case VT_INT:
	//			return intVal;
	//		case VT_I8:
	//			return hVal.QuadPart;
	//		case VT_UI1:
	//			return bVal;
	//		case VT_UI2:
	//			return uiVal;
	//		case VT_UI4:
	//			return ulVal;
	//		case VT_UINT:
	//			return uintVal;
	//		case VT_UI8:
	//			return uhVal.QuadPart;
	//	}
	//	CheckCom(E_INVALIDARG);
	//	return 0;
	}

	uint64_t PropVariant::as_uint() const {
		ULONGLONG ret = 0ll;
		CheckCom(propsys_dll::inst().PropVariantToUInt64(this, &ret));
		return ret;
	}

	PropVariant::pointer PropVariant::ref() {
		::PropVariantClear(this);
		return this;
	}

	void PropVariant::detach(pointer var) {
		if (var->vt != VT_EMPTY)
			CheckCom(::PropVariantClear(var));
		*var = *this;
		vt = VT_EMPTY;
	}

	void PropVariant::swap(this_type & rhs) {
		PROPVARIANT & a(*this), & b(rhs);
		using std::swap;
		swap(a, b);
	}

}
