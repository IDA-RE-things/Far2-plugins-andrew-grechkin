/**
	win_crypt
	Crypt utilites
	@classes	()
	@author		� 2011 Andrew Grechkin
	@link		(advapi32)
	@link		(crypt32)
	@link		(ole32)
**/

#include "crypt.h"

#include <libwin_com/win_com.h>

#define MY_ENCODING_TYPE  (PKCS_7_ASN_ENCODING | X509_ASN_ENCODING)

///========================================================================================== Base64
namespace	Base64 {
	void Decode(PCSTR in, auto_array<BYTE> &buf, DWORD flags) {
		DWORD	size = 0;
		CheckApi(::CryptStringToBinaryA(in, 0, flags, nullptr, &size, nullptr, nullptr));
		buf.reserve(size);
		CheckApi(::CryptStringToBinaryA(in, 0, flags, buf, &size, nullptr, nullptr));
	}

	void Decode(PCWSTR in, auto_array<BYTE> &buf, DWORD flags) {
		DWORD	size = 0;
		CheckApi(::CryptStringToBinaryW(in, 0, flags, nullptr, &size, nullptr, nullptr));
		buf.reserve(size);
		CheckApi(::CryptStringToBinaryW(in, 0, flags, buf, &size, nullptr, nullptr));
	}

	string EncodeA(PVOID buf, DWORD size, DWORD flags) {
		DWORD	len = 0;
		CheckApi(::CryptBinaryToStringA((const PBYTE)buf, size, flags, nullptr, &len));
		CHAR Result[len];
		CheckApi(::CryptBinaryToStringA((const PBYTE)buf, size, flags, Result, &len));
		return string(Result);
	}

	ustring	Encode(PVOID buf, DWORD size, DWORD flags) {
		DWORD	len = 0;
		CheckApi(::CryptBinaryToStringW((const PBYTE)buf, size, flags, nullptr, &len));
		WCHAR Result[len];
		CheckApi(::CryptBinaryToStringW((const PBYTE)buf, size, flags, Result, &len));
		return ustring(Result);
	}
}

namespace Crypt {
	///==================================================================================== DataBlob
	DataBlob::~DataBlob() {
		cbData = 0;
		WinMem::Free(pbData);
	}

	DataBlob::DataBlob() {
		cbData = 0;
		pbData = nullptr;
	}

	DataBlob::DataBlob(size_t size) {
		cbData = 0;
		pbData = nullptr;
		reserve(size);
	}

	DataBlob::DataBlob(const ustring & in) {
		cbData = 0;
		pbData = nullptr;
		reserve((in.size() + 1) * sizeof(WCHAR));
		Copy((PWSTR)pbData, in.c_str());
	}

	bool DataBlob::reserve() {
		return	WinMem::Realloc(pbData, cbData);
	}

	bool DataBlob::reserve(size_t size) {
		if (size > cbData) {
			WinMem::Realloc(pbData, size);
			cbData = size;
			return true;
		}
		return false;
	}

	void DataBlob::swap(this_type & rhs) {
		using std::swap;
		swap(pbData, rhs.pbData);
		swap(cbData, rhs.cbData);
	}

	NameBlob::NameBlob(const ustring & in, DWORD enc) {
		DWORD	dwStrType = CERT_X500_NAME_STR;
		if (in.find(L'\"') != ustring::npos)
			WinFlag::Set(dwStrType, CERT_NAME_STR_NO_QUOTING_FLAG);

		DWORD	size = 0;
		CheckApi(::CertStrToNameW(enc, in.c_str(), dwStrType, nullptr, nullptr, &size, nullptr));
		reserve(size);
		CheckApi(::CertStrToNameW(enc, in.c_str(), dwStrType, nullptr, pbData, &cbData, nullptr));
	}

	///==================================================================================== Provider
	Key::~Key() {
		::CryptDestroyKey(m_key);
	}

	Key::Key(HCRYPTKEY key):
		m_key(key) {
	}

	Provider::~Provider() {
		::CryptReleaseContext(m_hnd, 0);
	}

	Provider::Provider(PCWSTR name, DWORD flags, PCWSTR prov, DWORD type):
		m_hnd(nullptr) {
		if (::CryptAcquireContextW(&m_hnd, name, prov, type, flags))
			return;
		CheckApi(::CryptAcquireContextW(&m_hnd, name, prov, type, flags | CRYPT_NEWKEYSET));
	}

