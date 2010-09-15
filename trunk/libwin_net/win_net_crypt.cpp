/**
	win_net_cryptmgr
	Crypt utilites
	@classes	()
	@author		© 2009 Andrew Grechkin
	@link		(advapi32)
	@link		(crypt32)
	@link		(ole32)
**/

#include "win_net.h"

#define MY_ENCODING_TYPE  (PKCS_7_ASN_ENCODING | X509_ASN_ENCODING)

///========================================================================================== Base64
bool			Base64::Decode(PCSTR in, WinBuf<BYTE> &buf, DWORD flags) {
	DWORD	size = 0;
	if (::CryptStringToBinaryA(in, 0, flags, null_ptr, &size, null_ptr, null_ptr)) {
		buf.reserve(size);
		return	::CryptStringToBinaryA(in, 0, flags, buf, &size, null_ptr, null_ptr);
	}
	return	false;
}
bool			Base64::Decode(PCWSTR in, WinBuf<BYTE> &buf, DWORD flags) {
	DWORD	size = 0;
	if (::CryptStringToBinaryW(in, 0, flags, null_ptr, &size, null_ptr, null_ptr)) {
		buf.reserve(size);
		return	::CryptStringToBinaryW(in, 0, flags, buf, &size, null_ptr, null_ptr);
	}
	return	false;
}
CStrA			Base64::EncodeA(PVOID buf, DWORD size, DWORD flags) {
	CStrA	Result;
	DWORD	len = 0;
	if (::CryptBinaryToStringA((const PBYTE)buf, size, flags, null_ptr, &len)) {
		Result.reserve(len);
		::CryptBinaryToStringA((const PBYTE)buf, size, flags, (PSTR)Result.c_str(), &len);
	}
	return	Result;
}
AutoUTF			Base64::Encode(PVOID buf, DWORD size, DWORD flags) {
	AutoUTF	Result;
	DWORD	len = 0;
	if (::CryptBinaryToStringW((const PBYTE)buf, size, flags, null_ptr, &len)) {
		Result.reserve(len);
		::CryptBinaryToStringW((const PBYTE)buf, size, flags, (PWSTR)Result.c_str(), &len);
	}
	return	Result;
}

///==================================================================================== CertNameBlob
class			CertNameBlob : public WinErrorCheck {
	CERT_NAME_BLOB	m_info;

	CertNameBlob();
public:
	~CertNameBlob() {
		if (m_info.pbData) {
			WinMem::Free(m_info.pbData);
			m_info.cbData = 0;
			m_info.pbData = null_ptr;
		}
	}
//	CertNameBlob(const AutoUTF &in, DWORD enc = X509_ASN_ENCODING | PKCS_7_ASN_ENCODING) {
	CertNameBlob(const AutoUTF &in, DWORD enc = X509_ASN_ENCODING) {
		m_info.cbData = 0;
		m_info.pbData = null_ptr;
//		DWORD	dwStrType = CERT_X500_NAME_STR;
		DWORD	dwStrType = CERT_OID_NAME_STR;
		if (in.find(L'\"') == CStrA::npos)
			dwStrType |= CERT_NAME_STR_NO_QUOTING_FLAG;
		if (ChkSucc(::CertStrToNameW(enc, in.c_str(), dwStrType, null_ptr, m_info.pbData, &m_info.cbData, null_ptr))) {
			WinMem::Alloc(m_info.pbData, m_info.cbData);
			ChkSucc(::CertStrToNameW(enc, in.c_str(), dwStrType, null_ptr, m_info.pbData, &m_info.cbData, null_ptr));
		}
	}
	operator		CERT_NAME_BLOB*() {
		return	&m_info;
	}
	operator		BYTE*() {
		return	m_info.pbData;
	}
	CERT_NAME_BLOB*	GetBlob() {
		return	&m_info;
	}
	DWORD			GetSize() {
		return	m_info.cbData;
	}
};

