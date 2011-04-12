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
		winstd::shared_ptr<ArcCodec> tmp(new ArcCodec(lib, idx));
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
	UInt32 num_formats = 0;
	CheckCom(lib.GetNumberOfMethods(&num_formats));

	for (UInt32 idx = 0; idx < num_formats; ++idx) {
		winstd::shared_ptr<ArcMethod> tmp(new ArcMethod(lib, idx));
		insert(value_type(tmp->name, tmp));
	}
}

///===================================================================================== SevenZipLib
SevenZipLib::SevenZipLib(const AutoUTF &path):
	DynamicLibrary(path.c_str()) {
	CreateObject = (FCreateObject)get_function_nothrow("CreateObject");
	GetNumberOfMethods = (FGetNumberOfMethods)get_function_nothrow("GetNumberOfMethods");
	GetMethodProperty = (FGetMethodProperty)get_function_nothrow("GetMethodProperty");
	GetNumberOfFormats = (FGetNumberOfFormats)get_function_nothrow("GetNumberOfFormats");
	GetHandlerProperty = (FGetHandlerProperty)get_function_nothrow("GetHandlerProperty");
	GetHandlerProperty2 = (FGetHandlerProperty2)get_function_nothrow("GetHandlerProperty2");
	if (CreateObject && ((GetNumberOfFormats && GetHandlerProperty2) || GetHandlerProperty)) {
		m_codecs.cache(*this);
		return;
	}
	CheckApiError(ERROR_INVALID_LIBRARY);
}

const ArcCodecs &SevenZipLib::codecs() const {
	return m_codecs;
}

HRESULT SevenZipLib::get_prop(UInt32 index, PROPID prop_id, PROPVARIANT &prop) const {
	if (GetHandlerProperty2) {
		return GetHandlerProperty2(index, prop_id, &prop);
	} else {
		return GetHandlerProperty(prop_id, &prop);
	}
}

HRESULT SevenZipLib::get_prop(UInt32 index, PROPID prop_id, WinGUID& guid) const {
	PropVariant prop;
	HRESULT res = get_prop(index, prop_id, prop);
	if (res == S_OK)
		guid.init(prop);
	return res;
}

HRESULT SevenZipLib::get_prop(UInt32 index, PROPID prop_id, bool &value) const {
	PropVariant prop;
	HRESULT res = get_prop(index, prop_id, prop);
	return res == S_OK ? prop.as_bool_nt(value) : res;
}

HRESULT SevenZipLib::get_prop(UInt32 index, PROPID prop_id, AutoUTF& value) const {
	PropVariant prop;
	HRESULT res = get_prop(index, prop_id, prop);
	return res == S_OK ? prop.as_str_nt(value) : res;
}

HRESULT SevenZipLib::get_prop(UInt32 index, PROPID prop_id, ByteVector& value) const {
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

///=============================================================================== FileReadStream
HRESULT WINAPI FileReadStream::QueryInterface(REFIID riid, void** object) {
//	wcout << L"FileReadStream::QueryInterface()"<< wendl;
	UNKNOWN_IMPL_ITF(IInStream)
	UNKNOWN_IMPL_ITF(ISequentialInStream)
	if (riid == IID_IUnknown) {
		*object = static_cast<IUnknown*>(static_cast<ComBase*>(this));
		AddRef();
		return S_OK;
	}
	*object = nullptr;
	return E_NOINTERFACE;
}

ULONG FileReadStream::AddRef() {
	return ++ref_cnt;
}

ULONG FileReadStream::Release() {
	if (--ref_cnt == 0) {
		delete this;
	}
	return ref_cnt;
}

FileReadStream::~FileReadStream() {
//	printf(L"FileReadStream::~FileReadStream()\n");
}

FileReadStream::FileReadStream(PCWSTR path):
	file(path, FILE_GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN) {
	check_device_file();
//	printf(L"FileReadStream::FileReadStream(%s)\n", path);
}

HRESULT FileReadStream::Read(void *data, UInt32 size, UInt32 *processedSize) {
//	wcout << L"FileReadStream::Read(" << size << L")" << wendl;
	try {
		DWORD ridden;
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
			ridden = file.read(alligned_buffer, aligned_size);
			if (ridden < aligned_offset)
				ridden = 0;
			else
				ridden -= aligned_offset;
			if (ridden > size)
				ridden = size;
			device_pos += ridden;
			memcpy(data, alligned_buffer + aligned_offset, ridden);
		} else {
//			wcout << L"Read 1 pos:" << file.get_position() << wendl;
			ridden = file.read(data, size);
//			wcout << L"ridden: " << ridden << wendl;
//			wcout << L"Read 2 pos:" << file.get_position() << wendl;
		}
		if (processedSize)
			*processedSize = ridden;
	} catch (WinError &e) {
		return e.code();
	} catch (...) {
		return E_FAIL;
	}
	return S_OK;
}

