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

///========================================================================================= WinCert
WinCert::WinCert(PCCERT_CONTEXT in): m_cert(nullptr) {
	if (in)
		m_cert = ::CertDuplicateCertificateContext(in);
}

WinCert::WinCert(const WinCert &in): m_cert(nullptr) {
	if (in.m_cert)
		m_cert = ::CertDuplicateCertificateContext(in.m_cert);
}

bool WinCert::Gen(const ustring &in, const ustring &guid, PSYSTEMTIME until) {
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

bool WinCert::ToFile(const ustring &path) const {
	bool Result = false;
	if (m_cert) {
		WinStore	store(path);
		store.OpenMemoryStore();
		if (store.IsOK()) {
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

ustring WinCert::GetProp(PCCERT_CONTEXT pctx, DWORD in) {
	ustring	Result;
	DWORD	cbData = 0;
	::CertGetCertificateContextProperty(pctx, in, nullptr, &cbData);
	if (cbData) {
		auto_array<BYTE> buf(cbData);
		if (::CertGetCertificateContextProperty(pctx, in, buf.data(), &cbData)) {
			Result = (PCWSTR)buf.data();
		}
	}
	return Result;
}
bool WinCert::FriendlyName(PCCERT_CONTEXT pctx, const ustring &in) {
	CertDataBlob	blob(in.c_str());
	return ::CertSetCertificateContextProperty(pctx, CERT_FRIENDLY_NAME_PROP_ID, 0, &blob);
}

string				WinCert::HashString(PCCERT_CONTEXT pctx) {
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

///======================================================================================== WinStore
string WinStore::FromFile(const ustring &path, const ustring &pass, const ustring &add) const {
	string Result;
	if (m_hnd) {
		File_map	pfx(path.c_str());
		pfx.frame(1000); // frame 64 MB
		if (pfx.size() == pfx.frame()) {
			File_map::iterator it = pfx.begin();
			CRYPT_DATA_BLOB blob;
			blob.cbData   = pfx.size();
			blob.pbData   = (PBYTE)it.data();
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
	return Result;
}

///================================================================================= WinCertificates
//bool WinCertificates::Del() {
//	if (ValidPtr()) {
//		if (Value().Del()) {
//			Erase();
//			return true;
//		}
//	}
//	return false;
//}
//
//bool WinCertificates::FindByName(const ustring &in) {
//	ForEachIn(this) {
//		if (Value().name() == in)
//			return true;
//	}
//	return false;
//}
//
//bool WinCertificates::FindByFriendlyName(const ustring &in) {
//	ForEachIn(this) {
//		if (Value().FriendlyName() == in)
//			return true;
//	}
//	return false;
//}
