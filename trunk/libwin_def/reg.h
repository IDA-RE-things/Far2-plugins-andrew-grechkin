#ifndef WIN_DEF_REG_H_
#define WIN_DEF_REG_H_

#include "std.h"

struct Register: private Uncopyable {
	~Register() {
		close();
	}

	Register():
		m_key(nullptr) {
	}

	void close() {
		if (m_key) {
			::RegCloseKey(m_key);
			m_key = nullptr;
		}
	}

	bool open_key(ACCESS_MASK acc, PCWSTR path, HKEY key = HKEY_CURRENT_USER) {
		close();
		bool ret = false;
		if (acc & KEY_WRITE)
			ret = ::RegCreateKeyExW(key, path, 0, nullptr, 0, acc, 0, &m_key, 0) == ERROR_SUCCESS;
		else
			ret = ::RegOpenKeyExW(key, path, 0, acc, &m_key) == ERROR_SUCCESS;
		return ret;
	}

	bool del(PCWSTR name) const {
		return ::RegDeleteValueW(m_key, name) == ERROR_SUCCESS;
	}

	uint64_t get(PCWSTR name, uint64_t def) const {
		if (m_key) {
			uint64_t value;
			DWORD size = sizeof(def);
			if(::RegQueryValueExW(m_key, name, nullptr, nullptr, (PBYTE)(&value), &size) == ERROR_SUCCESS)
				return value;
		}
		return def;
	}

	bool get(PCWSTR name, PWSTR value, DWORD size) const {
		if (m_key) {
			size *= sizeof(WCHAR);
			return ::RegQueryValueExW(m_key, name, nullptr, nullptr, (PBYTE)value, &size) == ERROR_SUCCESS;
		}
		return false;
	}

	bool set(PCWSTR name, PCVOID value, size_t size, DWORD type = REG_BINARY) const {
		if (m_key) {
			return ::RegSetValueExW(m_key, name, 0, type, (PBYTE)value, size) == ERROR_SUCCESS;
		}
		return false;
	}

	void set(PCWSTR name, uint64_t value) const {
		set(name, &value, sizeof(value), REG_QWORD);
	}

	void set(PCWSTR name, uint32_t value) const {
		set(name, &value, sizeof(value), REG_DWORD);
	}

	void set(PCWSTR name, PCWSTR value) const {
		set(name, value, sizeof(WCHAR) * (wcslen(value) + 1), REG_SZ);
	}

private:
	HKEY m_key;
};

#endif
