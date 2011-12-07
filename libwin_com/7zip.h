/** 7zip.h
 *	@interface to 7-zip
 *	@author		© 2011 Andrew Grechkin
 *	@link (ole32 oleaut32 uuid)
 **/

#ifndef WIN_7ZIP_HPP
#define WIN_7ZIP_HPP

#include <libwin_net/win_net.h>
#include <libwin_net/file.h>
#include "win_com.h"

#include <initguid.h>
#include <shobjidl.h>

#include <7zipAPI/CPP/7zip/Archive/IArchive.h>
#include <7zipAPI/CPP/7zip/IPassword.h>

#include <map>
#include <vector>

using std::vector;
using std::map;
using winstd::shared_ptr;

const UInt64 max_check_start_position = 4096;

typedef vector<BYTE> ByteVector;
typedef ByteVector ArcType;
typedef std::vector<ArcType> ArcTypes;

class SevenZipLib;

///======================================================================================= ArcMethod
struct ArcMethod {
	uint64_t id;
	ustring name;
	ByteVector start_sign, finish_sign;

	bool operator<(const ArcMethod & rhs) const;

	bool operator==(const ArcMethod & rhs) const;

	bool operator!=(const ArcMethod & rhs) const;

private:
	ArcMethod(const SevenZipLib & arc_lib, size_t idx);

	friend class ArcMethods;
};

///====================================================================================== ArcMethods
class ArcMethods: public map<uint64_t, shared_ptr<ArcMethod> > {
public:
	typedef const_iterator iterator;

	void cache(const SevenZipLib & lib);

private:
	ArcMethods();

	ArcMethods(const SevenZipLib & lib);

	friend class SevenZipLib;
};

///======================================================================================== ArcCodec
struct ArcCodec {
	ustring name, ext, add_ext, kAssociate;
	WinGUID guid;
	ByteVector start_sign, finish_sign;
	bool updatable;
	bool kKeepName;

	bool operator<(const ArcCodec & rhs) const;

	bool operator==(const ArcCodec & rhs) const;

	bool operator!=(const ArcCodec & rhs) const;

	ustring default_extension() const;

private:
	ArcCodec(const SevenZipLib & arc_lib, size_t idx);

	friend class ArcCodecs;
};

///======================================================================================= ArcCodecs
struct ArcCodecs: public map<ustring, shared_ptr<ArcCodec> > {
	typedef const_iterator iterator;

	void cache(const SevenZipLib & lib);

	ArcTypes find_by_ext(const ustring & ext) const;

private:
	ArcCodecs();

	ArcCodecs(const SevenZipLib & lib);

	friend class SevenZipLib;
};

///===================================================================================== SevenZipLib
class SevenZipLib: private DynamicLibrary {
	typedef UInt32 (WINAPI *FCreateObject)(const GUID * clsID, const GUID * interfaceID, PVOID * outObject);
	typedef UInt32 (WINAPI *FGetNumberOfMethods)(UInt32 *numMethods);
	typedef UInt32 (WINAPI *FGetMethodProperty)(UInt32 index, PROPID propID, PROPVARIANT * value);
	typedef UInt32 (WINAPI *FGetNumberOfFormats)(UInt32 * numFormats);
	typedef UInt32 (WINAPI *FGetHandlerProperty)(PROPID propID, PROPVARIANT * value);
	typedef UInt32 (WINAPI *FGetHandlerProperty2)(UInt32 index, PROPID propID, PROPVARIANT * value);
	typedef UInt32 (WINAPI *FSetLargePageMode)();

public:
	FCreateObject CreateObject;
	FGetHandlerProperty GetHandlerProperty;
	FGetHandlerProperty2 GetHandlerProperty2;
	FGetMethodProperty GetMethodProperty;
	FGetNumberOfFormats GetNumberOfFormats;
	FGetNumberOfMethods GetNumberOfMethods;
	FSetLargePageMode SetLargePageMode;

	SevenZipLib(const ustring & path);
	const ArcCodecs & codecs() const;
	const ArcMethods & methods() const;

