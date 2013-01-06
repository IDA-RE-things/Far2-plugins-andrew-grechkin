#include <libbase/std.hpp>
#include <libbase/path.hpp>
#include <libext/dll.hpp>
#include <libext/file.hpp>
#include <libext/exception.hpp>

#include <wchar.h>

using namespace Base;

extern "C" {
	INT WINAPI SHCreateDirectoryExW(HWND, PCWSTR, PSECURITY_ATTRIBUTES);
}

namespace Ext {

	///===================================================================================== File system
	namespace {
		struct 	LargeInteger {
			LargeInteger() {
				m_data.QuadPart = 0LL;
			}

			LargeInteger(uint64_t in) {
				m_data.QuadPart = in;
			}

			operator LARGE_INTEGER() const {
				return m_data;
			}

			PLARGE_INTEGER operator&() const {
				return (PLARGE_INTEGER)&m_data;
			}

			operator uint64_t() const {
				return (uint64_t)m_data.QuadPart;
			}
		private:
			LARGE_INTEGER m_data;
		};

		struct TemporarySetAttributes: private Uncopyable {
			~TemporarySetAttributes() {
				::SetFileAttributesW(m_path.c_str(), m_attr);
			}
			TemporarySetAttributes(const ustring & path, DWORD attr):
				m_path(path),
				m_attr(FS::get_attr(m_path)) {
				if ((m_attr & attr) != attr)
					::SetFileAttributesW(m_path.c_str(), attr);
			}
		private:
			ustring m_path;
			DWORD m_attr;
		};

		///=================================================================================== psapi_dll
		struct psapi_dll: private DynamicLibrary {
			typedef DWORD (WINAPI *FGetMappedFileNameW)(HANDLE, LPVOID, LPWSTR, DWORD);

			DEFINE_FUNC(GetMappedFileNameW);

			static psapi_dll & inst() {
				static psapi_dll ret;
				return ret;
			}

		private:
			psapi_dll():
				DynamicLibrary(L"psapi.dll") {
				GET_DLL_FUNC(GetMappedFileNameW);
			}
		};
	}

	namespace FS {
		bool is_exist(PCWSTR path) {
			DWORD attr = ::GetFileAttributesW(path);
			if (attr != INVALID_FILE_ATTRIBUTES)
				return true;
			DWORD err = ::GetLastError();
			if (err != ERROR_FILE_NOT_FOUND)
				CheckApiError(err);
			return false;
		}

		DWORD get_attr(PCWSTR path) {
			DWORD ret = ::GetFileAttributesW(path);
			CheckApi(ret != INVALID_FILE_ATTRIBUTES);
			return ret;
		}

		void set_attr(PCWSTR path, DWORD attr) {
			CheckApi(::SetFileAttributesW(path, attr));
		}

		bool is_file(PCWSTR path) {
			return 0 == (get_attr(path) & FILE_ATTRIBUTE_DIRECTORY);
		}

		bool is_dir(PCWSTR path) {
			return 0 != (get_attr(path) & FILE_ATTRIBUTE_DIRECTORY);
		}

		void del_sh(PCWSTR path) {
			SHFILEOPSTRUCTW sh;

			sh.hwnd = nullptr;
			sh.wFunc = FO_DELETE;
			sh.pFrom = path;
			sh.pTo = nullptr;
			sh.fFlags = FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT;
			sh.hNameMappings = 0;
			sh.lpszProgressTitle = nullptr;
			CheckApiError(::SHFileOperationW(&sh));
		}

		void del_recycle(PCWSTR path) {
			SHFILEOPSTRUCTW	info = {0};
			info.wFunc	= FO_DELETE;
			info.pFrom	= path;
			info.fFlags	= FOF_ALLOWUNDO | FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT;
			CheckApiError(::SHFileOperationW(&info));
		}

		void del_on_reboot(PCWSTR path) {
			CheckApi(::MoveFileExW(path, nullptr, MOVEFILE_DELAY_UNTIL_REBOOT));
		}

		bool del_nt(PCWSTR path) {
			DWORD attr = ::GetFileAttributesW(path);
			if (attr != INVALID_FILE_ATTRIBUTES) {
				TemporarySetAttributes temp_attr(path, FILE_ATTRIBUTE_NORMAL);
				if (attr & FILE_ATTRIBUTE_DIRECTORY) {
					if (::RemoveDirectoryW(path))
						return true;
				} else {
					if (::DeleteFileW(path))
						return true;
				}
			}
			return false;
		}