	bool Provider::is_exist_key(DWORD type) const {
		HCRYPTKEY	key = nullptr;
		if (::CryptGetUserKey(m_hnd, type, &key)) {
			return ::CryptDestroyKey(key);
		}
		DWORD err = ::GetLastError();
		if (err != (DWORD)NTE_NO_KEY) {
			CheckApiError(err);
		}
		return false;
	}

	Key Provider::create_key(DWORD type, DWORD flags) const {
		HCRYPTKEY key = nullptr;
		CheckApi(::CryptGenKey(m_hnd, type, flags, &key));
		return Key(key);
	}

	Key Provider::get_key(DWORD type, DWORD flags) const {
		HCRYPTKEY key = nullptr;
		if (!::CryptGetUserKey(m_hnd, type, &key))
			CheckApi(::CryptGenKey(m_hnd, type, flags, &key));
		return Key(key);
	}

	///======================================================================================== Hash
	Hash::~Hash() {
		if (m_handle)
			::CryptDestroyHash(m_handle);
	}

	// alg = (CALG_MD5, CALG_SHA1, CALG_SHA_512)
	Hash::Hash(HCRYPTPROV prov, ALG_ID alg) {
		CheckApi(::CryptCreateHash(prov, alg, 0, 0, &m_handle));
	}

	Hash::Hash(HCRYPTHASH hash) {
		CheckApi(::CryptDuplicateHash(hash, nullptr, 0, &m_handle));
	}

	Hash::Hash(const PBYTE buf, size_t size) {
		CheckApi(::CryptHashData(m_handle, buf, size, 0));
	}

	Hash::Hash(const WinFile & wf, uint64_t size):
		m_handle(nullptr) {
		Hash(File_map(wf, size)).swap(*this);
	}

	Hash::Hash(const File_map & file) {
		bool ret = true;
		for (File_map::iterator it = file.begin(); ret && it != file.end(); ++it) {
			ret = ::CryptHashData(m_handle, (PBYTE)it.data(), it.size(), 0);
		}
		CheckApi(ret);
	}

	Hash::Hash(const Hash & in) {
		CheckApi(::CryptDuplicateHash(in, nullptr, 0, &m_handle));
	}

	Hash & Hash::operator=(const Hash & in) {
		if (this != &in)
			Hash(in).swap(*this);
		return *this;
	}

	size_t Hash::get_size() const {
		DWORD	Result = 0;
		DWORD	ret_size = sizeof(Result);
		CheckApi(::CryptGetHashParam(m_handle, HP_HASHSIZE, (PBYTE)&Result, &ret_size, 0));
		return Result;
	}

	ALG_ID Hash::get_algorithm() const {
		DWORD	Result = 0;
		CheckApi(::CryptGetHashParam(m_handle, HP_ALGID, nullptr, &Result, 0));
		return Result;
	}

	void Hash::get_hash(PBYTE buf, DWORD size) const {
		CheckApi(::CryptGetHashParam(m_handle, HP_HASHVAL, buf, &size, 0));
	}

	void Hash::swap(Hash & rhs) {
		using std::swap;
		swap(m_handle, rhs.m_handle);
	}

	///============================================================================ CertificateStore
	CertificateStore::~CertificateStore() {
		::CertCloseStore(m_hnd, CERT_CLOSE_STORE_CHECK_FLAG);
	}

	CertificateStore::CertificateStore(const ustring & in, StoreType type, DWORD flags):
		m_hnd(nullptr),
		m_name(in) {
		switch (type) {
			case stMachine:
				WinFlag::Set(flags, (DWORD)CERT_SYSTEM_STORE_LOCAL_MACHINE);
				m_hnd = CheckHandle(::CertOpenStore(CERT_STORE_PROV_SYSTEM, 0, (HCRYPTPROV)nullptr, flags, m_name.c_str()));
				break;
			case stUser:
				WinFlag::Set(flags, (DWORD)CERT_SYSTEM_STORE_CURRENT_USER);
				m_hnd = CheckHandle(::CertOpenStore(CERT_STORE_PROV_SYSTEM, 0, (HCRYPTPROV)nullptr, flags, m_name.c_str()));
				break;
			case stMemory:
				WinFlag::Set(flags, (DWORD)CERT_STORE_CREATE_NEW_FLAG);
				m_hnd = CheckHandle(::CertOpenStore(sz_CERT_STORE_PROV_MEMORY, 0, 0, flags, nullptr));
				break;
		}
	}