	HRESULT get_prop(UInt32 index, PROPID prop_id, PropVariant & prop) const;
	HRESULT get_prop(UInt32 index, PROPID prop_id, WinGUID & guid) const;
	HRESULT get_prop(UInt32 index, PROPID prop_id, bool & value) const;
	HRESULT get_prop(UInt32 index, PROPID prop_id, ustring & value) const;
	HRESULT get_prop(UInt32 index, PROPID prop_id, ByteVector & value) const;

private:
	ArcCodecs m_codecs;
	ArcMethods m_methods;
};

///======================================================================================= SfxModule
struct SfxModule {
	ustring path;
	ustring description() const;
	bool all_codecs() const;
	bool install_config() const;
};

class SfxModules: public vector<SfxModule> {
public:
	unsigned find_by_name(const ustring & name) const;
};

///=============================================================================== FileReadStream
class FileReadStream: public IInStream, private UnknownImp {
public:
	virtual ~FileReadStream();

	FileReadStream(PCWSTR path);

	// Unknown
	ULONG AddRef() {
		return UnknownImp::AddRef();
	}
	ULONG Release() {
		return UnknownImp::Release();
	}
	virtual HRESULT QueryInterface(REFIID riid, void ** object);

	// ISequentialInStream
	virtual HRESULT Read(void * data, UInt32 size, UInt32 * processedSize);

	// IInStream
	virtual HRESULT Seek(Int64 offset, UInt32 seekOrigin, UInt64 * newPosition);

private:
	void check_device_file();

	WinFile file;
	uint64_t device_pos;
	uint64_t device_size;
	UINT device_sector_size;
	bool device_file;
};

///=================================================================================== FileWriteStream
class FileWriteStream: public IOutStream, private WinFile, private UnknownImp {
public:
	virtual ~FileWriteStream();

	FileWriteStream(const ustring & path, DWORD creat = CREATE_NEW);

	// Unknown
	ULONG AddRef() {
		return UnknownImp::AddRef();
	}
	ULONG Release() {
		return UnknownImp::Release();
	}
	virtual HRESULT QueryInterface(REFIID riid, void ** object);

	// ISequentialOutStream
	virtual HRESULT Write(PCVOID data, UInt32 size, UInt32 * processedSize);

	// IOutStream
	virtual HRESULT Seek(Int64 offset, UInt32 seekOrigin, UInt64 * newPosition);
	virtual HRESULT SetSize(UInt64 newSize);

	using WinFile::set_mtime;
};

///============================================================================= ArchiveOpenCallback
struct ArchiveOpenCallback: public IArchiveOpenCallback, public ICryptoGetTextPassword, private UnknownImp {
	ustring Password;

	virtual ~ArchiveOpenCallback();

	ArchiveOpenCallback();

	// IUnknown
	ULONG AddRef() {
		return UnknownImp::AddRef();
	}
	ULONG Release() {
		return UnknownImp::Release();
	}
	virtual HRESULT QueryInterface(REFIID riid, void ** object);

	// IArchiveOpenCallback
	virtual HRESULT SetTotal(const UInt64 * files, const UInt64 * bytes);
	virtual HRESULT SetCompleted(const UInt64 * files, const UInt64 * bytes);

	// ICryptoGetTextPassword
	virtual HRESULT CryptoGetTextPassword(BSTR * password);
};

///====================================================================================== WinArchive
class WinArchive: private Uncopyable {
public:
	class const_input_iterator;

	typedef WinArchive class_type;
	typedef size_t size_type;
	typedef int flags_type;
	typedef const_input_iterator iterator;
	typedef const_input_iterator const_iterator;

	enum search_flags {
		incDots = 0x0001,
		skipDirs = 0x0002,
		skipFiles = 0x0004,
		skipLinks = 0x0008,
		skipHidden = 0x0010,
	};

	WinArchive(const SevenZipLib & lib, const ustring & path, flags_type flags = 0);

	WinArchive(const SevenZipLib & lib, const ustring & path, const ustring & mask, flags_type flags = 0);

	WinArchive(ComObject<IInArchive> arc, flags_type flags = 0);

	void InitArc(const SevenZipLib & lib);

	void InitProps();

	const ArcCodec &codec() const;

