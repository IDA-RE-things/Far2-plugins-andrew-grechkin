#ifndef FAR_FILEVERSION_HPP
#define FAR_FILEVERSION_HPP

#include <libwin_def/std.h>
#include "lang.hpp"

struct version_dll {
	typedef DWORD (WINAPI *FGetFileVersionInfoSizeW)(LPCWSTR, LPDWORD);
	typedef DWORD (WINAPI *FVerLanguageNameW)(DWORD, LPWSTR, DWORD);
	typedef WINBOOL (WINAPI *FGetFileVersionInfoW)(LPCWSTR, DWORD, DWORD, LPVOID);
	typedef WINBOOL (WINAPI *FVerQueryValueW)(PCVOID, LPCWSTR, PCWSTR *, PUINT);

	DEFINE_FUNC(GetFileVersionInfoSizeW);
	DEFINE_FUNC(VerLanguageNameW);
	DEFINE_FUNC(GetFileVersionInfoW);
	DEFINE_FUNC(VerQueryValueW);

	static version_dll & inst() {
		static version_dll ret;
		return ret;
	}

	~version_dll() {
		::FreeLibrary(m_hnd);
	}

	bool is_ok() const {
		return m_hnd && GetFileVersionInfoSizeW && VerLanguageNameW && GetFileVersionInfoW && VerQueryValueW;
	}

private:
	version_dll():
		m_hnd(::LoadLibraryW(L"version.dll")) {
		GET_DLL_FUNC(GetFileVersionInfoSizeW);
		GET_DLL_FUNC(VerLanguageNameW);
		GET_DLL_FUNC(GetFileVersionInfoW);
		GET_DLL_FUNC(VerQueryValueW);
	}

	FARPROC get_function(PCSTR name) const {
		return ::GetProcAddress(m_hnd, name);
	}

	HMODULE m_hnd;
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

	bool IsOK() const {
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