	string CertificateStore::import_pfx(const ustring & path, const ustring & pass, const ustring & friendly_name) const {
		string ret;
		File_map pfx(path);
		if (pfx.size() == pfx.set_frame(pfx.size())) {
			File_map::iterator it = pfx.begin();
			CRYPT_DATA_BLOB blob = {it.size(), (PBYTE)it.data()};
			CheckApiThrowError(::PFXIsPFXBlob(&blob), ERROR_INVALID_DATA);
			HCERTSTORE tmpStore = CheckHandle(::PFXImportCertStore(&blob, pass.c_str(), CRYPT_MACHINE_KEYSET | CRYPT_EXPORTABLE));
			PCCERT_CONTEXT cert = nullptr;
			while ((cert = ::CertEnumCertificatesInStore(tmpStore, cert)) != nullptr) {
				if (!friendly_name.empty())
					Certificate::set_friendly_name(cert, friendly_name);
				CheckApi(::CertAddCertificateContextToStore(m_hnd, cert, CERT_STORE_ADD_NEW, 0));
				ret = Certificate::get_hash(cert);
			}
			::CertCloseStore(tmpStore, CERT_CLOSE_STORE_FORCE_FLAG);
		}
		return ret;
	}

	///================================================================================= Certificate
	Certificate::~Certificate() {
		::CertFreeCertificateContext(m_cert);
	}

	Certificate::Certificate(PCCERT_CONTEXT in): m_cert(nullptr) {
		m_cert = ::CertDuplicateCertificateContext(in);
		CheckPointer(m_cert);
	}

	Certificate::Certificate(const ustring & in, const ustring & guid, PSYSTEMTIME until) {
		NameBlob blob(in);
		ustring capsule(guid.empty() ? WinGUID().as_str() : guid);
		Provider provider(capsule.c_str(), 0, MS_STRONG_PROV_W, PROV_RSA_FULL);
		provider.create_key(AT_KEYEXCHANGE);
		CRYPT_KEY_PROV_INFO info = {0};
		info.pwszContainerName = (WCHAR*)capsule.c_str();
		info.pwszProvName = (WCHAR*)MS_STRONG_PROV;
		info.dwProvType = PROV_RSA_FULL;
		info.dwFlags = CRYPT_MACHINE_KEYSET;
		info.dwKeySpec = AT_KEYEXCHANGE;
		//	CERT_EXTENSION ex1 = {szOID_PKIX_KP_SERVER_AUTH, FALSE,}
		//	CERT_EXTENSION	rrr[1];
		//	rrr[0].fCritical = false;
		//	rrr[0].pszObjId = (PSTR)szOID_PKIX_KP_SERVER_AUTH;
		//	WinMem::Zero(rrr[0].Value);
		//	CERT_EXTENSIONS	ext;
		//	ext.cExtension = 1;
		//	ext.rgExtension = rrr;

		m_cert = CheckPointerErr(::CertCreateSelfSignCertificate(provider, &blob, 0, &info, nullptr, nullptr, until, nullptr));
		//	CTL_USAGE	usage;
		//	usage.cUsageIdentifier = 1;
		//	usage.rgpszUsageIdentifier = (PSTR*)&szOID_PKIX_KP_SERVER_AUTH;
		CheckApi(::CertAddEnhancedKeyUsageIdentifier(m_cert, szOID_PKIX_KP_SERVER_AUTH));
	}

	Certificate::Certificate(const Certificate & in): m_cert(nullptr) {
		m_cert = ::CertDuplicateCertificateContext(in.m_cert);
		CheckPointer(m_cert);
	}

	Certificate & Certificate::operator=(const Certificate & in) {
		if (this != &in)
			Certificate(in).swap(*this);
		return *this;
	}

	void Certificate::del() {
		CheckApi(::CertDeleteCertificateFromStore(m_cert));
	}

	void Certificate::export_to_file(const ustring & path) const {
		CertificateStore store(path, CertificateStore::stMemory);
		CheckApi(::CertAddCertificateContextToStore(store, m_cert, CERT_STORE_ADD_NEW, nullptr));
		//	::CertSetCertificateContextProperty(stCert, CERT_KEY_PROV_INFO_PROP_ID, 0, KeyProviderInfo(randomContainerName, PROV_RSA_FULL, AT_KEYEXCHANGE))
		DataBlob blob;
		::PFXExportCertStoreEx(store, &blob, L"", 0, EXPORT_PRIVATE_KEYS);
		blob.reserve();
		CheckApi(::PFXExportCertStoreEx(store, &blob, L"", 0, EXPORT_PRIVATE_KEYS));
		File::write(path.c_str(), blob.pbData, blob.cbData);
	}

