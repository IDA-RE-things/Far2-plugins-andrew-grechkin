/** 7zip.h
 *	@interface to 7-zip
 *	@author © 2011 Andrew Grechkin
 *	@link (ole32 oleaut32 uuid)
 **/

#ifndef WIN_7ZIP_HPP
#define WIN_7ZIP_HPP

#include <libwin_net/win_net.h>
#include <libwin_net/file.h>
#include "win_com.h"

#include <initguid.h>
#include <shobjidl.h>

#include <map>
#include <vector>
#include <tr1/memory>

namespace SevenZip {
	#include <7zipAPI/CPP/7zip/Archive/IArchive.h>
	#include <7zipAPI/CPP/7zip/IPassword.h>

	class Lib;

	using std::vector;
	using std::map;

	const UInt64 max_check_start_position = 4096;
	extern NamedValues<int> ArcItemPropsNames[63];

	typedef vector<BYTE> ByteVector;
	typedef ByteVector ArcType;
	typedef std::vector<ArcType> ArcTypes;

	struct FailedFile {
		ustring path;
		HRESULT code;

		FailedFile(const ustring & p, HRESULT h):
			path(p),
			code(h) {
		}
	};

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

	///====================================================================================== Method
	struct Method {
		uint64_t id;
		ustring name;
		ByteVector start_sign;
		ByteVector finish_sign;

	private:
		bool operator<(const Method & rhs) const;

		bool operator==(const Method & rhs) const;

		bool operator!=(const Method & rhs) const;

		Method(const Lib & arc_lib, size_t idx);

		friend class Methods;
	};

	///===================================================================================== Methods
	struct Methods: private map<uint64_t, std::tr1::shared_ptr<Method> > {
		typedef map::const_iterator iterator;
		typedef map::const_iterator const_iterator;

		using map::begin;
		using map::end;
		using map::size;
		using map::at;

		void cache(const Lib & lib);

	private:
		Methods();
		Methods(const Lib & lib);

		friend class Lib;
	};

	///======================================================================================= Codec
	struct Codec {
		ustring name;
		ustring ext;
		ustring add_ext;
		ustring kAssociate;
		WinGUID guid;
		ByteVector start_sign;
		ByteVector finish_sign;
		bool updatable;
		bool kKeepName;

//		ustring default_extension() const;

	private:
		bool operator<(const Codec & rhs) const;

		bool operator==(const Codec & rhs) const;

		bool operator!=(const Codec & rhs) const;

		Codec(const Lib & arc_lib, size_t idx);

		friend class Codecs;
	};

	///====================================================================================== Codecs
	struct Codecs: private map<ustring, std::tr1::shared_ptr<Codec> > {
		typedef map::const_iterator iterator;
		typedef map::const_iterator const_iterator;

		using map::begin;
		using map::end;
		using map::size;
		using map::at;

		void cache(const Lib & lib);

		ArcTypes find_by_ext(const ustring & ext) const;

	private:
		Codecs();
		Codecs(const Lib & lib);

		friend class Lib;
	};

	///======================================================================================== Prop
	struct Prop {
		ustring name;
		PROPID id;
		PropVariant prop;

	private:
		Prop(const ComObject<IInArchive> & arc, size_t idx);

		friend class Props;
	};

	///======================================================================================= Props
	struct Props: private vector<Prop> {
		typedef vector::const_iterator iterator;
		typedef vector::const_iterator const_iterator;

		using vector::begin;
		using vector::end;
		using vector::size;

		void cache(const ComObject<IInArchive> & arc);

	private:
		using vector::push_back;

		Props();
		Props(const ComObject<IInArchive> & arc);

		friend class Archive;
	};

	///========================================================================================= Lib
	class Lib: public FileVersion, private DynamicLibrary {
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

		Lib(PCWSTR path);

		const Codecs & codecs() const;
		const Methods & methods() const;

