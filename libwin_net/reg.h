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
class	WinReg: private Uncopyable {
	HKEY	mutable	hKeyOpend;
	HKEY			hKeyReq;
	AutoUTF			m_path;

	void			CloseKey() const;
	bool			OpenKey(ACCESS_MASK acc) const {
		return	OpenKey(hKeyReq, m_path, acc);
	}
	bool			OpenKey(HKEY hkey, const AutoUTF &path, ACCESS_MASK acc) const;

	template <typename Type>
	void			SetRaw(const AutoUTF &name, const Type &value, DWORD type = REG_BINARY) const {
		if (OpenKey(KEY_WRITE)) {
			::RegSetValueExW(hKeyOpend, name.c_str(), 0, type, (PBYTE)(&value), sizeof(value));
			CloseKey();
		}
	}
	template <typename Type>
	bool			GetRaw(const AutoUTF &name, Type &value, const Type &def) const {
		bool	Result = OpenKey(KEY_READ);
		value = def;
		if (Result) {
			DWORD	size = sizeof(value);
			Result = ::RegQueryValueExW(hKeyOpend, name.c_str(), nullptr, nullptr, (PBYTE)(&value), &size) == ERROR_SUCCESS;
			CloseKey();
		}
		return	Result;
	}
public:
	~WinReg() {
		CloseKey();
	}
	WinReg(): hKeyOpend(0), hKeyReq(HKEY_CURRENT_USER) {
	}
	WinReg(HKEY hkey, const AutoUTF &path): hKeyOpend(0), hKeyReq(hkey), m_path(path) {
	}
	WinReg(const AutoUTF &path);

	AutoUTF			path() const {
		return	m_path;
	}
	void			path(const AutoUTF &path) {
		m_path = path;
	}
	void			key(HKEY hkey) {
		hKeyReq = hkey;
	}

	bool			Add(const AutoUTF &name) const;
	bool			Del(const AutoUTF &name) const;

	void			Set(const AutoUTF &name, PCWSTR value) const;
	void			Set(const AutoUTF &name, const AutoUTF &value) const {
		Set(name, value.c_str());
	}
	void			Set(const AutoUTF &name, int value) const;

	bool			Get(const AutoUTF &name, AutoUTF &value, const AutoUTF &def) const;
	bool			Get(const AutoUTF &name, int &value, int def) const;
};

#endif // WIN_REG_HPP
