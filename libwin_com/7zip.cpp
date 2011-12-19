#include "7zip.h"
#include <libwin_net/exception.h>

#define UNKNOWN_IMPL_ITF(iid) \
	if (riid == IID_##iid) { *object = static_cast<iid*>(this); AddRef(); return S_OK; }

//static const wchar_t *kCantDeleteOutputFile = L"ERROR: Can not delete output file ";

//static const wchar_t *kTestingString    =  L"Testing     ";
//static const wchar_t *kExtractingString =  L"Extracting  ";
//static const wchar_t *kSkippingString   =  L"Skipping    ";

//static const wchar_t *kUnsupportedMethod = L"Unsupported Method";
//static const wchar_t *kCRCFailed = L"CRC Failed";
//static const wchar_t *kDataError = L"Data Error";
//static const wchar_t *kUnknownError = L"Unknown Error";
//static const wchar_t *kEmptyFileAlias = L"[Content]";

namespace SevenZip {
	///======================================================================================== Prop
	Prop::Prop(const ComObject<IInArchive> & arc, size_t idx) {
		BStr m_nm;
		VARTYPE type;
		CheckApiError(arc->GetArchivePropertyInfo(idx, &m_nm, &id, &type));
		CheckApiError(arc->GetArchiveProperty(id, prop.ref()));
		name = (m_nm) ? ustring(m_nm.c_str()) : ustring();
	}

	///======================================================================================= Props
	void Props::cache(const ComObject<IInArchive> & arc) {
		UInt32 num_props = 0;
		CheckApiError(arc->GetNumberOfArchiveProperties(&num_props));
		clear();
		for (UInt32 idx = 0; idx < num_props; ++idx) {
			push_back(Prop(arc, idx));
		}
	}

	Props::Props() {
	}

	Props::Props(const ComObject<IInArchive> & arc) {
		cache(arc);
	}

	///========================================================================================= Lib
	Lib::Lib(PCWSTR path):
		FileVersion(path),
		DynamicLibrary(path) {
		CreateObject = (FCreateObject)get_function_nt("CreateObject");
		GetHandlerProperty = (FGetHandlerProperty)get_function_nt("GetHandlerProperty");
		GetHandlerProperty2 = (FGetHandlerProperty2)get_function_nt("GetHandlerProperty2");
		GetMethodProperty = (FGetMethodProperty)get_function_nt("GetMethodProperty");
		GetNumberOfFormats = (FGetNumberOfFormats)get_function_nt("GetNumberOfFormats");
		GetNumberOfMethods = (FGetNumberOfMethods)get_function_nt("GetNumberOfMethods");
		SetLargePageMode = (FSetLargePageMode)get_function_nt("SetLargePageMode");
		if (CreateObject && ((GetNumberOfFormats && GetHandlerProperty2) || GetHandlerProperty)) {
			m_codecs.cache(*this);
			m_methods.cache(*this);
			return;
		}
		CheckApiError(ERROR_INVALID_LIBRARY);
	}

	const Codecs & Lib::codecs() const {
		return m_codecs;
	}

	const Methods & Lib::methods() const {
		return m_methods;
	}

	HRESULT Lib::get_prop(UInt32 index, PROPID prop_id, PropVariant & prop) const {
		if (GetHandlerProperty2) {
			return GetHandlerProperty2(index, prop_id, prop.ref());
		} else {
			return GetHandlerProperty(prop_id, prop.ref());
		}
	}

	HRESULT Lib::get_prop(UInt32 index, PROPID prop_id, WinGUID & guid) const {
		PropVariant prop;
		HRESULT res = get_prop(index, prop_id, prop);
		if (res == S_OK)
			guid.init(prop);
		return res;
	}

	HRESULT Lib::get_prop(UInt32 index, PROPID prop_id, bool & value) const {
		PropVariant prop;
		HRESULT res = get_prop(index, prop_id, prop);
		return res == S_OK ? prop.as_bool_nt(value) : res;
	}

	HRESULT Lib::get_prop(UInt32 index, PROPID prop_id, ustring & value) const {
		PropVariant prop;
		HRESULT res = get_prop(index, prop_id, prop);
		return res == S_OK ? prop.as_str_nt(value) : res;
	}

	HRESULT Lib::get_prop(UInt32 index, PROPID prop_id, ByteVector & value) const {
		PropVariant prop;
		HRESULT res = get_prop(index, prop_id, prop);
		if (res != S_OK)
			return res;
		if (prop.vt == VT_BSTR) {
			size_t len = SysStringByteLen(prop.bstrVal);
			BYTE* data = reinterpret_cast<BYTE*>(prop.bstrVal);
			value.assign(&data[0], &data[len]);
		} else
			return E_FAIL;
		return S_OK;
	}
}
