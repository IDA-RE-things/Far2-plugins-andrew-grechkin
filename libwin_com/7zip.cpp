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
	///====================================================================================== Method
	Method::Method(const Lib & arc_lib, size_t idx) {
		PropVariant prop;
		CheckApiError(arc_lib.GetMethodProperty(idx, 0, prop.ref()));
		id = prop.as_uint();
		CheckApiError(arc_lib.GetMethodProperty(idx, 1, prop.ref()));
		name = prop.as_str();
		arc_lib.GetMethodProperty(idx, 2, prop.ref());
		if (prop.vt == VT_BSTR) {
			size_t len = SysStringByteLen(prop.bstrVal);
			BYTE* data = reinterpret_cast<BYTE*>(prop.bstrVal);
			start_sign.assign(&data[0], &data[len]);
		};
		arc_lib.GetMethodProperty(idx, 3, prop.ref());
		if (prop.vt == VT_BSTR) {
			size_t len = SysStringByteLen(prop.bstrVal);
			BYTE* data = reinterpret_cast<BYTE*>(prop.bstrVal);
			finish_sign.assign(&data[0], &data[len]);
		};
	}

	bool Method::operator<(const Method &rhs) const {
		return name < rhs.name;
	}

	bool Method::operator==(const Method &rhs) const {
		return name == rhs.name;
	}

	bool Method::operator!=(const Method &rhs) const {
		return name != rhs.name;
	}

	///===================================================================================== Methods
	Methods::Methods() {
	}

	Methods::Methods(const Lib & lib) {
		cache(lib);
	}

	void Methods::cache(const Lib & lib) {
		clear();
		UInt32 num_methods = 0;
		CheckCom(lib.GetNumberOfMethods(&num_methods));

		for (UInt32 idx = 0; idx < num_methods; ++idx) {
			std::tr1::shared_ptr<Method> tmp(new Method(lib, idx));
			insert(value_type(tmp->id, tmp));
		}
	}

	///======================================================================================= Codec
	Codec::Codec(const Lib & arc_lib, size_t idx):
		updatable(false) {
		CheckApiError(arc_lib.get_prop(idx, NArchive::kClassID, guid));
		CheckApiError(arc_lib.get_prop(idx, NArchive::kName, name));
		arc_lib.get_prop(idx, NArchive::kExtension, ext);
		arc_lib.get_prop(idx, NArchive::kAddExtension, add_ext);
		arc_lib.get_prop(idx, NArchive::kAssociate, kAssociate);
		arc_lib.get_prop(idx, NArchive::kStartSignature, start_sign);
		arc_lib.get_prop(idx, NArchive::kFinishSignature, finish_sign);
		arc_lib.get_prop(idx, NArchive::kUpdate, updatable);
		arc_lib.get_prop(idx, NArchive::kKeepName, kKeepName);
	}

	bool Codec::operator<(const Codec & rhs) const {
		return name < rhs.name;
	}

	bool Codec::operator==(const Codec & rhs) const {
		return name == rhs.name;
	}

	bool Codec::operator!=(const Codec & rhs) const {
		return name != rhs.name;
	}

	///====================================================================================== Codecs
	Codecs::Codecs() {
	}

	Codecs::Codecs(const Lib & lib) {
		cache(lib);
	}

	void Codecs::cache(const Lib & lib) {
		UInt32 num_formats = 0;
		CheckCom(lib.GetNumberOfFormats(&num_formats));
		clear();
		for (UInt32 idx = 0; idx < num_formats; ++idx) {
			std::tr1::shared_ptr<Codec> tmp(new Codec(lib, idx));
			insert(value_type(tmp->name, tmp));
		}
	}

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
	Lib::Lib(const ustring & path):
		DynamicLibrary(path.c_str()) {
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

	///============================================================================== FileReadStream
	ULONG FileReadStream::AddRef() {
		return UnknownImp::AddRef();
	}

	ULONG FileReadStream::Release() {
		return UnknownImp::Release();
	}

	HRESULT WINAPI FileReadStream::QueryInterface(REFIID riid, void** object) {
		//	printf(L"FileReadStream::QueryInterface()\n");
		UNKNOWN_IMPL_ITF(IInStream)
		UNKNOWN_IMPL_ITF(ISequentialInStream)
		return UnknownImp::QueryInterface(riid, object);
	}

	FileReadStream::~FileReadStream() {
		//	printf(L"FileReadStream::~FileReadStream()\n");
	}

	FileReadStream::FileReadStream(const ustring & path):
		WinFile(path, FILE_GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN) {
		//	printf(L"FileReadStream::FileReadStream(%s)\n", path);
	}

	HRESULT FileReadStream::Read(void * data, UInt32 size, UInt32 * processedSize) {
		//	printf(L"FileReadStream::Read(%d)\n", size);
		try {
			DWORD read = WinFile::read(data, size);
			if (processedSize)
				*processedSize = read;
		} catch (WinError &e) {
			return e.code();
		} catch (...) {
			return E_FAIL;
		}
		return S_OK;
	}

	HRESULT FileReadStream::Seek(Int64 offset, UInt32 seekOrigin, UInt64 * newPosition) {
		//	printf(L"FileReadStream::Seek(%Id, %d)\n", offset, seekOrigin);
		try {
			uint64_t new_position;
			WinFile::set_position(offset, seekOrigin);
			new_position = WinFile::get_position();
			if (newPosition)
				*newPosition = new_position;
		} catch (WinError &e) {
			return e.code();
		} catch (...) {
			return E_FAIL;
		}
		return S_OK;
	}

	///============================================================================= FileWriteStream
	ULONG FileWriteStream::AddRef() {
		return UnknownImp::AddRef();
	}

	ULONG FileWriteStream::Release() {
		return UnknownImp::Release();
	}

	HRESULT WINAPI FileWriteStream::QueryInterface(REFIID riid, void ** object) {
		UNKNOWN_IMPL_ITF(IOutStream)
		UNKNOWN_IMPL_ITF(ISequentialOutStream)
		return UnknownImp::QueryInterface(riid, object);
	}

	FileWriteStream::~FileWriteStream() {
		//	printf(L"FileWriteStream::~FileWriteStream()\n");
	}

	FileWriteStream::FileWriteStream(const ustring & path, DWORD creat):
		WinFile(path, GENERIC_READ | GENERIC_WRITE, 0, nullptr, creat, 0) {
	}

	HRESULT FileWriteStream::Write(PCVOID data, UInt32 size, UInt32 * processedSize) {
		DWORD written;
		bool result = write_nt(data, size, written);
		if (processedSize != NULL)
			*processedSize = written;
		return ConvertBoolToHRESULT(result);
	}

	HRESULT FileWriteStream::Seek(Int64 offset, UInt32 seekOrigin, UInt64 * newPosition) {
		HRESULT result = ConvertBoolToHRESULT(set_position_nt(offset, seekOrigin));
		if (newPosition != NULL)
			*newPosition = get_position();
		return result;
	}

	HRESULT FileWriteStream::SetSize(UInt64 newSize) {
		uint64_t currentPos = get_position();
		set_position(newSize);
		HRESULT result = ConvertBoolToHRESULT(set_eof());
		set_position(currentPos);
		return result;
	}
}