		HRESULT get_prop(UInt32 index, PROPID prop_id, PropVariant & prop) const;
		HRESULT get_prop(UInt32 index, PROPID prop_id, WinGUID & guid) const;
		HRESULT get_prop(UInt32 index, PROPID prop_id, bool & value) const;
		HRESULT get_prop(UInt32 index, PROPID prop_id, ustring & value) const;
		HRESULT get_prop(UInt32 index, PROPID prop_id, ByteVector & value) const;

	private:
		Codecs m_codecs;
		Methods m_methods;
	};

	///=================================================================================== SfxModule
//	struct SfxModule {
//		ustring path;
//		ustring description() const;
//		bool all_codecs() const;
//		bool install_config() const;
//	};
//
//	class SfxModules: public vector<SfxModule> {
//	public:
//		unsigned find_by_name(const ustring & name) const;
//	};

	///============================================================================== FileReadStream
	struct FileReadStream: public IInStream, private WinFile, private UnknownImp {
		virtual ~FileReadStream();

		FileReadStream(const ustring & path);

		// Unknown
		virtual ULONG AddRef();
		virtual ULONG Release();
		virtual HRESULT QueryInterface(REFIID riid, void ** object);

		// ISequentialInStream
		virtual HRESULT Read(void * data, UInt32 size, UInt32 * processedSize);

		// IInStream
		virtual HRESULT Seek(Int64 offset, UInt32 seekOrigin, UInt64 * newPosition);
	};

	///============================================================================= FileWriteStream
	struct FileWriteStream: public IOutStream, private WinFile, private UnknownImp {
		virtual ~FileWriteStream();

		FileWriteStream(const ustring & path, DWORD creat = CREATE_NEW);

		// Unknown
		virtual ULONG AddRef();
		virtual ULONG Release();
		virtual HRESULT QueryInterface(REFIID riid, void ** object);

		// ISequentialOutStream
		virtual HRESULT Write(PCVOID data, UInt32 size, UInt32 * processedSize);

		// IOutStream
		virtual HRESULT Seek(Int64 offset, UInt32 seekOrigin, UInt64 * newPosition);
		virtual HRESULT SetSize(UInt64 newSize);

		using WinFile::set_mtime;
	};

	///================================================================================ OpenCallback
	struct OpenCallback: public IArchiveOpenCallback, public ICryptoGetTextPassword, private UnknownImp {
		ustring Password;

		virtual ~OpenCallback();

		OpenCallback();

		// IUnknown
		virtual ULONG AddRef();
		virtual ULONG Release();
		virtual HRESULT QueryInterface(REFIID riid, void ** object);

		// IArchiveOpenCallback
		virtual HRESULT SetTotal(const UInt64 * files, const UInt64 * bytes);
		virtual HRESULT SetCompleted(const UInt64 * files, const UInt64 * bytes);

		// ICryptoGetTextPassword
		virtual HRESULT CryptoGetTextPassword(BSTR * password);
	};

	///===================================================================================== Archive
	struct Archive: private Uncopyable {
		class const_input_iterator;

		typedef Archive this_type;
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

		Archive(const Lib & lib, const ustring & path, flags_type flags = 0);

		Archive(const Lib & lib, const ustring & path, const ustring & mask, flags_type flags = 0);

		Archive(ComObject<IInArchive> arc, flags_type flags = 0);

		const Codec & codec() const;

		ComObject<IInArchive> operator->() const;

		const_iterator begin() const;

		const_iterator end() const;

		const_iterator at(size_t index) const;

		const_iterator operator[](int index) const;

		bool empty() const;

		size_t size() const;

		flags_type flags() const;

		const Props & props() const;

		size_t test() const;

		void extract(const ustring & dest) const;

		operator ComObject<IInArchive>() const;

		struct const_input_iterator {
			typedef const_input_iterator class_type;

