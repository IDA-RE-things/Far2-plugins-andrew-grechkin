#include "7zip.h"

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

///======================================================================================== ArcCodec
ArcCodec::ArcCodec(const SevenZipLib &arc_lib, size_t idx):
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

bool ArcCodec::operator<(const ArcCodec &rhs) const {
	return name < rhs.name;
}

bool ArcCodec::operator==(const ArcCodec &rhs) const {
	return name == rhs.name;
}

bool ArcCodec::operator!=(const ArcCodec &rhs) const {
	return name != rhs.name;
}

///======================================================================================= ArcCodecs
ArcCodecs::ArcCodecs() {
}

ArcCodecs::ArcCodecs(const SevenZipLib &lib) {
	cache(lib);
}

void ArcCodecs::cache(const SevenZipLib &lib) {
	clear();
	UInt32 num_formats = 0;
	CheckCom(lib.GetNumberOfFormats(&num_formats));

	for (UInt32 idx = 0; idx < num_formats; ++idx) {
		shared_ptr<ArcCodec> tmp(new ArcCodec(lib, idx));
		insert(value_type(tmp->name, tmp));
	}
}

///======================================================================================= ArcMethod
ArcMethod::ArcMethod(const SevenZipLib &arc_lib, size_t idx) {
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

bool ArcMethod::operator<(const ArcMethod &rhs) const {
	return name < rhs.name;
}

bool ArcMethod::operator==(const ArcMethod &rhs) const {
	return name == rhs.name;
}

bool ArcMethod::operator!=(const ArcMethod &rhs) const {
	return name != rhs.name;
}

///======================================================================================= ArcCodecs
ArcMethods::ArcMethods() {
}

ArcMethods::ArcMethods(const SevenZipLib &lib) {
	cache(lib);
}

void ArcMethods::cache(const SevenZipLib &lib) {
	clear();
	UInt32 num_methods = 0;
	CheckCom(lib.GetNumberOfMethods(&num_methods));

	for (UInt32 idx = 0; idx < num_methods; ++idx) {
		shared_ptr<ArcMethod> tmp(new ArcMethod(lib, idx));
		insert(value_type(tmp->id, tmp));
	}
}

///===================================================================================== SevenZipLib
SevenZipLib::SevenZipLib(const ustring &path):
	DynamicLibrary(path.c_str()) {
	CreateObject = (FCreateObject)get_function_nothrow("CreateObject");
	GetHandlerProperty = (FGetHandlerProperty)get_function_nothrow("GetHandlerProperty");
	GetHandlerProperty2 = (FGetHandlerProperty2)get_function_nothrow("GetHandlerProperty2");
	GetMethodProperty = (FGetMethodProperty)get_function_nothrow("GetMethodProperty");
	GetNumberOfFormats = (FGetNumberOfFormats)get_function_nothrow("GetNumberOfFormats");
	GetNumberOfMethods = (FGetNumberOfMethods)get_function_nothrow("GetNumberOfMethods");
	SetLargePageMode = (FSetLargePageMode)get_function_nothrow("SetLargePageMode");
	if (CreateObject && ((GetNumberOfFormats && GetHandlerProperty2) || GetHandlerProperty)) {
		m_codecs.cache(*this);
		m_methods.cache(*this);
		return;
	}
	CheckApiError(ERROR_INVALID_LIBRARY);
}

const ArcCodecs &SevenZipLib::codecs() const {
	return m_codecs;
}

const ArcMethods &SevenZipLib::methods() const {
	return m_methods;
}

HRESULT SevenZipLib::get_prop(UInt32 index, PROPID prop_id, PropVariant & prop) const {
	if (GetHandlerProperty2) {
		return GetHandlerProperty2(index, prop_id, prop.ref());
	} else {
		return GetHandlerProperty(prop_id, prop.ref());
	}
}

HRESULT SevenZipLib::get_prop(UInt32 index, PROPID prop_id, WinGUID & guid) const {
	PropVariant prop;
	HRESULT res = get_prop(index, prop_id, prop);
	if (res == S_OK)
		guid.init(prop);
	return res;
}

HRESULT SevenZipLib::get_prop(UInt32 index, PROPID prop_id, bool & value) const {
	PropVariant prop;
	HRESULT res = get_prop(index, prop_id, prop);
	return res == S_OK ? prop.as_bool_nt(value) : res;
}

HRESULT SevenZipLib::get_prop(UInt32 index, PROPID prop_id, ustring & value) const {
	PropVariant prop;
	HRESULT res = get_prop(index, prop_id, prop);
	return res == S_OK ? prop.as_str_nt(value) : res;
}

HRESULT SevenZipLib::get_prop(UInt32 index, PROPID prop_id, ByteVector & value) const {
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

///================================================================================== FileReadStream
HRESULT WINAPI FileReadStream::QueryInterface(REFIID riid, void** object) {
//	printf(L"FileReadStream::QueryInterface()\n");
	UNKNOWN_IMPL_ITF(IInStream)
	UNKNOWN_IMPL_ITF(ISequentialInStream)
	return UnknownImp::QueryInterface(riid, object);
}

FileReadStream::~FileReadStream() {
//	printf(L"FileReadStream::~FileReadStream()\n");
}

FileReadStream::FileReadStream(PCWSTR path):
	file(path, FILE_GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN) {
	check_device_file();
//	printf(L"FileReadStream::FileReadStream(%s)\n", path);
}

HRESULT FileReadStream::Read(void * data, UInt32 size, UInt32 * processedSize) {
//	printf(L"FileReadStream::Read(%d)\n", size);
	try {
		DWORD read;
		if (device_file) {
			uint64_t aligned_pos = device_pos / device_sector_size
				* device_sector_size;
			unsigned aligned_offset = static_cast<unsigned> (device_pos - aligned_pos);
			unsigned aligned_size = aligned_offset + size;
			aligned_size = (aligned_size / device_sector_size + (aligned_size
				% device_sector_size ? 1 : 0)) * device_sector_size;
			auto_array<unsigned char> buffer(aligned_size + device_sector_size);
			ptrdiff_t buffer_addr = buffer.data() - static_cast<unsigned char*> (0);
			unsigned char* alligned_buffer = reinterpret_cast<unsigned char*> (buffer_addr
				% device_sector_size ? (buffer_addr / device_sector_size + 1)
				                     * device_sector_size : buffer_addr);
			file.set_position(aligned_pos, FILE_BEGIN);
			read = file.read(alligned_buffer, aligned_size);
			if (read < aligned_offset)
				read = 0;
			else
				read -= aligned_offset;
			if (read > size)
				read = size;
			device_pos += read;
			memcpy(data, alligned_buffer + aligned_offset, read);
		} else {
//			wcout << L"Read 1 pos:" << file.get_position() << wendl;
			read = file.read(data, size);
		}
		if (processedSize)
			*processedSize = read;
	} catch (WinError &e) {
		return e.code();
	} catch (...) {
		return E_FAIL;
	}
	return S_OK;
}

HRESULT FileReadStream::Seek(Int64 offset, UInt32 seekOrigin, UInt64 *newPosition) {
//	printf(L"FileReadStream::Seek(%Id, %d)\n", offset, seekOrigin);
	try {
		uint64_t new_position;
		if (device_file) {
			switch (seekOrigin) {
				case STREAM_SEEK_SET:
					device_pos = offset;
					break;
				case STREAM_SEEK_CUR:
					device_pos += offset;
					break;
				case STREAM_SEEK_END:
					device_pos = device_size + offset;
					break;
				default:
					CheckCom(E_INVALIDARG);
					break;
			}
			new_position = device_pos;
		} else {
			file.set_position(offset, seekOrigin);
			new_position = file.get_position();
		}
		if (newPosition)
			*newPosition = new_position;
	} catch (WinError &e) {
		return e.code();
	} catch (...) {
		return E_FAIL;
	}
	return S_OK;
}

void FileReadStream::check_device_file() {
	device_pos = 0;
	device_file = false;
	if (file.size_nt(device_size))
		return;

	PARTITION_INFORMATION part_info;
	if (file.io_control_out_nt(IOCTL_DISK_GET_PARTITION_INFO, part_info)) {
		device_size = part_info.PartitionLength.QuadPart;
		DWORD sectors_per_cluster, bytes_per_sector, number_of_free_clusters,
		total_number_of_clusters;
		ustring pp = file.path();
		ensure_end_path_separator(pp);

		if (GetDiskFreeSpaceW(pp.c_str(), &sectors_per_cluster,
							  &bytes_per_sector, &number_of_free_clusters,
							  &total_number_of_clusters))
			device_sector_size = bytes_per_sector;
		else
			device_sector_size = 4096;
		device_file = true;
		return;
	}

	DISK_GEOMETRY disk_geometry;
	if (file.io_control_out_nt(IOCTL_DISK_GET_DRIVE_GEOMETRY, disk_geometry)) {
		device_size = disk_geometry.Cylinders.QuadPart * disk_geometry.TracksPerCylinder
					  * disk_geometry.SectorsPerTrack * disk_geometry.BytesPerSector;
		device_sector_size = disk_geometry.BytesPerSector;
		device_file = true;
		return;
	}
}

///=================================================================================== FileWriteStream
HRESULT WINAPI FileWriteStream::QueryInterface(REFIID riid, void** object) {
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

STDMETHODIMP FileWriteStream::Write(PCVOID data, UInt32 size, UInt32 *processedSize) {
	DWORD written;
	bool result = write_nt(data, size, written);
	if (processedSize != NULL)
		*processedSize = written;
	return ConvertBoolToHRESULT(result);
}

STDMETHODIMP FileWriteStream::Seek(Int64 offset, UInt32 seekOrigin, UInt64 *newPosition) {
	HRESULT result = ConvertBoolToHRESULT(set_position_nt(offset, seekOrigin));
	if (newPosition != NULL)
		*newPosition = get_position();
	return result;
}

STDMETHODIMP FileWriteStream::SetSize(UInt64 newSize) {
	uint64_t currentPos = get_position();
	set_position(newSize);
	HRESULT result = ConvertBoolToHRESULT(set_eof());
	set_position(currentPos);
	return result;
}
