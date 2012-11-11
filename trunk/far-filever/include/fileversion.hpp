/**
	filever: File Version FAR plugin
	Displays version information from file resource in dialog
	FAR3 plugin

	© 2012 Andrew Grechkin

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
**/

#ifndef FAR_FILEVERSION_HPP
#define FAR_FILEVERSION_HPP

#include <libbase/std.hpp>
#include <libbase/dll.hpp>

#include "lang.hpp"


struct version_dll: private Base::DynamicLibrary {
	typedef DWORD (WINAPI *FGetFileVersionInfoSizeW)(LPCWSTR, LPDWORD);
	typedef DWORD (WINAPI *FVerLanguageNameW)(DWORD, LPWSTR, DWORD);
	typedef WINBOOL (WINAPI *FGetFileVersionInfoW)(LPCWSTR, DWORD, DWORD, LPVOID);
	typedef WINBOOL (WINAPI *FVerQueryValueW)(PCVOID, LPCWSTR, PCWSTR *, PUINT);

	DEFINE_FUNC(GetFileVersionInfoSizeW);
	DEFINE_FUNC(VerLanguageNameW);
	DEFINE_FUNC(GetFileVersionInfoW);
	DEFINE_FUNC(VerQueryValueW);

	static version_dll & inst();

	bool is_valid() const;

private:
	version_dll();
};

struct FileVersion {
	~FileVersion();

	FileVersion(PCWSTR path);

	PCWSTR ver() const {
		return	m_ver;
	}

	PCWSTR lng() const {
		return	m_lng;
	}

	PCWSTR lngID() const {
		return	m_lngId;
	}

	PCWSTR lngIDerr() const {
		return	m_lngIderr;
	}

	bool is_ok() const {
		return	m_data;
	}

	PCVOID GetData() const {
		return	m_data;
	}

	WORD machine() const {
		return	m_machine;
	}

	bool Is64Bit() const {
		return	m_machine == IMAGE_FILE_MACHINE_IA64 || m_machine == IMAGE_FILE_MACHINE_AMD64;
	};

private:
	WCHAR m_ver[32];
	WCHAR m_lng[32];
	WCHAR m_lngId[16];
	WCHAR m_lngIderr[16];
	PBYTE m_data;
	WORD m_MajorVersion, m_MinorVersion;
	WORD m_BuildNumber, m_RevisionNumber;

	WORD m_machine;
	WORD m_flags;
};

struct FileVerInfo_ {
	PCWSTR data;
	PCWSTR SubBlock;
	FarMessage msgTxt;
};

struct FVI {
	FVI(const FileVersion & in);

	size_t size() const {
		return m_size;
	}

	FileVerInfo_ & operator [](int index) {
		return m_data[index];
	}

private:
	FileVerInfo_ * m_data;
	size_t m_size;
};

#endif
