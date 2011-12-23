#include "7zip.h"
#include <libwin_net/exception.h>

#define UNKNOWN_IMPL_ITF(iid) \
	if (riid == IID_##iid) { *object = static_cast<iid*>(this); AddRef(); return S_OK; }

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
		GET_DLL_FUNC(CreateObject);
		GET_DLL_FUNC(GetMethodProperty);
		GET_DLL_FUNC(GetNumberOfFormats);
		GET_DLL_FUNC(GetNumberOfMethods);
		GET_DLL_FUNC_NT(GetHandlerProperty);
		GET_DLL_FUNC_NT(GetHandlerProperty2);
		GET_DLL_FUNC_NT(SetLargePageMode);
		CheckApiThrowError(GetHandlerProperty2 || GetHandlerProperty, ERROR_INVALID_LIBRARY);
		m_codecs.cache(*this);
		m_methods.cache(*this);
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

	HRESULT Lib::get_prop(UInt32 index, PROPID prop_id, std::vector<BYTE> & value) const {
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
