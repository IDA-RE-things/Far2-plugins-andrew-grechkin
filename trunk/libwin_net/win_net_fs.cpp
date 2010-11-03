#include "win_net.h"
#include <winioctl.h>

bool copy_file_security(PCWSTR path, PCWSTR dest) {
	return false;
}

bool del_by_mask(PCWSTR mask) {
	bool Result = false;
	WIN32_FIND_DATAW wfd;
	HANDLE hFind = ::FindFirstFileW(mask, &wfd);
	if (hFind != INVALID_HANDLE_VALUE) {
		Result = true;
		AutoUTF fullpath = get_path_from_mask(mask);
		do {
			if (!is_valid_filename(wfd.cFileName))
				continue;
			AutoUTF path = MakePath(fullpath, wfd.cFileName);
			if (wfd.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) {
				Result = delete_junc(path);
			}
			if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				del_by_mask(MakePath(path, L"*"));
				Result = delete_dir(path.c_str());
			} else {
				Result = delete_file(path.c_str());
			}
		} while (::FindNextFileW(hFind, &wfd));
		::FindClose(hFind);
	}
	return Result;
}

bool unlink(PCWSTR path) {
	bool Result = false;
	if (is_path_mask(path)) {
		Result = del_by_mask(path);
	} else {
		if (!file_exists(path))
			return true;
		if (is_dir(path)) {
			if (is_junction(path))
				delete_junc(path);
			del_by_mask(MakePath(path, L"*"));
			Result = delete_dir(path);
		} else {
			Result = delete_file(path);
		}
	}
	return Result;
}

#ifndef IO_REPARSE_TAG_VALID_VALUES
#define IO_REPARSE_TAG_VALID_VALUES 0xE000FFFF
#endif
#ifndef IsReparseTagValid
#define IsReparseTagValid(x) (!((x)&~IO_REPARSE_TAG_VALID_VALUES)&&((x)>IO_REPARSE_TAG_RESERVED_RANGE))
#endif

struct TMN_REPARSE_DATA_BUFFER {
	DWORD ReparseTag;
	WORD ReparseDataLength;
	WORD Reserved;

	// IO_REPARSE_TAG_MOUNT_POINT specifics follow
	WORD SubstituteNameOffset;
	WORD SubstituteNameLength;
	WORD PrintNameOffset;
	WORD PrintNameLength;
	WCHAR PathBuffer[1];

	// Some helper functions
	//BOOL	Init(PCSTR szJunctionPoint);
	//BOOL	Init(PCWSTR wszJunctionPoint);
	//int	BytesForIoControl() const;
};

HANDLE FileSys::Handle(PCWSTR path, bool bWrite) {
	// Obtain backup/restore privilege in case we don't have it
	bWrite ? WinPriv::Enable(SE_RESTORE_NAME) : WinPriv::Enable(SE_BACKUP_NAME);
	DWORD dwAccess = (bWrite) ? (GENERIC_READ | GENERIC_WRITE) : GENERIC_READ;
	HANDLE hFile =
	    ::CreateFileW(path, dwAccess, 0, nullptr, OPEN_EXISTING, FILE_FLAG_OPEN_REPARSE_POINT
	        | FILE_FLAG_BACKUP_SEMANTICS, nullptr);
	return CheckHandle(hFile);
}

HANDLE FileSys::Handle(const AutoUTF &path, bool bWrite) {
	return Handle(path.c_str(), bWrite);
}

///========================================================================================= WinJunc
bool is_junction(PCWSTR path) {
	DWORD JUNC_ATTR = FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_REPARSE_POINT;
	DWORD Attr = get_attributes(path);
	return (Attr != INVALID_FILE_ATTRIBUTES) && ((Attr & JUNC_ATTR) == JUNC_ATTR);

	HANDLE hDir = FileSys::Handle(path);
	if (hDir == INVALID_HANDLE_VALUE)
		return false; // Failed to open directory

	BYTE buf[MAXIMUM_REPARSE_DATA_BUFFER_SIZE];
	REPARSE_GUID_DATA_BUFFER *rgdb = (REPARSE_GUID_DATA_BUFFER*)&buf;

	DWORD dwRet;
	BOOL br = ::DeviceIoControl(hDir, FSCTL_GET_REPARSE_POINT, nullptr, 0, rgdb,
	                            MAXIMUM_REPARSE_DATA_BUFFER_SIZE, &dwRet, nullptr);
	::CloseHandle(hDir);
	return (br ? (rgdb->ReparseTag == IO_REPARSE_TAG_MOUNT_POINT) : false);
}

