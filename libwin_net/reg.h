/**
	win_reg

	@classes	()
	@author		© 2009 Andrew Grechkin
	@link		()
**/

#ifndef WIN_REG_HPP
#define WIN_REG_HPP

#include "win_net.h"

///========================================================================================== WinReg
struct WinReg: private Uncopyable {
	~WinReg() {
		CloseKey();
	}
	WinReg(): hKeyOpend(0), hKeyReq(HKEY_CURRENT_USER) {
	}
	WinReg(HKEY hkey, const ustring &path): hKeyOpend(0), hKeyReq(hkey), m_path(path) {
	}
	WinReg(const ustring &path);

	ustring	path() const {
		return m_path;
	}
	void	path(const ustring &path) {
		m_path = path;
	}
	void	key(HKEY hkey) {
		hKeyReq = hkey;
	}

	bool	Add(const ustring &name) const;
	bool	Del(const ustring &name) const;

	void	Set(const ustring &name, PCWSTR value) const;
	void	Set(const ustring &name, const ustring &value) const {
		Set(name, value.c_str());
	}
	void	Set(const ustring &name, int value) const;

	bool	Get(const ustring &name, ustring &value, const ustring &def) const;
	bool	Get(const ustring &name, int &value, int def) const;

private:
	HKEY	mutable	hKeyOpend;
	HKEY	hKeyReq;
	ustring	m_path;

	void	CloseKey() const;
	bool	OpenKey(ACCESS_MASK acc) const {
		return OpenKey(hKeyReq, m_path, acc);
	}
	bool	OpenKey(HKEY hkey, const ustring &path, ACCESS_MASK acc) const;

	template <typename Type>
	void	SetRaw(const ustring &name, const Type &value, DWORD type = REG_BINARY) const {
		if (OpenKey(KEY_WRITE)) {
			::RegSetValueExW(hKeyOpend, name.c_str(), 0, type, (PBYTE)(&value), sizeof(value));
			CloseKey();
		}
	}
	template <typename Type>
	bool	GetRaw(const ustring &name, Type &value, const Type &def) const {
		bool Result = OpenKey(KEY_READ);
		value = def;
		if (Result) {
			DWORD	size = sizeof(value);
			Result = ::RegQueryValueExW(hKeyOpend, name.c_str(), nullptr, nullptr, (PBYTE)(&value), &size) == ERROR_SUCCESS;
			CloseKey();
		}
		return Result;
	}
};

#endif