HRESULT FileReadStream::Seek(Int64 offset, UInt32 seekOrigin, UInt64 *newPosition) {
//	wcout << L"FileReadStream::Seek(" << offset << L", " << seekOrigin << L")" << wendl;
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
		AutoUTF pp = file.path();
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
	if (riid == IID_IUnknown) {
		*object = static_cast<IUnknown*>(static_cast<ComBase*>(this));
		AddRef();
		return S_OK;
	}
	*object = nullptr;
	return E_NOINTERFACE;
}

ULONG FileWriteStream::AddRef() {
	return ++ref_cnt;
}

ULONG FileWriteStream::Release() {
	if (--ref_cnt == 0) {
		delete this;
	}
	return ref_cnt;
}

FileWriteStream::~FileWriteStream() {
//	printf(L"FileWriteStream::~FileWriteStream()\n");
}

FileWriteStream::FileWriteStream(PCWSTR path, DWORD creat):
	WinFile(path, GENERIC_READ | GENERIC_WRITE, 0, nullptr, creat, 0) {
//	printf(L"FileWriteStream::FileWriteStream(%s)\n", path);
}

STDMETHODIMP FileWriteStream::Write(PCVOID data, UInt32 size, UInt32 *processedSize) {
	DWORD written;
	bool result = write_nt(data, size, written);
	if (processedSize != NULL)
		*processedSize = written;
	return ConvertBoolToHRESULT(result);
}

STDMETHODIMP FileWriteStream::Seek(Int64 offset, UInt32 seekOrigin, UInt64 *newPosition) {
	bool result = set_position_nt(offset, seekOrigin);
	if (newPosition != NULL)
		*newPosition = get_position();
	return ConvertBoolToHRESULT(result);
}

STDMETHODIMP FileWriteStream::SetSize(UInt64 newSize) {
	uint64_t currentPos = get_position();
	set_position(newSize);
	bool result = set_eof();
	set_position(currentPos);
	return result ? S_OK : E_FAIL;
}

///============================================================================= ArchiveOpenCallback
HRESULT ArchiveOpenCallback::QueryInterface(REFIID riid, void** object) {
	UNKNOWN_IMPL_ITF(IArchiveOpenCallback)
	UNKNOWN_IMPL_ITF(ICryptoGetTextPassword)
	if (riid == IID_IUnknown) {
		*object = static_cast<IUnknown*>(static_cast<ComBase*>(this));
		AddRef();
		return S_OK;
	}
	*object = nullptr;
	return E_NOINTERFACE;
}

ULONG ArchiveOpenCallback::AddRef() {
	return ++ref_cnt;
}

ULONG  ArchiveOpenCallback::Release() {
	if (--ref_cnt == 0) {
		delete this;
	}
	return ref_cnt;
}

ArchiveOpenCallback::ArchiveOpenCallback() {
}

HRESULT ArchiveOpenCallback::SetTotal(const UInt64 */*files*/, const UInt64 */*bytes*/) {
	return S_OK;
}

HRESULT ArchiveOpenCallback::SetCompleted(const UInt64 */*files*/, const UInt64 */*bytes*/) {
	return S_OK;
}

HRESULT ArchiveOpenCallback::CryptoGetTextPassword(BSTR */*password*/) {
	if (Password.empty()) {
		// You can ask real password here from user
		// Password = GetPassword(OutStream);
		// PasswordIsDefined = true;
		return E_ABORT;
	}
	return S_OK;
}

