/**
	win_net_auth
	Auth utilites
	@classes ()
	@author © 2012 Andrew Grechkin
	@link (advapi32)
 **/

#include <libbase/std.hpp>
#include <libbase/str.hpp>
#include <libext/auth.hpp>
#include <libext/exception.hpp>

#include <ntsecapi.h>

namespace Ext {

	//typedef enum _CRED_PROTECTION_TYPE {
	//	CredUnprotected         = 0,
	//	CredUserProtection      = 1,
	//	CredTrustedProtection   = 2
	//} CRED_PROTECTION_TYPE, *PCRED_PROTECTION_TYPE;
	//
	//extern "C" {
	//	BOOL WINAPI CredProtectW(BOOL fAsSelf, PCWSTR pszCredentials, DWORD cchCredentials, PWSTR pszProtectedCredentials, DWORD *pcchMaxChars, CRED_PROTECTION_TYPE *ProtectionType);
	//	BOOL WINAPI CredUnprotectW(BOOL fAsSelf, PCWSTR pszProtectedCredentials, DWORD cchCredentials, PWSTR pszCredentials, DWORD *pcchMaxChars);
	//}

	///========================================================================================== Base64
	Credential_t::~Credential_t() {
		//	::SecureZeroMemory(m_cred->CredentialBlob, m_cred->CredentialBlobSize);
		::CredFree(m_cred);
	}

	Credential_t::Credential_t(PCWSTR name, DWORD type) {
		CheckApi(::CredReadW(name, type, 0, &m_cred));
	}

	const CREDENTIALW * Credential_t::operator ->() const {
		return m_cred;
	}

	void Credential_t::set(PCWSTR name, PCWSTR pass, PCWSTR target) {
		CREDENTIALW cred = {0};
		cred.Type = CRED_TYPE_GENERIC;
		cred.Persist = CRED_PERSIST_LOCAL_MACHINE;
		cred.TargetName = target ? (PWSTR)target : (PWSTR)name;
		cred.UserName = (PWSTR)name;
		cred.CredentialBlob = (PBYTE)pass;
		cred.CredentialBlobSize = sizeof(*pass) * (Base::Str::length(pass) + 1);
		CheckApi(::CredWriteW(&cred, 0));
	}

	void Credential_t::del(PCWSTR name, DWORD type) {
		CheckApi(::CredDeleteW(name, type, 0));
	}


	Credentials_t::~Credentials_t() {
		::CredFree(m_creds);
	}

	Credentials_t::Credentials_t() {
		CheckApi(::CredEnumerateW(nullptr, 0, &m_size, &m_creds));
		//	CREDENTIAL_TARGET_INFORMATION TargetInfo = {(PWSTR)L"PC"};
		//	CheckApi(::CredReadDomainCredentials(&TargetInfo, 0, &m_size, &m_creds));
	}

	bool Credentials_t::empty() const {
		return m_size == 0;
	}

	size_t Credentials_t::size() const {
		return m_size;
	}

	Credentials_t::value_type Credentials_t::at(size_t ind) const {
		CheckApiThrowError(ind < m_size, ERROR_INVALID_INDEX);
		return m_creds[ind];
	}

	void parse_username(PCWSTR fullname, ustring & dom, ustring name) {
		wchar_t d[MAX_PATH];
		wchar_t n[MAX_PATH];
		CheckApiError(CredUIParseUserName(fullname, d, Base::lengthof(d), n, Base::lengthof(n)));
		dom = d;
		name = n;
	}

	void PassProtect(PCWSTR pass, PWSTR prot, DWORD size) {
		CRED_PROTECTION_TYPE type;
		CheckApi(CredProtectW(true, (PWSTR)pass, Base::Str::length(pass) + 1, prot, &size, &type));
	}

	void PassUnProtect(PCWSTR prot, DWORD ps, PWSTR pass, DWORD size) {
		CheckApi(::CredUnprotectW(true, (PWSTR)prot, ps, pass, &size));
	}

}
