#ifndef WIN_DEF_REG_H_
#define WIN_DEF_REG_H_

#include "win_def.h"

class Register: private Uncopyable {
public:
	~Register() {
		Close();
	}

	Register(): m_key(nullptr) {
	}

	void	Close() {
		if (m_key) {
			::RegCloseKey(m_key);
			m_key = nullptr;
		}
	}

	bool	Open(ACCESS_MASK acc, PCWSTR path, HKEY key = HKEY_CURRENT_USER) {
		Close();
		bool	ret = false;
		if (WinFlag::Check(acc, KEY_WRITE))
			ret = ::RegCreateKeyExW(key, path, 0, nullptr, 0, acc, 0, &m_key, 0) == ERROR_SUCCESS;
		else
			ret = ::RegOpenKeyExW(key, path, 0, acc, &m_key) == ERROR_SUCCESS;
		return	ret;
	}

	template <typename Type1, typename Type2>
	bool	Get(PCWSTR name, Type1 &value, const Type2 &def) const {
		value = def;
		if (m_key) {
			DWORD	size = sizeof(value);
			return ::RegQueryValueExW(m_key, name, nullptr, nullptr, (PBYTE)(&value), &size) == ERROR_SUCCESS;
		}
		return	false;
	}

	bool	GetStr(PCWSTR name, PWSTR value, DWORD size) const {
		if (m_key) {
			return ::RegQueryValueExW(m_key, name, nullptr, nullptr, (PBYTE)value, &size) == ERROR_SUCCESS;
		}
		return false;
	}

	template <typename Type>
	bool	SetRaw(PCWSTR name, const Type &value, DWORD type = REG_BINARY) const {
		if (m_key) {
			return ::RegSetValueExW(m_key, name, 0, type, (PBYTE)(&value), sizeof(value))  == ERROR_SUCCESS;
		}
		return false;
	}

	void	Set(PCWSTR name, int value) const {
		SetRaw(name, value, REG_DWORD);
	}

	bool	Set(PCWSTR name, PCWSTR value) const {
		if (m_key) {
			return ::RegSetValueExW(m_key, name, 0, REG_SZ, (PBYTE)value, (Len(value) + 1) * sizeof(WCHAR))  == ERROR_SUCCESS;
		}
		return false;
	}

	bool	Del(PCWSTR name) const {
		return ::RegDeleteValueW(m_key, name) == ERROR_SUCCESS;
	}

private:
	HKEY m_key;
};

#endif
