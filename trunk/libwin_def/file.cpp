#include "win_def.h"

#include <wchar.h>

extern "C" {
	BOOL WINAPI		SHGetSpecialFolderPathW(HWND, LPWSTR, int, BOOL);
	DWORD WINAPI GetMappedFileNameW(HANDLE hProcess,LPVOID lpv,LPWSTR lpFilename,DWORD nSize);
}

#ifndef CSIDL_WINDOWS
#define CSIDL_WINDOWS 0x0024
#endif
#ifndef CSIDL_SYSTEM
#define CSIDL_SYSTEM 0x0025
#endif

///============================================================================================ path
AutoUTF				Secure(PCWSTR path) {
	AutoUTF	Result(path);
	ReplaceAll(Result, L"..", L"");
	ReplaceAll(Result, L"%", L"");
	return	Validate(Result);
}
AutoUTF				Secure(const AutoUTF& path) {
	return	Secure(path.c_str());
}
AutoUTF				UnExpand(PCWSTR path) {
//	bool	unx = IsPathUnix(path);
//	if (unx)
//		Result.PathWin();
	WCHAR	buf[MAX_PATH_LEN];
	if (::PathUnExpandEnvStringsW(path, buf, sizeofa(buf))) {
		return	buf;
	}
//	return	unx ? Result.PathUnix() : Result;
	return	AutoUTF();
}
AutoUTF				UnExpand(const AutoUTF &path) {
	return	UnExpand(path.c_str());
}
AutoUTF				Validate(PCWSTR path) {
	AutoUTF	Result(Canonicalize(Expand(path)));
	ReplaceAll(Result, L"...", L"");
	ReplaceAll(Result, L"\\\\", L"\\");
	if (Result == L"\\")
		Result.clear();
	return	Result;
}
AutoUTF				Validate(const AutoUTF &path) {
	return	Validate(path.c_str());
}
/*
AutoUTF				SlashAddNec(PCWSTR path) {
#ifndef NoStlString
	AutoUTF	Result(path);
	return	Result.SlashAddNec();
#endif
}
AutoUTF				SlashAddNec(const AutoUTF &path) {
	return	SlashAddNec(path.c_str());
}
*/
AutoUTF				SlashDel(PCWSTR path) {
	AutoUTF	Result(path);
	if (!Result.empty()) {
		size_t	pos = Result.size() - 1;
		if (Result.at(pos) == L'\\' || Result.at(pos) == L'/')
			Result.erase(pos);
	}
	return	Result;
}
AutoUTF				SlashDel(const AutoUTF &path) {
	return	SlashDel(path.c_str());
}

bool				IsPathUnix(PCWSTR path) {
	return	Find(path, L'/') != nullptr;
}
bool				IsPathUnix(const AutoUTF &path) {
	return	IsPathUnix(path.c_str());
}

AutoUTF				ExtractFile(PCWSTR path, WCHAR sep) {
	size_t	len = Len(path);
	PWSTR	ch = RFind((PWSTR)path, sep);
	if (ch && ++ch < (path + len)) {
		return	AutoUTF(ch);
	}
	return	L"";
}
AutoUTF				ExtractFile(const AutoUTF &path, WCHAR sep) {
	return	ExtractFile(path.c_str(), sep);
}
AutoUTF				ExtractPath(PCWSTR path, WCHAR sep) {
	size_t	len = Len(path);
	PWSTR	ch = RFind((PWSTR)path, sep);
	if (ch && ch < (path + len)) {
		return	AutoUTF(path, ch - path);
	}
	return	L"";
}
AutoUTF				ExtractPath(const AutoUTF &path, WCHAR sep) {
	return	ExtractPath(path.c_str(), sep);
}
AutoUTF				GetSpecialPath(int csidl, bool create) {
	WCHAR	buf[MAX_PATH];
	if (::SHGetSpecialFolderPathW(nullptr, buf, csidl, create))
		return	buf;
	return	AutoUTF();
}