	ComObject<IInArchive> operator->() const;

	const_iterator begin() const;

	const_iterator end() const;

	const_iterator at(size_t index) const;

	const_iterator operator[](int index) const;

	bool empty() const;

	size_t size() const;

	ustring path() const;

	ustring mask() const;

	flags_type flags() const;

	size_t get_num_props() const;

	size_t get_num_item_props() const;

	bool get_prop_info(size_t index, ustring & name, PROPID & id) const;

	PropVariant get_prop(PROPID id) const;

	size_t test() const;

	void extract(const ustring & dest) const;

	operator ComObject<IInArchive>() const;

	static ComObject<IInArchive> open(const SevenZipLib & lib, PCWSTR path);

	class const_input_iterator {
	public:
		typedef const_input_iterator class_type;

		class_type& operator++() {
			flags_type flags = m_seq->flags();
			while (true) {
				if (++m_index >= m_seq->m_size) {
					m_end = true;
					break;
				}

				if ((flags & skipHidden) && ((attr() & FILE_ATTRIBUTE_HIDDEN) == FILE_ATTRIBUTE_HIDDEN)) {
					continue;
				}

				if ((flags & skipDirs) && is_dir()) {
					continue;
				}

				if ((flags & skipFiles) && is_file()) {
					continue;
				}
				break;
			}
			return *this;
		}
		class_type operator++(int) {
			class_type ret(*this);
			operator ++();
			return ret;
		}
//		const value_type operator *() const {
//			return WinArchive::value_type(m_impl->m_seq->path());
//		}

		ustring path() const;

		uint64_t size() const;

		size_t attr() const;

		FILETIME mtime() const;

		bool is_file() const;

		bool is_dir() const;

		bool get_prop_info(size_t index, ustring & name, PROPID & id) const;

		PropVariant get_prop(PROPID id) const;

		bool operator==(const class_type & rhs) const {
			if (m_end && rhs.m_end)
				return true;
			return m_seq == rhs.m_seq && m_index == rhs.m_index;
		}
		bool operator!=(const class_type & rhs) const {
			return !operator==(rhs);
		}

	private:
		const_input_iterator() :
			m_seq(nullptr), m_index(0), m_end(true) {
		}
		const_input_iterator(const WinArchive & seq) :
			m_seq((WinArchive*)&seq), m_index(0), m_end(!m_seq->m_size) {
		}
		const_input_iterator(const WinArchive & seq, UInt32 index) :
			m_seq((WinArchive*)&seq), m_index(index), m_end(!m_seq->m_size || index >= m_seq->m_size) {
//			printf(L"\tconst_input_iterator::const_input_iterator(%d, %d)", m_seq, index);
		}

		WinArchive *m_seq;
		UInt32 m_index;
		bool m_end;
		friend class WinArchive;
	};

private:
	ustring m_path;
	ustring m_mask;
	flags_type m_flags;
	ComObject<IInArchive> m_arc;
	ArcCodecs::const_iterator m_codec;
	UInt32 m_size;
	UInt32 m_num_props;
};

///========================================================================== ArchiveExtractCallback
struct ArchiveExtractCallback: public IArchiveExtractCallback, public ICryptoGetTextPassword, private UnknownImp {
public:
	UInt64 NumErrors;
	ustring Password;

	virtual ~ArchiveExtractCallback();

	ArchiveExtractCallback(const WinArchive & arc, const ustring & dest_path, const ustring & pass = ustring());

	// Unknown
	ULONG AddRef() {
		return UnknownImp::AddRef();
	}
	ULONG Release() {
		return UnknownImp::Release();
	}
	virtual HRESULT QueryInterface(REFIID riid, void ** object);

	// IProgress
	virtual HRESULT SetTotal(UInt64 size);
	virtual HRESULT SetCompleted(const UInt64 * completeValue);

	// IArchiveExtractCallback
	virtual HRESULT GetStream(UInt32 index, ISequentialOutStream ** outStream, Int32 askExtractMode);
	virtual HRESULT PrepareOperation(Int32 askExtractMode);
	virtual HRESULT SetOperationResult(Int32 resultEOperationResult);