///========================================================================== ArchiveExtractCallback
HRESULT WINAPI ArchiveExtractCallback::QueryInterface(REFIID riid, void** object) {
//	printf(L"ArchiveExtractCallback::QueryInterface()\n");
	UNKNOWN_IMPL_ITF(IArchiveExtractCallback)
	UNKNOWN_IMPL_ITF(ICryptoGetTextPassword)
	if (riid == IID_IUnknown) {
		*object = static_cast<IUnknown*>(static_cast<ComBase*>(this));
		AddRef();
		return S_OK;
	}
	*object = nullptr;
	return E_NOINTERFACE;
}

ULONG ArchiveExtractCallback::AddRef() {
	return ++ref_cnt;
}

ULONG ArchiveExtractCallback::Release() {
	if (--ref_cnt == 0) {
		delete this;
	}
	return ref_cnt;
}

ArchiveExtractCallback::~ArchiveExtractCallback() {
//	printf(L"ArchiveExtractCallback::~ArchiveExtractCallback()\n");
}

ArchiveExtractCallback::ArchiveExtractCallback(const WinArchive &arc, const AutoUTF &dest_path, const AutoUTF &pass):
	NumErrors(0),
	Password(pass),
	m_wa(arc),
	m_dest(MakeGoodPath(dest_path)) {
//	printf(L"ArchiveExtractCallback::ArchiveExtractCallback(%s)\n", dest_path.c_str());
	ensure_end_path_separator(m_dest);
}

STDMETHODIMP ArchiveExtractCallback::SetTotal(UInt64 /*size*/) {
	// return total size
//	printf(L"ArchiveExtractCallback::SetTotal(%d)\n", size);
	return S_OK;
}

STDMETHODIMP ArchiveExtractCallback::SetCompleted(const UInt64 */*completeValue*/) {
	// return processed size
//	if (completeValue) {
//		printf(L"ArchiveExtractCallback::SetCompleted(%d)\n", *completeValue);
//	}
	return S_OK;
}

STDMETHODIMP ArchiveExtractCallback::GetStream(UInt32 index, ISequentialOutStream **outStream, Int32 askExtractMode) {
//	printf(L"ArchiveExtractCallback::GetStream(%d, %d)\n", index, askExtractMode);
	*outStream = nullptr;
	m_index = index;

	if (askExtractMode != NArchive::NExtract::NAskMode::kExtract)
		return S_OK;


	m_diskpath = m_dest + m_wa.at(m_index).path();
//	printf(L"ArchiveExtractCallback::GetStream(%s)\n", m_diskpath.c_str());

	if (m_wa[m_index].is_dir()) {
		create_directory_full(m_diskpath);
	} else {
		// Create folders for file
		size_t pos = m_diskpath.find_last_of(PATH_SEPARATORS);
		if (pos != AutoUTF::npos) {
			create_directory_full(m_diskpath.substr(0, pos));
		}

		if (file_exists(m_diskpath) && !delete_file(m_diskpath)) {
//			PrintString(AutoUTF(kCantDeleteOutputFile) + m_diskpath);
			return E_ABORT;
		}

		ComObject<ISequentialOutStream> stream(new FileWriteStream(m_diskpath.c_str(), true));
		m_stream = stream;
		stream.detach(*outStream);
	}
	return S_OK;
}

STDMETHODIMP ArchiveExtractCallback::PrepareOperation(Int32 askExtractMode) {
//	printf(L"ArchiveExtractCallback::PrepareOperation(%d)\n", askExtractMode);
	ExtractMode = askExtractMode;

	switch (askExtractMode) {
		case NArchive::NExtract::NAskMode::kExtract:
//			PrintString(kExtractingString);
			break;
		case NArchive::NExtract::NAskMode::kTest:
//			PrintString(kTestingString);
			break;
		case NArchive::NExtract::NAskMode::kSkip:
//			PrintString(kSkippingString);
			break;
	};

//	PrintString(_processedFileInfo.path);
	return S_OK;
}

