#ifndef _WIN_DEF_PRIV_H_
#define _WIN_DEF_PRIV_H_

#include "win_net.h"

///======================================================================================== WinToken
/// Обертка токена
class WinToken: private Uncopyable {
public:
	WinToken(ACCESS_MASK mask = TOKEN_QUERY);

	WinToken(HANDLE process, ACCESS_MASK mask);

	operator HANDLE() const {
		return m_token;
	}

	static bool check_membership(PSID sid, HANDLE token = nullptr);

private:
	auto_close<HANDLE> m_token;
};

///========================================================================================= WinPriv
/// Функции работы с привилегиями
namespace	WinPriv {
	bool is_exist(HANDLE token, const LUID & priv);
	bool is_exist(HANDLE token, PCWSTR priv_name);
	bool is_exist(const LUID & priv);
	bool is_exist(PCWSTR priv_name);

	bool is_enabled(HANDLE token, const LUID & priv);
	bool is_enabled(HANDLE token, PCWSTR priv_name);
	bool is_enabled(const LUID & priv);
	bool is_enabled(PCWSTR priv_name);

	void modify(HANDLE token, const LUID & priv, bool enable);
	void modify(HANDLE token, PCWSTR priv_name, bool enable);
	void modify(const LUID & priv, bool enable);
	void modify(PCWSTR priv_name, bool enable);

	inline void disable(const LUID & in) {
		modify(in, false);
	}
	inline void disable(PCWSTR in) {
		modify(in, false);
	}

	inline void enable(const LUID & in) {
		modify(in, true);
	}
	inline void enable(PCWSTR in) {
		modify(in, true);
	}

	ustring GetName(PCWSTR priv_name);
}

///======================================================================================= Privilege
/// Класс автоматической установки привилегии и ее отмены
class Privilege: private Uncopyable {
public:
	~Privilege();

	explicit Privilege(PCWSTR priv_name);

private:
	TOKEN_PRIVILEGES m_tp;
	bool	m_disable;
};

#endif