///========================================================================================= WinCert
WinCert::WinCert(PCCERT_CONTEXT in): m_cert(null_ptr) {
	if (in)
		m_cert = ::CertDuplicateCertificateContext(in);
}
WinCert::WinCert(const WinCert &in): m_cert(null_ptr) {
	if (in.m_cert)
		m_cert = ::CertDuplicateCertificateContext(in.m_cert);
}
bool				WinCert::Gen(const AutoUTF &in, const AutoUTF &guid, PSYSTEMTIME until) {
	CertDataBlob blob;
	if (blob.MakeCertNameBlob(in)) {
		AutoUTF		capsule(guid.c_str());
		if (capsule.empty()) {
			capsule = WinGUID::Gen();
		}
		WinCryptProv	provider(MS_STRONG_PROV_W, PROV_RSA_FULL);
		if (provider.Open(capsule.c_str())) {
			if (provider.KeyCheck(AT_KEYEXCHANGE)) {
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

				m_cert = ::CertCreateSelfSignCertificate(provider, &blob, 0, &info, null_ptr, null_ptr, until, null_ptr);
				if (m_cert) {
//					CTL_USAGE	usage;
//					usage.cUsageIdentifier = 1;
//					usage.rgpszUsageIdentifier = (PSTR*)&szOID_PKIX_KP_SERVER_AUTH;
					::CertAddEnhancedKeyUsageIdentifier(m_cert, szOID_PKIX_KP_SERVER_AUTH);
				}
				return	ChkSucc(m_cert);
			}
		}
	}
	return	false;
}
bool				WinCert::Del() {
	if (m_cert) {
		if (ChkSucc(::CertDeleteCertificateFromStore(m_cert))) {
			m_cert = null_ptr;
			return	true;
		}
	}
	return	false;
}
bool				WinCert::ToFile(const AutoUTF &path) const {
	bool Result = false;
	if (m_cert) {
		WinStore	store(path);
		store.OpenMemoryStore();
		if (store.IsOK()) {
			if (::CertAddCertificateContextToStore(store, m_cert, CERT_STORE_ADD_NEW, null_ptr)) {
				//	::CertSetCertificateContextProperty(stCert, CERT_KEY_PROV_INFO_PROP_ID, 0, KeyProviderInfo(randomContainerName, PROV_RSA_FULL, AT_KEYEXCHANGE))
				CertDataBlob	blob;
				::PFXExportCertStoreEx(store, &blob, L"", 0, EXPORT_PRIVATE_KEYS);
				blob.reserve();
				if (::PFXExportCertStoreEx(store, &blob, L"", 0, EXPORT_PRIVATE_KEYS)) {
					Result = FileWrite(path.c_str(), blob.pbData, blob.cbData);
				}
			}
		}
	}
	return	Result;
}

bool				WinCert::AddKey(const AutoUTF &in) {
//	::CertSetCertificateContextProperty(_cert, CERT_KEY_PROV_INFO_PROP_ID, 0);
	return	false;
}
bool				WinCert::Store(HANDLE in) {
	return	ChkSucc(::CertAddCertificateContextToStore(in, m_cert, CERT_STORE_ADD_ALWAYS, null_ptr));
}
AutoUTF				WinCert::GetAttr(DWORD in) const {
	size_t	size = ::CertGetNameStringW(m_cert, in, 0, null_ptr, null_ptr, 0);
	WCHAR	buf[size];
	if (::CertGetNameStringW(m_cert, in, 0, null_ptr, buf, size)) {
		return	buf;
	}
	return	AutoUTF();
}
AutoUTF				WinCert::GetProp(DWORD in) const {
	AutoUTF	Result;
	DWORD	cbData = 0;
	::CertGetCertificateContextProperty(m_cert, in, null_ptr, &cbData);
	if (cbData) {
		WCHAR	buf[cbData];
		if (::CertGetCertificateContextProperty(m_cert, in, (PVOID)buf, &cbData)) {
			Result = buf;
		}
	}
	return	Result;
}
CStrA				WinCert::GetHashString() const {
	CStrA	Result;
	DWORD	cbData = 0;
	::CertGetCertificateContextProperty(m_cert, CERT_HASH_PROP_ID, null_ptr, &cbData);
	if (cbData) {
		WinBuf<BYTE>	buf(cbData, true);
		if (::CertGetCertificateContextProperty(m_cert, CERT_HASH_PROP_ID, (PVOID)buf.data(), &cbData)) {
			Result = Hash2Str(buf, cbData);
		}
	}
	return	Result;
}
size_t				WinCert::GetHashSize() const {
	DWORD	Result = 0;
	::CertGetCertificateContextProperty(m_cert, CERT_HASH_PROP_ID, null_ptr, &Result);
	return	Result;
}
bool				WinCert::GetHash(PVOID hash, DWORD size) const {
	DWORD	cbData = 0;
	::CertGetCertificateContextProperty(m_cert, CERT_HASH_PROP_ID, null_ptr, &cbData);
	if (cbData <= size) {
		if (::CertGetCertificateContextProperty(m_cert, CERT_HASH_PROP_ID, hash, &cbData)) {
			return	true;
		}
	}
	return	false;
}
bool				WinCert::GetHash(WinBuf<BYTE> &hash) const {
	hash.reserve(GetHashSize());
	DWORD	size = hash.size();
	if (::CertGetCertificateContextProperty(m_cert, CERT_HASH_PROP_ID, hash, &size)) {
		return	true;
	}
	return	false;
}

