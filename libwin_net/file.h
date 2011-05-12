/**
	win_file

	@classes	()
	@author		© 2009 Andrew Grechkin
	@link		()
**/

#ifndef WIN_FILE_HPP
#define WIN_FILE_HPP

#include "win_net.h"

extern "C" {
	DWORD WINAPI GetMappedFileNameW(HANDLE hProcess, LPVOID lpv, LPWSTR lpFilename, DWORD nSize);
}

namespace	FileSys {
	HANDLE HandleRead(PCWSTR path);

	HANDLE HandleWrite(PCWSTR path);

	inline HANDLE HandleRead(const AutoUTF &path) {
		return HandleRead(path.c_str());
	}

	inline HANDLE HandleWrite(const AutoUTF &path) {
		return HandleWrite(path.c_str());
	}
}

void copy_file_security(PCWSTR path, PCWSTR dest);
inline void copy_file_security(const AutoUTF &path, const AutoUTF &dest) {
	copy_file_security(path.c_str(), dest.c_str());
}

bool del_by_mask(PCWSTR mask);
inline bool del_by_mask(const AutoUTF &mask) {
	return del_by_mask(mask.c_str());
}

bool ensure_dir_exist(PCWSTR path, LPSECURITY_ATTRIBUTES lpsa = nullptr);
inline bool ensure_dir_exist(const AutoUTF &path, LPSECURITY_ATTRIBUTES lpsa = nullptr) {
	return	ensure_dir_exist(path.c_str(), lpsa);
}

bool remove_dir(PCWSTR path, bool follow_links = false);
inline bool remove_dir(const AutoUTF &path, bool follow_links = false) {
	return remove_dir(path.c_str(), follow_links);
}

void SetOwnerRecur(const AutoUTF &path, PSID owner, SE_OBJECT_TYPE type = SE_FILE_OBJECT);

///===================================================================================== WinFileInfo
struct WinFileInfo: public BY_HANDLE_FILE_INFORMATION {
	WinFileInfo(HANDLE hndl) {
		refresh(hndl);
	}

	WinFileInfo(const AutoUTF &path) {
		auto_close<HANDLE> hndl(FileSys::HandleRead(path));
		refresh(hndl);
	}

	bool refresh (HANDLE hndl) {
		return CheckApi(::GetFileInformationByHandle(hndl, this));
	}

	DWORD attr() const {
		return dwFileAttributes;
	}

	uint64_t ctime() const {
		return HighLow64(ftCreationTime.dwHighDateTime, ftCreationTime.dwLowDateTime);
	}

	uint64_t atime() const {
		return HighLow64(ftLastAccessTime.dwHighDateTime, ftLastAccessTime.dwLowDateTime);
	}

	uint64_t mtime() const {
		return HighLow64(ftLastWriteTime.dwHighDateTime, ftLastWriteTime.dwLowDateTime);
	}

	FILETIME ctime_ft() const {
		return ftCreationTime;
	}

	FILETIME atime_ft() const {
		return ftLastAccessTime;
	}

	FILETIME mtime_ft() const {
		return ftLastWriteTime;
	}

	uint64_t size() const {
		return HighLow64(nFileSizeHigh, nFileSizeLow);
	}

	DWORD dev() const {
		return dwVolumeSerialNumber;
	}

	size_t nlink() const {
		return nNumberOfLinks;
	}

	int64_t ino() const {
		return HighLow64(nFileIndexHigh, nFileIndexLow) & 0x0000FFFFFFFFFFFFLL;
	}

