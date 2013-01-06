#include <libext/reg.hpp>
#include <libext/exception.hpp>
#include <libbase/logger.hpp>
#include <libbase/pcstr.hpp>
#include <libbase/bit.hpp>

#include <cassert>

using namespace Base;

namespace Ext {

//	void	WinReg::CloseKey() const {
//		if (hKeyOpend) {
//			::RegCloseKey(hKeyOpend);
//			hKeyOpend = nullptr;
//		}
//	}
//
//	bool	WinReg::OpenKey(HKEY hkey, const ustring & path, ACCESS_MASK acc) const {
//		CloseKey();
//		bool	Result = false;
//		if (Base::WinFlag::Check(acc, (ACCESS_MASK)KEY_READ))
//			Result = ::RegOpenKeyExW(hkey, path.c_str(), 0, acc, &hKeyOpend) == ERROR_SUCCESS;
//		else
//			Result = ::RegCreateKeyExW(hkey, path.c_str(), 0, nullptr, 0, acc, 0, &hKeyOpend, 0) == ERROR_SUCCESS;
//		return Result;
//	}
//
//	WinReg::WinReg(const ustring & path): hKeyOpend(0), hKeyReq(0), m_path(path) {
//		hKeyReq = HKEY_CURRENT_USER;
//		ustring	tmp = L"HKEY_CURRENT_USER\\";
//		if (find_str(m_path.c_str(), tmp.c_str())) {
//			m_path = m_path.c_str() + tmp.size();
//			return;
//		}
//		tmp = L"HKCU\\";
//		if (find_str(m_path.c_str(), tmp.c_str())) {
//			m_path = m_path.c_str() + tmp.size();
//			return;
//		}
//		tmp = L"HKEY_LOCAL_MACHINE\\";
//		if (find_str(m_path.c_str(), tmp.c_str())) {
//			m_path = m_path.c_str() + tmp.size();
//			hKeyReq = HKEY_LOCAL_MACHINE;
//			return;
//		}
//		tmp = L"HKLM\\";
//		if (find_str(m_path.c_str(), tmp.c_str())) {
//			m_path = m_path.c_str() + tmp.size();
//			hKeyReq = HKEY_LOCAL_MACHINE;
//			return;
//		}
//		tmp = L"HKEY_USERS\\";
//		if (find_str(m_path.c_str(), tmp.c_str())) {
//			m_path = m_path.c_str() + tmp.size();
//			hKeyReq = HKEY_USERS;
//			return;
//		}
//		tmp = L"HKU\\";
//		if (find_str(m_path.c_str(), tmp.c_str())) {
//			m_path = m_path.c_str() + tmp.size();
//			hKeyReq = HKEY_USERS;
//			return;
//		}
//		tmp = L"HKEY_CLASSES_ROOT\\";
//		if (find_str(m_path.c_str(), tmp.c_str())) {
//			m_path = m_path.c_str() + tmp.size();
//			hKeyReq = HKEY_CLASSES_ROOT;
//			return;
//		}
//		tmp = L"HKCR\\";
//		if (find_str(m_path.c_str(), tmp.c_str())) {
//			m_path = m_path.c_str() + tmp.size();
//			hKeyReq = HKEY_CLASSES_ROOT;
//			return;
//		}
//	}
//
//	bool	WinReg::Add(const ustring & name) const {
//		bool	Result = OpenKey(KEY_WRITE);
//		if (Result) {
//			HKEY tmp = nullptr;
//			Result = (::RegCreateKeyW(hKeyOpend, name.c_str(), &tmp) == ERROR_SUCCESS);
//			if (Result) {
//				::RegCloseKey(tmp);
//			}
//			CloseKey();
//		}
//		return Result;
//	}
//
//	bool	WinReg::Del(const ustring & name) const {
//		bool	Result = OpenKey(KEY_WRITE);
//		if (Result) {
//			Result = (::RegDeleteValueW(hKeyOpend, name.c_str()) == ERROR_SUCCESS);
//			CloseKey();
//		}
//		return Result;
//	}
//
//	void	WinReg::Set(const ustring & name, PCWSTR value) const {
//		if (OpenKey(KEY_WRITE)) {
//			::RegSetValueExW(hKeyOpend, name.c_str(), 0, REG_SZ, (PBYTE)value, (get_str_len(value) + 1) * sizeof(WCHAR));
//			CloseKey();
//		}
//	}
//
//	void	WinReg::Set(const ustring & name, int value) const {
//		SetRaw(name, value, REG_DWORD);
//	}
//
//	bool	WinReg::Get(const ustring & name, ustring &value, const ustring & def) const {
//		bool	Result = OpenKey(KEY_READ);
//		value = def;
//		if (Result) {
//			Result = false;
//			DWORD	size = 0;
//			DWORD	type = 0;
//			DWORD	err = ::RegQueryValueExW(hKeyOpend, name.c_str(), nullptr, &type, nullptr, &size);
//			if (err == ERROR_SUCCESS && (type == REG_EXPAND_SZ ||
//				type == REG_LINK || type == REG_MULTI_SZ || type == REG_SZ)) {
//				WCHAR	data[size];
//				if (::RegQueryValueExW(hKeyOpend, name.c_str(), nullptr, nullptr, (PBYTE)data, &size) == ERROR_SUCCESS) {
//					value = data;
//					Result = true;
//				}
//			}
//			CloseKey();
//		}
//		return Result;
//	}
//
//	bool	WinReg::Get(const ustring & name, int &value, int def) const {
//		return GetRaw(name, value, def);
//	}