	void Certificate::add_key(const ustring &/*in*/) {
		//	::CertSetCertificateContextProperty(_cert, CERT_KEY_PROV_INFO_PROP_ID, 0);
	}

	void Certificate::add_to_store(HANDLE in) {
		CheckApi(::CertAddCertificateContextToStore(in, m_cert, CERT_STORE_ADD_ALWAYS, nullptr));
	}

	string Certificate::get_hash_string() const {
		auto_array<BYTE> hash(get_hash());
		return Hash2Str(hash, hash.size());
	}

	size_t Certificate::get_hash_size() const {
		DWORD ret = 0;
		::CertGetCertificateContextProperty(m_cert, CERT_HASH_PROP_ID, nullptr, &ret);
		CheckApi(::GetLastError() == ERROR_MORE_DATA);
		return ret;
	}

	void Certificate::get_hash(PVOID hash, DWORD size) const {
		DWORD cbData = get_hash_size();
		CheckApiThrowError(cbData <= size, ERROR_MORE_DATA);
		CheckApi(::CertGetCertificateContextProperty(m_cert, CERT_HASH_PROP_ID, hash, &cbData));
	}

	auto_array<BYTE> Certificate::get_hash() const {
		DWORD size = get_hash_size();
		auto_array<BYTE> hash(size);
		CheckApi(::CertGetCertificateContextProperty(m_cert, CERT_HASH_PROP_ID, hash, &size));
		return hash;
	}

	ustring Certificate::get_attr(PCCERT_CONTEXT pctx, DWORD in) {
		size_t size = ::CertGetNameStringW(pctx, in, 0, nullptr, nullptr, 0);
		CheckApi(size);
		WCHAR buf[size];
		CheckApi(::CertGetNameStringW(pctx, in, 0, nullptr, buf, size));
		return buf;
	}

	ustring Certificate::get_property(PCCERT_CONTEXT pctx, DWORD in) {
		DWORD cbData = 0;
		::CertGetCertificateContextProperty(pctx, in, nullptr, &cbData);
		CheckApi(cbData);
		auto_array<BYTE> buf(cbData);
		CheckApi(::CertGetCertificateContextProperty(pctx, in, buf.data(), &cbData));
		return ustring((PCWSTR)buf.data());
	}

	void Certificate::set_friendly_name(PCCERT_CONTEXT pctx, const ustring & in) {
		DataBlob blob(in.c_str());
		CheckApi(::CertSetCertificateContextProperty(pctx, CERT_FRIENDLY_NAME_PROP_ID, 0, &blob));
	}

	string Certificate::get_hash(PCCERT_CONTEXT pctx) {
		string	Result;
		DWORD	cbData = 0;
		::CertGetCertificateContextProperty(pctx, CERT_HASH_PROP_ID, nullptr, &cbData);
		if (cbData) {
			auto_array<BYTE> buf(cbData);
			if (::CertGetCertificateContextProperty(pctx, CERT_HASH_PROP_ID, buf.data(), &cbData)) {
				Result = Hash2Str(buf, cbData);
			}
		}
		return Result;
	}

	void Certificate::swap(Certificate & rhs) {
		using std::swap;
		swap(m_cert, rhs.m_cert);
	}

	///============================================================================= WinCertificates
	bool Certificates::cache(const CertificateStore & in) {
		HRESULT	err = 0;
		PCCERT_CONTEXT  pCert = nullptr;
		while ((pCert = ::CertEnumCertificatesInStore(in, pCert))) {
			Certificate	info(pCert);
			insert(value_type(info.get_hash_string(), info));
		}
		err = ::GetLastError();
		return err == CRYPT_E_NOT_FOUND;
	}

	Certificates::iterator Certificates::find(const ustring & name) {
		return find(name);
	}

	Certificates::iterator Certificates::find_by_name(const ustring & in) {
		for (iterator it = begin(); it != end(); ++it) {
			if (it->second.get_name() == in)
				return it;
		}
		return end();
	}

	Certificates::iterator Certificates::find_by_friendly(const ustring & in) {
		for (iterator it = begin(); it != end(); ++it) {
			if (it->second.get_friendly_name() == in)
				return it;
		}
		return end();
	}

	void Certificates::del(const string & hash) {
		iterator it = find(hash);
		if (it != end())
			del(it);
	}

	void Certificates::del(iterator it) {
		it->second.del();
		erase(it);
	}
}