AutoUTF				WinCert::GetProp(PCCERT_CONTEXT pctx, DWORD in) {
	AutoUTF	Result;
	DWORD	cbData = 0;
	::CertGetCertificateContextProperty(pctx, in, null_ptr, &cbData);
	if (cbData) {
		WinBuf<BYTE>	buf(cbData);
		if (::CertGetCertificateContextProperty(pctx, in, (PVOID)buf.data(), &cbData)) {
			Result = (PCWSTR)buf.data();
		}
	}
	return	Result;
}
bool				WinCert::FriendlyName(PCCERT_CONTEXT pctx, const AutoUTF &in) {
	CertDataBlob	blob(in.c_str());
	return	::CertSetCertificateContextProperty(pctx, CERT_FRIENDLY_NAME_PROP_ID, 0, &blob);
}
CStrA				WinCert::HashString(PCCERT_CONTEXT pctx) {
	CStrA	Result;
	DWORD	cbData = 0;
	::CertGetCertificateContextProperty(pctx, CERT_HASH_PROP_ID, null_ptr, &cbData);
	if (cbData) {
		WinBuf<BYTE>	buf(cbData);
		if (::CertGetCertificateContextProperty(pctx, CERT_HASH_PROP_ID, (PVOID)buf.data(), &cbData)) {
			Result = Hash2Str(buf, cbData);
		}
	}
	return	Result;
}

///======================================================================================== WinStore
CStrA				WinStore::FromFile(const AutoUTF &path, const AutoUTF &pass, const AutoUTF &add) const {
	CStrA	Result;
	if (m_hnd) {
		FileMap	pfx(path.c_str());
		if (pfx.Next() && pfx.size() == pfx.sizeOfMap()) {
			CRYPT_DATA_BLOB blob;
			blob.cbData   = pfx.size();
			blob.pbData   = (BYTE*)pfx.data();
			if (::PFXIsPFXBlob(&blob)) {
				HCERTSTORE pfxStore = ::PFXImportCertStore(&blob, pass.c_str(), CRYPT_MACHINE_KEYSET | CRYPT_EXPORTABLE);
				if (pfxStore) {
					PCCERT_CONTEXT pctx   = 0;
					while (0 != (pctx = ::CertEnumCertificatesInStore(pfxStore, pctx))) {
						WinCert::FriendlyName(pctx, add);
						if (::CertAddCertificateContextToStore(m_hnd, pctx, CERT_STORE_ADD_NEW, 0)) {
							Result = WinCert::HashString(pctx);
						}
					}
					::CertCloseStore(pfxStore, CERT_CLOSE_STORE_FORCE_FLAG);
				}
			}
		}
	}
	return	Result;
}

///================================================================================= WinCertificates
bool				WinCertificates::Del() {
	if (ValidPtr()) {
		if (Value().Del()) {
			Erase();
			return	true;
		}
	}
	return	false;
}
bool				WinCertificates::FindByName(const AutoUTF &in) {
	ForEachIn(this) {
		if (Value().name() == in)
			return	true;
	}
	return	false;
}
bool				WinCertificates::FindByFriendlyName(const AutoUTF &in) {
	ForEachIn(this) {
		if (Value().FriendlyName() == in)
			return	true;
	}
	return	false;
}