			class_type & operator++() {
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

//			const value_type operator *() const {
//				return WinArchive::value_type(m_impl->m_seq->path());
//			}

			ustring path() const;

			uint64_t size() const;

			size_t attr() const;

			FILETIME mtime() const;

			bool is_file() const;

			bool is_dir() const;

			size_t get_props_count() const;

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
			const_input_iterator():
				m_seq(nullptr), m_index(0), m_end(true) {
			}
			const_input_iterator(const Archive & seq):
				m_seq((Archive*)&seq), m_index(0), m_end(!m_seq->m_size) {
			}
			const_input_iterator(const Archive & seq, UInt32 index):
				m_seq((Archive*)&seq), m_index(index), m_end(!m_seq->m_size || index >= m_seq->m_size) {
				//			printf(L"\tconst_input_iterator::const_input_iterator(%d, %d)", m_seq, index);
			}

			Archive * m_seq;
			UInt32 m_index;
			bool m_end;

			friend class Archive;
		};

	private:
		static ComObject<IInArchive> open(const Lib & lib, PCWSTR path);

		void open_archive(const Lib & lib, const ustring & path);

		void init_props();

		ComObject<IInArchive> m_arc;
		Codecs::const_iterator m_codec;
		Props m_props;
		UInt32 m_size;
		flags_type m_flags;
	};

	///============================================================================= ExtractCallback
	struct ExtractCallback: public IArchiveExtractCallback, public ICryptoGetTextPassword, private UnknownImp {
		std::vector<FailedFile> failed_files;
		ustring Password;

		virtual ~ExtractCallback();

		// Unknown
		virtual ULONG AddRef();
		virtual ULONG Release();
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
		ExtractCallback(const Archive & arc, const ustring & dest_path = ustring(), const ustring & pass = ustring());

		const Archive & m_wa;
		ustring m_dest;				// Output directory

		struct CurrItem;

		std::tr1::shared_ptr<CurrItem> m_curr;

		friend class Archive;
	};

	///============================================================================== UpdateCallback
	struct DirItem: public WinFileInfo {
		ustring path;
		ustring name;
		DirItem(const ustring & file_path, const ustring & file_name);
	};

	struct DirStructure: public std::vector<DirItem> {
		typedef const_iterator iterator;

		DirStructure();
		DirStructure(const ustring & path);

		void add(const ustring & add_path);

	private:
		void base_add(const ustring & base_path, const ustring & name);
	};

	struct UpdateCallback: public IArchiveUpdateCallback2, public ICryptoGetTextPassword2, private UnknownImp {
		std::vector<FailedFile> failed_files;
		ustring Password;
		bool AskPassword;

		virtual ~UpdateCallback();

		UpdateCallback(const DirStructure & items, const ustring & pass = ustring());

		// Unknown
		virtual ULONG AddRef();
		virtual ULONG Release();
		virtual HRESULT QueryInterface(REFIID riid, void ** object);

		// IProgress
		virtual HRESULT SetTotal(UInt64 size);
		virtual HRESULT SetCompleted(const UInt64 * completeValue);

		// GetUpdateItemInfo
		virtual HRESULT GetUpdateItemInfo(UInt32 index, Int32 *newData, Int32 *newProperties, UInt32 *indexInArchive);
		virtual HRESULT GetProperty(UInt32 index, PROPID propID, PROPVARIANT *value);
		virtual HRESULT GetStream(UInt32 index, ISequentialInStream **inStream);
		virtual HRESULT SetOperationResult(Int32 operationResult);

		// IArchiveUpdateCallback2
		virtual HRESULT GetVolumeSize(UInt32 index, UInt64 *size);
		virtual HRESULT GetVolumeStream(UInt32 index, ISequentialOutStream **volumeStream);

		// ICryptoGetTextPassword2
		virtual HRESULT CryptoGetTextPassword2(Int32 *passwordIsDefined, BSTR *password);

	private:
		const std::vector<DirItem> & DirItems;

		std::vector<UInt64> VolumesSizes;
		ustring VolName;
		ustring VolExt;
	};

	///=========================================================================== ArchiveProperties
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

	///=============================================================================== CreateArchive
	struct CreateArchive: public DirStructure, public ArchiveProperties {
		CreateArchive(const Lib & lib, const ustring & path, const ustring & codec);

		void compress();

		ComObject<IOutArchive> operator->() const;

	private:
		void set_properties();

		const Lib & m_lib;
		const ustring m_path;
		const ustring m_codec;
		ComObject<IOutArchive> m_arc;
	};
}

#endif
