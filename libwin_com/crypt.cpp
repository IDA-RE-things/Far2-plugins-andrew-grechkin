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

///========================================================================================= WinCert
WinCert::WinCert(PCCERT_CONTEXT in): m_cert(nullptr) {
	if (in)
		m_cert = ::CertDuplicateCertificateContext(in);
}

WinCert::WinCert(const WinCert &in): m_cert(nullptr) {
	if (in.m_cert)
		m_cert = ::CertDuplicateCertificateContext(in.m_cert);
}

bool WinCert::Gen(const ustring &in, const ustring & guid, PSYSTEMTIME until) {
	CertNameBlob blob(in);
		ustring		capsule(guid.c_str());
		if (capsule.empty()) {
			capsule = WinGUID().as_str();
		}
		WinCryptProv	provider(capsule.c_str(), 0, MS_STRONG_PROV_W, PROV_RSA_FULL);
			provider.create_key(AT_KEYEXCHANGE);
				CRYPT_KEY_PROV_INFO info = {0};
				info.pwszContainerName = (WCHAR*)capsule.c_str();
				info.pwszProvName = (WCHAR*)MS_STRONG_PROV;
				info.dwProvType = PROV_RSA_FULL;
				info.dwFlags = CRYPT_MACHINE_KEYSET;
				info.dwKeySpec = AT_KEYEXCHANGE;

//				CERT_EXTENSION ex1 = {szOID_PKIX_KP_SERVER_AUTH, FALSE,}
//				CERT_EXTENSION	rrr[1];
//				rrr[0].fCritical = false;
//				rrr[0].pszObjId = (PSTR)szOID_PKIX_KP_SERVER_AUTH;
//				WinMem::Zero(rrr[0].Value);
//				CERT_EXTENSIONS	ext;
//				ext.cExtension = 1;
//				ext.rgExtension = rrr;

				m_cert = ::CertCreateSelfSignCertificate(provider, &blob, 0, &info, nullptr, nullptr, until, nullptr);
				if (m_cert) {
//					CTL_USAGE	usage;
//					usage.cUsageIdentifier = 1;
//					usage.rgpszUsageIdentifier = (PSTR*)&szOID_PKIX_KP_SERVER_AUTH;
					::CertAddEnhancedKeyUsageIdentifier(m_cert, szOID_PKIX_KP_SERVER_AUTH);
				}
				return ChkSucc(m_cert);
	return false;
}

bool WinCert::Del() {
	if (m_cert) {
		if (ChkSucc(::CertDeleteCertificateFromStore(m_cert))) {
			m_cert = nullptr;
			return true;
		}
	}
	return false;
}

bool WinCert::ToFile(const ustring & path) const {
	bool Result = false;
	if (m_cert) {
		CertificateStore store(path, CertificateStore::stMemory);
		if (::CertAddCertificateContextToStore(store, m_cert, CERT_STORE_ADD_NEW, nullptr)) {
			//	::CertSetCertificateContextProperty(stCert, CERT_KEY_PROV_INFO_PROP_ID, 0, KeyProviderInfo(randomContainerName, PROV_RSA_FULL, AT_KEYEXCHANGE))
			CertDataBlob	blob;
			::PFXExportCertStoreEx(store, &blob, L"", 0, EXPORT_PRIVATE_KEYS);
			blob.reserve();
			if (::PFXExportCertStoreEx(store, &blob, L"", 0, EXPORT_PRIVATE_KEYS)) {
				File::write(path.c_str(), blob.pbData, blob.cbData);
			}
		}
	}
	return Result;
}

bool WinCert::AddKey(const ustring &/*in*/) {
//	::CertSetCertificateContextProperty(_cert, CERT_KEY_PROV_INFO_PROP_ID, 0);
	return false;
}

bool WinCert::Store(HANDLE in) {
	return ChkSucc(::CertAddCertificateContextToStore(in, m_cert, CERT_STORE_ADD_ALWAYS, nullptr));
}

ustring WinCert::GetAttr(DWORD in) const {
	size_t	size = ::CertGetNameStringW(m_cert, in, 0, nullptr, nullptr, 0);
	WCHAR	buf[size];
	if (::CertGetNameStringW(m_cert, in, 0, nullptr, buf, size)) {
		return buf;
	}
	return ustring();
}

ustring WinCert::GetProp(DWORD in) const {
	ustring	Result;
	DWORD	cbData = 0;
	::CertGetCertificateContextProperty(m_cert, in, nullptr, &cbData);
	if (cbData) {
		WCHAR	buf[cbData];
		if (::CertGetCertificateContextProperty(m_cert, in, (PVOID)buf, &cbData)) {
			Result = buf;
		}
	}
	return Result;
}

string WinCert::GetHashString() const {
	string	Result;
	DWORD	cbData = 0;
	::CertGetCertificateContextProperty(m_cert, CERT_HASH_PROP_ID, nullptr, &cbData);
	if (cbData) {
		auto_array<BYTE> buf(cbData);
		if (::CertGetCertificateContextProperty(m_cert, CERT_HASH_PROP_ID, (PVOID)buf.data(), &cbData)) {
			Result = Hash2Str(buf, cbData);
		}
	}
	return Result;
}

