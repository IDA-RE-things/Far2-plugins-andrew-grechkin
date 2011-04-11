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

#include <assert.h>

#include <list>
#include <map>
#include <vector>

using std::vector;
using std::list;
using std::map;

const UInt64 max_check_start_position = 4096;

typedef vector<BYTE> ByteVector;
typedef ByteVector ArcType;
typedef std::vector<ArcType> ArcTypes;

//static NamedValues<int> ArcItemPropsNames[] = {
//	{L"kpidNoProperty =    ", kpidNoProperty},
//	{L"kpidMainSubfile =   ", kpidMainSubfile},
//	{L"kpidHandlerItemIndex", kpidHandlerItemIndex},
//	{L"kpidPath,           ", kpidPath},
//	{L"kpidName,           ", kpidName},
//	{L"kpidExtension,      ", kpidExtension},
//	{L"kpidIsDir,          ", kpidIsDir},
//	{L"kpidSize,           ", kpidSize},
//	{L"kpidPackSize,       ", kpidPackSize},
//	{L"kpidAttrib,         ", kpidAttrib},
//	{L"kpidCTime,          ", kpidCTime},
//	{L"kpidATime,          ", kpidATime},
//	{L"kpidMTime,          ", kpidMTime},
//	{L"kpidSolid,          ", kpidSolid},
//	{L"kpidCommented,      ", kpidCommented},
//	{L"kpidEncrypted,      ", kpidEncrypted},
//	{L"kpidSplitBefore,    ", kpidSplitBefore},
//	{L"kpidSplitAfter,     ", kpidSplitAfter},
//	{L"kpidDictionarySize, ", kpidDictionarySize},
//	{L"kpidCRC,            ", kpidCRC},
//	{L"kpidType,           ", kpidType},
//	{L"kpidIsAnti,         ", kpidIsAnti},
//	{L"kpidMethod,         ", kpidMethod},
//	{L"kpidHostOS,         ", kpidHostOS},
//	{L"kpidFileSystem,     ", kpidFileSystem},
//	{L"kpidUser,           ", kpidUser},
//	{L"kpidGroup,          ", kpidGroup},
//	{L"kpidBlock,          ", kpidBlock},
//	{L"kpidComment,        ", kpidComment},
//	{L"kpidPosition,       ", kpidPosition},
//	{L"kpidPrefix,         ", kpidPrefix},
//	{L"kpidNumSubDirs,     ", kpidNumSubDirs},
//	{L"kpidNumSubFiles,    ", kpidNumSubFiles},
//	{L"kpidUnpackVer,      ", kpidUnpackVer},
//	{L"kpidVolume,         ", kpidVolume},
//	{L"kpidIsVolume,       ", kpidIsVolume},
//	{L"kpidOffset,         ", kpidOffset},
//	{L"kpidLinks,          ", kpidLinks},
//	{L"kpidNumBlocks,      ", kpidNumBlocks},
//	{L"kpidNumVolumes,     ", kpidNumVolumes},
//	{L"kpidTimeType,       ", kpidTimeType},
//	{L"kpidBit64,          ", kpidBit64},
//	{L"kpidBigEndian,      ", kpidBigEndian},
//	{L"kpidCpu,            ", kpidCpu},
//	{L"kpidPhySize,        ", kpidPhySize},
//	{L"kpidHeadersSize,    ", kpidHeadersSize},
//	{L"kpidChecksum,       ", kpidChecksum},
//	{L"kpidCharacts,       ", kpidCharacts},
//	{L"kpidVa,             ", kpidVa},
//	{L"kpidId,             ", kpidId},
//	{L"kpidShortName,      ", kpidShortName},
//	{L"kpidCreatorApp,     ", kpidCreatorApp},
//	{L"kpidSectorSize,     ", kpidSectorSize},
//	{L"kpidPosixAttrib,    ", kpidPosixAttrib},
//	{L"kpidLink,           ", kpidLink},
//	{L"kpidError,          ", kpidError},
//
//	{L"kpidTotalSize =     ", kpidTotalSize},
//	{L"kpidFreeSpace,      ", kpidFreeSpace},
//	{L"kpidClusterSize,    ", kpidClusterSize},
//	{L"kpidVolumeName,     ", kpidVolumeName},
//
//	{L"kpidLocalName =     ", kpidLocalName},
//	{L"kpidProvider,       ", kpidProvider},
//
//	{L"kpidUserDefined =   ", kpidUserDefined},
//};

class SevenZipLib;
///======================================================================================== ArcCodec
struct ArcCodec {
	AutoUTF name, ext, add_ext, kAssociate;
	WinGUID guid;
	ByteVector start_sign, finish_sign;
	bool updatable;
	bool kKeepName;