	// ICryptoGetTextPassword
	virtual HRESULT CryptoGetTextPassword(BSTR * pass);

private:
	const WinArchive & m_wa;
	ustring m_dest;		// Output directory

	ustring m_diskpath;
	ComObject<FileWriteStream> m_stream;
	size_t m_index;
	Int32 ExtractMode;
};

extern NamedValues<int> ArcItemPropsNames[63];

///=========================================================================== ArchiveUpdateCallback
struct DirItem: public WinFileInfo {
	ustring path;
	ustring name;
	DirItem(const ustring & file_path, const ustring & file_name);
};

class DirStructure: public std::vector<DirItem> {
public:
	typedef const_iterator iterator;

	DirStructure();
	DirStructure(const ustring &path);

	void add(const ustring &add_path);

private:
	void base_add(const ustring &base_path, const ustring &name);
};

struct FailedFile {
	ustring path;
	HRESULT code;

	FailedFile(const ustring & p, HRESULT h):
		path(p),
		code(h) {
	}
};

class ArchiveUpdateCallback: public IArchiveUpdateCallback2, public ICryptoGetTextPassword2, private UnknownImp {
public:
	virtual ~ArchiveUpdateCallback();

	ArchiveUpdateCallback(const DirStructure & items, const ustring & pass = ustring());

	// Unknown
	ULONG AddRef() {
		return UnknownImp::AddRef();
	}
	ULONG Release() {
		return UnknownImp::Release();
	}
	STDMETHOD(QueryInterface)(REFIID riid, void** object);

	// IProgress
	STDMETHOD(SetTotal)(UInt64 size);
	STDMETHOD(SetCompleted)(const UInt64 *completeValue);

	// GetUpdateItemInfo
	STDMETHOD(GetUpdateItemInfo)(UInt32 index, Int32 *newData, Int32 *newProperties, UInt32 *indexInArchive);
	STDMETHOD(GetProperty)(UInt32 index, PROPID propID, PROPVARIANT *value);
	STDMETHOD(GetStream)(UInt32 index, ISequentialInStream **inStream);
	STDMETHOD(SetOperationResult)(Int32 operationResult);

	// IArchiveUpdateCallback2
	STDMETHOD(GetVolumeSize)(UInt32 index, UInt64 *size);
	STDMETHOD(GetVolumeStream)(UInt32 index, ISequentialOutStream **volumeStream);

	// ICryptoGetTextPassword2
	STDMETHOD(CryptoGetTextPassword2)(Int32 *passwordIsDefined, BSTR *password);

public:
	std::vector<FailedFile> failed_files;

	ustring Password;
	bool AskPassword;

private:
	const std::vector<DirItem> & DirItems;

	std::vector<UInt64> VolumesSizes;
	ustring VolName;
	ustring VolExt;
};

///=============================================================================== ArchiveProperties
enum CompressMethod {
	metCopy	= 0,
	metDelta = 3,
	met7zAES = 116459265,
	metARM = 50529537,
	metARMT	= 50530049,
	metBCJ = 50528515,
	metBCJ2	= 50528539,
	metBZip2 = 262658,
	metDeflate = 262408,
	metDeflate64 = 262409,
	metIA64	= 50529281,
	metLZMA	= 196865,
	metLZMA2 = 33,
	metPPC = 50528773,
	metPPMD	= 197633,
	metRar1	= 262913,
	metRar2	= 262914,
	metRar3	= 262915,
	metSPARC = 50530309,
	metSwap2 = 131842,
	metSwap4 = 131844,
};

struct ArchiveProperties {
	size_t level;
	CompressMethod method;
	bool solid;

	ArchiveProperties():
		level(5),
		method(metCopy),
		solid(false) {
	}
};

///================================================================================ WinCreateArchive
class WinCreateArchive: public DirStructure, public ArchiveProperties {
public:
	WinCreateArchive(const SevenZipLib & lib, const ustring & path, const ustring & codec);

	void compress();

	ComObject<IOutArchive> operator->() const;

private:
	void set_properties();

	const SevenZipLib & m_lib;
	const ustring m_path;
	const ustring m_codec;
	ComObject<IOutArchive> m_arc;
};

#endif
