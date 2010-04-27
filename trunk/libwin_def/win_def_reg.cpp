#include "win_def.h"

///========================================================================================== WinReg
void			WinReg::CloseKey() const {
	if (hKeyOpend) {
		::RegCloseKey(hKeyOpend);
		hKeyOpend = NULL;
	}
}
bool			WinReg::OpenKey(HKEY hkey, const AutoUTF &path, ACCESS_MASK acc) const {
	CloseKey();
	bool	Result = false;
	if (acc == KEY_READ)
		Result = ::RegOpenKeyExW(hkey, path.c_str(), 0, acc, &hKeyOpend) == ERROR_SUCCESS;
	else
		Result = ::RegCreateKeyExW(hkey, path.c_str(), 0, NULL, 0, acc, 0, &hKeyOpend, 0) == ERROR_SUCCESS;
	return	Result;
}

WinReg::WinReg(const AutoUTF &path): hKeyOpend(0), hKeyReq(0), m_path(path) {
	hKeyReq = HKEY_CURRENT_USER;
	AutoUTF	tmp = L"HKEY_CURRENT_USER\\";
	if (Find(m_path.c_str(), tmp.c_str())) {
		m_path = m_path.c_str() + tmp.size();
		return;
	}
	tmp = L"HKCU\\";
	if (Find(m_path.c_str(), tmp.c_str())) {
		m_path = m_path.c_str() + tmp.size();
		return;
	}
	tmp = L"HKEY_LOCAL_MACHINE\\";
	if (Find(m_path.c_str(), tmp.c_str())) {
		m_path = m_path.c_str() + tmp.size();
		hKeyReq = HKEY_LOCAL_MACHINE;
		return;
	}
	tmp = L"HKLM\\";
	if (Find(m_path.c_str(), tmp.c_str())) {
		m_path = m_path.c_str() + tmp.size();
		hKeyReq = HKEY_LOCAL_MACHINE;
		return;
	}
	tmp = L"HKEY_USERS\\";
	if (Find(m_path.c_str(), tmp.c_str())) {
		m_path = m_path.c_str() + tmp.size();
		hKeyReq = HKEY_USERS;
		return;
	}
	tmp = L"HKU\\";
	if (Find(m_path.c_str(), tmp.c_str())) {
		m_path = m_path.c_str() + tmp.size();
		hKeyReq = HKEY_USERS;
		return;
	}
	tmp = L"HKEY_CLASSES_ROOT\\";
	if (Find(m_path.c_str(), tmp.c_str())) {
		m_path = m_path.c_str() + tmp.size();
		hKeyReq = HKEY_CLASSES_ROOT;
		return;
	}
	tmp = L"HKCR\\";
	if (Find(m_path.c_str(), tmp.c_str())) {
		m_path = m_path.c_str() + tmp.size();
		hKeyReq = HKEY_CLASSES_ROOT;
		return;
	}
}

bool			WinReg::Add(const AutoUTF &name) const {
	bool	Result = OpenKey(KEY_WRITE);
	if (Result) {
		HKEY tmp = NULL;
		Result = (::RegCreateKeyW(hKeyOpend, name.c_str(), &tmp) == ERROR_SUCCESS);
		if (Result) {
			::RegCloseKey(tmp);
		}
		CloseKey();
	}
	return	Result;
}
bool			WinReg::Del(const AutoUTF &name) const {
	bool	Result = OpenKey(KEY_WRITE);
	if (Result) {
		Result = (::RegDeleteValueW(hKeyOpend, name.c_str()) == ERROR_SUCCESS);
		CloseKey();
	}
	return	Result;
}

void			WinReg::Set(const AutoUTF &name, PCWSTR value) const {
	if (OpenKey(KEY_WRITE)) {
		::RegSetValueExW(hKeyOpend, name.c_str(), NULL, REG_SZ, (PBYTE)value, (Len(value) + 1) * sizeof(TCHAR));
		CloseKey();
	}
}
void			WinReg::Set(const AutoUTF &name, int value) const {
	SetRaw(name, value, REG_DWORD);
}

bool			WinReg::Get(const AutoUTF &name, AutoUTF &value, const AutoUTF &def) const {
	bool	Result = OpenKey(KEY_READ);
	value = def;
	if (Result) {
		DWORD	size = 0;
		if (::RegQueryValueExW(hKeyOpend, name.c_str(), NULL, NULL, NULL, &size) == ERROR_MORE_DATA) {
			WCHAR	data[size];
			if (::RegQueryValueExW(hKeyOpend, name.c_str(), NULL, NULL, (PBYTE)data, &size) == ERROR_SUCCESS) {
				value = data;
				Result = true;
			}
		}
		CloseKey();
	}
	return	Result;
}
bool			WinReg::Get(const AutoUTF &name, int &value, int def) const {
	return	GetRaw(name, value, def);
}
