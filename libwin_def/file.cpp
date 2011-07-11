#include "win_def.h"

#include <wchar.h>

///===================================================================================== File system
uint64_t get_size(PCWSTR path) {
	WIN32_FILE_ATTRIBUTE_DATA info;
	::GetFileAttributesExW(path, GetFileExInfoStandard, &info);
	return ((uint64_t)info.nFileSizeHigh) << 32 | info.nFileSizeLow;
}

uint64_t get_size(HANDLE hFile) {
	LARGE_INTEGER	size;
	::GetFileSizeEx(hFile, &size);
	return size.QuadPart;
}

uint64_t get_position(HANDLE hFile) {
	LARGE_INTEGER pos;
	pos.QuadPart = 0LL;
	::SetFilePointerEx(hFile, pos, &pos, FILE_CURRENT);
	return pos.QuadPart;
}

bool	set_position(HANDLE hFile, uint64_t pos, DWORD m) {
	LARGE_INTEGER tmp;
	tmp.QuadPart = pos;
	return ::SetFilePointerEx(hFile, tmp, nullptr, m) != 0;
}

bool create_directory(PCWSTR path, LPSECURITY_ATTRIBUTES lpsa) {
	if (::CreateDirectoryW(path, lpsa) ||
		(::GetLastError() == ERROR_ALREADY_EXISTS && is_dir(path))) {
		return true;
	}
	return false;
}

bool create_directory_full(const AutoUTF &p, LPSECURITY_ATTRIBUTES sa) {
	AutoUTF path(PathNice(p));
	path = ensure_path_prefix(ensure_end_path_separator(path));
	if (create_directory(path, sa)) {
		return true;
	}

	if (get_root(path) == path)
		return true;

	path = get_fullpath(path);
	size_t pos = path.find(L":");
	if (pos == AutoUTF::npos)
		return false;
	pos = path.find_first_of(PATH_SEPARATORS, pos + 1);
	if (pos == AutoUTF::npos)
		return false;
	do {
		pos = path.find_first_of(PATH_SEPARATORS, pos + 1);
		AutoUTF tmp(path.substr(0, pos));
		if (!create_directory(tmp, sa))
			return false;
	} while (pos != AutoUTF::npos);
	return true;
}

bool create_file(PCWSTR path, LPSECURITY_ATTRIBUTES lpsa) {
	auto_close<HANDLE> file(::CreateFileW(path, 0, 0, lpsa, OPEN_ALWAYS, 0, nullptr));
	return file && file != INVALID_HANDLE_VALUE;
}

bool delete_dir(PCWSTR path) {
	DWORD	attr = get_attributes(path);
	if (::SetFileAttributesW(path, FILE_ATTRIBUTE_NORMAL)) {
		if (::RemoveDirectoryW(path)) {
			return true;
		}
		::SetFileAttributesW(path, attr);
	}
	return false;
}

bool delete_file(PCWSTR path) {
	DWORD	attr = get_attributes(path);
	if (::SetFileAttributesW(path, FILE_ATTRIBUTE_NORMAL)) {
		if (::DeleteFileW(path)) {
			return true;
		}
		::SetFileAttributesW(path, attr);
	}
	return false;
}

bool delete_sh(PCWSTR path) {
	SHFILEOPSTRUCTW sh;

	sh.hwnd = nullptr;
	sh.wFunc = FO_DELETE;
	sh.pFrom = path;
	sh.pTo = nullptr;
	sh.fFlags = FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT;
	sh.hNameMappings = 0;
	sh.lpszProgressTitle = nullptr;
	::SHFileOperationW(&sh);
	return true;
}

bool delete_recycle(PCWSTR path) {
	SHFILEOPSTRUCTW	info = {0};
	info.wFunc	= FO_DELETE;
	info.pFrom	= path;
	info.fFlags	= FOF_ALLOWUNDO | FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT;
	return ::SHFileOperationW(&info) == 0;
}




bool read_file(PCWSTR path, string &buf) {
	auto_close<HANDLE>	hFile(::CreateFileW(path, GENERIC_READ, 0, nullptr, OPEN_EXISTING,
								 FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS, nullptr));
	if (hFile != INVALID_HANDLE_VALUE) {
		DWORD	size = (DWORD)get_size(hFile);
		buf.reserve(size);
		return ::ReadFile(hFile, (PWSTR)buf.c_str(), buf.size(), &size, nullptr) != 0;
	}
	return false;
}

bool FileCreate(PCWSTR path, PCWSTR name, PCSTR content) {
	DWORD	dwBytesToWrite = Len(content);
	DWORD	dwBytesWritten = 0;
	AutoUTF	fpath = MakePath(path, name);
	auto_close<HANDLE> file(::CreateFileW(fpath.c_str(),
								 GENERIC_WRITE,
								 FILE_SHARE_READ,
								 nullptr,
								 CREATE_NEW,
								 FILE_ATTRIBUTE_NORMAL,
								 nullptr));
	if (file != INVALID_HANDLE_VALUE) {
		HRESULT err = ::GetLastError();
		if (err != ERROR_FILE_EXISTS) {
			::WriteFile(file, (PCVOID)content, dwBytesToWrite, &dwBytesWritten, nullptr);
		}
		if (dwBytesToWrite == dwBytesWritten)
			return true;
	}
	return false;
}

bool FileWrite(PCWSTR path, PCVOID buf, size_t size, bool rewrite) {
	bool	Result = false;
	DWORD	dwCreationDisposition = rewrite ? CREATE_ALWAYS : CREATE_NEW;
	auto_close<HANDLE> file(::CreateFileW(path, GENERIC_WRITE, 0, nullptr, dwCreationDisposition, FILE_ATTRIBUTE_NORMAL, nullptr));
	if (file != INVALID_HANDLE_VALUE) {
		DWORD	cbWritten = 0;
		Result = ::WriteFile(file, buf, size, &cbWritten, nullptr) != 0;
	}
	return Result;
}

bool get_file_inode(PCWSTR path, uint64_t &inode, size_t &nlink) {
	auto_close<HANDLE> file(::CreateFileW(path, FILE_READ_ATTRIBUTES,
	                  	                 FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
	                  	                 nullptr, OPEN_EXISTING,
	                  	                 FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS,
	                  	                 nullptr));
	if (file != INVALID_HANDLE_VALUE) {
		BY_HANDLE_FILE_INFORMATION	info;
		if (::GetFileInformationByHandle(file, &info)) {
			inode = info.nNumberOfLinks;
			nlink = info.nNumberOfLinks;
		}
		return true;
	}
	return false;
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

	AutoUTF strFullName;
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
