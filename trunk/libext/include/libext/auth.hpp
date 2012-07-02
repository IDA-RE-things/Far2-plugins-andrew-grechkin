#ifndef WIN_NET_AUTH_HPP
#define WIN_NET_AUTH_HPP

#include <libbase/std.hpp>

#include <wincred.h>

namespace Ext {

	///▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒ net_auth
	struct Credential_t {
		~Credential_t();

		Credential_t(PCWSTR name, DWORD type = CRED_TYPE_GENERIC);

		const CREDENTIALW * operator ->() const;

	public:
		static void set(PCWSTR name, PCWSTR pass, PCWSTR target = nullptr);

		static void del(PCWSTR name, DWORD type = CRED_TYPE_GENERIC);

	private:
		PCREDENTIALW m_cred;
	};

	struct Credentials_t {
		typedef PCREDENTIALW value_type;

		~Credentials_t();

		Credentials_t();

		bool empty() const;

		size_t size() const;

		value_type at(size_t ind) const;

	private:
		PCREDENTIALW * m_creds;
		DWORD m_size;
	};

	void parse_username(PCWSTR fullname, ustring & dom, ustring name);

}

#endif
