#ifndef _WIN_DEF_PRIV_H_
#define _WIN_DEF_PRIV_H_

#include "win_def.h"

///========================================================================================= WinPriv
/// Функции работы с привилегиями
namespace	WinPriv {
	bool 		IsExist(HANDLE hToken, LUID priv);
	bool 		IsExist(HANDLE hToken, PCWSTR sPriv);
	bool 		IsExist(LUID priv);
	bool 		IsExist(PCWSTR sPriv);

	bool		IsEnabled(HANDLE hToken, LUID priv);
	bool 		IsEnabled(HANDLE hToken, PCWSTR sPriv);
	bool		IsEnabled(LUID priv);
	bool 		IsEnabled(PCWSTR sPriv);

	bool 		Modify(HANDLE hToken, LUID priv, bool bEnable);
	bool 		Modify(HANDLE hToken, PCWSTR sPriv, bool bEnable);
	bool 		Modify(LUID priv, bool bEnable);
	bool 		Modify(PCWSTR sPriv, bool bEnable);

	inline bool	Disable(LUID in) {
		return	Modify(in, false);
	}
	inline bool	Disable(PCWSTR in) {
		return	Modify(in, false);
	}

	inline bool	Enable(LUID in) {
		return	Modify(in, true);
	}
	inline bool	Enable(PCWSTR in) {
		return	Modify(in, true);
	}

	AutoUTF		GetName(PCWSTR sPriv);
}

///======================================================================================= Privilege
class Privilege: private Uncopyable {
public:
	~Privilege();

	explicit Privilege(PCWSTR name);

private:
	TOKEN_PRIVILEGES m_tp;
	bool	m_disable;
};

///========================================================================================= WinProc
/// Обертка хэндла процесса
class		WinProcess {
	auto_close<HANDLE>	m_hndl;
public:
	WinProcess(): m_hndl(::GetCurrentProcess()) {
	}

	WinProcess(ACCESS_MASK mask, DWORD pid): m_hndl(::OpenProcess(mask, false, pid)) {
	}

	operator		HANDLE() const {
		return	m_hndl;
	}

	DWORD			GetId() const {
		return	::GetProcessId(m_hndl);
	}

	static	DWORD	id() {
		return	::GetCurrentProcessId();
	}
	static	DWORD	id(HANDLE hProc) {
		return	::GetProcessId(hProc);
	}
	static	AutoUTF	User();
	static	AutoUTF	FullPath();
	static	AutoUTF	CmdLine() {
		return	::GetCommandLineW();
	}
};


#endif
