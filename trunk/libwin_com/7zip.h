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
class SevenZipLib: private DynamicLibrary {
	typedef UInt32 (WINAPI *FCreateObject)(const GUID *clsID, const GUID *interfaceID, PVOID *outObject);
	typedef UInt32 (WINAPI *FGetNumberOfMethods)(UInt32 *numMethods);
	typedef UInt32 (WINAPI *FGetMethodProperty)(UInt32 index, PROPID propID, PROPVARIANT *value);
	typedef UInt32 (WINAPI *FGetNumberOfFormats)(UInt32 *numFormats);
	typedef UInt32 (WINAPI *FGetHandlerProperty)(PROPID propID, PROPVARIANT *value);
	typedef UInt32 (WINAPI *FGetHandlerProperty2)(UInt32 index, PROPID propID, PROPVARIANT *value);
	typedef UInt32 (WINAPI *FSetLargePageMode)();

	ArcCodecs	m_codecs;
public:
	FCreateObject CreateObject;
	FGetNumberOfMethods GetNumberOfMethods;
	FGetMethodProperty GetMethodProperty;
	FGetNumberOfFormats GetNumberOfFormats;
	FGetHandlerProperty GetHandlerProperty;
	FGetHandlerProperty2 GetHandlerProperty2;

	SevenZipLib(const AutoUTF &path);
	const ArcCodecs &codecs() const;

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

	WinArchive(const SevenZipLib &lib, const AutoUTF &path, flags_type flags = 0);

	WinArchive(const SevenZipLib &lib, const AutoUTF &path, const AutoUTF &mask, flags_type flags = 0);

	WinArchive(ComObject<IInArchive> arc, flags_type flags = 0);

	void InitArc(const SevenZipLib &lib);

	void InitProps();

	const ArcCodec &codec() const;

	ComObject<IInArchive> operator->() const;

	const_iterator begin() const;

	const_iterator end() const;

	const_iterator at(size_t index) const;

	const_iterator operator[](int index) const;

	bool empty() const ;

	size_t size() const;

	AutoUTF path() const;

	AutoUTF mask() const;

	flags_type flags() const;

	size_t get_num_props() const;

	size_t get_num_item_props() const;

	bool get_prop_info(size_t index, AutoUTF &name, PROPID &id) const;

	PropVariant get_prop(PROPID id) const;

	size_t test() const;

	void extract(const AutoUTF &dest) const;

	operator ComObject<IInArchive>() const;

	static ComObject<IInArchive> open(const SevenZipLib &lib, PCWSTR path);

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

		AutoUTF path() const;

		uint64_t size() const;

		size_t attr() const;

		FILETIME mtime() const;

		bool is_file() const;

		bool is_dir() const;

		bool get_prop_info(size_t index, AutoUTF &name, PROPID &id) const;

		PropVariant get_prop(PROPID id) const;

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
//			printf(L"\tconst_input_iterator::const_input_iterator(%d, %d)", m_seq, index);
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
	ArcCodecs::const_iterator m_codec;
	UInt32		m_size;
	UInt32		m_num_props;
};

///=============================================================================== FileReadStream
class FileReadStream: public IInStream, private ComBase {
public:
	~FileReadStream();

	FileReadStream(PCWSTR path);

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

	FileWriteStream(PCWSTR path, DWORD creat = CREATE_NEW);

	STDMETHOD(QueryInterface)(REFIID riid, void** object);

	STDMETHOD_(ULONG, AddRef)();

	STDMETHOD_(ULONG, Release)();

	STDMETHOD(Write)(PCVOID data, UInt32 size, UInt32 *processedSize);

	STDMETHOD(Seek)(Int64 offset, UInt32 seekOrigin, UInt64 *newPosition);

	STDMETHOD(SetSize)(UInt64 newSize);
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

///=============================================================================== ArchiveProperties
struct ArchiveProperties {
	size_t level;
	bool solid;
	AutoUTF method;

	ArchiveProperties():
		level(5),
		solid(false) {
	}
};

///================================================================================ WinCreateArchive
class WinCreateArchive: public DirStructure, public ArchiveProperties {
public:
	WinCreateArchive(const SevenZipLib &lib, const AutoUTF &path, const AutoUTF &codec);

	void compress();

	ComObject<IOutArchive> operator->() const;

private:
	const SevenZipLib &m_lib;
	const AutoUTF &m_path;
	const AutoUTF &m_codec;
	ComObject<IOutArchive> m_arc;
	ArchiveProperties m_props;
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

#endif
