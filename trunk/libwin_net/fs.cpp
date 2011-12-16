#include "file.h"
#include "exception.h"
#include "priv.h"
#include "reg.h"

#include <winioctl.h>

namespace FS {
	bool del_by_mask(PCWSTR mask) {
		bool Result = false;
		WIN32_FIND_DATAW wfd;
		HANDLE hFind = ::FindFirstFileW(mask, &wfd);
		if (hFind != INVALID_HANDLE_VALUE) {
			Result = true;
			ustring fullpath = get_path_from_mask(mask);
			do {
				if (!is_valid_filename(wfd.cFileName))
					continue;
				ustring path = MakePath(fullpath, wfd.cFileName);
				if (wfd.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) {
					Link::del(path);
				}
				if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
					del_by_mask(MakePath(path, L"*"));
					Result = Directory::del_nt(path);
				} else {
					Result = File::del_nt(path);
				}
			} while (::FindNextFileW(hFind, &wfd));
			::FindClose(hFind);
		}
		return Result;
	}
}


namespace	FileSys {
	HANDLE	HandleRead(PCWSTR path) {
		// Obtain backup/restore privilege in case we don't have it
		Privilege priv(SE_BACKUP_NAME);

		return CheckHandle(::CreateFileW(path, GENERIC_READ, FILE_SHARE_DELETE | FILE_SHARE_READ,
										 nullptr, OPEN_EXISTING,
										 FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS,
										 nullptr));
	}

	HANDLE	HandleWrite(PCWSTR path) {
		Privilege priv(SE_RESTORE_NAME);

		return CheckHandle(::CreateFileW(path, GENERIC_READ | GENERIC_WRITE, 0, nullptr,
										 OPEN_EXISTING, FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS,
										 nullptr));
	}
}

namespace File {
	void replace(PCWSTR from, PCWSTR to, PCWSTR backup) {
		CheckApi(::ReplaceFileW(from, to, backup, 0, nullptr, nullptr));
	}

}

namespace Directory {
	bool remove_dir(PCWSTR path, bool follow_links) {
		bool Result = false;
		if (is_path_mask(path)) {
			Result = FS::del_by_mask(path);
		} else {
			if (!FS::is_exists(path))
				return true;
			if (FS::is_dir(path)) {
				if (!follow_links && FS::is_link(path)) {
					Link::del(path);
				} else {
					FS::del_by_mask(MakePath(path, L"*"));
					Result = Directory::del_nt(path);
				}
			} else {
				Result = File::del_nt(path);
			}
		}
		return Result;
	}

	bool ensure_dir_exist(PCWSTR path, LPSECURITY_ATTRIBUTES lpsa) {
		if (FS::is_exists(path) && FS::is_dir(path))
			return true;
		CheckApiError(::SHCreateDirectoryExW(nullptr, path, lpsa));
		return true;
	}
}

void copy_file_security(PCWSTR path, PCWSTR dest) {
	WinSDW sd(path);
	SetSecurity(dest, sd, SE_FILE_OBJECT);
}

void SetOwnerRecur(const ustring &path, PSID owner, SE_OBJECT_TYPE type) {
	try {
		SetOwner(path, owner, type);
	} catch (...) {
	}
	if (FS::is_dir(path)) {
		WinDir dir(path);
		for (WinDir::iterator it = dir.begin(); it != dir.end(); ++it) {
			if (it.is_dir() || it.is_link_dir()) {
				SetOwnerRecur(it.path(), owner, type);
			} else {
				try {
					SetOwner(it.path(), owner, type);
				} catch (...) {
				}
			}
		}
	}
}

///========================================================================================= WinFile
WinFile::~WinFile() {
	::CloseHandle(m_hndl);
}

WinFile::WinFile(const ustring & path, bool write) :
	m_path(path) {
	Open(m_path, write);
}

WinFile::WinFile(const ustring & path, ACCESS_MASK access, DWORD share, PSECURITY_ATTRIBUTES sa, DWORD creat, DWORD flags) :
	m_path(path) {
	Open(m_path, access, share, sa, creat, flags);
}

//	bool	WinFile::Path(PWSTR path, size_t len) const {
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
//					return true;
//				}
//			}
//		}
//		return false;
//	}

uint64_t WinFile::size() const {
	uint64_t ret = 0;
	CheckApi(size_nt(ret));
	return ret;
}

bool WinFile::size_nt(uint64_t & size) const {
	LARGE_INTEGER fs;
	if (::GetFileSizeEx(m_hndl, &fs)) {
		size = fs.QuadPart;
		return true;
	}
	return false;
}

DWORD WinFile::read(PVOID data, size_t size) {
	DWORD read;
	CheckApi(read_nt(data, size, read));
	return read;
}

bool WinFile::read_nt(PVOID buf, size_t size, DWORD & read) {
	return ::ReadFile(m_hndl, buf, size, &read, nullptr);
}

DWORD WinFile::write(PCVOID buf, size_t size) {
	DWORD written;
	CheckApi(write_nt(buf, size, written));
	return written;
}

bool WinFile::write_nt(PCVOID buf, size_t size, DWORD & written) {
	return ::WriteFile(m_hndl, buf, size, &written, nullptr);
}

bool WinFile::set_attr(DWORD at) {
	return ::SetFileAttributesW(m_path.c_str(), at);
}

uint64_t WinFile::get_position() const {
	LARGE_INTEGER tmp, np;
	tmp.QuadPart = 0;
	CheckApi(::SetFilePointerEx(m_hndl, tmp, &np, FILE_CURRENT));
	return np.QuadPart;
}

