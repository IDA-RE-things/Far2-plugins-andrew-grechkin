/**
	win_crypt
	Crypt utilites
	@classes	()
	@author		© 2011 Andrew Grechkin
	@link		(advapi32)
	@link		(crypt32)
	@link		(ole32)
**/

#ifndef WIN_CRYPT_HPP
#define WIN_CRYPT_HPP

#include <libwin_net/win_net.h>
#include <libwin_net/exception.h>
#include <libwin_net/file.h>

#include <wincrypt.h>

#define MY_ENCODING_TYPE  (PKCS_7_ASN_ENCODING | X509_ASN_ENCODING)

///▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒ net_crypt
///========================================================================================== Base64
namespace	Base64 {
	void Decode(PCSTR in, auto_array<BYTE> &buf, DWORD flags = CRYPT_STRING_BASE64_ANY);

	void Decode(PCWSTR in, auto_array<BYTE> &buf, DWORD flags = CRYPT_STRING_BASE64_ANY);

	string EncodeA(PVOID buf, DWORD size, DWORD flags = CRYPT_STRING_BASE64);

	ustring	Encode(PVOID buf, DWORD size, DWORD flags = CRYPT_STRING_BASE64);
}

///==================================================================================== CertDataBlob
class	CertDataBlob : public _CRYPTOAPI_BLOB {
	typedef CertDataBlob class_type;
protected:
public:
	using _CRYPTOAPI_BLOB::cbData;
	using _CRYPTOAPI_BLOB::pbData;

	~CertDataBlob() {
		cbData = 0;
		WinMem::Free(pbData);
	}

	CertDataBlob() {
		cbData = 0;
		pbData = nullptr;
	}

	CertDataBlob(size_t size) {
		cbData = 0;
		pbData = nullptr;
		reserve(size);
	}

	CertDataBlob(const ustring & in) {
		cbData = 0;
		pbData = nullptr;
		reserve((in.size() + 1) * sizeof(WCHAR));
		Copy((PWSTR)pbData, in.c_str());
	}

	bool reserve() {
		return	WinMem::Realloc(pbData, cbData);
	}

	bool reserve(size_t size) {
		if (size > cbData) {
			WinMem::Realloc(pbData, size);
			cbData = size;
			return true;
		}
		return false;
	}

	void swap(class_type& rhs) {
		using std::swap;
		swap(pbData, rhs.pbData);
		swap(cbData, rhs.cbData);
	}
};

class	CertNameBlob: public CertDataBlob {
public:
	CertNameBlob(const ustring &in, DWORD enc = X509_ASN_ENCODING | PKCS_7_ASN_ENCODING) {
		DWORD	dwStrType = CERT_X500_NAME_STR;
		if (in.find(L'\"') != ustring::npos)
			WinFlag::Set(dwStrType, CERT_NAME_STR_NO_QUOTING_FLAG);

		DWORD	size = 0;
		CheckApi(::CertStrToNameW(enc, in.c_str(), dwStrType, nullptr, nullptr, &size, nullptr));
		reserve(size);
		CheckApi(::CertStrToNameW(enc, in.c_str(), dwStrType, nullptr, pbData, &cbData, nullptr));
	}
};

///==================================================================================== WinCryptProv
class	WinCryptKey {
public:
	~WinCryptKey();

	operator HCRYPTKEY() const {
		return m_key;
	}

private:
	WinCryptKey(HCRYPTKEY key);

	HCRYPTKEY m_key;

	friend class WinCryptProv;
};

class	WinCryptProv: private Uncopyable {
public:
	~WinCryptProv();

	// type = (PROV_RSA_FULL, PROV_RSA_AES) flags = CRYPT_MACHINE_KEYSET
	WinCryptProv(PCWSTR name = nullptr, DWORD flags = CRYPT_VERIFYCONTEXT, PCWSTR prov = nullptr, DWORD type = PROV_RSA_FULL);

	bool is_exist_key(DWORD type) const;

	WinCryptKey create_key(DWORD type, DWORD flags = CRYPT_EXPORTABLE) const;

	WinCryptKey get_key(DWORD type, DWORD flags = CRYPT_EXPORTABLE) const;

	operator HCRYPTPROV() const {
		return m_hnd;
	}

private:
	HCRYPTPROV	m_hnd;
};

///==================================================================================== WinCryptHash
class	WinCryptHash {
public:
	~WinCryptHash() {
		Close();
	}

	// alg = (CALG_MD5, CALG_SHA1, CALG_SHA_512)
	WinCryptHash(HCRYPTPROV prov, ALG_ID alg): m_handle(nullptr) {
		CheckApi(::CryptCreateHash(prov, alg, 0, 0, &m_handle));
	}

	WinCryptHash(HCRYPTHASH hash): m_handle(nullptr) {
		CheckApi(::CryptDuplicateHash(hash, nullptr, 0, &m_handle));
	}

	WinCryptHash(const WinCryptHash &in): m_handle(nullptr) {
		CheckApi(::CryptDuplicateHash(in, nullptr, 0, &m_handle));
	}

	const WinCryptHash& operator=(const WinCryptHash &in) {
		if (this != &in) {
			WinCryptHash tmp(in);
			swap(tmp);
		}
		return *this;
	}

	void Hash(const PBYTE buf, size_t size) {
		CheckApi(::CryptHashData(m_handle, buf, size, 0));
	}

	void Hash(const WinFile &wf, uint64_t size = (uint64_t) - 1) {
		File_map	file(wf, size);
		Hash(file);
	}