AutoUTF				PathUnix(PCWSTR path) {
	AutoUTF	Result(path);
	return	ReplaceAll(Result, L"\\", L"/");
}
AutoUTF				PathUnix(const AutoUTF &path) {
	return	PathUnix(path.c_str());
}
AutoUTF				PathWin(PCWSTR path) {
	AutoUTF	Result(path);
	return	ReplaceAll(Result, L"/", L"\\");
}
AutoUTF				PathWin(const AutoUTF &path) {
	return	PathWin(path.c_str());
}

AutoUTF				GetWorkDirectory() {
	WCHAR	Result[::GetCurrentDirectoryW(0, nullptr)];
	::GetCurrentDirectoryW(sizeofa(Result), Result);
	return	Result;
}
bool				SetWorkDirectory(PCWSTR path) {
	if (Empty(path))
		return	false;
	return	::SetCurrentDirectoryW(path);
}

///========================================================================================= SysPath
namespace	SysPath {
AutoUTF	Winnt() {
	return	SlashAdd(GetSpecialPath(CSIDL_WINDOWS));
}
AutoUTF	Sys32() {
	return	SlashAdd(GetSpecialPath(CSIDL_SYSTEM));
}

AutoUTF	SysNative() {
	AutoUTF	Result = Win64::IsWOW64() ? Validate(AutoUTF(L"%systemroot%\\sysnative\\")) : Validate(AutoUTF(L"%systemroot%\\system32\\"));
	return	SlashAdd(Result);
}
AutoUTF	InetSrv() {
	return	MakePath(Sys32().c_str(), L"inetsrv\\");
}
AutoUTF	Dns() {
	return	MakePath(SysNative().c_str(), L"dns\\");
}
AutoUTF	Temp() {
	return	Validate(AutoUTF(L"%TEMP%\\"));
}

AutoUTF	Users() {
	AutoUTF	Result = Validate(AutoUTF(L"%PUBLIC%\\..\\"));
	if (Result.empty() || (Result == L"\\"))
		Result = Validate(AutoUTF(L"%ALLUSERSPROFILE%\\..\\"));
	return	SlashAdd(Result);
}
}

///========================================================================================== SysApp
namespace	SysApp {
AutoUTF			appcmd() {
	return	SysPath::InetSrv() + L"appcmd.exe ";
}
AutoUTF			dnscmd() {
	return	SysPath::SysNative() + L"dnscmd.exe ";
}
}

///===================================================================================== File system

bool read_file(PCWSTR path, astring &buf) {
	bool	Result = false;
	HANDLE	hFile = ::CreateFileW(path, GENERIC_READ, 0, nullptr, OPEN_EXISTING,
								 FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS, nullptr);
	if (hFile != INVALID_HANDLE_VALUE) {
		DWORD	size = (DWORD)FileSize(hFile);
		buf.reserve(size);
		Result = ::ReadFile(hFile, (PWSTR)buf.c_str(), buf.size(), &size, nullptr) != 0;
		::CloseHandle(hFile);
	}
	return	Result;
}

bool				FileCreate(PCWSTR path, PCWSTR name, PCSTR content) {
	DWORD	dwBytesToWrite = Len(content);
	DWORD	dwBytesWritten = 0;
	AutoUTF	fpath = MakePath(path, name);
	HANDLE	hFile = ::CreateFileW(fpath.c_str(),
								 GENERIC_WRITE,
								 FILE_SHARE_READ,
								 nullptr,
								 CREATE_NEW,
								 FILE_ATTRIBUTE_NORMAL,
								 nullptr);
	if (hFile != INVALID_HANDLE_VALUE) {
		HRESULT err = ::GetLastError();
		if (err != ERROR_FILE_EXISTS) {
			::WriteFile(hFile, (PCVOID)content, dwBytesToWrite, &dwBytesWritten, nullptr);
		}
		::CloseHandle(hFile);
		if (dwBytesToWrite == dwBytesWritten)
			return	true;
	}
	return	false;
}

