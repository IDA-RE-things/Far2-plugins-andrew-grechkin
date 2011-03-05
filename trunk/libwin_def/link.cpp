#include "link.h"
#include "priv.h"

#include <wchar.h>

///============================================================================================ Link
//#ifndef SE_CREATE_SYMBOLIC_LINK_NAME
//#define SE_CREATE_SYMBOLIC_LINK_NAME      L"SeCreateSymbolicLinkPrivilege"
//#endif

#ifndef IO_REPARSE_TAG_VALID_VALUES
#define IO_REPARSE_TAG_VALID_VALUES 0xF000FFFF
#endif

#ifndef IsReparseTagValid
#define IsReparseTagValid(_tag) (!((_tag)&~IO_REPARSE_TAG_VALID_VALUES)&&((_tag)>IO_REPARSE_TAG_RESERVED_RANGE))
#endif

namespace {
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
		return ::CreateFileW(path, acc, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		                     nullptr, create,
		      	             FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS,
		      	             nullptr);
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
			return sizeofa(buf);
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