	bool is_dir() const {
		return !(dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) && (dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
	}

	bool is_dir_or_link() const {
		return dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
	}

	bool is_file() const {
		return !(dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) && !(dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
	}

	bool is_file_or_link() const {
		return !(dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
	}

	bool is_lnk() const {
		return dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT;
	}

	bool operator==(const WinFileInfo &rhs) const {
		return dev() == rhs.dev() && ino() == rhs.ino();
	}

protected:
	WinFileInfo() {
	}
};

inline bool same_file(const WinFileInfo &f1, const WinFileInfo &f2) {
	return (f1.dev() == f2.dev() && f1.ino() == f2.ino());
}

///========================================================================================= WinFile
class WinFile: private Uncopyable, public WinFileInfo {
public:
	~WinFile() {
		::CloseHandle(m_hndl);
	}

	WinFile(PCWSTR path, bool write = false) :
		m_path(path) {
		Open(m_path, write);
	}

	WinFile(PCWSTR path, ACCESS_MASK access, DWORD share, PSECURITY_ATTRIBUTES sa, DWORD creat, DWORD flags) :
		m_path(path) {
		Open(m_path, access, share, sa, creat, flags);
	}

	WinFile(const AutoUTF &path, bool write = false) :
		m_path(path) {
		Open(m_path, write);
	}

	WinFile(const AutoUTF &path, ACCESS_MASK access, DWORD share, PSECURITY_ATTRIBUTES sa, DWORD creat, DWORD flags) :
		m_path(path) {
		Open(m_path, access, share, sa, creat, flags);
	}

//	bool	Path(PWSTR path, size_t len) const {
//		if (m_hndl && m_hndl != INVALID_HANDLE_VALUE) {
//			// Create a file mapping object.
//			HANDLE	hFileMap = ::CreateFileMapping(m_hndl, nullptr, PAGE_READONLY, 0, 1, nullptr);
//			if (hFileMap) {
//				PVOID	pMem = ::MapViewOfFile(hFileMap, FILE_MAP_READ, 0, 0, 1);
//				if (pMem) {
//					if (::GetMappedFileNameW(::GetCurrentProcess(), pMem, path, len)) {
//						// Translate path with device name to drive letters.
//						WCHAR	szTemp[len];
//						szTemp[0] = L'\0';
//						if (::GetLogicalDriveStringsW(len - 1, szTemp)) {
//							WCHAR	szName[MAX_PATH], *p = szTemp;
//							WCHAR	szDrive[3] = L" :";
//							bool	bFound = false;
//
//							do {
//								// Copy the drive letter to the template string
//								*szDrive = *p;
//								// Look up each device name
//								if (::QueryDosDeviceW(szDrive, szName, sizeofa(szName))) {
//									size_t uNameLen = Len(szName);
//
//									if (uNameLen < sizeofa(szName)) {
//										bFound = Find(path, szName) == path;
//										if (bFound) {
//											// Reconstruct pszFilename using szTempFile Replace device path with DOS path
//											WCHAR	szTempFile[MAX_PATH];
//											_snwprintf(szTempFile, sizeofa(szTempFile), L"%s%s", szDrive, path + uNameLen);
//											Copy(path, szTempFile, len);
//										}
//									}
//								}
//								// Go to the next nullptr character.
//								while (*p++);
//							} while (!bFound && *p); // end of string
//						}
//					}
//					::UnmapViewOfFile(pMem);
//					return	true;
//				}
//			}
//		}
//		return	false;
//	}

	uint64_t size() const {
		uint64_t ret;
		CheckApi(size_nt(ret));
		return ret;
	}

	bool size_nt(uint64_t &size) const {
		LARGE_INTEGER fs;
		if (::GetFileSizeEx(m_hndl, &fs)) {
			size = fs.QuadPart;
			return true;
		}
		return false;
	}

	DWORD read(PVOID data, size_t size) {
		DWORD ridden;
		CheckApi(read_nt(data, size, ridden));
		return ridden;
	}

	bool read_nt(PVOID buf, size_t size, DWORD &ridden) {
		return ::ReadFile(m_hndl, buf, size, &ridden, nullptr);
	}

	DWORD write(PCVOID buf, size_t size) {
		DWORD written;
		CheckApi(write_nt(buf, size, written));
		return written;
	}

	bool write_nt(PCVOID buf, size_t size, DWORD &written) {
		return ::WriteFile(m_hndl, buf, size, &written, nullptr);
	}

	bool set_attr(DWORD at) {
		return ::SetFileAttributesW(m_path.c_str(), at);
	}

	uint64_t get_position() const {
		LARGE_INTEGER tmp, np;
		tmp.QuadPart = 0;
		CheckApi(::SetFilePointerEx(m_hndl, tmp, &np, FILE_CURRENT));
		return np.QuadPart;
	}

	void set_position(int64_t dist, DWORD method = FILE_BEGIN) {
		CheckApi(set_position_nt(dist, method));
	}

	bool set_position_nt(int64_t dist, DWORD method = FILE_BEGIN) {
		LARGE_INTEGER tmp;
		tmp.QuadPart = dist;
		return ::SetFilePointerEx(m_hndl, tmp, nullptr, method);
	}

	bool set_eof() {
		return ::SetEndOfFile(m_hndl);
	}

	bool set_time(const FILETIME &ctime, const FILETIME &atime, const FILETIME &mtime) {
		return ::SetFileTime(m_hndl, &ctime, &atime, &mtime);
	}

	bool set_mtime(const FILETIME &mtime) {
		return ::SetFileTime(m_hndl, nullptr, nullptr, &mtime);
	}

	AutoUTF path() const {
		return m_path;
	}

	operator HANDLE() const {
		return m_hndl;
	}

	void refresh() {
		WinFileInfo::refresh(m_hndl);
	}

	template<typename Type>
	bool io_control_out_nt(DWORD code, Type& data) throw() {
		DWORD size_ret;
		return ::DeviceIoControl(m_hndl, code, nullptr, 0, &data, sizeof(Type), &size_ret, nullptr) != 0;
	}

private:
	void Open(const AutoUTF &path, bool write = false) {
		ACCESS_MASK amask = (write) ? GENERIC_READ | GENERIC_WRITE : GENERIC_READ;
		DWORD share = (write) ? 0 : FILE_SHARE_DELETE | FILE_SHARE_READ;
		DWORD creat = (write) ? OPEN_ALWAYS : OPEN_EXISTING;
		DWORD flags = (write) ? FILE_ATTRIBUTE_NORMAL : FILE_FLAG_OPEN_REPARSE_POINT
					  | FILE_FLAG_BACKUP_SEMANTICS;
		Open(path, amask, share, nullptr, creat, flags);
	}

	void Open(const AutoUTF &path, ACCESS_MASK access, DWORD share, PSECURITY_ATTRIBUTES sa, DWORD creat, DWORD flags) {
		m_hndl = ::CreateFileW(path.c_str(), access, share, sa, creat, flags, nullptr);
		CheckHandleErr(m_hndl);
		WinFileInfo::refresh(m_hndl);
	}

	AutoUTF m_path;
	HANDLE m_hndl;
};

///========================================================================================= FileMap
/// Отображение файла в память блоками
class File_map: private Uncopyable {
	class file_map_iterator;
public:
	typedef File_map class_type;
	typedef uint64_t size_type;
	typedef file_map_iterator iterator;
	typedef const file_map_iterator const_iterator;

	static const size_type DEFAULT_FRAME = 128;

	~File_map() {
		if (m_map) {
			::CloseHandle(m_map);
			m_map = nullptr;
		}
	}

	File_map(const WinFile &wf, size_type size = (size_type) - 1, bool write = false) :
			m_size(std::min(wf.size(), size)),
			m_frame(check_frame(DEFAULT_FRAME)),
			m_map(nullptr),
			m_write(write) {
		m_map = ::CreateFileMapping(wf, nullptr, (m_write) ? PAGE_READWRITE : PAGE_READONLY,
									(DWORD)(m_size >> 32), (DWORD)(m_size & 0xFFFFFFFF), nullptr);
		CheckApi(m_map != nullptr);
	}

	HANDLE map() const {
		return m_map;
	}

	size_type size() const {
		return m_size;
	}

	size_type frame() const {
		return m_frame;
	}

	void frame(size_type mul) { // in pages
		m_frame = check_frame(mul);
	}

	bool is_writeble() const {
		return m_write;
	}

	iterator begin() {
		return file_map_iterator(this);
	}

	iterator end() {
		return file_map_iterator();
	}

	const_iterator begin() const {
		return file_map_iterator(this);
	}

	const_iterator end() const {
		return file_map_iterator();
	}

	bool empty() const {
		return !size();
	}

private:
	class file_map_iterator {
	public:
		typedef file_map_iterator class_type;

		class_type& operator++() {
			m_impl->close();
			if ((m_impl->m_seq->size() - m_impl->m_offs) > 0) {
				if ((m_impl->m_seq->size() - m_impl->m_offs) < m_impl->m_seq->frame())
					m_impl->m_size = m_impl->m_seq->size() - m_impl->m_offs;
				ACCESS_MASK amask = (m_impl->m_seq->is_writeble()) ? FILE_MAP_WRITE : FILE_MAP_READ;
				m_impl->m_data = ::MapViewOfFile(m_impl->m_seq->map(), amask, (DWORD)(m_impl->m_offs >> 32),
												 (DWORD)(m_impl->m_offs & 0xFFFFFFFF), m_impl->m_size);
				CheckApi(m_impl->m_data != nullptr);
				m_impl->m_offs += m_impl->m_size;
			}
			return *this;
		}

		class_type operator++(int) {
			class_type ret(*this);
			operator ++();
			return ret;
		}

		void* operator *() const {
			return m_impl->m_data;
		}

		void* data() const {
			return m_impl->m_data;
		}

		size_type size() const {
			return m_impl->m_size;
		}

		size_type offset() const {
			return m_impl->m_offs;
		}

		bool operator==(const class_type &rhs) const {
			return m_impl->m_data == rhs.m_impl->m_data;
		}

		bool operator!=(const class_type &rhs) const {
			return !operator==(rhs);
		}

	private:
		file_map_iterator() :
				m_impl(new impl) {
		}

		file_map_iterator(const File_map *seq) :
				m_impl(new impl(seq)) {
			operator++();
		}

		class impl {
		public:
			~impl() {
				close();
			}

			void close() {
				if (m_data) {
					::UnmapViewOfFile(m_data);
					m_data = nullptr;
				}
			}
		private:
			impl():
					m_seq(nullptr),
					m_data(nullptr),
					m_size(0),
					m_offs(0) {
			}
			impl(const File_map* seq):
					m_seq(seq),
					m_data(nullptr),
					m_size(seq->frame()),
					m_offs(0) {
			}

			const File_map* m_seq;
			PVOID m_data;
			size_type m_size;
			size_type m_offs;

			friend class file_map_iterator;
		};
		winstd::shared_ptr<impl> m_impl;
		friend class File_map;
	};

	size_type check_frame(size_type mul) const { // in pages
		SYSTEM_INFO info;
		::GetSystemInfo(&info);
		return std::min(m_size, (size_type)info.dwAllocationGranularity * mul);
	}

	size_type m_size;
	size_type m_frame;
	HANDLE m_map;
	bool m_write;
};

///========================================================================================== WinDir
class WinDir {
public:
	class const_input_iterator;

	typedef WinDir					class_type;
	typedef WinFileInfo				value_type;
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

	WinDir(const AutoUTF &path, flags_type flags = 0):
			m_path(path),
			m_mask(L"*"),
			m_flags(flags) {
	}

	WinDir(const AutoUTF &path, const AutoUTF &mask, flags_type flags = 0):
			m_path(path),
			m_mask(mask),
			m_flags(flags) {
	}

	const_iterator begin() const {
		return const_iterator(*this);
	}
	const_iterator end() const {
		return const_iterator();
	}

	bool empty() const;
	AutoUTF path() const {
		return m_path;
	}
	AutoUTF mask() const {
		return m_mask;
	}
	flags_type flags() const {
		return m_flags;
	}

	class const_input_iterator {
	public:
		typedef const_input_iterator	class_type;

		class_type& operator++() {
			flags_type flags = m_impl->m_seq->flags();


			while (true) {
				WIN32_FIND_DATAW& st = m_impl->m_stat;
				if (m_impl->m_handle == INVALID_HANDLE_VALUE) {
					AutoUTF path = MakePath(m_impl->m_seq->path(), m_impl->m_seq->mask());
					m_impl->m_handle = ::FindFirstFileW(path.c_str(), &m_impl->m_stat);
					if (m_impl->m_handle == INVALID_HANDLE_VALUE)
						throw "Cant";
				} else {
					if (!::FindNextFileW(m_impl->m_handle, &st)) {
						::FindClose(m_impl->m_handle);
						m_impl->m_handle = NULL; // end()
						break;
					}
				}

				const AutoUTF& name = this->name();
				if (!(flags & incDots) && (name == L"." || name == L"..")) {
					continue;
				}

				if ((flags & skipHidden) && (st.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)) {
					continue;
				}

				if ((flags & skipDirs) && (st.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
					continue;
				}

				if ((flags & skipLinks) && (st.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)) {
					continue;
				}

				if ((flags & skipFiles) && !(st.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ||
											 st.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)) {
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
		const value_type operator *() const {
			return WinDir::value_type(m_impl->m_seq->path());
		}

		PCWSTR name() const {
			return m_impl->m_stat.cFileName;
		}
		AutoUTF path() const {
			return MakePath(m_impl->m_seq->path(), m_impl->m_stat.cFileName);
		}
		uint64_t size() const {
			return HighLow64(m_impl->m_stat.nFileSizeHigh, m_impl->m_stat.nFileSizeLow);
		}
		size_t attr() const {
			return m_impl->m_stat.dwFileAttributes;
		}
		bool is_file() const {
			return !(m_impl->m_stat.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) && !(m_impl->m_stat.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
		}
		bool is_dir() const {
			return !(m_impl->m_stat.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) && (m_impl->m_stat.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
		}
		bool is_link() const {
			return m_impl->m_stat.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT;
		}
		bool is_link_file() const {
			return (m_impl->m_stat.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) && !(m_impl->m_stat.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
		}
		bool is_link_dir() const {
			return (m_impl->m_stat.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) && (m_impl->m_stat.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
		}

		bool operator==(const class_type &rhs) const {
			return m_impl->m_handle == rhs.m_impl->m_handle;
		}
		bool operator!=(const class_type &rhs) const {
			return m_impl->m_handle != rhs.m_impl->m_handle;
		}

	private:
		const_input_iterator():
				m_impl(new impl()) {
		}
		const_input_iterator(const WinDir &seq):
				m_impl(new impl(seq)) {
			operator++();
		}

		struct impl {
			~impl() throw() {
				if (m_handle && m_handle != INVALID_HANDLE_VALUE) {
					::FindClose(m_handle);
				}
			}
			impl():
					m_seq(nullptr),
					m_handle(nullptr) {
			}

			impl(const WinDir &seq):
					m_seq(&seq),
					m_handle(INVALID_HANDLE_VALUE) {
			}

			const WinDir *m_seq;
			HANDLE		m_handle;
			WIN32_FIND_DATAW m_stat;
		};

		winstd::shared_ptr<impl> m_impl;

		friend class WinDir;
	};

private:
	WinDir(const class_type&);  // deny copy constructor and operator =
	class_type& operator=(const class_type&);

	AutoUTF 	m_path;
	AutoUTF 	m_mask;
	flags_type	m_flags;
};

///========================================================================================== WinVol
class WinVol : private Uncopyable, public WinErrorCheck {
public:
	~WinVol() {
		Close();
	}
	WinVol(): m_hnd(INVALID_HANDLE_VALUE) {
	}
	bool 			Next();

	AutoUTF			GetName() const {
		return	name;
	}
	AutoUTF			GetPath() const;
	AutoUTF			GetDevice() const;

	uint64_t		GetSize() const {
//		long long tmp = f_.nFileSizeHigh;
//		tmp = tmp << (sizeof(f_.nFileSizeHigh) * 8);
//		tmp |= f_.nFileSizeLow;
		return	0;
	}

	DWORD			GetFlag() const {
		DWORD	Result = 0;
//		::GetVolumeInformation(path.c_str(), nullptr, 0, nullptr, nullptr, &Result, nullptr, 0);
		return	Result;
	}
	UINT			GetType() const {
		return	::GetDriveTypeW(name.c_str());
	}

	bool			IsSuppCompress() const {
		return	WinFlag::Check(GetFlag(), (DWORD)FILE_FILE_COMPRESSION);
	}
	bool			IsSuppEncrypt() const {
		return	WinFlag::Check(GetFlag(), (DWORD)FILE_SUPPORTS_ENCRYPTION);
	}
	bool			IsSuppStreams() const {
		return	WinFlag::Check(GetFlag(), (DWORD)FILE_NAMED_STREAMS);
	}
	bool			IsSuppACL() const {
		return	WinFlag::Check(GetFlag(), (DWORD)FILE_PERSISTENT_ACLS);
	}
	bool			IsReadOnly() const {
		return	WinFlag::Check(GetFlag(), (DWORD)FILE_READ_ONLY_VOLUME);
	}

	bool			IsRemovable() const {
		return	WinFlag::Check(GetType(), DRIVE_REMOVABLE);
	}
	bool			IsFixed() const {
		return	WinFlag::Check(GetType(), DRIVE_FIXED);
	}
	bool			IsRemote() const {
		return	WinFlag::Check(GetType(), (UINT)DRIVE_REMOTE);
	}
	bool			IsCdRom() const {
		return	WinFlag::Check(GetType(), (UINT)DRIVE_CDROM);
	}
	bool			IsRamdisk() const {
		return	WinFlag::Check(GetType(), (UINT)DRIVE_RAMDISK);
	}

	bool			GetSize(uint64_t &uiUserFree, uint64_t &uiTotalSize, uint64_t &uiTotalFree) const;
	uint64_t		GetSizeTotal() const {
		uint64_t uf = 0, ts = 0, tf = 0;
		GetSize(uf, ts, tf);
		return	ts;
	}

private:
	void	Close();

	HANDLE	m_hnd;
	AutoUTF	name;
};

///========================================================================================== WinDir
//class WinDir : private Uncopyable, public WinErrorCheck {
//	WIN32_FIND_DATAW	m_find;
//	HANDLE				m_handle;
//	AutoUTF				m_path;
//	AutoUTF				m_mask;
//
//	void			Close() {
//		if (m_handle && m_handle != INVALID_HANDLE_VALUE) {
//			::FindClose(m_handle);
//			m_handle = nullptr;
//		}
//	}
//public:
//	~WinDir() {
//		Close();
//	}
//	WinDir(const AutoUTF &path, const AutoUTF &mask = L"*"): m_handle(nullptr), m_path(path), m_mask(mask) {
//	}
//
//	bool 			Next() {
//		AutoUTF	tmp(SlashAdd(m_path));
//		tmp += m_mask;
//		if (m_handle == nullptr) {
//			m_handle = ::FindFirstFileW(tmp.c_str(), &m_find);
//			ChkSucc(m_handle != INVALID_HANDLE_VALUE);
//		} else {
//			ChkSucc(::FindNextFileW(m_handle, &m_find) != 0);
//		}
//		if (IsOK() && !is_valid_filename(m_find.cFileName))
//			return	Next();
//		return	IsOK();
//	}
//
//	void			path(const AutoUTF &in) {
//		Close();
//		m_path = in;
//	}
//	void			mask(const AutoUTF &in) {
//		Close();
//		m_mask = in;
//	}
//
//	bool			IsDir() const {
//		return	m_find.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
//	}
//	bool			IsJunc() const {
//		return	m_find.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT;
//	}
//	DWORD			attr() const {
//		return	m_find.dwFileAttributes;
//	}
//	FILETIME		time_cr() const {
//		return	m_find.ftCreationTime;
//	}
//	FILETIME		time_ac() const {
//		return	m_find.ftLastAccessTime;
//	}
//	FILETIME		time_wr() const {
//		return	m_find.ftLastWriteTime;
//	}
//	PCWSTR			name() const {
//		return	m_find.cFileName;
//	}
//	AutoUTF			name_full() const {
//		AutoUTF	Result(SlashAdd(m_path));
//		Result += m_find.cFileName;
//		return	Result;
//	}
//	PCWSTR			name_dos() const {
//		return	m_find.cAlternateFileName;
//	}
//	PCWSTR			path() const {
//		return	m_path.c_str();
//	}
//	uint64_t		size() const {
//		return	MyUI64(m_find.nFileSizeLow, m_find.nFileSizeHigh);
//	}
//};

bool	FileWipe(PCWSTR path);

#endif // WIN_FILE_HPP