STDMETHODIMP ArchiveExtractCallback::SetOperationResult(Int32 operationResult) {
//	printf(L"ArchiveExtractCallback::SetOperationResult(%d)\n", operationResult);
	switch (operationResult) {
		case NArchive::NExtract::NOperationResult::kOK:
			break;
		default: {
			NumErrors++;
//			PrintString("     ");
			switch (operationResult) {
				case NArchive::NExtract::NOperationResult::kUnSupportedMethod:
//					PrintString(kUnsupportedMethod);
					break;
				case NArchive::NExtract::NOperationResult::kCRCError:
//					PrintString(kCRCFailed);
					break;
				case NArchive::NExtract::NOperationResult::kDataError:
//					PrintString(kDataError);
					break;
				default:
					;
//					PrintString(kUnknownError);
			}
		}
	}

	if (ExtractMode == NArchive::NExtract::NAskMode::kExtract) {
		set_attributes(m_diskpath.c_str(), m_wa[m_index].attr());
		if (m_stream) {
			FileWriteStream * file((FileWriteStream*)(ISequentialOutStream*)m_stream);
			file->set_mtime(m_wa[m_index].mtime());
		}
	}
	m_stream.Release();
//	PrintString(L"\n");
	return S_OK;
}

STDMETHODIMP ArchiveExtractCallback::CryptoGetTextPassword(BSTR *pass) {
//	printf(L"ArchiveExtractCallback::CryptoGetTextPassword()\n");
	if (Password.empty()) {
		// You can ask real password here from user
		// PrintError("Password is not defined");
//		return E_ABORT;
	}
	BStr(Password).detach(*pass);
	return S_OK;
}

///==================================================================================== DirStructure
DirStructure::DirStructure() {
}

DirStructure::DirStructure(const AutoUTF &path) {
	add(path);
}

void DirStructure::add(const AutoUTF &add_path) {
	AutoUTF path(PathNice(add_path));
	path = get_fullpath(ensure_no_end_path_separator(path));
	path = ensure_path_prefix(path);
	if (is_exists(path)) {
		size_t pos = path.find_last_of(PATH_SEPARATORS);
		if (pos != AutoUTF::npos) {
			base_add(path.substr(0, pos), path.substr(pos + 1));
		}
	}
}

void DirStructure::base_add(const AutoUTF &base_path, const AutoUTF &name) {
//	printf(L"DirStructure::base_add(%s, %s)\n", base_path.c_str(), name.c_str());
	push_back(DirItem(base_path, name));
	AutoUTF path(MakePath(base_path, name));
	if (is_dir(path)) {
		WinDir dir(path);
		for (WinDir::iterator it = dir.begin(); it != dir.end(); ++it) {
			if (it.is_dir() || it.is_link_dir()) {
				base_add(base_path, MakePath(name, it.name()));
			} else {
				base_add(base_path, MakePath(name, it.name()));
//				push_back(DirItem(base_path, MakePath(name, it.name())));
			}
		}
	}
}

///=========================================================================== ArchiveUpdateCallback
DirItem::DirItem(const AutoUTF &file_path, const AutoUTF &file_name):
	WinFileInfo(MakePath(file_path, file_name)),
	path(file_path),
	name(file_name) {
}

ArchiveUpdateCallback::~ArchiveUpdateCallback() {
//	printf(L"ArchiveUpdateCallback::~ArchiveUpdateCallback()\n");
}

ArchiveUpdateCallback::ArchiveUpdateCallback(const DirStructure &items, const AutoUTF &pass):
	Password(pass),
	AskPassword(false),
	DirItems(items) {
//	printf(L"ArchiveUpdateCallback::ArchiveUpdateCallback()\n");
};

HRESULT WINAPI ArchiveUpdateCallback::QueryInterface(REFIID riid, void** object) {
	UNKNOWN_IMPL_ITF(IArchiveUpdateCallback2)
	UNKNOWN_IMPL_ITF(ICryptoGetTextPassword2)
	if (riid == IID_IUnknown) {
		*object = static_cast<IUnknown*>(static_cast<ComBase*>(this));
		AddRef();
		return S_OK;
	}
	*object = nullptr;
	return E_NOINTERFACE;
}

ULONG ArchiveUpdateCallback::AddRef() {
	return ++ref_cnt;
}

ULONG ArchiveUpdateCallback::Release() {
	if (--ref_cnt == 0) {
		delete this;
	}
	return ref_cnt;
}

STDMETHODIMP ArchiveUpdateCallback::SetTotal(UInt64 /* size */) {
	return S_OK;
}

