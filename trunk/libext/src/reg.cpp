#include <libext/reg.hpp>
#include <libbase/pcstr.hpp>
#include <libbase/bit.hpp>

using namespace Base;

namespace Ext {

///========================================================================================== WinReg
void	WinReg::CloseKey() const {
	if (hKeyOpend) {
		::RegCloseKey(hKeyOpend);
		hKeyOpend = nullptr;
	}
}

bool	WinReg::OpenKey(HKEY hkey, const ustring & path, ACCESS_MASK acc) const {
	CloseKey();
	bool	Result = false;
	if (Base::WinFlag::Check(acc, (ACCESS_MASK)KEY_READ))
		Result = ::RegOpenKeyExW(hkey, path.c_str(), 0, acc, &hKeyOpend) == ERROR_SUCCESS;
	else
		Result = ::RegCreateKeyExW(hkey, path.c_str(), 0, nullptr, 0, acc, 0, &hKeyOpend, 0) == ERROR_SUCCESS;
	return Result;
}

WinReg::WinReg(const ustring & path): hKeyOpend(0), hKeyReq(0), m_path(path) {
	hKeyReq = HKEY_CURRENT_USER;
	ustring	tmp = L"HKEY_CURRENT_USER\\";
	if (find_str(m_path.c_str(), tmp.c_str())) {
		m_path = m_path.c_str() + tmp.size();
		return;
	}
	tmp = L"HKCU\\";
	if (find_str(m_path.c_str(), tmp.c_str())) {
		m_path = m_path.c_str() + tmp.size();
		return;
	}
	tmp = L"HKEY_LOCAL_MACHINE\\";
	if (find_str(m_path.c_str(), tmp.c_str())) {
		m_path = m_path.c_str() + tmp.size();
		hKeyReq = HKEY_LOCAL_MACHINE;
		return;
	}
	tmp = L"HKLM\\";
	if (find_str(m_path.c_str(), tmp.c_str())) {
		m_path = m_path.c_str() + tmp.size();
		hKeyReq = HKEY_LOCAL_MACHINE;
		return;
	}
	tmp = L"HKEY_USERS\\";
	if (find_str(m_path.c_str(), tmp.c_str())) {
		m_path = m_path.c_str() + tmp.size();
		hKeyReq = HKEY_USERS;
		return;
	}
	tmp = L"HKU\\";
	if (find_str(m_path.c_str(), tmp.c_str())) {
		m_path = m_path.c_str() + tmp.size();
		hKeyReq = HKEY_USERS;
		return;
	}
	tmp = L"HKEY_CLASSES_ROOT\\";
	if (find_str(m_path.c_str(), tmp.c_str())) {
		m_path = m_path.c_str() + tmp.size();
		hKeyReq = HKEY_CLASSES_ROOT;
		return;
	}
	tmp = L"HKCR\\";
	if (find_str(m_path.c_str(), tmp.c_str())) {
		m_path = m_path.c_str() + tmp.size();
		hKeyReq = HKEY_CLASSES_ROOT;
		return;
	}
}

bool	WinReg::Add(const ustring & name) const {
	bool	Result = OpenKey(KEY_WRITE);
	if (Result) {
		HKEY tmp = nullptr;
		Result = (::RegCreateKeyW(hKeyOpend, name.c_str(), &tmp) == ERROR_SUCCESS);
		if (Result) {
			::RegCloseKey(tmp);
		}
		CloseKey();
	}
	return Result;
}

bool	WinReg::Del(const ustring & name) const {
	bool	Result = OpenKey(KEY_WRITE);
	if (Result) {
		Result = (::RegDeleteValueW(hKeyOpend, name.c_str()) == ERROR_SUCCESS);
		CloseKey();
	}
	return Result;
}

void	WinReg::Set(const ustring & name, PCWSTR value) const {
	if (OpenKey(KEY_WRITE)) {
		::RegSetValueExW(hKeyOpend, name.c_str(), 0, REG_SZ, (PBYTE)value, (get_str_len(value) + 1) * sizeof(WCHAR));
		CloseKey();
	}
}

void	WinReg::Set(const ustring & name, int value) const {
	SetRaw(name, value, REG_DWORD);
}

bool	WinReg::Get(const ustring & name, ustring &value, const ustring & def) const {
	bool	Result = OpenKey(KEY_READ);
	value = def;
	if (Result) {
		Result = false;
		DWORD	size = 0;
		DWORD	type = 0;
		DWORD	err = ::RegQueryValueExW(hKeyOpend, name.c_str(), nullptr, &type, nullptr, &size);
		if (err == ERROR_SUCCESS && (type == REG_EXPAND_SZ ||
					type == REG_LINK || type == REG_MULTI_SZ || type == REG_SZ)) {
			WCHAR	data[size];
			if (::RegQueryValueExW(hKeyOpend, name.c_str(), nullptr, nullptr, (PBYTE)data, &size) == ERROR_SUCCESS) {
				value = data;
				Result = true;
			}
		}
		CloseKey();
	}
	return Result;
}

bool	WinReg::Get(const ustring & name, int &value, int def) const {
	return GetRaw(name, value, def);
}

}