	ArcCodec(const SevenZipLib &arc_lib, size_t idx);

	bool operator<(const ArcCodec &rhs) const;

	bool operator==(const ArcCodec &rhs) const;

	bool operator!=(const ArcCodec &rhs) const;

	AutoUTF default_extension() const;
};

///======================================================================================= ArcCodecs
struct ArcCodecs: public map<AutoUTF, winstd::shared_ptr<ArcCodec> > {
	typedef const_iterator iterator;

	ArcCodecs();

	ArcCodecs(const SevenZipLib &lib);

	void cache(const SevenZipLib &lib);

	ArcTypes find_by_ext(const AutoUTF& ext) const;
};

///======================================================================================= ArcMethod
struct ArcMethod {
	uint64_t id;
	AutoUTF name;
	ByteVector start_sign, finish_sign;

	ArcMethod(const SevenZipLib &arc_lib, size_t idx);

	bool operator<(const ArcMethod &rhs) const;

	bool operator==(const ArcMethod &rhs) const;

	bool operator!=(const ArcMethod &rhs) const;
};

///====================================================================================== ArcMethods
struct ArcMethods: public map<AutoUTF, winstd::shared_ptr<ArcMethod> > {
	typedef const_iterator iterator;

	ArcMethods();

	ArcMethods(const SevenZipLib &lib);

	void cache(const SevenZipLib &lib);
};

///===================================================================================== SevenZipLib
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
	ArcCodecs	codecs;

	SevenZipLib(const AutoUTF &path);

	HRESULT get_prop(UInt32 index, PROPID prop_id, PROPVARIANT &prop) const;

	HRESULT get_prop(UInt32 index, PROPID prop_id, WinGUID& guid) const;

	HRESULT get_prop(UInt32 index, PROPID prop_id, bool &value) const;

	HRESULT get_prop(UInt32 index, PROPID prop_id, AutoUTF& value) const;

	HRESULT get_prop(UInt32 index, PROPID prop_id, ByteVector& value) const;
};

///====================================================================================== WinArchive
class WinArchive {
public:
	class const_input_iterator;

	typedef WinArchive				class_type;
	typedef size_t					size_type;
	typedef int						flags_type;
	typedef const_input_iterator	iterator;
	typedef const_input_iterator	const_iterator;

	enum search_flags {
		incDots			=   0x0001,
		skipDirs		=   0x0002,
		skipFiles		=   0x0004,
		skipLinks		=   0x0008,
		skipHidden		=   0x0010,
	};

	WinArchive(const SevenZipLib &lib, const AutoUTF &path, flags_type flags = 0):
		m_path(path),
		m_mask(L"*"),
		m_flags(flags),
		m_arc(open(lib, m_path.c_str())) {
		Init();
	}

	WinArchive(const SevenZipLib &lib, const AutoUTF &path, const AutoUTF &mask, flags_type flags = 0):
		m_path(path),
		m_mask(mask),
		m_flags(flags),
		m_arc(open(lib, m_path.c_str())) {
		Init();
	}

	WinArchive(ComObject<IInArchive> arc, flags_type flags = 0):
		m_mask(L"*"),
		m_flags(flags),
		m_arc(arc) {
		Init();
	}

	void Init() {
		m_arc->GetNumberOfItems(&m_size);
		m_arc->GetNumberOfArchiveProperties(&m_num_props);
	}

	ComObject<IInArchive> operator->() const {
		return m_arc;
	}

	const_iterator begin() const {
		return const_iterator(*this);
	}

	const_iterator end() const {
		return const_iterator();
	}

	const_iterator operator[](int index) const {
		return const_iterator(*this, index);
	}

	bool empty() const {
		return m_size == 0;
	}

	size_t size() const {
		return m_size;
	}

	AutoUTF path() const {
		return m_path;
	}

	AutoUTF mask() const {
		return m_mask;
	}

	flags_type flags() const {
		return m_flags;
	}

	size_t get_num_props() const {
		return m_num_props;
	}

	size_t get_num_item_props() const {
		UInt32 props = 0;
		m_arc->GetNumberOfProperties(&props);
		return props;
	}

	bool get_prop_info(size_t index, AutoUTF &name, PROPID &id) const {
		BStr m_nm;
		VARTYPE type;
		HRESULT err = m_arc->GetArchivePropertyInfo(index, &m_nm, &id, &type);
		if (err == S_OK && m_nm)
			name = m_nm.c_str();
		return err == S_OK;
	}

	PropVariant get_prop(PROPID id) const {
		PropVariant prop;
		m_arc->GetArchiveProperty(id, prop.ref());
		return prop;
	}

	size_t test() const;

	bool extract(const AutoUTF &dest) const;

	operator ComObject<IInArchive>() const {
		return m_arc;
	}

