/** win_7zip.hpp
 *	@interface to 7-zip
 *	@author		© 2011 Andrew Grechkin
 *	@link (ole32 oleaut32 uuid)
**/

#ifndef WIN_7ZIP_HPP
#define WIN_7ZIP_HPP

#include <win_com.h>

#include <initguid.h>
#include <shobjidl.h>
#include <CPP/7zip/Archive/IArchive.h>
#include <CPP/7zip/IPassword.h>

#include <assert.h>

#include <list>
#include <map>

using std::list;
using std::map;

const UInt64 max_check_start_position = 512;

#define FACILITY_INTERNAL 0xFFF
#define E_MESSAGE MAKE_HRESULT(SEVERITY_ERROR, FACILITY_INTERNAL, 0)

struct Error {
	HRESULT code;
	list<AutoUTF> messages;
	const char* file;
	int line;
	Error() :
			code(NO_ERROR), file(__FILE__), line(__LINE__) {
	}
	Error(HRESULT code, const char* file, int line) :
			code(code), file(file), line(line) {
	}
	Error(HRESULT code, const AutoUTF& message, const char* file, int line) :
			code(code), messages(1, message), file(file), line(line) {
	}
	Error(const AutoUTF& message, const char* file, int line) :
			code(E_MESSAGE), messages(1, message), file(file), line(line) {
	}
	Error(const AutoUTF& message1, const AutoUTF& message2, const char* file, int line) :
			code(E_MESSAGE), messages(1, message1), file(file), line(line) {
		messages.push_back(message2);
	}
	Error(const std::exception& e) :
			code(E_MESSAGE), file(__FILE__), line(__LINE__) {
		string message(string(typeid(e).name()) + ": " + e.what());
		messages.push_back(AutoUTF(message));
	}
	operator bool() const {
		return code != NO_ERROR;
	}
};

#define FAIL(code) throw Error(code, __FILE__, __LINE__)
#define FAIL_MSG(message) throw Error(message, __FILE__, __LINE__)

#define CHECK_SYS(code) { if (!(code)) FAIL(HRESULT_FROM_WIN32(GetLastError())); }
#define CHECK_ADVSYS(code) { DWORD __ret = (code); if (__ret != ERROR_SUCCESS) FAIL(HRESULT_FROM_WIN32(__ret)); }
#define CHECK_COM(code) { HRESULT __ret = (code); if (FAILED(__ret)) FAIL(__ret); }
#define CHECK(code) { if (!(code)) FAIL_MSG(L#code); }

#define IGNORE_ERRORS(code) { try { code; } catch (...) { } }

#define UNKNOWN_DECL \
	STDMETHOD(QueryInterface)(REFIID riid, void** object); \
	STDMETHOD_(ULONG, AddRef)(); \
	STDMETHOD_(ULONG, Release)();

#define UNKNOWN_IMPL_BEGIN \
	STDMETHOD(QueryInterface)(REFIID riid, void** object) {

#define UNKNOWN_IMPL_ITF(iid) \
	if (riid == IID_##iid) { *object = static_cast<iid*>(this); AddRef(); return S_OK; }

#define UNKNOWN_IMPL_END \
	if (riid == IID_IUnknown) { *object = static_cast<IUnknown*>(static_cast<ComBase*>(this)); AddRef(); return S_OK; } \
	*object = nullptr; return E_NOINTERFACE; \
	} \
	STDMETHOD_(ULONG, AddRef)() { return ++ref_cnt; } \
	STDMETHOD_(ULONG, Release)() { if (--ref_cnt == 0) { delete this; return 0; } else return ref_cnt; }

#define COM_ERROR_HANDLER_BEGIN \
	try { \
		try {

#define COM_ERROR_HANDLER_END \
	} \
	catch (const Error& e) { \
		g_com_error = e; \
	} \
	catch (const std::exception& e) { \
		g_com_error = e; \
	} \
	} \
	catch (...) { \
		g_com_error.code = E_FAIL; \
	} \
	return g_com_error.code;

#define COM_ERROR_CHECK(code) { \
		g_com_error = Error(); \
		HRESULT __res = (code); \
		if (FAILED(__res)) { \
			if (g_com_error) \
				throw g_com_error; \
			else \
				FAIL(__res); \
		} \
	}