	void Hash(const File_map &file) {
		bool	ret = true;
		for (File_map::iterator it = file.begin(); ret && it != file.end(); ++it) {
			ret = ::CryptHashData(m_handle, (PBYTE)it.data(), it.size(), 0);
		}
		CheckApi(ret);
	}

	size_t GetHashSize() const {
		DWORD	Result = 0;
		DWORD	ret_size = sizeof(Result);
		CheckApi(::CryptGetHashParam(m_handle, HP_HASHSIZE, (PBYTE)&Result, &ret_size, 0));
		return Result;
	}

	ALG_ID GetHashAlg() const {
		DWORD	Result = 0;
		CheckApi(::CryptGetHashParam(m_handle, HP_ALGID, nullptr, &Result, 0));
		return Result;
	}

	void GetHash(PBYTE buf, DWORD size) const {
		CheckApi(::CryptGetHashParam(m_handle, HP_HASHVAL, buf, &size, 0));
	}

	operator		HCRYPTHASH() const {
		return m_handle;
	}

	void swap(WinCryptHash &rhs) {
		using std::swap;
		swap(m_handle, rhs.m_handle);
	}
private:
	void	Close() {
		if (m_handle) {
			::CryptDestroyHash(m_handle);
			m_handle = nullptr;
		}
	}

	HCRYPTHASH	m_handle;
};

///========================================================================================= WinCert
class WinCert : public WinErrorCheck {
public:
	~WinCert() {
		CertClose();
	}
	WinCert(): m_cert(nullptr) {
	}
	explicit WinCert(PCCERT_CONTEXT in);
	explicit WinCert(const WinCert & in);

	bool				Gen(const ustring &in, const ustring &guid, PSYSTEMTIME until = nullptr);
	bool				Del();

	bool				ToFile(const ustring &path) const;

	bool				AddKey(const ustring &in);
	bool				Store(HANDLE in);
	ustring				GetAttr(DWORD in) const;
	ustring				GetProp(DWORD in) const;
	ustring				name() const {
		return GetAttr(CERT_NAME_SIMPLE_DISPLAY_TYPE);
	}
	ustring				GetDNS() const {
		return GetAttr(CERT_NAME_DNS_TYPE);
	}
	ustring				GetURL() const {
		return GetAttr(CERT_NAME_URL_TYPE);
	}
	ustring				GetUPN() const {
		return GetAttr(CERT_NAME_UPN_TYPE);
	}
	ustring				GetMail() const {
		return GetAttr(CERT_NAME_EMAIL_TYPE);
	}
	ustring				GetRdn() const {
		return GetAttr(CERT_NAME_RDN_TYPE);
	}
	FILETIME			GetStart() const {
		return m_cert->pCertInfo->NotBefore;
	}
	FILETIME			GetEnd() const {
		return m_cert->pCertInfo->NotBefore;
	}

	string				GetHashString() const;
	size_t				GetHashSize() const;
	bool				GetHash(PVOID hash, DWORD size) const;
	bool				GetHash(auto_array<BYTE> &hash) const;

	ustring				get_friendly_name() const {
//		return GetAttr(CERT_NAME_FRIENDLY_DISPLAY_TYPE);
		return GetProp(CERT_FRIENDLY_NAME_PROP_ID);
	}
	void				set_friendly_name(const ustring & in) const {
		set_friendly_name(m_cert, in);
	}

	operator 			PCCERT_CONTEXT() {
		return m_cert;
	}

	static ustring		get_property(PCCERT_CONTEXT pctx, DWORD in);
	static ustring		get_friendly_name(PCCERT_CONTEXT pctx) {
		return get_property(pctx, CERT_FRIENDLY_NAME_PROP_ID);
	}
	static void			set_friendly_name(PCCERT_CONTEXT pctx, const ustring & in);
	static string		get_hash(PCCERT_CONTEXT pctx);

private:
	void 				CertClose() {
		if (m_cert != nullptr) {
			::CertFreeCertificateContext(m_cert);
			m_cert = nullptr;
		}
	}

	PCCERT_CONTEXT  	m_cert;
};

///======================================================================================== WinStore
class CertificateStore: private Uncopyable {
public:
	enum StoreType {
		stMachine,
		stUser,
		stMemory,
	};

	~CertificateStore();

	CertificateStore(const ustring & in, StoreType type, DWORD flags = 0);

	operator HCERTSTORE() const {
		return m_hnd;
	}

	ustring name() const {
		return m_name;
	}

	string import_pfx(const ustring & path, const ustring & pass, const ustring & friendly_name = ustring()) const;

private:
	HCERTSTORE	m_hnd;
	ustring		m_name;
};

///================================================================================= WinCertificates
class WinCertificates: private std::map<string, WinCert> {
public:
	typedef std::map<string, WinCert> this_type;
	typedef this_type::value_type value_type;
	typedef this_type::iterator iterator;
	typedef this_type::const_iterator const_iterator;
	using this_type::begin;
	using this_type::end;
	using this_type::size;

public:
	~WinCertificates() {
	}

	WinCertificates(const CertificateStore & in) {
		cache(in);
	}

	bool cache(const CertificateStore & in);

	iterator find(const ustring & name);
	iterator find_by_name(const ustring & name);
	iterator find_by_friendly(const ustring & name);

//	void	add(const ustring & name, const ustring & pass = ustring());
	void del(const string & hash);
	void del(iterator it);
};

#endif