	static ComObject<IInArchive> open(const SevenZipLib &lib, const AutoUTF &path);

	class const_input_iterator {
	public:
		typedef const_input_iterator	class_type;

		class_type& operator++() {
			flags_type flags = m_seq->flags();
			while (true) {
				if (++m_index >= m_seq->m_size) {
					m_end = true;
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
			class_type  ret(*this);
			operator ++();
			return ret;
		}
//		const value_type operator *() const {
//			return WinArchive::value_type(m_impl->m_seq->path());
//		}

		AutoUTF path() const {
			PropVariant prop;
			m_seq->m_arc->GetProperty(m_index, kpidPath, prop.ref());
			return prop.as_str();
		}

		uint64_t size() const {
			PropVariant prop;
			m_seq->m_arc->GetProperty(m_index, kpidSize, prop.ref());
			return prop.as_uint();
		}

		size_t attr() const {
			PropVariant prop;
			m_seq->m_arc->GetProperty(m_index, kpidAttrib, prop.ref());
			return prop.as_uint();
		}

		FILETIME mtime() const {
			PropVariant prop;
			m_seq->m_arc->GetProperty(m_index, kpidMTime, prop.ref());
			return prop.as_time();
		}

		bool is_file() const {
			return !is_dir();
		}

		bool is_dir() const {
			PropVariant prop;
			m_seq->m_arc->GetProperty(m_index, kpidIsDir, prop.ref());
			return prop.as_bool();
		}

		bool get_prop_info(size_t index, AutoUTF &name, PROPID &id) const {
			BStr m_nm;
			VARTYPE type;
			HRESULT err = m_seq->m_arc->GetPropertyInfo(index, &m_nm, &id, &type);
			if (err == S_OK && m_nm)
				name = m_nm.c_str();
			return err == S_OK;
		}

		PropVariant get_prop(PROPID id) const {
			PropVariant prop;
			m_seq->m_arc->GetProperty(m_index, id, prop.ref());
			return prop;
		}

		bool operator==(const class_type &rhs) const {
			if (m_end && rhs.m_end)
				return true;
			return m_seq == rhs.m_seq && m_index == rhs.m_index;
		}
		bool operator!=(const class_type &rhs) const {
			return !operator==(rhs);
		}

	private:
		const_input_iterator():
			m_seq(nullptr),
			m_index(0),
			m_end(true) {
		}
		const_input_iterator(const WinArchive &seq):
			m_seq((WinArchive*)&seq),
			m_index(0),
			m_end(!m_seq->m_size) {
		}
		const_input_iterator(const WinArchive &seq, UInt32 index):
			m_seq((WinArchive*)&seq),
			m_index(index),
			m_end(!m_seq->m_size || index >= m_seq->m_size) {
		}

		WinArchive *m_seq;
		UInt32 m_index;
		bool m_end;
		friend class WinArchive;
	};

private:
	WinArchive(const class_type&);  // deny copy constructor and operator =
	class_type& operator=(const class_type&);

	AutoUTF 	m_path;
	AutoUTF 	m_mask;
	flags_type	m_flags;
	ComObject<IInArchive> m_arc;
	UInt32		m_size;
	UInt32		m_num_props;
};

///=============================================================================== FileReadStream
class FileReadStream: public IInStream, private ComBase {
public:
	~FileReadStream();

	FileReadStream(const AutoUTF& path);

	STDMETHOD(QueryInterface)(REFIID riid, void** object);

	STDMETHOD_(ULONG, AddRef)();

	STDMETHOD_(ULONG, Release)();

	STDMETHODIMP Read(void *data, UInt32 size, UInt32 *processedSize);

	STDMETHODIMP Seek(Int64 offset, UInt32 seekOrigin, UInt64 *newPosition);

private:
	void check_device_file();

	WinFile file;
	uint64_t device_pos;
	uint64_t device_size;
	UINT device_sector_size;
	bool device_file;
};

///=================================================================================== FileWriteStream
class FileWriteStream: public IOutStream, public WinFile, private ComBase {
public:
	virtual ~FileWriteStream();

	FileWriteStream(PCWSTR fileName, bool rewrite = false);

	STDMETHOD(QueryInterface)(REFIID riid, void** object);

	STDMETHOD_(ULONG, AddRef)();

	STDMETHOD_(ULONG, Release)();

	STDMETHOD(Write)(PCVOID data, UInt32 size, UInt32 *processedSize);

	STDMETHOD(Seek)(Int64 offset, UInt32 seekOrigin, UInt64 *newPosition);

	STDMETHOD(SetSize)(UInt64 newSize);

private:
	UInt64 ProcessedSize;
};

///============================================================================= ArchiveOpenCallback
struct ArchiveOpenCallback: public IArchiveOpenCallback, public ICryptoGetTextPassword, private ComBase {
	ArchiveOpenCallback();