Error g_com_error;

typedef vector<BYTE> ByteVector;

struct SevenZipLib: private DynamicLibrary {
	typedef UInt32 (WINAPI *FCreateObject)(const GUID *clsID, const GUID *interfaceID, PVOID *outObject);
	typedef UInt32 (WINAPI *FGetNumberOfMethods)(UInt32 *numMethods);
	typedef UInt32 (WINAPI *FGetMethodProperty)(UInt32 index, PROPID propID, PROPVARIANT *value);
	typedef UInt32 (WINAPI *FGetNumberOfFormats)(UInt32 *numFormats);
	typedef UInt32 (WINAPI *FGetHandlerProperty)(PROPID propID, PROPVARIANT *value);
	typedef UInt32 (WINAPI *FGetHandlerProperty2)(UInt32 index, PROPID propID, PROPVARIANT *value);
	typedef UInt32 (WINAPI *FSetLargePageMode)();

	FCreateObject CreateObject;
	FGetNumberOfMethods GetNumberOfMethods;
	FGetMethodProperty GetMethodProperty;
	FGetNumberOfFormats GetNumberOfFormats;
	FGetHandlerProperty GetHandlerProperty;
	FGetHandlerProperty2 GetHandlerProperty2;

	SevenZipLib(const AutoUTF &path) : DynamicLibrary(path) {
		CreateObject = (FCreateObject)get_function_nothrow("CreateObject");
		GetNumberOfMethods = (FGetNumberOfMethods)get_function_nothrow("GetNumberOfMethods");
		GetMethodProperty = (FGetMethodProperty)get_function_nothrow("GetMethodProperty");
		GetNumberOfFormats = (FGetNumberOfFormats)get_function_nothrow("GetNumberOfFormats");
		GetHandlerProperty = (FGetHandlerProperty)get_function_nothrow("GetHandlerProperty");
		GetHandlerProperty2 = (FGetHandlerProperty2)get_function_nothrow("GetHandlerProperty2");
		if (CreateObject && ((GetNumberOfFormats && GetHandlerProperty2) || GetHandlerProperty)) {
			return;
		}
		CheckApiError(ERROR_INVALID_LIBRARY);
	}

	HRESULT get_prop(UInt32 index, PROPID prop_id, PROPVARIANT &prop) const {
		if (GetHandlerProperty2) {
			return GetHandlerProperty2(index, prop_id, &prop);
		} else {
			assert(index == 0);
			return GetHandlerProperty(prop_id, &prop);
		}
	}

	HRESULT get_prop(UInt32 index, PROPID prop_id, WinGUID& guid) const {
		PropVariant prop;
		HRESULT res = get_prop(index, prop_id, prop);
		if (res != S_OK)
			return res;
//		if (prop.vt == VT_BSTR) {
//			size_t len = SysStringByteLen(prop.bstrVal);
//			if (len != sizeof(value))
//				return E_FAIL;
//			WinMem::Copy(&value, prop.bstrVal, len);
//		} else
//			return E_FAIL;
		guid.init(prop);
		return S_OK;
	}

	HRESULT get_bool_prop(UInt32 index, PROPID prop_id, bool &value) const {
		PropVariant prop;
		HRESULT res = get_prop(index, prop_id, prop);
		if (res != S_OK)
			return res;
		if (!prop.is_bool())
			return E_FAIL;
		value = prop.as_bool();
		return S_OK;
	}

	HRESULT get_string_prop(UInt32 index, PROPID prop_id, AutoUTF& value) const {
		PropVariant prop;
		HRESULT res = get_prop(index, prop_id, prop);
		if (res != S_OK)
			return res;
		if (!prop.is_str())
			return E_FAIL;
		value = prop.as_str();
		return S_OK;
	}