size_t WinCert::GetHashSize() const {
	DWORD	Result = 0;
	::CertGetCertificateContextProperty(m_cert, CERT_HASH_PROP_ID, nullptr, &Result);
	return Result;
}

bool WinCert::GetHash(PVOID hash, DWORD size) const {
	DWORD	cbData = 0;
	::CertGetCertificateContextProperty(m_cert, CERT_HASH_PROP_ID, nullptr, &cbData);
	if (cbData <= size) {
		if (::CertGetCertificateContextProperty(m_cert, CERT_HASH_PROP_ID, hash, &cbData)) {
			return true;
		}
	}
	return false;
}

bool WinCert::GetHash(auto_array<BYTE> &hash) const {
	hash.reserve(GetHashSize());
	DWORD	size = hash.size();
	if (::CertGetCertificateContextProperty(m_cert, CERT_HASH_PROP_ID, hash, &size)) {
		return true;
	}
	return false;
}

ustring WinCert::get_property(PCCERT_CONTEXT pctx, DWORD in) {
	DWORD cbData = 0;
	::CertGetCertificateContextProperty(pctx, in, nullptr, &cbData);
	CheckApi(cbData);
	auto_array<BYTE> buf(cbData);
	CheckApi(::CertGetCertificateContextProperty(pctx, in, buf.data(), &cbData));
	return ustring((PCWSTR)buf.data());
}

void WinCert::set_friendly_name(PCCERT_CONTEXT pctx, const ustring & in) {
	CertDataBlob blob(in.c_str());
	CheckApi(::CertSetCertificateContextProperty(pctx, CERT_FRIENDLY_NAME_PROP_ID, 0, &blob));
}

string				WinCert::get_hash(PCCERT_CONTEXT pctx) {
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

///==================================================================================== WinCryptProv
WinCryptKey::~WinCryptKey() {
	::CryptDestroyKey(m_key);
}

WinCryptKey::WinCryptKey(HCRYPTKEY key):
	m_key(key) {
}

WinCryptProv::~WinCryptProv() {
	::CryptReleaseContext(m_hnd, 0);
}

WinCryptProv::WinCryptProv(PCWSTR name, DWORD flags, PCWSTR prov, DWORD type):
	m_hnd(nullptr) {
	if (::CryptAcquireContextW(&m_hnd, name, prov, type, flags))
		return;
	CheckApi(::CryptAcquireContextW(&m_hnd, name, prov, type, flags | CRYPT_NEWKEYSET));
}

bool WinCryptProv::is_exist_key(DWORD type) const {
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

WinCryptKey WinCryptProv::create_key(DWORD type, DWORD flags) const {
	HCRYPTKEY key = nullptr;
	CheckApi(::CryptGenKey(m_hnd, type, flags, &key));
	return WinCryptKey(key);
}

WinCryptKey WinCryptProv::get_key(DWORD type, DWORD flags) const {
	HCRYPTKEY key = nullptr;
	if (!::CryptGetUserKey(m_hnd, type, &key))
		CheckApi(::CryptGenKey(m_hnd, type, flags, &key));
	return WinCryptKey(key);
}

///======================================================================================== WinStore
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
	if (pfx.size() == pfx.frame(1000)) {
		File_map::iterator it = pfx.begin();
		CRYPT_DATA_BLOB blob = {it.size(), (PBYTE)it.data()};
		CheckApiThrowError(::PFXIsPFXBlob(&blob), ERROR_INVALID_DATA);
		HCERTSTORE tmpStore = CheckHandle(::PFXImportCertStore(&blob, pass.c_str(), CRYPT_MACHINE_KEYSET | CRYPT_EXPORTABLE));
		PCCERT_CONTEXT cert = nullptr;
		while ((cert = ::CertEnumCertificatesInStore(tmpStore, cert)) != nullptr) {
			if (!friendly_name.empty())
				WinCert::set_friendly_name(cert, friendly_name);
			CheckApi(::CertAddCertificateContextToStore(m_hnd, cert, CERT_STORE_ADD_NEW, 0));
			ret = WinCert::get_hash(cert);
		}
		::CertCloseStore(tmpStore, CERT_CLOSE_STORE_FORCE_FLAG);
	}
	return ret;
}

///================================================================================= WinCertificates
bool WinCertificates::cache(const CertificateStore & in) {
	HRESULT	err = 0;
	PCCERT_CONTEXT  pCert = nullptr;
	while ((pCert = ::CertEnumCertificatesInStore(in, pCert))) {
		WinCert	info(pCert);
		insert(value_type(info.GetHashString(), info));
	}
	err = ::GetLastError();
	return err == CRYPT_E_NOT_FOUND;
}

WinCertificates::iterator WinCertificates::find(const ustring & name) {
	return find(name);
}

WinCertificates::iterator WinCertificates::find_by_name(const ustring & in) {
	return find(in);
}

WinCertificates::iterator WinCertificates::find_by_friendly(const ustring & in) {
	return find(in);
}

void WinCertificates::del(const string & hash) {
	iterator it = find(hash);
	if (it != end())
		del(it);
}

void WinCertificates::del(iterator it) {
	it->second.Del();
	erase(it);
}