bool create_junc(PCWSTR path, PCWSTR dest) {
	if (Empty(path) || Empty(dest) || !file_exists(dest)) {
		return false;
	}
	if (!ensure_dir_exist(path) && !dir_is_empty(path))
		return false;

	AutoUTF targ;
	targ.Add(dest, L"\\??\\", false);
	//	BYTE buf[MAXIMUM_REPARSE_DATA_BUFFER_SIZE + MAX_PATH * sizeof(WCHAR)] = {0};
	WinBuf<TMN_REPARSE_DATA_BUFFER> buf(MAXIMUM_REPARSE_DATA_BUFFER_SIZE + MAX_PATH_LEN
	    * sizeof(WCHAR), true);
	//	TMN_REPARSE_DATA_BUFFER &rdb = *(TMN_REPARSE_DATA_BUFFER*)buf;
	//	TMN_REPARSE_DATA_BUFFER &rdb = buf.data();

	DWORD nDestMountPointBytes = (targ.size() * sizeof(WCHAR));
	buf->ReparseTag = IO_REPARSE_TAG_MOUNT_POINT;
	buf->ReparseDataLength = nDestMountPointBytes + 12;
	buf->SubstituteNameLength = nDestMountPointBytes;
	buf->PrintNameOffset = nDestMountPointBytes + 2;
	::wcscpy(buf->PathBuffer, targ.c_str());

	HANDLE hDir = FileSys::Handle(path, true);
	if (hDir == INVALID_HANDLE_VALUE)
		return false;

	DWORD dwTMN_REPARSE_DATA_BUFFER_HEADER_SIZE =
	    FIELD_OFFSET(TMN_REPARSE_DATA_BUFFER, SubstituteNameOffset);
	DWORD dwBytes;
	if (!::DeviceIoControl(hDir, FSCTL_SET_REPARSE_POINT, buf.data(), buf->ReparseDataLength
	    + dwTMN_REPARSE_DATA_BUFFER_HEADER_SIZE, nullptr, 0, &dwBytes, 0)) {
		::CloseHandle(hDir);
		::RemoveDirectoryW(path);
		return (false);
	}
	::CloseHandle(hDir);
	return true;
}

bool delete_junc(PCWSTR path) {
	bool Result = false;
	HANDLE hDir = FileSys::Handle(path, true);
	if (hDir == INVALID_HANDLE_VALUE) {
		::SetLastError(ERROR_PATH_NOT_FOUND);
		return Result;
	}

	REPARSE_GUID_DATA_BUFFER rgdb = {0};
	rgdb.ReparseTag = IO_REPARSE_TAG_MOUNT_POINT;
	DWORD dwBytes;
	Result = ::DeviceIoControl(hDir, FSCTL_DELETE_REPARSE_POINT, &rgdb,
	                           REPARSE_GUID_DATA_BUFFER_HEADER_SIZE, nullptr, 0, &dwBytes, 0);
	::CloseHandle(hDir);
	return Result;
}

AutoUTF junc_destination(PCWSTR path) {
	AutoUTF Result;
	HANDLE hDir = FileSys::Handle(path);
	if (hDir != INVALID_HANDLE_VALUE) {
		WinBuf<TMN_REPARSE_DATA_BUFFER> buf(MAXIMUM_REPARSE_DATA_BUFFER_SIZE, true);

		DWORD dwRet;
		if (::DeviceIoControl(hDir, FSCTL_GET_REPARSE_POINT, nullptr, 0, buf.data(),
		                      MAXIMUM_REPARSE_DATA_BUFFER_SIZE, &dwRet, nullptr)) {
			if (IsReparseTagValid(buf->ReparseTag)) {
				PCWSTR pPath = buf->PathBuffer;
				if (::wcsncmp(pPath, L"\\??\\", 4) == 0)
					pPath += 4;
				Result = pPath;
			}
		}
		::CloseHandle(hDir);
	}
	return Result;
}