STDMETHODIMP ArchiveUpdateCallback::SetCompleted(const UInt64 * /* completeValue */) {
	return S_OK;
}

STDMETHODIMP ArchiveUpdateCallback::GetUpdateItemInfo(UInt32 /*index*/, Int32 *newData, Int32 *newProperties, UInt32 *indexInArchive) {
//	printf(L"ArchiveUpdateCallback::GetUpdateItemInfo(%d)\n", index);
	if (newData)
		*newData = Int32(true);
	if (newProperties)
		*newProperties = Int32(true);
	if (indexInArchive)
		*indexInArchive = (UInt32)-1;
	return S_OK;
}

STDMETHODIMP ArchiveUpdateCallback::GetProperty(UInt32 index, PROPID propID, PROPVARIANT *value) {
//	printf(L"ArchiveUpdateCallback::GetProperty(%d, %d)\n", index, propID);
	PropVariant prop;

	if (propID == kpidIsAnti) {
		prop = false;
		prop.detach(value);
		return S_OK;
	}

	const DirItem &dirItem = DirItems[index];
	switch (propID) {
		case kpidPath:
			prop = dirItem.name.c_str();
			break;
		case kpidIsDir:
			prop = dirItem.is_dir_or_link();
			break;
		case kpidSize:
			prop = dirItem.size();
			break;
		case kpidAttrib:
			prop = (uint32_t)dirItem.attr();
			break;
		case kpidCTime:
			prop = dirItem.ctime_ft();
			break;
		case kpidATime:
			prop = dirItem.atime_ft();
			break;
		case kpidMTime:
			prop = dirItem.mtime_ft();
			break;
	}
	prop.detach(value);
	return S_OK;
}

STDMETHODIMP ArchiveUpdateCallback::EnumProperties(IEnumSTATPROPSTG ** /* enumerator */) {
	return E_NOTIMPL;
}

STDMETHODIMP ArchiveUpdateCallback::GetStream(UInt32 index, ISequentialInStream **inStream) {
//	printf(L"ArchiveUpdateCallback::GetStream(%d)\n", index);

	const DirItem &dirItem = DirItems[index];
//	PrintString("Compressing  ");
	PrintString(dirItem.name);

	if (dirItem.is_dir_or_link())
		return S_OK;

	try {
		ComObject<ISequentialInStream>(new FileReadStream(MakePath(dirItem.path, dirItem.name).c_str())).detach(*inStream);
	} catch (WinError &e) {
		FailedCodes.push_back(e.code());
		FailedFiles.push_back(dirItem.name);
		// if (systemError == ERROR_SHARING_VIOLATION)
		{
			PrintString(L"\nWARNING: can't open file");
			PrintString(e.msg());
			return S_FALSE;
		}
	}
	return S_OK;
}

STDMETHODIMP ArchiveUpdateCallback::SetOperationResult(Int32 /* operationResult */) {
//	printf(L"ArchiveUpdateCallback::SetOperationResult()\n");
	return S_OK;
}

STDMETHODIMP ArchiveUpdateCallback::GetVolumeSize(UInt32 index, UInt64 *size) {
	printf(L"ArchiveUpdateCallback::GetVolumeSize()\n");
	if (VolumesSizes.size() == 0)
		return S_FALSE;
	if (index >= (UInt32)VolumesSizes.size())
		index = VolumesSizes.size() - 1;
	*size = VolumesSizes[index];
	return S_OK;
}

STDMETHODIMP ArchiveUpdateCallback::GetVolumeStream(UInt32 index, ISequentialOutStream **volumeStream) {
	printf(L"ArchiveUpdateCallback::GetVolumeStream(%d)\n", index);
	AutoUTF res = Num2Str(index + 1);
	while (res.size() < 2)
		res = AutoUTF(L"0") + res;
	AutoUTF fileName = VolName;
	fileName += L'.';
	fileName += res;
	fileName += VolExt;
	FileWriteStream *streamSpec = new FileWriteStream(fileName.c_str(), false);
	ComObject<ISequentialOutStream> streamLoc(streamSpec);
	streamLoc.detach(*volumeStream);
	return S_OK;
}