		void del(PCWSTR path) {
			CheckApi(del_nt(path));
		}

		ustring device_path_to_disk(PCWSTR path) {
			WCHAR local_disks[MAX_PATH] = {0}, *p = local_disks;
			CheckApi(::GetLogicalDriveStringsW(sizeofa(local_disks) - 1, local_disks));
			WCHAR drive[3] = L" :";
			WCHAR device[MAX_PATH];
			while (*p) {
				*drive = *p;
				CheckApi(::QueryDosDeviceW(drive, device, sizeofa(device)));
				if (Str::find(path, device) == path) {
					WCHAR new_path[MAX_PATH_LEN];
					_snwprintf(new_path, sizeofa(new_path), L"%s%s", drive, path + Str::length(device));
					return ustring(new_path);
				}
				while (*p++);
			};
			return ustring(path);
		}

		ustring get_path(HANDLE hndl) {
			CheckHandle(hndl);
			auto_close<HANDLE> hmap(CheckHandleErr(::CreateFileMappingW(hndl, nullptr, PAGE_READONLY, 0, 1, nullptr)));
			auto_close<PVOID const> view(CheckPointerErr(::MapViewOfFile(hmap, FILE_MAP_READ, 0, 0, 1)), ::UnmapViewOfFile);
			WCHAR path[MAX_PATH_LEN];
			CheckApi(psapi_dll::inst().GetMappedFileNameW(::GetCurrentProcess(), view, path, sizeofa(path)));
			return device_path_to_disk(path);
		}
	}

	namespace File {
		bool is_exist(PCWSTR path) {
			return FS::is_exist(path) && FS::is_file(path);
		}

		uint64_t get_size(PCWSTR path) {
			WIN32_FILE_ATTRIBUTE_DATA info;
			CheckApi(::GetFileAttributesExW(path, GetFileExInfoStandard, &info));
			return make_uint64(info.nFileSizeHigh, info.nFileSizeLow);
		}

		uint64_t get_size(HANDLE hFile) {
			LargeInteger size;
			CheckApi(::GetFileSizeEx(hFile, &size));
			return size;
		}

		uint64_t get_position(HANDLE hFile) {
			LargeInteger pos;
			CheckApi(::SetFilePointerEx(hFile, pos, &pos, FILE_CURRENT));
			return pos;
		}

		void set_position(HANDLE hFile, uint64_t pos, DWORD m) {
			CheckApi(::SetFilePointerEx(hFile, LargeInteger(pos), nullptr, m));
		}

		void create(PCWSTR path, LPSECURITY_ATTRIBUTES lpsa) {
			auto_close<HANDLE> file(CheckHandle(::CreateFileW(path, 0, 0, lpsa,
			                                                  CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr)));
		}

		void create(PCWSTR path, PCSTR content, LPSECURITY_ATTRIBUTES lpsa) {
			auto_close<HANDLE> file(CheckHandle(::CreateFileW(path, GENERIC_WRITE, 0, lpsa,
			                                                  CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr)));
			DWORD bytesWritten = 0;
			DWORD bytesToWrite = Str::length(content);
			CheckApi(::WriteFile(file, (PCVOID)content, bytesToWrite, &bytesWritten, nullptr) && bytesToWrite == bytesWritten);
		}

		void create_hardlink(PCWSTR path, PCWSTR new_path) {
			CheckApi(::CreateHardLinkW(new_path, path, nullptr));
		}

		bool del_nt(PCWSTR path) {
			DWORD attr = ::GetFileAttributesW(path);
			if (attr != INVALID_FILE_ATTRIBUTES) {
				TemporarySetAttributes temp_attr(path, FILE_ATTRIBUTE_NORMAL);
				if (::DeleteFileW(path)) {
					return true;
				}
			}
			return false;
		}

		void del(PCWSTR path) {
			CheckApi(del_nt(path));
		}

		void read(PCWSTR path, astring &buf) {
			auto_close<HANDLE> file(::CreateFileW(path, GENERIC_READ, 0, nullptr, OPEN_EXISTING,
			                                      FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS, nullptr));
			if (file != INVALID_HANDLE_VALUE) {
				DWORD size = (DWORD)get_size(file);
				buf.reserve(size);
				CheckApi(::ReadFile(file, (PWSTR)buf.c_str(), buf.size(), &size, nullptr));
			}
		}