	STDMETHOD(QueryInterface)(REFIID riid, void** object);

	STDMETHOD_(ULONG, AddRef)();

	STDMETHOD_(ULONG, Release)();

	STDMETHOD(SetTotal)(const UInt64 *files, const UInt64 *bytes);

	STDMETHOD(SetCompleted)(const UInt64 *files, const UInt64 *bytes);

	STDMETHOD(CryptoGetTextPassword)(BSTR *password);

	bool PasswordIsDefined;
	AutoUTF Password;
};

///========================================================================== ArchiveExtractCallback
struct ArchiveExtractCallback: public IArchiveExtractCallback, public ICryptoGetTextPassword, private ComBase {
public:
	~ArchiveExtractCallback();

	ArchiveExtractCallback(const WinArchive &arc, const AutoUTF &dest_path, const AutoUTF &pass = AutoUTF());

	STDMETHOD(QueryInterface)(REFIID riid, void** object);

	STDMETHOD_(ULONG, AddRef)();

	STDMETHOD_(ULONG, Release)();

	// IProgress
	STDMETHOD(SetTotal)(UInt64 size);
	STDMETHOD(SetCompleted)(const UInt64 *completeValue);

	// IArchiveExtractCallback
	STDMETHOD(GetStream)(UInt32 index, ISequentialOutStream **outStream, Int32 askExtractMode);
	STDMETHOD(PrepareOperation)(Int32 askExtractMode);
	STDMETHOD(SetOperationResult)(Int32 resultEOperationResult);

	// ICryptoGetTextPassword
	STDMETHOD(CryptoGetTextPassword)(BSTR *pass);

	UInt64 NumErrors;
	AutoUTF Password;

private:
	const WinArchive &m_wa;
	AutoUTF m_dest;		// Output directory

	AutoUTF m_diskpath;
	ComObject<ISequentialOutStream> m_stream;
	size_t m_index;
	Int32 ExtractMode;
};

///=========================================================================== ArchiveUpdateCallback
struct DirItem: public WinFileInfo {
	AutoUTF path;
	AutoUTF name;
	DirItem(const AutoUTF &file_path, const AutoUTF &file_name);
};

class DirStructure: public std::vector<DirItem> {
public:
	typedef const_iterator iterator;

	DirStructure();
	DirStructure(const AutoUTF &path);

	void add(const AutoUTF &add_path);

private:
	void base_add(const AutoUTF &base_path, const AutoUTF &name);
};

class ArchiveUpdateCallback: public IArchiveUpdateCallback2, public ICryptoGetTextPassword2, private ComBase {
public:
	~ArchiveUpdateCallback();

	ArchiveUpdateCallback(const DirStructure &items, const AutoUTF &pass = AutoUTF());

	STDMETHOD(QueryInterface)(REFIID riid, void** object);
	STDMETHOD_(ULONG, AddRef)();
	STDMETHOD_(ULONG, Release)();

	// IProgress
	STDMETHOD(SetTotal)(UInt64 size);
	STDMETHOD(SetCompleted)(const UInt64 *completeValue);

	// IUpdateCallback2
	STDMETHOD(GetUpdateItemInfo)(UInt32 index, Int32 *newData, Int32 *newProperties, UInt32 *indexInArchive);
	STDMETHOD(GetProperty)(UInt32 index, PROPID propID, PROPVARIANT *value);
	STDMETHOD(EnumProperties)(IEnumSTATPROPSTG **enumerator);
	STDMETHOD(GetStream)(UInt32 index, ISequentialInStream **inStream);
	STDMETHOD(SetOperationResult)(Int32 operationResult);
	STDMETHOD(GetVolumeSize)(UInt32 index, UInt64 *size);
	STDMETHOD(GetVolumeStream)(UInt32 index, ISequentialOutStream **volumeStream);

	STDMETHOD(CryptoGetTextPassword2)(Int32 *passwordIsDefined, BSTR *password);

	HRESULT Finilize();

public:
	std::vector<AutoUTF> FailedFiles;
	std::vector<HRESULT> FailedCodes;

	AutoUTF Password;
	bool AskPassword;

private:
	const std::vector<DirItem> &DirItems;

	std::vector<UInt64> VolumesSizes;
	AutoUTF VolName;
	AutoUTF VolExt;

};

///======================================================================================= SfxModule
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
	static const ArcCodecs& formats() {
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
	void load_libs(const AutoUTF& /*path*/) {
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
	ArcCodecs arc_formats;
	SfxModules sfx_modules;
	static ArcAPI* arc_api;

	static ArcAPI* get();

	void find_sfx_modules(const AutoUTF& path);
};

#endif