void WinFile::set_position(int64_t dist, DWORD method) {
	CheckApi(set_position_nt(dist, method));
}

bool WinFile::set_position_nt(int64_t dist, DWORD method) {
	LARGE_INTEGER tmp;
	tmp.QuadPart = dist;
	return ::SetFilePointerEx(m_hndl, tmp, nullptr, method);
}

bool WinFile::set_eof() {
	return ::SetEndOfFile(m_hndl);
}

bool WinFile::set_time(const FILETIME & ctime, const FILETIME & atime, const FILETIME & mtime) {
	return ::SetFileTime(m_hndl, &ctime, &atime, &mtime);
}

bool WinFile::set_mtime(const FILETIME & mtime) {
	return ::SetFileTime(m_hndl, nullptr, nullptr, &mtime);
}

void WinFile::Open(const ustring & path, bool write) {
	ACCESS_MASK amask = (write) ? GENERIC_READ | GENERIC_WRITE : GENERIC_READ;
	DWORD share = (write) ? 0 : FILE_SHARE_DELETE | FILE_SHARE_READ;
	DWORD creat = (write) ? OPEN_ALWAYS : OPEN_EXISTING;
	DWORD flags = (write) ? FILE_ATTRIBUTE_NORMAL : FILE_FLAG_OPEN_REPARSE_POINT
				  | FILE_FLAG_BACKUP_SEMANTICS;
	Open(path, amask, share, nullptr, creat, flags);
}

void WinFile::Open(const ustring & path, ACCESS_MASK access, DWORD share, PSECURITY_ATTRIBUTES sa, DWORD creat, DWORD flags) {
	m_hndl = ::CreateFileW(path.c_str(), access, share, sa, creat, flags, nullptr);
	CheckHandleErr(m_hndl);
	WinFileInfo::refresh(m_hndl);
}

///========================================================================================== WinVol
void WinVol::Close() {
	if (m_hnd != INVALID_HANDLE_VALUE) {
		::FindVolumeClose(m_hnd);
		m_hnd = INVALID_HANDLE_VALUE;
	}
}

bool WinVol::Next() {
	WCHAR buf[MAX_PATH];
	if (m_hnd != INVALID_HANDLE_VALUE) {
		ChkSucc(::FindNextVolumeW(m_hnd, buf, sizeofa(buf)));
	} else {
		m_hnd = ::FindFirstVolumeW(buf, sizeofa(buf));
		ChkSucc(m_hnd != INVALID_HANDLE_VALUE);
	}
	if (IsOK()) {
		name = buf;
	}
	return IsOK();
}

ustring WinVol::GetPath() const {
	ustring Result;
	if (IsOK()) {
		DWORD size;
		::GetVolumePathNamesForVolumeNameW(name.c_str(), nullptr, 0, &size);
		if (::GetLastError() == ERROR_MORE_DATA) {
			auto_array<WCHAR> buf(size);
			::GetVolumePathNamesForVolumeNameW(name.c_str(), buf, size, &size);
			Result = buf.data();
			CutWord(Result, L"\\");
		}
	}
	return Result;
}

ustring WinVol::GetDevice() const {
	auto_array<WCHAR> Result(MAX_PATH);
	::QueryDosDeviceW(GetPath().c_str(), Result, Result.size());
	return ustring(Result);
}

bool WinVol::GetSize(uint64_t &uiUserFree, uint64_t &uiTotalSize, uint64_t &uiTotalFree) const {
	UINT mode = ::SetErrorMode(SEM_FAILCRITICALERRORS);
	bool Result = ::GetDiskFreeSpaceExW(name.c_str(), (PULARGE_INTEGER)&uiUserFree,
	                                    (PULARGE_INTEGER)&uiTotalSize,
	                                    (PULARGE_INTEGER)&uiTotalFree);
	::SetErrorMode(mode);
	return Result;
}

///=================================================================================================
bool FileWipe(PCWSTR path) {
//	{
//		DWORD attr = get_attributes(path);
//		if (!set_attributes(path, FILE_ATTRIBUTE_NORMAL))
//			return false;
//		WinFile WipeFile;
//		if (!WipeFile.Open(path, GENERIC_WRITE, FILE_SHARE_DELETE | FILE_SHARE_READ, nullptr,
//		                   OPEN_EXISTING, FILE_FLAG_WRITE_THROUGH | FILE_FLAG_SEQUENTIAL_SCAN)) {
//			set_attributes(path, attr);
//			return false;
//		}
//
//		uint64_t size = WipeFile.size();
//		{
//			const uint64_t BufSize = 65536;
//			char *buf[BufSize];
//			WinMem::Fill(buf, BufSize, (char)'\0'); // используем символ заполнитель
//
//			DWORD Written;
//			while (size > 0) {
//				DWORD WriteSize = std::min(BufSize, size);
//				WipeFile.Write(buf, WriteSize, Written);
//				size -= WriteSize;
//			}
//			WipeFile.Write(buf, BufSize, Written);
//		}
//		WipeFile.Pointer(0, FILE_BEGIN);
//		WipeFile.SetEnd();
//	}
//	ustring TmpName(TempFile(ExtractPath(path).c_str()));
//	if (!move_file(path, TmpName.c_str(), MOVEFILE_REPLACE_EXISTING))
//		return delete_file(path);
//	return delete_file(TmpName);
	return path;
}