		uint64_t get_inode(PCWSTR path, size_t * nlink) {
			auto_close<HANDLE> file(CheckHandle(::CreateFileW(path, FILE_READ_ATTRIBUTES,
			                                                  FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
			                                                  nullptr, OPEN_EXISTING,
			                                                  FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS,
			                                                  nullptr)));
			BY_HANDLE_FILE_INFORMATION	info;
			CheckApi(::GetFileInformationByHandle(file, &info));
			if (nlink)
				*nlink = info.nNumberOfLinks;
			return make_uint64(info.nFileIndexHigh, info.nFileIndexLow);
		}

		size_t write(HANDLE file, PCVOID data, size_t bytesToWrite) {
			DWORD bytesWritten = 0;
			CheckApi(::WriteFile(file, data, bytesToWrite, &bytesWritten, nullptr));
			return bytesWritten;
		}

		void write(PCWSTR path, PCVOID data, size_t bytesToWrite, bool rewrite) {
			DWORD creationDisposition = rewrite ? CREATE_ALWAYS : CREATE_NEW;
			auto_close<HANDLE> file(CheckHandle(::CreateFileW(path, GENERIC_WRITE, 0, nullptr,
			                                                  creationDisposition, FILE_ATTRIBUTE_NORMAL, nullptr)));
			write(file, data, bytesToWrite);
		}
	}

	namespace Directory {
		bool is_exist(PCWSTR path) {
			return FS::is_exist(path) && FS::is_dir(path);
		}

		bool create_nt(PCWSTR path, LPSECURITY_ATTRIBUTES lpsa) {
			return ::CreateDirectoryW(path, lpsa) || (::GetLastError() == ERROR_ALREADY_EXISTS && FS::is_dir(path));
		}

		void create(PCWSTR path, LPSECURITY_ATTRIBUTES lpsa) {
			CheckApi(create_nt(path, lpsa));
		}

		bool create_full_nt(const ustring & p, LPSECURITY_ATTRIBUTES sa) throw() {
			try {
				ustring path(get_fullpath(p));
				path = PathNice(path);
				path = ensure_path_prefix(ensure_end_path_separator(path));

				if (get_root(path) == path)
					return false;

				if (create_nt(path.c_str(), sa)) {
					return true;
				}

				size_t pos = path.find(L":");
				if (pos == ustring::npos)
					return false;
				pos = path.find_first_of(PATH_SEPARATORS, pos + 1);
				if (pos == ustring::npos)
					return false;
				do {
					pos = path.find_first_of(PATH_SEPARATORS, pos + 1);
					ustring tmp(path.substr(0, pos));
					if (!create_nt(tmp.c_str(), sa))
						return false;
				} while (pos != ustring::npos);
			} catch (AbstractError & e) {
				return false;
			}
			return true;
		}

		void create_full(const ustring & p, LPSECURITY_ATTRIBUTES sa) {
			CheckApi(create_full_nt(p, sa));
		}

		bool create_dir(PCWSTR path, LPSECURITY_ATTRIBUTES lpsa) {
			return ::SHCreateDirectoryExW(nullptr, path, lpsa) == ERROR_SUCCESS;
		}

		bool del_nt(PCWSTR path) {
			DWORD attr = ::GetFileAttributesW(path);
			if (attr != INVALID_FILE_ATTRIBUTES) {
				TemporarySetAttributes temp_attr(path, FILE_ATTRIBUTE_NORMAL);
				if (::RemoveDirectoryW(path)) {
					return true;
				}
			}
			return false;
		}

		void del(PCWSTR path) {
			CheckApi(del_nt(path));
		}
	}

	///===================================================================================== WinFileInfo
	WinFileInfo::WinFileInfo(PCWSTR path) {
		auto_close<HANDLE> hndl(FS::HandleRead(path));
		refresh(hndl);
	}

	bool WinFileInfo::refresh(HANDLE hndl) {
		return CheckApi(::GetFileInformationByHandle(hndl, this));
	}

	WinFileInfo & WinFileInfo::operator =(HANDLE hndl) {
		refresh(hndl);
		return *this;
	}