	HRESULT get_bytes_prop(UInt32 index, PROPID prop_id, ByteVector& value) const {
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
};

typedef ByteVector ArcType;
typedef list<ArcType> ArcTypes;

struct ArcFormat {
	AutoUTF name, ext, add_ext;
	WinGUID guid;
	ByteVector start_signature;
	bool updatable;

	ArcFormat(const SevenZipLib &arc_lib, size_t idx): updatable(false) {
		CheckApiError(arc_lib.get_prop(idx, NArchive::kClassID, guid));
		arc_lib.get_string_prop(idx, NArchive::kName, name);
		arc_lib.get_string_prop(idx, NArchive::kExtension, ext);
		arc_lib.get_string_prop(idx, NArchive::kAddExtension, add_ext);
		arc_lib.get_bytes_prop(idx, NArchive::kStartSignature, start_signature);
		arc_lib.get_bool_prop(idx, NArchive::kUpdate, updatable);
	}

	AutoUTF default_extension() const;
};

class ArcFormats: public map<ArcType, ArcFormat> {
public:
	ArcTypes get_arc_types() const;
	ArcTypes find_by_name(const AutoUTF& name) const;
	ArcTypes find_by_ext(const AutoUTF& ext) const;
};

struct SfxModule {
	AutoUTF path;
	AutoUTF description() const;
	bool all_codecs() const;
	bool install_config() const;
};

class SfxModules: public vector<SfxModule> {
public:
	unsigned find_by_name(const AutoUTF& name) const;
};

typedef vector<SevenZipLib> ArcLibs;

class ArcAPI {
public:
	static const ArcLibs& libs() {
		return get()->arc_libs;
	}
	static const ArcFormats& formats() {
		return get()->arc_formats;
	}
	static const SfxModules& sfx() {
		return get()->sfx_modules;
	}
	static void create_in_archive(const ArcType& arc_type, IInArchive** in_arc);
	static void create_out_archive(const ArcType& format, IOutArchive** out_arc);
	static void free();
	ArcAPI() {
	}
	~ArcAPI() {
	}
	void load_libs(const AutoUTF& path) {
		SevenZipLib arc_lib(L"C:\\7z.dll");
	}
	void load() {
		load_libs(L"C:\\7z.dll");
		for (unsigned i = 0; i < arc_libs.size(); i++) {
			const SevenZipLib& arc_lib = arc_libs[i];

			UInt32 num_formats;
			if (arc_lib.GetNumberOfFormats) {
				if (arc_lib.GetNumberOfFormats(&num_formats) != S_OK)
					num_formats = 0;
			} else
				num_formats = 1;

			for (UInt32 idx = 0; idx < num_formats; idx++) {
//				ArcFormat arc_format;
//				ArcType type;
//				if (arc_lib.get_bytes_prop(idx, NArchive::kClassID, type) != S_OK)
//					continue;
//				arc_lib.get_string_prop(idx, NArchive::kName, arc_format.name);
//				wcout << "Name: " << utf8(arc_format.name) << endl;
//				if (arc_lib.get_bool_prop(idx, NArchive::kUpdate, arc_format.updatable) != S_OK)
//					arc_format.updatable = false;
//				arc_lib.get_bytes_prop(idx, NArchive::kStartSignature, arc_format.start_signature);
//				arc_lib.get_string_prop(idx, NArchive::kExtension, arc_format.ext);
//				wcout << "ext: " << utf8(arc_format.ext) << endl;
//				ArcFormats::const_iterator existing_format = arc_formats.find(type);
			}
		}
	}
private:
	ArcLibs arc_libs;
	ArcFormats arc_formats;
	SfxModules sfx_modules;
	static ArcAPI* arc_api;

	static ArcAPI* get();