bool				FileWrite(PCWSTR path, PCVOID buf, size_t size, bool rewrite) {
	bool	Result = false;
	return	false;
	DWORD	dwCreationDisposition = rewrite ? CREATE_ALWAYS : CREATE_NEW;
	HANDLE	hFile = ::CreateFileW(path, GENERIC_WRITE, 0, nullptr, dwCreationDisposition, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (hFile != INVALID_HANDLE_VALUE) {
		DWORD	cbWritten = 0;
		Result = ::WriteFile(hFile, buf, size, &cbWritten, nullptr) != 0;
		::CloseHandle(hFile);
	}
	return	Result;
}

AutoUTF				GetDrives() {
	WCHAR	Result[MAX_PATH] = {0};
	WCHAR	szTemp[::GetLogicalDriveStringsW(0, nullptr)];
	if (::GetLogicalDriveStringsW(sizeofa(szTemp), szTemp)) {
		bool	bFound = false;
		WCHAR	*p = szTemp;
		do {
			Cat(Result, p, sizeofa(Result));
			Cat(Result, L";", sizeofa(Result));
			while (*p++);
		} while (!bFound && *p); // end of string
	}
	return	Result;
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

///============================================================================================ Link
#ifndef SE_CREATE_SYMBOLIC_LINK_NAME
#define SE_CREATE_SYMBOLIC_LINK_NAME      L"SeCreateSymbolicLinkPrivilege"
#endif

#ifndef IO_REPARSE_TAG_VALID_VALUES
#define IO_REPARSE_TAG_VALID_VALUES 0xF000FFFF
#endif

#ifndef IsReparseTagValid
#define IsReparseTagValid(_tag) (!((_tag)&~IO_REPARSE_TAG_VALID_VALUES)&&((_tag)>IO_REPARSE_TAG_RESERVED_RANGE))
#endif

#ifndef REPARSE_DATA_BUFFER_HEADER_SIZE
typedef struct _REPARSE_DATA_BUFFER {
	ULONG ReparseTag;
	USHORT ReparseDataLength;
	USHORT Reserved;
	union {
		struct {
			USHORT SubstituteNameOffset;
			USHORT SubstituteNameLength;
			USHORT PrintNameOffset;
			USHORT PrintNameLength;
			ULONG Flags;
			WCHAR PathBuffer[1];
		} SymbolicLinkReparseBuffer;
		struct {
			USHORT SubstituteNameOffset;
			USHORT SubstituteNameLength;
			USHORT PrintNameOffset;
			USHORT PrintNameLength;
			WCHAR PathBuffer[1];
		} MountPointReparseBuffer;
		struct {
			UCHAR DataBuffer[1];
		} GenericReparseBuffer;
	};
} REPARSE_DATA_BUFFER, *PREPARSE_DATA_BUFFER;

#define IO_REPARSE_TAG_SYMLINK                  (0xA000000CL)

#define REPARSE_DATA_BUFFER_HEADER_SIZE FIELD_OFFSET(REPARSE_DATA_BUFFER,GenericReparseBuffer)
#endif

HANDLE OpenLinkHandle(PCWSTR path, ACCESS_MASK acc = 0, DWORD create = OPEN_EXISTING) {
	return ::CreateFileW(path, acc, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, create,
	      	                      FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS, nullptr);
}

class REPARSE_BUF{
public:
	bool get(PCWSTR path) {
		DWORD attr = get_attributes(path);
		if ((attr != INVALID_FILE_ATTRIBUTES) && (attr & FILE_ATTRIBUTE_REPARSE_POINT)) {
			auto_close<HANDLE> hLink(OpenLinkHandle(path));
			if (hLink) {
				DWORD dwBytesReturned;
				return ::DeviceIoControl(hLink, FSCTL_GET_REPARSE_POINT, nullptr, 0, buf, size(), &dwBytesReturned, nullptr) &&
			                             IsReparseTagValid(rdb.ReparseTag);
			}
		}
		return false;
	}

	bool set(PCWSTR path) const {
		bool ret = false;
		if (IsReparseTagValid(rdb.ReparseTag)) {
			DWORD attr = get_attributes(path);
			if (attr != INVALID_FILE_ATTRIBUTES) {
				if (attr & FILE_ATTRIBUTE_READONLY) {
					set_attributes(path, attr & ~FILE_ATTRIBUTE_READONLY);
				}
				Privilege CreateSymlinkPrivilege(SE_CREATE_SYMBOLIC_LINK_NAME);
				auto_close<HANDLE> hLink(OpenLinkHandle(path, GENERIC_WRITE));
				if (hLink) {
					DWORD dwBytesReturned;
					ret = ::DeviceIoControl(hLink, FSCTL_SET_REPARSE_POINT,(PVOID)&rdb, rdb.ReparseDataLength + REPARSE_DATA_BUFFER_HEADER_SIZE,
					                      nullptr, 0, &dwBytesReturned, nullptr);
				}
				if (attr & FILE_ATTRIBUTE_READONLY) {
					set_attributes(path, attr);
				}
			}
		}
		return ret;
	}

	bool fill(PCWSTR PrintName, size_t PrintNameLength, PCWSTR SubstituteName, size_t SubstituteNameLength) {
		bool Result = false;
		rdb.Reserved = 0;

		switch (rdb.ReparseTag) {
			case IO_REPARSE_TAG_MOUNT_POINT:
				rdb.MountPointReparseBuffer.SubstituteNameOffset=0;
				rdb.MountPointReparseBuffer.SubstituteNameLength=static_cast<WORD>(SubstituteNameLength*sizeof(wchar_t));
				rdb.MountPointReparseBuffer.PrintNameOffset=rdb.MountPointReparseBuffer.SubstituteNameLength+2;
				rdb.MountPointReparseBuffer.PrintNameLength=static_cast<WORD>(PrintNameLength*sizeof(wchar_t));
				rdb.ReparseDataLength=FIELD_OFFSET(REPARSE_DATA_BUFFER,MountPointReparseBuffer.PathBuffer)+rdb.MountPointReparseBuffer.PrintNameOffset+rdb.MountPointReparseBuffer.PrintNameLength+1*sizeof(wchar_t)-REPARSE_DATA_BUFFER_HEADER_SIZE;

				if (rdb.ReparseDataLength+REPARSE_DATA_BUFFER_HEADER_SIZE<=static_cast<USHORT>(MAXIMUM_REPARSE_DATA_BUFFER_SIZE/sizeof(wchar_t))) {
					wmemcpy(&rdb.MountPointReparseBuffer.PathBuffer[rdb.MountPointReparseBuffer.SubstituteNameOffset/sizeof(wchar_t)],SubstituteName,SubstituteNameLength+1);
					wmemcpy(&rdb.MountPointReparseBuffer.PathBuffer[rdb.MountPointReparseBuffer.PrintNameOffset/sizeof(wchar_t)],PrintName,PrintNameLength+1);
					Result=true;
				}

				break;
			case IO_REPARSE_TAG_SYMLINK:
				rdb.SymbolicLinkReparseBuffer.PrintNameOffset=0;
				rdb.SymbolicLinkReparseBuffer.PrintNameLength=static_cast<WORD>(PrintNameLength*sizeof(wchar_t));
				rdb.SymbolicLinkReparseBuffer.SubstituteNameOffset=rdb.MountPointReparseBuffer.PrintNameLength;
				rdb.SymbolicLinkReparseBuffer.SubstituteNameLength=static_cast<WORD>(SubstituteNameLength*sizeof(wchar_t));
				rdb.ReparseDataLength=FIELD_OFFSET(REPARSE_DATA_BUFFER,SymbolicLinkReparseBuffer.PathBuffer)+rdb.SymbolicLinkReparseBuffer.SubstituteNameOffset+rdb.SymbolicLinkReparseBuffer.SubstituteNameLength-REPARSE_DATA_BUFFER_HEADER_SIZE;

				if (rdb.ReparseDataLength+REPARSE_DATA_BUFFER_HEADER_SIZE<=static_cast<USHORT>(MAXIMUM_REPARSE_DATA_BUFFER_SIZE/sizeof(wchar_t))) {
					wmemcpy(&rdb.SymbolicLinkReparseBuffer.PathBuffer[rdb.SymbolicLinkReparseBuffer.SubstituteNameOffset/sizeof(wchar_t)],SubstituteName,SubstituteNameLength);
					wmemcpy(&rdb.SymbolicLinkReparseBuffer.PathBuffer[rdb.SymbolicLinkReparseBuffer.PrintNameOffset/sizeof(wchar_t)],PrintName,PrintNameLength);
					Result=true;
				}
				break;
		}
		return Result;
	}

	size_t size() const {
		return MAXIMUM_REPARSE_DATA_BUFFER_SIZE;
	}

	PREPARSE_DATA_BUFFER operator->() const {
		return (PREPARSE_DATA_BUFFER)&rdb;
	}
private:
	union {
		BYTE				buf[MAXIMUM_REPARSE_DATA_BUFFER_SIZE];
		REPARSE_DATA_BUFFER	rdb;
	};
};

bool delete_reparse_point(PCWSTR path) {
	bool ret = false;
	DWORD attr = get_attributes(path);
	if (attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_REPARSE_POINT)) {
		if (attr & FILE_ATTRIBUTE_READONLY) {
			set_attributes(path, attr & ~FILE_ATTRIBUTE_READONLY);
		}
		REPARSE_BUF rdb;
		if (rdb.get(path)) {
			Privilege CreateSymlinkPrivilege(SE_CREATE_SYMBOLIC_LINK_NAME);
			auto_close<HANDLE> hLink(OpenLinkHandle(path, GENERIC_WRITE));
			if (hLink) {
				REPARSE_GUID_DATA_BUFFER rgdb = {0};
				rgdb.ReparseTag = rdb->ReparseTag;
				DWORD dwBytesReturned;
				ret = ::DeviceIoControl(hLink, FSCTL_DELETE_REPARSE_POINT, &rgdb,
				                           REPARSE_GUID_DATA_BUFFER_HEADER_SIZE, nullptr, 0, &dwBytesReturned, 0);
			}
		}
		if (attr & FILE_ATTRIBUTE_READONLY) {
			set_attributes(path, attr);
		}
	}
	return ret;
}

bool is_link(PCWSTR path) {
	REPARSE_BUF rdb;
	return rdb.get(path) && (rdb->ReparseTag == IO_REPARSE_TAG_MOUNT_POINT || rdb->ReparseTag == IO_REPARSE_TAG_SYMLINK);
}

bool is_symlink(PCWSTR path) {
	REPARSE_BUF rdb;
	return rdb.get(path) && rdb->ReparseTag == IO_REPARSE_TAG_SYMLINK;
}

bool is_junction(PCWSTR path) {
	REPARSE_BUF rdb;
	return rdb.get(path) && rdb->ReparseTag == IO_REPARSE_TAG_MOUNT_POINT;
}

bool copy_link(PCWSTR from, PCWSTR to) {
	bool ret = false;
	if (!is_exists(to)) {
		REPARSE_BUF rdb;
		if (rdb.get(from)) {
			DWORD attr = get_attributes(from);
			if (attr & FILE_ATTRIBUTE_DIRECTORY) {
				create_dir(to);
			} else {
				create_file(to);
			}
			ret = rdb.set(to);
			set_attributes(to, attr);
		}
	}
	return ret;
}

bool create_link(PCWSTR path, PCWSTR dest) {
	if (Empty(dest) || !is_exists(dest)) {
		return false;
	}

	if (Empty(path) || is_exists(path))
		return false;

	if (is_dir(dest))
		create_dir(path);
	else
		create_file(path);

	auto_close<HANDLE> hLink(OpenLinkHandle(path, GENERIC_WRITE));
	if (hLink) {
		AutoUTF SubstituteName (AutoUTF(L"\\??\\") + remove_path_prefix(dest));
		REPARSE_BUF rdb;
		rdb->ReparseTag = IO_REPARSE_TAG_SYMLINK;
		rdb.fill(dest, Len(dest), SubstituteName.c_str(), SubstituteName.size());
		if (rdb.set(path)) {
			return true;
		}
	}
	if (is_dir(dest))
		delete_dir(path);
	else
		delete_file(path);
	return false;
}

bool create_junc(PCWSTR path, PCWSTR dest) {
	if (Empty(dest) || !is_exists(dest)) {
		return false;
	}

	if (Empty(path) || is_exists(path))
		return false;

	create_dir(path);
	auto_close<HANDLE> hLink(OpenLinkHandle(path, GENERIC_WRITE));
	if (hLink) {
		AutoUTF SubstituteName (AutoUTF(L"\\??\\") + remove_path_prefix(dest));
		REPARSE_BUF rdb;
		rdb->ReparseTag = IO_REPARSE_TAG_MOUNT_POINT;
		rdb.fill(dest, Len(dest), SubstituteName.c_str(), SubstituteName.size());
		if (rdb.set(path)) {
			return true;
		}
	}
	delete_dir(path);
	return false;
}

bool delete_link(PCWSTR path) {
	bool ret = delete_reparse_point(path);
	if (ret) {
		if (is_dir(path))
			ret = delete_dir(path);
		else
			ret = delete_file(path);
	}
	return ret;
}

bool break_link(PCWSTR path) {
	return delete_reparse_point(path);
}

AutoUTF read_link(PCWSTR path) {
	AutoUTF ret;
	REPARSE_BUF rdb;
	if (rdb.get(path)) {
		size_t NameLength = 0;
		if (rdb->ReparseTag == IO_REPARSE_TAG_SYMLINK) {
			NameLength = rdb->SymbolicLinkReparseBuffer.PrintNameLength / sizeof(WCHAR);
			if (NameLength) {
				ret.assign(&rdb->SymbolicLinkReparseBuffer.PathBuffer[rdb->SymbolicLinkReparseBuffer.PrintNameOffset / sizeof(WCHAR)], NameLength);
			} else {
				NameLength = rdb->SymbolicLinkReparseBuffer.SubstituteNameLength / sizeof(WCHAR);
				ret.assign(&rdb->SymbolicLinkReparseBuffer.PathBuffer[rdb->SymbolicLinkReparseBuffer.SubstituteNameOffset / sizeof(WCHAR)], NameLength);
			}
		} else {
			NameLength = rdb->MountPointReparseBuffer.PrintNameLength / sizeof(WCHAR);
			if (NameLength) {
				ret.assign(&rdb->MountPointReparseBuffer.PathBuffer[rdb->MountPointReparseBuffer.PrintNameOffset / sizeof(WCHAR)], NameLength);
			} else 	{
				NameLength = rdb->MountPointReparseBuffer.SubstituteNameLength / sizeof(WCHAR);
				ret.assign(&rdb->MountPointReparseBuffer.PathBuffer[rdb->MountPointReparseBuffer.SubstituteNameOffset / sizeof(WCHAR)], NameLength);
			}
		}
	}
	if (ret.find(L"\\??\\") == 0) {
		ret.erase(0, 4);
	}
	return ret;
}