	WinFileInfo & WinFileInfo::operator =(PCWSTR path) {
		auto_close<HANDLE> hndl(FS::HandleRead(path));
		refresh(hndl);
		return *this;
	}


	///========================================================================================= FileMap
	struct File_map::file_map_iterator::impl {
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

		impl(const File_map * seq):
			m_seq(seq),
			m_data(nullptr),
			m_size(seq->get_frame()),
			m_offs(0) {
		}

		const File_map * m_seq;
		PVOID m_data;
		size_type m_size;
		size_type m_offs;

		friend class file_map_iterator;
	};

	File_map::file_map_iterator & File_map::file_map_iterator::operator ++() {
		m_impl->close();
		if ((m_impl->m_seq->size() - m_impl->m_offs) > 0) {
			if ((m_impl->m_seq->size() - m_impl->m_offs) < m_impl->m_seq->get_frame())
				m_impl->m_size = m_impl->m_seq->size() - m_impl->m_offs;
			ACCESS_MASK amask = (m_impl->m_seq->is_writeble()) ? FILE_MAP_WRITE : FILE_MAP_READ;
			m_impl->m_data = ::MapViewOfFile(m_impl->m_seq->map(), amask, high_part_64(m_impl->m_offs),
			                                 low_part_64(m_impl->m_offs), m_impl->m_size);
			CheckApi(m_impl->m_data != nullptr);
			m_impl->m_offs += m_impl->m_size;
		}
		return *this;
	}

	File_map::file_map_iterator::this_type File_map::file_map_iterator::operator ++(int) {
		this_type ret(*this);
		operator ++();
		return ret;
	}

	void * File_map::file_map_iterator::operator *() const {
		return m_impl->m_data;
	}

	void * File_map::file_map_iterator::data() const {
		return m_impl->m_data;
	}

	File_map::size_type File_map::file_map_iterator::size() const {
		return m_impl->m_size;
	}

	File_map::size_type File_map::file_map_iterator::offset() const {
		return m_impl->m_offs;
	}

	bool File_map::file_map_iterator::operator ==(const this_type & rhs) const {
		return m_impl->m_data == rhs.m_impl->m_data;
	}

	bool File_map::file_map_iterator::operator !=(const this_type & rhs) const {
		return !operator==(rhs);
	}

	File_map::file_map_iterator::file_map_iterator():
		m_impl(new impl) {
	}

	File_map::file_map_iterator::file_map_iterator(const File_map * seq) :
		m_impl(new impl(seq)) {
		operator++();
	}

	File_map::~File_map() {
		::CloseHandle(m_map);
	}

	File_map::File_map(const WinFile & wf, size_type size, bool write):
		m_size(std::min(wf.size(), size)),
		m_frame(check_frame(DEFAULT_FRAME)),
		m_map(CheckHandle(::CreateFileMapping(wf, nullptr, (write) ? PAGE_READWRITE : PAGE_READONLY,
			high_part_64(m_size), low_part_64(m_size), nullptr))),
			m_write(write) {
	}

	File_map::size_type File_map::set_frame(size_type size) {
		return m_frame = check_frame(size);
	}

	File_map::iterator File_map::begin() {
		return file_map_iterator(this);
	}

	File_map::iterator File_map::end() {
		return file_map_iterator();
	}

	File_map::const_iterator File_map::begin() const {
		return file_map_iterator(this);
	}

	File_map::const_iterator File_map::end() const {
		return file_map_iterator();
	}

	bool File_map::empty() const {
		return !size();
	}

	SYSTEM_INFO get_system_info() {
		SYSTEM_INFO info;
		::GetSystemInfo(&info);
		return info;
	}

	File_map::size_type File_map::check_frame(size_type size) const {
		static SYSTEM_INFO info(get_system_info());
		size_type ret = (!size || size % info.dwAllocationGranularity) ?
			(size / info.dwAllocationGranularity + 1) * info.dwAllocationGranularity :
			size;
		return std::min(m_size, ret);
	}

	///========================================================================================== WinDir
	WinDir::WinDir(const ustring & path, flags_type flags):
		m_path(path),
		m_mask(L"*"),
		m_flags(flags) {
	}

	WinDir::WinDir(const ustring & path, const ustring & mask, flags_type flags):
		m_path(path),
		m_mask(mask),
		m_flags(flags) {
	}

