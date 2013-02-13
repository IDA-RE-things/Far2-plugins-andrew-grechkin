#include <libext/file.hpp>
#include <libext/exception.hpp>
#include <libext/priv.hpp>
#include <libbase/std.hpp>
#include <libbase/path.hpp>

#include <wchar.h>

using namespace Base;

namespace Ext {

	///======================================================================================== Link
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
					wchar_t PathBuffer[1];
				} SymbolicLinkReparseBuffer;
				struct {
					USHORT SubstituteNameOffset;
					USHORT SubstituteNameLength;
					USHORT PrintNameOffset;
					USHORT PrintNameLength;
					wchar_t PathBuffer[1];
				} MountPointReparseBuffer;
				struct {
					UCHAR DataBuffer[1];
				} GenericReparseBuffer;
			};
		} REPARSE_DATA_BUFFER, *PREPARSE_DATA_BUFFER;

//#define IO_REPARSE_TAG_SYMLINK                  (0xA000000CL)
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
			REPARSE_BUF(PCWSTR path) {
				DWORD attr = FS::get_attr(path);
				if (!(attr & FILE_ATTRIBUTE_REPARSE_POINT)) {
					CheckApiError(ERROR_NOT_A_REPARSE_POINT);
				}
				auto_close<HANDLE> link(CheckHandle(OpenLinkHandle(path)));
				DWORD returned;
				CheckApi(::DeviceIoControl(link, FSCTL_GET_REPARSE_POINT, nullptr, 0, buf, size(), &returned, nullptr) && IsReparseTagValid(rdb.ReparseTag));
			}

			REPARSE_BUF(ULONG tag, PCWSTR PrintName, size_t PrintNameLength, PCWSTR SubstituteName, size_t SubstituteNameLength) {
				rdb.Reserved = 0;
				rdb.ReparseTag = tag;

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
						}
						break;
				}
			}

			bool set(PCWSTR path) const {
				bool ret = false;
				if (IsReparseTagValid(rdb.ReparseTag)) {
					DWORD attr = FS::get_attr(path);
					if (attr != INVALID_FILE_ATTRIBUTES) {
						if (attr & FILE_ATTRIBUTE_READONLY) {
							FS::set_attr(path, attr & ~FILE_ATTRIBUTE_READONLY);
						}
						Privilege CreateSymlinkPrivilege(SE_CREATE_SYMBOLIC_LINK_NAME);
						auto_close<HANDLE> hLink(OpenLinkHandle(path, GENERIC_WRITE));
						if (hLink) {
							DWORD dwBytesReturned;
							ret = ::DeviceIoControl(hLink, FSCTL_SET_REPARSE_POINT,(PVOID)&rdb, rdb.ReparseDataLength + REPARSE_DATA_BUFFER_HEADER_SIZE,
							                        nullptr, 0, &dwBytesReturned, nullptr);
						}
						if (attr & FILE_ATTRIBUTE_READONLY) {
							FS::set_attr(path, attr);
						}
					}
				}
				return ret;
			}

			bool is_symlink() const {
				return rdb.ReparseTag == IO_REPARSE_TAG_SYMLINK;
			}

			bool is_junction() const {
				return rdb.ReparseTag == IO_REPARSE_TAG_MOUNT_POINT;
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
	}

	namespace FS {
		bool is_link(PCWSTR path) {
			try {
				REPARSE_BUF rdb(path);
				return rdb->ReparseTag == IO_REPARSE_TAG_MOUNT_POINT || rdb->ReparseTag == IO_REPARSE_TAG_SYMLINK;
			} catch (...) {
			}
			return false;
		}

		bool is_symlink(PCWSTR path) {
			return REPARSE_BUF(path).is_symlink();
		}

		bool is_junction(PCWSTR path) {
			return REPARSE_BUF(path).is_junction();
		}
	}

	namespace Link {
		void copy(PCWSTR from, PCWSTR to) {
			REPARSE_BUF rdb(from);
			DWORD attr = FS::get_attr(from);
			if (attr & FILE_ATTRIBUTE_DIRECTORY) {
				Directory::create(to);
			} else {
				File::create(to);
			}
			rdb.set(to);
			FS::set_attr(to, attr);
		}

		bool create_sym(PCWSTR path, PCWSTR new_path) {
			if (Str::is_empty(path) || !FS::is_exist(path)) {
				return false;
			}

			if (Str::is_empty(new_path) || FS::is_exist(new_path))
				return false;

			if (FS::is_dir(path))
				Directory::create(new_path);
			else
				File::create(new_path);

			auto_close<HANDLE> hLink(OpenLinkHandle(new_path, GENERIC_WRITE));
			if (hLink) {
				ustring SubstituteName (ustring(L"\\??\\") + remove_path_prefix(path));
				REPARSE_BUF rdb(IO_REPARSE_TAG_SYMLINK, path, Str::length(path), SubstituteName.c_str(), SubstituteName.size());
				if (rdb.set(new_path)) {
					return true;
				}
			}
			FS::del_nt(new_path);
			return false;
		}

		bool create_junc(PCWSTR path, PCWSTR new_path) {
			if (Str::is_empty(path)/* || !FS::is_exists(dest)*/) {
				return false;
			}

			if (Str::is_empty(new_path) || FS::is_exist(new_path))
				return false;

			Directory::create(new_path);
			auto_close<HANDLE> hLink(OpenLinkHandle(new_path, GENERIC_WRITE));
			if (hLink) {
				ustring SubstituteName (ustring(L"\\??\\") + remove_path_prefix(path));
				REPARSE_BUF rdb(IO_REPARSE_TAG_MOUNT_POINT, path, Str::length(path), SubstituteName.c_str(), SubstituteName.size());
				if (rdb.set(new_path)) {
					return true;
				}
			}
			Directory::del_nt(new_path);
			return false;
		}

		void del(PCWSTR path) {
			break_link(path);
			FS::del(path);
		}

		void break_link(PCWSTR path) {
			DWORD attr = FS::get_attr(path);
			if (!(attr & FILE_ATTRIBUTE_REPARSE_POINT)) {
				CheckApiError(ERROR_NOT_A_REPARSE_POINT);
			}

			REPARSE_BUF rdb(path);
			if (attr & FILE_ATTRIBUTE_READONLY) {
				FS::set_attr(path, attr & ~FILE_ATTRIBUTE_READONLY);
			}


			Privilege CreateSymlinkPrivilege(SE_CREATE_SYMBOLIC_LINK_NAME);
			auto_close<HANDLE> hLink(OpenLinkHandle(path, GENERIC_WRITE));
			if (hLink) {
				REPARSE_GUID_DATA_BUFFER rgdb = {0};
				rgdb.ReparseTag = rdb->ReparseTag;
				DWORD dwBytesReturned;
				::DeviceIoControl(hLink, FSCTL_DELETE_REPARSE_POINT, &rgdb,
				                  REPARSE_GUID_DATA_BUFFER_HEADER_SIZE, nullptr, 0, &dwBytesReturned, 0);
			}
			if (attr & FILE_ATTRIBUTE_READONLY) {
				FS::set_attr(path, attr);
			}
		}

		ustring read(PCWSTR path) {
			ustring ret;
			REPARSE_BUF rdb(path);
			size_t NameLength = 0;
			if (rdb->ReparseTag == IO_REPARSE_TAG_SYMLINK) {
				NameLength = rdb->SymbolicLinkReparseBuffer.PrintNameLength / sizeof(wchar_t);
				if (NameLength) {
					ret.assign(&rdb->SymbolicLinkReparseBuffer.PathBuffer[rdb->SymbolicLinkReparseBuffer.PrintNameOffset / sizeof(wchar_t)], NameLength);
				} else {
					NameLength = rdb->SymbolicLinkReparseBuffer.SubstituteNameLength / sizeof(wchar_t);
					ret.assign(&rdb->SymbolicLinkReparseBuffer.PathBuffer[rdb->SymbolicLinkReparseBuffer.SubstituteNameOffset / sizeof(wchar_t)], NameLength);
				}
			} else {
				NameLength = rdb->MountPointReparseBuffer.PrintNameLength / sizeof(wchar_t);
				if (NameLength) {
					ret.assign(&rdb->MountPointReparseBuffer.PathBuffer[rdb->MountPointReparseBuffer.PrintNameOffset / sizeof(wchar_t)], NameLength);
				} else 	{
					NameLength = rdb->MountPointReparseBuffer.SubstituteNameLength / sizeof(wchar_t);
					ret.assign(&rdb->MountPointReparseBuffer.PathBuffer[rdb->MountPointReparseBuffer.SubstituteNameOffset / sizeof(wchar_t)], NameLength);
				}
			}
			if (ret.find(L"\\??\\") == 0) {
				ret.erase(0, 4);
			}
			return ret;
		}
	}

}
