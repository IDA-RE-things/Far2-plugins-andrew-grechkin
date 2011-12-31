/**
	win_net_auth
	Auth utilites
	@classes	()
	@author		© 2010 Andrew Grechkin
	@link		(advapi32)
**/

#include "win_net.h"
#include "auth.h"
#include "exception.h"

#include <wincred.h>
#include <ntsecapi.h>

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
Credential::~Credential() {
	::CredFree(m_cred);
}

Credential::Credential(PCWSTR name, DWORD type) {
	CheckApi(::CredReadW(name, type, 0, &m_cred));
}

const _CREDENTIALW* Credential::operator->() const {
	return m_cred;
}

void Credential::add(PCWSTR name, PCWSTR pass, PCWSTR target) {
	CREDENTIALW cred = {0};
	cred.Type = CRED_TYPE_GENERIC;
	cred.Persist = CRED_PERSIST_LOCAL_MACHINE;
	cred.TargetName = target ? (PWSTR)target : (PWSTR)name;
	cred.UserName = (PWSTR)name;
	cred.CredentialBlob = (PBYTE)pass;
	cred.CredentialBlobSize = sizeof(*pass) * (Len(pass) + 1);
	CheckApi(::CredWriteW(&cred, 0));
}

void Credential::del(PCWSTR name, DWORD type) {
	CheckApi(::CredDeleteW(name, type, 0));
}

Credentials::~Credentials() {
	::CredFree(m_creds);
}

Credentials::Credentials() {
	CheckApi(::CredEnumerateW(nullptr, 0, &m_size, &m_creds));
}

void Credentials::Update() {
	::CredFree(m_creds);
	CheckApi(::CredEnumerateW(nullptr, 0, &m_size, &m_creds));
}

size_t Credentials::size() const {
	return m_size;
}

const _CREDENTIALW* Credentials::operator[](size_t ind) const {
	return m_creds[ind];
}

void	PassProtect(PCWSTR pass, PWSTR prot, DWORD size) {
	CRED_PROTECTION_TYPE type;
	CheckApi(CredProtectW(true, (PWSTR)pass, Len(pass) + 1, prot, &size, &type));
}

void	PassUnProtect(PCWSTR prot, DWORD ps, PWSTR pass, DWORD size) {
	CheckApi(::CredUnprotectW(true, (PWSTR)prot, ps, pass, &size));
}

void	PassSave(PCWSTR name, PCWSTR pass) {
//	WCHAR	prot[512];
//	PassProtect(pass, prot, sizeofa(prot));
	CREDENTIALW cred = {0};
	cred.Type = CRED_TYPE_GENERIC;
	cred.Persist = CRED_PERSIST_LOCAL_MACHINE;
	cred.TargetName = (PWSTR)name;
	cred.UserName = (PWSTR)name;
	cred.CredentialBlob = (PBYTE)pass;
	cred.CredentialBlobSize = sizeof(*pass) * (Len(pass) + 1);
	CheckApi(CredWriteW(&cred, 0));
}

void	PassDel(PCWSTR name) {
	CheckApi(CredDeleteW(name, CRED_TYPE_GENERIC, 0));
}

ustring	PassRead(PCWSTR name) {
	PCREDENTIALW cred = nullptr;
	CheckApi(CredReadW(name, CRED_TYPE_GENERIC, 0, &cred));
//	WCHAR	pass[512];
//	PassUnProtect((PCWSTR)cred->CredentialBlob, cred->CredentialBlobSize, pass, sizeofa(pass));
//	return ustring(pass);
	return ustring((PCWSTR)cred->CredentialBlob);
}

void	PassList() {
//	DWORD cnt = 0;
//	PCREDENTIAL* cre;
//	CredEnumerate(nullptr, 0, &cnt, &cre);
//	CredFree(cre);
//
//	printf(L"size: %d\n", cnt);
//
//	for (size_t i = 0; i < cnt; ++i) {
//		printf(L"cre [%s] flags: ", i , cre[i]->Flags);
//		printf(L" Name: %s", cre[i]->TargetName);
//		printf(L" Blob: %s", (PCWSTR)cre[i]->CredentialBlob);
//	}
}