STDMETHODIMP ArchiveUpdateCallback::CryptoGetTextPassword2(Int32 *passwordIsDefined, BSTR *password) {
//	printf(L"ArchiveUpdateCallback::CryptoGetTextPassword2()\n");
	if (Password.empty() && AskPassword) {
		// You can ask real password here from user
		// Password = GetPassword(OutStream);
		// PasswordIsDefined = true;
		PrintString("Password is not defined");
		return E_ABORT;
	}
	try {
		*passwordIsDefined = !Password.empty();
		BStr(Password).detach(*password);
	} catch (WinError &e) {
		return E_ABORT;
	}
	return S_OK;
}

///====================================================================================== WinArchive
WinArchive::WinArchive(const SevenZipLib &lib, const AutoUTF &path, flags_type flags):
	m_path(path),
	m_mask(L"*"),
	m_flags(flags) {
	InitArc(lib);
	InitProps();
}

WinArchive::WinArchive(const SevenZipLib &lib, const AutoUTF &path, const AutoUTF &mask, flags_type flags):
	m_path(path),
	m_mask(mask),
	m_flags(flags) {
	InitArc(lib);
	InitProps();
}

WinArchive::WinArchive(ComObject<IInArchive> arc, flags_type flags):
	m_mask(L"*"),
	m_flags(flags),
	m_arc(arc) {
	InitProps();
}

void WinArchive::InitArc(const SevenZipLib &lib) {
	ComObject<IInArchive> arc;
	ComObject<IInStream> stream(new FileReadStream(m_path.c_str()));
	ComObject<IArchiveOpenCallback> openCallback(new ArchiveOpenCallback);
	for (ArcCodecs::const_iterator it = lib.codecs().begin(); it != lib.codecs().end(); ++it) {
		CheckCom(lib.CreateObject(&it->second->guid, &IID_IInArchive, (PVOID*)&arc));
		CheckCom(stream->Seek(0, STREAM_SEEK_SET, nullptr));
		if (arc->Open(stream, &max_check_start_position, openCallback) == S_OK) {
			m_codec = it;
			m_arc = arc;
			return;
		}
	}
	CheckApiError(ERROR_INVALID_DATA);
}

void WinArchive::InitProps() {
	m_arc->GetNumberOfItems(&m_size);
	m_arc->GetNumberOfArchiveProperties(&m_num_props);
}

const ArcCodec & WinArchive::codec() const {
	return *(m_codec->second);
}

ComObject<IInArchive> WinArchive::operator->() const {
	return m_arc;
}


WinArchive::const_iterator WinArchive::begin() const {
	return const_iterator(*this);
}

WinArchive::const_iterator WinArchive::end() const {
	return const_iterator();
}

WinArchive::const_iterator WinArchive::at(size_t index) const {
	if (index >= (size_t)m_size)
		CheckCom(TYPE_E_OUTOFBOUNDS);
	return const_iterator(*this, index);
}

WinArchive::const_iterator WinArchive::operator[](int index) const {
	return const_iterator(*this, index);
}

bool WinArchive::empty() const {
	return m_size == 0;
}

size_t WinArchive::size() const {
	return m_size;
}

AutoUTF WinArchive::path() const {
	return m_path;
}

AutoUTF WinArchive::mask() const {
	return m_mask;
}

WinArchive::flags_type WinArchive::flags() const {
	return m_flags;
}

size_t WinArchive::get_num_props() const {
	return m_num_props;
}

size_t WinArchive::get_num_item_props() const {
	UInt32 props = 0;
	m_arc->GetNumberOfProperties(&props);
	return props;
}

bool WinArchive::get_prop_info(size_t index, AutoUTF &name, PROPID &id) const {
	BStr m_nm;
	VARTYPE type;
	HRESULT err = m_arc->GetArchivePropertyInfo(index, &m_nm, &id, &type);
	if (err == S_OK && m_nm)
		name = m_nm.c_str();
	return err == S_OK;
}

PropVariant WinArchive::get_prop(PROPID id) const {
	PropVariant prop;
	m_arc->GetArchiveProperty(id, prop.ref());
	return prop;
}