	WinDir::const_iterator WinDir::begin() const {
		return const_iterator(*this);
	}

	WinDir::const_iterator WinDir::end() const {
		return const_iterator();
	}

	WinDir::const_input_iterator & WinDir::const_input_iterator::operator ++() {
		flags_type flags = m_impl->m_seq->flags();

		while (true) {
			WIN32_FIND_DATAW& st = m_impl->m_stat;
			if (m_impl->m_handle == INVALID_HANDLE_VALUE) {
				ustring path = MakePath(m_impl->m_seq->path(), m_impl->m_seq->mask());
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

			const ustring& name = this->name();
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

			if ((flags & skipFiles) && !((st.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ||
				(st.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT))) {
				continue;
			}

			break;
		}
		return *this;
	}

	WinDir::const_input_iterator WinDir::const_input_iterator::operator ++(int) {
		this_type ret(*this);
		operator ++();
		return ret;
	}

	const WinDir::value_type WinDir::const_input_iterator::operator *() const {
		return WinDir::value_type(m_impl->m_seq->path().c_str());
	}

	PCWSTR WinDir::const_input_iterator::name() const {
		return m_impl->m_stat.cFileName;
	}

	ustring WinDir::const_input_iterator::path() const {
		return MakePath(m_impl->m_seq->path(), m_impl->m_stat.cFileName);
	}

	uint64_t WinDir::const_input_iterator::size() const {
		return make_uint64(m_impl->m_stat.nFileSizeHigh, m_impl->m_stat.nFileSizeLow);
	}

	size_t WinDir::const_input_iterator::attr() const {
		return m_impl->m_stat.dwFileAttributes;
	}

	bool WinDir::const_input_iterator::is_file() const {
		return !(m_impl->m_stat.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) && !(m_impl->m_stat.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
	}

	bool WinDir::const_input_iterator::is_dir() const {
		return !(m_impl->m_stat.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) && (m_impl->m_stat.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
	}

	bool WinDir::const_input_iterator::is_link() const {
		return m_impl->m_stat.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT;
	}

	bool WinDir::const_input_iterator::is_link_file() const {
		return (m_impl->m_stat.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) && !(m_impl->m_stat.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
	}

	bool WinDir::const_input_iterator::is_link_dir() const {
		return (m_impl->m_stat.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) && (m_impl->m_stat.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
	}

	bool WinDir::const_input_iterator::operator ==(const this_type & rhs) const {
		return m_impl->m_handle == rhs.m_impl->m_handle;
	}

	bool WinDir::const_input_iterator::operator !=(const this_type & rhs) const {
		return m_impl->m_handle != rhs.m_impl->m_handle;
	}

	WinDir::const_input_iterator::const_input_iterator():
			m_impl(new impl()) {
	}

	WinDir::const_input_iterator::const_input_iterator(const WinDir & seq):
			m_impl(new impl(seq)) {
		operator++();
	}

	WinDir::const_input_iterator::impl::~impl() throw() {
		if (m_handle && m_handle != INVALID_HANDLE_VALUE) {
			::FindClose(m_handle);
		}
	}

	WinDir::const_input_iterator::impl::impl():
		m_seq(nullptr),
		m_handle(nullptr) {
	}

	WinDir::const_input_iterator::impl::impl(const WinDir & seq):
		m_seq(&seq),
		m_handle(INVALID_HANDLE_VALUE) {
	}


	///================================================================================================
	/*
#define BUFF_SIZE (64*1024)

BOOL WipeFileW(wchar_t *filename) {
	DWORD Error = 0, OldAttr, needed;
	void *SD = nullptr;
	int correct_SD = FALSE;
	wchar_t dir[2*MAX_PATH], tmpname[MAX_PATH], *fileptr = wcsrchr(filename, L'\\');
	unsigned char *buffer = (unsigned char *)malloc(BUFF_SIZE);
	if (fileptr && buffer) {
		OldAttr = GetFileAttributesW(filename);
		SetFileAttributesW(filename, OldAttr&(~FILE_ATTRIBUTE_READONLY));
		if (!GetFileSecurityW(filename, DACL_SECURITY_INFORMATION, nullptr, 0, &needed))
			if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
				SD = malloc(needed);
				if (SD)
					if (GetFileSecurityW(filename, DACL_SECURITY_INFORMATION, SD, needed, &needed)) correct_SD = TRUE;
			}
		wcsncpy(dir, filename, fileptr - filename + 1);
		dir[fileptr-filename+1] = 0;
		if (GetTempFileNameW(dir, L"bc", 0, tmpname)) {
			if (MoveFileExW(filename, tmpname, MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
				HANDLE f = CreateFileW(tmpname, FILE_GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_SEQUENTIAL_SCAN, nullptr);
				if (f != INVALID_HANDLE_VALUE) {
					BY_HANDLE_FILE_INFORMATION info;
					if (GetFileInformationByHandle(f, &info)) {
						unsigned long long size = (unsigned long long)info.nFileSizeLow + (unsigned long long)info.nFileSizeHigh * 4294967296ULL;
						unsigned long long processed_size = 0;
						while (size) {
							unsigned long outsize = (unsigned long)((size >= BUFF_SIZE) ? BUFF_SIZE : size), transferred;
							WriteFile(f, buffer, outsize, &transferred, nullptr);
							size -= outsize;
							processed_size += outsize;
							if (UpdatePosInfo(0ULL, processed_size)) break;
						}
					}
					if ((SetFilePointer(f, 0, nullptr, FILE_BEGIN) == INVALID_SET_FILE_POINTER) || (!SetEndOfFile(f))) Error = GetLastError();
					CloseHandle(f);
				}
				if (Error) MoveFileExW(tmpname, filename, MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
				else if (!DeleteFileW(tmpname)) Error = GetLastError();
			} else {
				Error = GetLastError();
				DeleteFileW(tmpname);
			}
		} else Error = GetLastError();
		if (Error) {
			SetFileAttributesW(filename, OldAttr);
			if (correct_SD) SetFileSecurityW(filename, DACL_SECURITY_INFORMATION, SD);
		}
	}
	free(SD);
	free(buffer);
	if (Error) {
		SetLastError(Error);
		return FALSE;
	}
	return TRUE;
}
	 */
	/*
int			WipeDirectory(const wchar_t *Name) {

	string strTempName, strSavePath(Opt.strTempPath);
	BOOL usePath = FALSE;

	if (FirstSlash(Name)) {
		Opt.strTempPath = Name;
		DeleteEndSlash(Opt.strTempPath);
		CutToSlash(Opt.strTempPath);
		usePath = TRUE;
	}

	FarMkTempEx(strTempName, nullptr, usePath);
	Opt.strTempPath = strSavePath;

	if (!apiMoveFile(Name, strTempName)) {
		SetLastError((_localLastError = GetLastError()));
		return FALSE;
	}

	return apiRemoveDirectory(strTempName);

}
	 */
	/*
int			DeleteFileWithFolder(const wchar_t *FileName) {
	string strFileOrFolderName;
	strFileOrFolderName = FileName;
	Unquote(strFileOrFolderName);
	BOOL Ret = apiSetFileAttributes(strFileOrFolderName, FILE_ATTRIBUTE_NORMAL);

	if (Ret) {
		if (apiDeleteFile(strFileOrFolderName)) { //BUGBUG
			CutToSlash(strFileOrFolderName, true);
			return apiRemoveDirectory(strFileOrFolderName);
		}
	}

	SetLastError((_localLastError = GetLastError()));
	return FALSE;
}
	 */
	/*
void		DeleteDirTree(PCWSTR Dir) {
	if (*Dir == 0 ||
			(IsSlash(Dir[0]) && Dir[1] == 0) ||
			(Dir[1] == L':' && IsSlash(Dir[2]) && Dir[3] == 0))
		return;

	ustring strFullName;
	FAR_FIND_DATA_EX FindData;
	ScanTree ScTree(TRUE, TRUE, FALSE);
	ScTree.SetFindPath(Dir, L"*", 0);

	while (ScTree.GetNextName(&FindData, strFullName)) {
		apiSetFileAttributes(strFullName, FILE_ATTRIBUTE_NORMAL);

		if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			if (ScTree.IsDirSearchDone())
				apiRemoveDirectory(strFullName);
		} else
			apiDeleteFile(strFullName);
	}

	apiSetFileAttributes(Dir, FILE_ATTRIBUTE_NORMAL);
	apiRemoveDirectory(Dir);
}
	 */

}
