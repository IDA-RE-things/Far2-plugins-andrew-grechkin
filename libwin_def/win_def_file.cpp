#include "win_def.h"

#include <psapi.h>

extern "C" {
	INT WINAPI		SHCreateDirectoryExA(HWND, PCSTR, PSECURITY_ATTRIBUTES);
	INT WINAPI		SHCreateDirectoryExW(HWND, PCWSTR, PSECURITY_ATTRIBUTES);
	BOOL WINAPI		SHGetSpecialFolderPathW(HWND, LPWSTR, int, BOOL);
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
	PWSTR	ch = RFind((PWSTR)path, PATH_SEPARATOR_C);
	if (ch && ++ch < (path + len)) {
		return	AutoUTF(ch);
	}
	return	L"";
}
AutoUTF				ExtractFile(const AutoUTF &path, WCHAR sep) {
	return	ExtractFile(path.c_str());
}
AutoUTF				ExtractPath(PCWSTR path, WCHAR sep) {
	size_t	len = Len(path);
	PWSTR	ch = RFind((PWSTR)path, PATH_SEPARATOR_C);
	if (ch && ch < (path + len)) {
		return	AutoUTF(path, ch - path);
	}
	return	L"";
}
AutoUTF				ExtractPath(const AutoUTF &path, WCHAR sep) {
	return	ExtractPath(path.c_str());
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
bool file_exists(PCWSTR path) {
	return	::GetFileAttributesW(path) != INVALID_FILE_ATTRIBUTES;
}

bool is_file(PCWSTR path) {
	DWORD	attr = ::GetFileAttributesW(path);
	return attr != INVALID_FILE_ATTRIBUTES && 0 == (attr & FILE_ATTRIBUTE_DIRECTORY);
}

bool is_dir(PCWSTR path) {
	DWORD	attr = ::GetFileAttributesW(path);
	return attr != INVALID_FILE_ATTRIBUTES && 0 != (attr & FILE_ATTRIBUTE_DIRECTORY);
}

bool dir_is_empty(PCWSTR path) {
	return	::PathIsDirectoryEmptyW(path);
}

bool create_dir(PCWSTR path, LPSECURITY_ATTRIBUTES lpsa) {
	return	::SHCreateDirectoryExW(nullptr, path, lpsa) == ERROR_SUCCESS;
}

bool ensure_dir_exist(PCWSTR path) {
	if (file_exists(path) && is_dir(path))
		return true;
	return	create_dir(path);
}

bool is_path_mask(PCWSTR path) {
	PCWSTR	pos = CharLastOf(path, L"?*");
	return	(pos && pos != (path + 2));
}

AutoUTF remove_path_prefix(const AutoUTF &path, PCWSTR pref) {
	if (path.find(pref) == 0)
		return path.substr(Len(pref));
	return	path;
}

AutoUTF ensure_path_prefix(const AutoUTF &path, PCWSTR pref) {
	if (path.find(pref) != 0)
		return AutoUTF(pref) + path;
	return	path;
}

AutoUTF	get_path_from_mask(const AutoUTF &mask) {
	AutoUTF	tmp(remove_path_prefix(mask));
	AutoUTF::size_type pos = std::min(tmp.find_first_of(L"?*"), tmp.find_last_of(L"\\/"));
	if (pos != AutoUTF::npos) {
		tmp.erase(pos);
	}
	return	ensure_path_prefix(tmp);
}

bool delete_dir(PCWSTR path) {
	::SetFileAttributesW(path, FILE_ATTRIBUTE_NORMAL);
	return	::RemoveDirectoryW(path) != 0;
}

bool delete_file(PCWSTR path) {
	DWORD	attr = get_attributes(path);
	if (::SetFileAttributesW(path, FILE_ATTRIBUTE_NORMAL)) {
		if (::DeleteFileW(path)) {
			return	true;
		}
		::SetFileAttributesW(path, attr);
	}
	return	false;
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
	return	true;
}

bool delete_recycle(PCWSTR path) {
	SHFILEOPSTRUCTW	info = {0};
	info.wFunc	= FO_DELETE;
	info.pFrom	= path;
	info.fFlags	= FOF_ALLOWUNDO | FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT;
	return	::SHFileOperationW(&info) == 0;
}

bool read_file(PCWSTR path, CStrA &buf) {
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
bool				FileWipe(PCWSTR path) {
	{
		DWORD	attr = get_attributes(path);
		if (!set_attributes(path, FILE_ATTRIBUTE_NORMAL))
			return	false;
		WinFile	WipeFile;
		if (!WipeFile.Open(path, GENERIC_WRITE, FILE_SHARE_DELETE | FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_FLAG_WRITE_THROUGH | FILE_FLAG_SEQUENTIAL_SCAN)) {
			set_attributes(path, attr);
			return	false;
		}

		uint64_t	size = 0;
		if (!WipeFile.Size(size)) {
			set_attributes(path, attr);
			return	false;
		}
		{
			const uint64_t BufSize = 65536;
			char	*buf[BufSize];
			WinMem::Fill(buf, BufSize, (char)'\0'); // используем символ заполнитель

			DWORD	Written;
			while (size > 0) {
				DWORD	WriteSize = std::min(BufSize, size);
				WipeFile.Write(buf, WriteSize, Written);
				size -= WriteSize;
			}
			WipeFile.Write(buf, BufSize, Written);
		}
		WipeFile.Pointer(0, FILE_BEGIN);
		WipeFile.SetEnd();
	}
	AutoUTF	TmpName(TempFile(ExtractPath(path).c_str()));
	if (!move_file(path, TmpName.c_str(), MOVEFILE_REPLACE_EXISTING))
		return	delete_file(path);
	return	delete_file(TmpName);
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
bool				WinFile::Path(PWSTR path, size_t len) const {
	if (m_hndl && m_hndl != INVALID_HANDLE_VALUE) {
		// Create a file mapping object.
		HANDLE	hFileMap = ::CreateFileMapping(m_hndl, nullptr, PAGE_READONLY, 0, 1, nullptr);
		if (hFileMap) {
			PVOID	pMem = ::MapViewOfFile(hFileMap, FILE_MAP_READ, 0, 0, 1);
			if (pMem) {
				if (::GetMappedFileNameW(::GetCurrentProcess(), pMem, path, len)) {
					// Translate path with device name to drive letters.
					WCHAR	szTemp[len];
					szTemp[0] = L'\0';
					if (::GetLogicalDriveStringsW(len - 1, szTemp)) {
						WCHAR	szName[MAX_PATH], *p = szTemp;
						WCHAR	szDrive[3] = L" :";
						bool	bFound = false;

						do {
							// Copy the drive letter to the template string
							*szDrive = *p;
							// Look up each device name
							if (::QueryDosDeviceW(szDrive, szName, sizeofa(szName))) {
								size_t uNameLen = Len(szName);

								if (uNameLen < sizeofa(szName)) {
									bFound = Find(path, szName) == path;
									if (bFound) {
										// Reconstruct pszFilename using szTempFile Replace device path with DOS path
										WCHAR	szTempFile[MAX_PATH];
										_snwprintf(szTempFile, sizeofa(szTempFile), TEXT("%s%s"), szDrive, path + uNameLen);
										Copy(path, szTempFile, len);
									}
								}
							}
							// Go to the next nullptr character.
							while (*p++);
						} while (!bFound && *p); // end of string
					}
				}
				::UnmapViewOfFile(pMem);
				return	true;
			}
		}
	}
	return	false;
}

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