	///==================================================================================== Register
	Register::~Register() {
		if (m_hndl)
			::RegCloseKey(m_hndl);
	}

	Register::Register(PCWSTR path, HKEY hkey, ACCESS_MASK acc):
		m_hndl(nullptr),
		m_access(acc)
	{
//		LogDebug(L"path: '%s'\n", path);
		if (acc & KEY_READ)
			CheckApiError(::RegOpenKeyExW(hkey, path, 0, acc, &m_hndl));
		else
			CheckApiError(::RegCreateKeyExW(hkey, path, 0, nullptr, 0, acc, 0, &m_hndl, 0));
	}

	Register::Register(Register && right):
		m_hndl(std::move(right.m_hndl)),
		m_access(std::move(right.m_access))
	{
		assert(right.m_hndl == nullptr);
	}

	Register & Register::operator = (Register & right) {
		m_hndl = std::move(right.m_hndl);
		m_access = std::move(right.m_access);
		assert(right.m_hndl == nullptr);
		return *this;
	}


	bool is_exist_key(PCWSTR path, HKEY hkey) {
		HKEY l_key = nullptr;
		bool ret = ::RegOpenKeyExW(hkey, path, 0, KEY_READ, &l_key) == ERROR_SUCCESS;
		::RegCloseKey(l_key);
		return ret;
	}

	size_t Register::get(PCWSTR name, PVOID value, size_t size) const {
		DWORD l_size = size;
		CheckApi(::RegQueryValueExW(m_hndl, name, nullptr, nullptr, (PBYTE)&value, &l_size));
		return l_size;
	}

	ustring Register::get(PCWSTR name, PCWSTR def) const {
//		LogDebug(L"name: '%s', def: '%s'\n", name, def);
		Base::auto_array<WCHAR> buf(Base::Str::length(def) + 1, def);
		DWORD l_size = buf.size_in_bytes();
		if (::RegQueryValueExW(m_hndl, name, nullptr, nullptr, (PBYTE)buf.data(), &l_size) == ERROR_MORE_DATA) {
			buf.reserve(l_size / sizeof(WCHAR));
			::RegQueryValueExW(m_hndl, name, nullptr, nullptr, (PBYTE)buf.data(), &l_size);
		}
//		LogDebug(L"return: '%s'\n", (PCWSTR)buf.data());
		return ustring(buf.data());
	}

	uint64_t Register::get(PCWSTR name, uint64_t def) const {
		uint64_t ret = def;
		DWORD l_size = sizeof(def);
		::RegQueryValueExW(m_hndl, name, nullptr, nullptr, (PBYTE)&ret, &l_size);
		return ret;
	}

	int64_t Register::get(PCWSTR name, int64_t def) const {
		int64_t ret = def;
		DWORD l_size = sizeof(def);
		::RegQueryValueExW(m_hndl, name, nullptr, nullptr, (PBYTE)&ret, &l_size);
		return ret;
	}

	uint32_t Register::get(PCWSTR name, uint32_t def) const {
		uint32_t ret = def;
		DWORD l_size = sizeof(def);
		::RegQueryValueExW(m_hndl, name, nullptr, nullptr, (PBYTE)&ret, &l_size);
		return ret;
	}

	int32_t Register::get(PCWSTR name, int32_t def) const {
		int32_t ret = def;
		DWORD l_size = sizeof(def);
		::RegQueryValueExW(m_hndl, name, nullptr, nullptr, (PBYTE)&ret, &l_size);
		return ret;
	}

	bool Register::get(PCWSTR name, bool def) const {
		bool ret = def;
		DWORD l_size = sizeof(def);
		::RegQueryValueExW(m_hndl, name, nullptr, nullptr, (PBYTE)&ret, &l_size);
		return ret;
	}

	void Register::set(PCWSTR name, PCVOID value, size_t size) {
		CheckApiError(::RegSetValueExW(m_hndl, name, 0, REG_BINARY, (PBYTE)&value, size));
	}

	void Register::set(PCWSTR name, PCWSTR value) {
		CheckApiError(::RegSetValueExW(m_hndl, name, 0, REG_SZ, (PBYTE)&value, (Base::Str::length(value) + 1) * sizeof(WCHAR)));
	}

	void Register::set(PCWSTR name, uint64_t value) {
		CheckApiError(::RegSetValueExW(m_hndl, name, 0, REG_QWORD, (PBYTE)&value, sizeof(value)));
	}

	Register::operator HKEY () const {
		return m_hndl;
	}

	Register Register::open_subkey(PCWSTR name, ACCESS_MASK acc) const {
		return Register(name, m_hndl, acc ? acc : m_access);
	}

	Register Register::open_subkey_stored_in_key(PCWSTR key_name, ACCESS_MASK acc) const {
		return Register(get(key_name, Base::EMPTY_STR).c_str(), m_hndl, acc ? acc : m_access);
	}
}