	void find_sfx_modules(const AutoUTF& path);
};

#define CHECK_FILE(code) { if (!(code)) throw Error(HRESULT_FROM_WIN32(GetLastError()), file_path, __FILE__, __LINE__); }
class File: private Uncopyable {
protected:
	HANDLE h_file;
	AutoUTF file_path;
public:
	File(): h_file(INVALID_HANDLE_VALUE) {
	}
	~File() {
		close();
	}
	File(const AutoUTF& file_path, DWORD desired_access, DWORD share_mode, DWORD creation_disposition, DWORD flags_and_attributes);
	void open(const AutoUTF& file_path, DWORD desired_access, DWORD share_mode, DWORD creation_disposition, DWORD flags_and_attributes) {
		CHECK_FILE(open_nt(file_path, desired_access, share_mode, creation_disposition, flags_and_attributes));
	}
	bool open_nt(const AutoUTF& file_path, DWORD desired_access, DWORD share_mode, DWORD creation_disposition, DWORD flags_and_attributes) {
		close();
		this->file_path = file_path;
		h_file = CreateFileW(file_path.c_str(), desired_access, share_mode, nullptr, creation_disposition, flags_and_attributes, nullptr);
		return h_file != INVALID_HANDLE_VALUE;

	}
	void close() {
		if (h_file != INVALID_HANDLE_VALUE) {
			CloseHandle(h_file);
			h_file = INVALID_HANDLE_VALUE;
		}
	}
	bool is_open() const {
		return h_file != INVALID_HANDLE_VALUE;
	}
	HANDLE handle() const {
		return h_file;
	}
	const AutoUTF& path() const {
		return file_path;
	}
	uint64_t size();
	bool size_nt(uint64_t& file_size) {
		LARGE_INTEGER fs;
		if (GetFileSizeEx(h_file, &fs)) {
			file_size = fs.QuadPart;
			return true;
		} else
			return false;
	}
	unsigned read(void* data, unsigned size) {
		unsigned size_read;
		CHECK_FILE(read_nt(data, size, size_read));
		return size_read;
	}
	bool read_nt(void* data, unsigned size, unsigned& size_read) {
		DWORD sz;
		if (ReadFile(h_file, data, size, &sz, nullptr)) {
			size_read = sz;
			return true;
		} else
			return false;
	}
	unsigned write(const void* data, unsigned size);
	bool write_nt(const void* data, unsigned size, unsigned& size_written);
	void set_time(const FILETIME& ctime, const FILETIME& atime, const FILETIME& mtime);
	bool set_time_nt(const FILETIME& ctime, const FILETIME& atime, const FILETIME& mtime);
	uint64_t set_pos(int64_t offset, DWORD method = FILE_BEGIN) {
		uint64_t new_pos;
		CHECK_FILE(set_pos_nt(offset, method, &new_pos));
		return new_pos;
	}
	bool set_pos_nt(int64_t offset, DWORD method = FILE_BEGIN, uint64_t* new_pos = nullptr) {
		LARGE_INTEGER distance_to_move, new_file_pointer;
		distance_to_move.QuadPart = offset;
		if (!SetFilePointerEx(h_file, distance_to_move, &new_file_pointer, method))
			return false;
		if (new_pos)
			*new_pos = new_file_pointer.QuadPart;
		return true;
	}
	void set_end();
	bool set_end_nt() throw();
	BY_HANDLE_FILE_INFORMATION get_info();
	bool get_info_nt(BY_HANDLE_FILE_INFORMATION& info) throw();
	template<typename Type> bool io_control_out_nt(DWORD code, Type& data) throw() {
		DWORD size_ret;
		return DeviceIoControl(h_file, code, nullptr, 0, &data, sizeof(Type), &size_ret, nullptr) != 0;
	}
	static bool exists(const AutoUTF& file_path) throw();
	static void set_attr(const AutoUTF& file_path, DWORD attr);
	static bool set_attr_nt(const AutoUTF& file_path, DWORD attr) throw();
	static void delete_file(const AutoUTF& file_path);
	static bool delete_file_nt(const AutoUTF& file_path) throw();
	static void create_dir(const AutoUTF& dir_path);
	static bool create_dir_nt(const AutoUTF& dir_path) throw();
	static void remove_dir(const AutoUTF& file_path);
	static bool remove_dir_nt(const AutoUTF& file_path) throw();
	static void move_file(const AutoUTF& file_path, const AutoUTF& new_path, DWORD flags);
	static bool move_file_nt(const AutoUTF& file_path, const AutoUTF& new_path, DWORD flags) throw();
};

DWORD translate_seek_method(UInt32 seek_origin) {
	DWORD method;
	switch (seek_origin) {
		case STREAM_SEEK_SET:
			method = FILE_BEGIN;
			break;
		case STREAM_SEEK_CUR:
			method = FILE_CURRENT;
			break;
		case STREAM_SEEK_END:
			method = FILE_END;
			break;
		default:
			FAIL(E_INVALIDARG);
	}
	return method;
}

class ArchiveOpenStream: public IInStream, private ComBase, private File {
public:
	ArchiveOpenStream(const AutoUTF& file_path) {
		open(file_path, FILE_GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN);
		check_device_file();
	}

