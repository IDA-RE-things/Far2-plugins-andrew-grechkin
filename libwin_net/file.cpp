#include "file.h"
#include "exception.h"

#include <wchar.h>

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

	class TempSetAttr: private Uncopyable {
	public:
		~TempSetAttr() {
			::SetFileAttributesW(m_path.c_str(), m_attr);
		}

		TempSetAttr(const ustring & path, DWORD attr):
			m_path(path),
			m_attr(FS::get_attr(m_path)) {
			if ((m_attr & attr) != attr)
				::SetFileAttributesW(m_path.c_str(), attr);
		}

	private:
		ustring m_path;
		DWORD m_attr;
	};
}

namespace FS {
	bool is_exists(PCWSTR path) {
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
			TempSetAttr temp_attr(path, FILE_ATTRIBUTE_NORMAL);
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
			if (Find(path, device) == path) {
				WCHAR new_path[MAX_PATH_LEN];
				_snwprintf(new_path, sizeofa(new_path), L"%s%s", drive, path + Len(device));
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
		CheckApi(::GetMappedFileNameW(::GetCurrentProcess(), view, path, sizeofa(path)));
		return device_path_to_disk(path);
	}
}

namespace File {
	bool is_exists(PCWSTR path) {
		return FS::is_exists(path) && FS::is_file(path);
	}

	uint64_t get_size(PCWSTR path) {
		WIN32_FILE_ATTRIBUTE_DATA info;
		CheckApi(::GetFileAttributesExW(path, GetFileExInfoStandard, &info));
		return HighLow64(info.nFileSizeHigh, info.nFileSizeLow);
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
		DWORD bytesToWrite = Len(content);
		CheckApi(::WriteFile(file, (PCVOID)content, bytesToWrite, &bytesWritten, nullptr) && bytesToWrite == bytesWritten);
	}

	void create_hardlink(PCWSTR path, PCWSTR new_path) {
		CheckApi(::CreateHardLinkW(new_path, path, nullptr));
	}

	bool del_nt(PCWSTR path) {
		DWORD attr = ::GetFileAttributesW(path);
		if (attr != INVALID_FILE_ATTRIBUTES) {
			TempSetAttr temp_attr(path, FILE_ATTRIBUTE_NORMAL);
			if (::DeleteFileW(path)) {
				return true;
			}
		}
		return false;
	}

	void del(PCWSTR path) {
		CheckApi(del_nt(path));
	}

	void read(PCWSTR path, string &buf) {
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
		return HighLow64(info.nFileIndexHigh, info.nFileIndexLow);
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
	bool is_exists(PCWSTR path) {
		return FS::is_exists(path) && FS::is_dir(path);
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
		} catch (WinError &e) {
			return false;
		}
		return true;
	}

	void create_full(const ustring & p, LPSECURITY_ATTRIBUTES sa) {
		CheckApi(create_full_nt(p, sa));
	}

	bool del_nt(PCWSTR path) {
		DWORD attr = ::GetFileAttributesW(path);
		if (attr != INVALID_FILE_ATTRIBUTES) {
			TempSetAttr temp_attr(path, FILE_ATTRIBUTE_NORMAL);
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
WinFileInfo::WinFileInfo(const ustring & path) {
	auto_close<HANDLE> hndl(FileSys::HandleRead(path));
	refresh(hndl);
}

bool WinFileInfo::refresh(HANDLE hndl) {
	return CheckApi(::GetFileInformationByHandle(hndl, this));
}

///========================================================================================= FileMap
File_map::File_map(const WinFile & wf, size_type size, bool write):
	m_size(std::min(wf.size(), size)),
	m_frame(check_frame(DEFAULT_FRAME)),
	m_map(CheckHandle(::CreateFileMapping(wf, nullptr, (write) ? PAGE_READWRITE : PAGE_READONLY,
			HighPart64(m_size), LowPart64(m_size), nullptr))),
	m_write(write) {
}

File_map::file_map_iterator & File_map::file_map_iterator::operator++() {
	m_impl->close();
	if ((m_impl->m_seq->size() - m_impl->m_offs) > 0) {
		if ((m_impl->m_seq->size() - m_impl->m_offs) < m_impl->m_seq->frame())
			m_impl->m_size = m_impl->m_seq->size() - m_impl->m_offs;
		ACCESS_MASK amask = (m_impl->m_seq->is_writeble()) ? FILE_MAP_WRITE : FILE_MAP_READ;
		m_impl->m_data = ::MapViewOfFile(m_impl->m_seq->map(), amask, HighPart64(m_impl->m_offs),
										 LowPart64(m_impl->m_offs), m_impl->m_size);
		CheckApi(m_impl->m_data != nullptr);
		m_impl->m_offs += m_impl->m_size;
	}
	return *this;
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