size_t WinArchive::test() const {
	ArchiveExtractCallback *callback(new ArchiveExtractCallback(*this, L""));
	ComObject<IArchiveExtractCallback> extractCallback(callback);
	m_arc->Extract(nullptr, (UInt32) - 1, true, extractCallback);
	return callback->NumErrors;
}

void WinArchive::extract(const AutoUTF &dest) const {
	ComObject<IArchiveExtractCallback> extractCallback(new ArchiveExtractCallback(*this, dest));
	CheckCom(m_arc->Extract(nullptr, (UInt32) - 1, false, extractCallback));
}

WinArchive::operator ComObject<IInArchive>() const {
	return m_arc;
}

ComObject<IInArchive> WinArchive::open(const SevenZipLib &lib, PCWSTR path) {
	ComObject<IInArchive> arc;
	ComObject<IInStream> stream(new FileReadStream(path));
	ComObject<IArchiveOpenCallback> openCallback(new ArchiveOpenCallback);
	for (ArcCodecs::iterator it = lib.codecs().begin(); it != lib.codecs().end(); ++it) {
		CheckCom(lib.CreateObject(&it->second->guid, &IID_IInArchive, (PVOID*)&arc));
		CheckCom(stream->Seek(0, STREAM_SEEK_SET, nullptr));
		if (arc->Open(stream, &max_check_start_position, openCallback) == S_OK) {
			return arc;
		}
	}
	CheckApiError(ERROR_INVALID_DATA);
	return ComObject<IInArchive>();
}

AutoUTF WinArchive::const_input_iterator::path() const {
	return get_prop(kpidPath).as_str();
}

uint64_t WinArchive::const_input_iterator::size() const {
	return get_prop(kpidSize).as_uint();
}

size_t WinArchive::const_input_iterator::attr() const {
	return get_prop(kpidAttrib).as_uint();
}

FILETIME WinArchive::const_input_iterator::mtime() const {
	return get_prop(kpidMTime).as_time();
}

bool WinArchive::const_input_iterator::is_file() const {
	return !is_dir();
}

bool WinArchive::const_input_iterator::is_dir() const {
	return get_prop(kpidIsDir).as_bool();
}

bool WinArchive::const_input_iterator::get_prop_info(size_t index, AutoUTF &name, PROPID &id) const {
	BStr m_nm;
	VARTYPE type;
	HRESULT err = m_seq->m_arc->GetPropertyInfo(index, &m_nm, &id, &type);
	if (err == S_OK && m_nm)
		name = m_nm.c_str();
	return err == S_OK;
}

PropVariant WinArchive::const_input_iterator::get_prop(PROPID id) const {
	PropVariant prop;
	m_seq->m_arc->GetProperty(m_index, id, prop.ref());
	return prop;
}

///================================================================================ WinCreateArchive
WinCreateArchive::WinCreateArchive(const SevenZipLib &lib, const AutoUTF &path, const AutoUTF &codec):
	m_lib(lib),
	m_path(path),
	m_codec(codec) {
	CheckCom(m_lib.CreateObject(&m_lib.codecs().at(codec)->guid, &IID_IOutArchive, (void **)&m_arc));
}

void WinCreateArchive::compress() {
	ComObject<ISetProperties> setProperties;
	m_arc->QueryInterface(IID_ISetProperties, (void **)&setProperties);
	if (setProperties) {
		std::vector<PCWSTR> prop_names;
		std::vector<PropVariant> prop_vals;

		prop_names.push_back(L"x"); prop_vals.push_back(PropVariant((UInt32)level));
		if (m_codec == L"7z") {
			prop_names.push_back(L"s"); prop_vals.push_back(PropVariant(solid));
			if (!method.empty())
				prop_names.push_back(L"0"); prop_vals.push_back(PropVariant(method));
		}
		CheckCom(setProperties->SetProperties(&prop_names[0], &prop_vals[0], prop_names.size()));
	}

	ComObject<IOutStream> outFileStream(new FileWriteStream(m_path.c_str(), CREATE_NEW));

	ComObject<IArchiveUpdateCallback2> updateCallback(new ArchiveUpdateCallback(*this));
	CheckCom(m_arc->UpdateItems(outFileStream, size(), updateCallback));
}

ComObject<IOutArchive> WinCreateArchive::operator->() const {
	return m_arc;
}