	UNKNOWN_IMPL_BEGIN UNKNOWN_IMPL_ITF(ISequentialInStream)
	UNKNOWN_IMPL_ITF(IInStream)
	UNKNOWN_IMPL_END

	STDMETHODIMP Read(void *data, UInt32 size, UInt32 *processedSize) {
		COM_ERROR_HANDLER_BEGIN
		unsigned size_read;
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
			set_pos(aligned_pos, FILE_BEGIN);
			size_read = read(alligned_buffer, aligned_size);
			if (size_read < aligned_offset)
				size_read = 0;
			else
				size_read -= aligned_offset;
			if (size_read > size)
				size_read = size;
			device_pos += size_read;
			memcpy(data, alligned_buffer + aligned_offset, size_read);
		} else {
			size_read = read(data, size);
		}
		if (processedSize)
			*processedSize = size_read;
		return S_OK;
		COM_ERROR_HANDLER_END
	}

	STDMETHODIMP Seek(Int64 offset, UInt32 seekOrigin, UInt64 *newPosition) {
		COM_ERROR_HANDLER_BEGIN
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
					FAIL(E_INVALIDARG);
			}
			new_position = device_pos;
		} else {
			new_position = set_pos(offset, translate_seek_method(seekOrigin));
		}
		if (newPosition)
			*newPosition = new_position;
		return S_OK;
		COM_ERROR_HANDLER_END
	}

private:
	void check_device_file() {
		device_pos = 0;
		device_file = false;
		if (size_nt(device_size))
			return;

		PARTITION_INFORMATION part_info;
		if (io_control_out_nt(IOCTL_DISK_GET_PARTITION_INFO, part_info)) {
			device_size = part_info.PartitionLength.QuadPart;
			DWORD sectors_per_cluster, bytes_per_sector, number_of_free_clusters,
			total_number_of_clusters;
			if (GetDiskFreeSpaceW(add_trailing_slash(path()).c_str(), &sectors_per_cluster,
								  &bytes_per_sector, &number_of_free_clusters,
								  &total_number_of_clusters))
				device_sector_size = bytes_per_sector;
			else
				device_sector_size = 4096;
			device_file = true;
			return;
		}

		DISK_GEOMETRY disk_geometry;
		if (io_control_out_nt(IOCTL_DISK_GET_DRIVE_GEOMETRY, disk_geometry)) {
			device_size = disk_geometry.Cylinders.QuadPart * disk_geometry.TracksPerCylinder
						  * disk_geometry.SectorsPerTrack * disk_geometry.BytesPerSector;
			device_sector_size = disk_geometry.BytesPerSector;
			device_file = true;
			return;
		}
	}

	uint64_t device_pos;
	uint64_t device_size;
	UINT device_sector_size;
	bool device_file;
};

#endif
