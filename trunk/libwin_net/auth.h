#ifndef WIN_NET_AUTH_HPP
#define WIN_NET_AUTH_HPP

#include "win_net.h"

///▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒ net_auth
struct _CREDENTIALW;
const DWORD my_CRED_TYPE_GENERIC = 1;

class Credential {
public:
	~Credential();

	Credential(PCWSTR name, DWORD type = my_CRED_TYPE_GENERIC);

	const _CREDENTIALW* operator->() const;

public:
	static void add(PCWSTR name, PCWSTR pass, PCWSTR target = nullptr);

	static void del(PCWSTR name, DWORD type = my_CRED_TYPE_GENERIC);

private:
	_CREDENTIALW * m_cred;
};

class Credentials {
public:
	~Credentials();

	Credentials();

	void Update();

	size_t size() const;

	const _CREDENTIALW * operator[](size_t ind) const;

private:
	_CREDENTIALW ** m_creds;
	DWORD m_size;
};

void	PassSave(PCWSTR name, PCWSTR pass);
inline void	PassSave(const ustring &name, const ustring &pass) {
	PassSave(name.c_str(), pass.c_str());
}

void	PassDel(PCWSTR name);
inline void	PassDel(const ustring &name) {
	PassDel(name.c_str());
}

ustring	PassRead(PCWSTR name);
inline ustring	PassRead(const ustring &name) {
	return 	PassRead(name.c_str());
}

void	PassList();

#endif // WIN_NET_HPP
