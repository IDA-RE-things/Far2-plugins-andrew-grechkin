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

#include <map>

const int MY_ENCODING_TYPE = (PKCS_7_ASN_ENCODING | X509_ASN_ENCODING);

///▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒ net_crypt
///========================================================================================== Base64
namespace	Base64 {
	auto_array<BYTE> Decode(PCSTR in, DWORD flags = CRYPT_STRING_BASE64_ANY);

	auto_array<BYTE> Decode(PCWSTR in, DWORD flags = CRYPT_STRING_BASE64_ANY);

	astring EncodeA(const auto_array<BYTE> & buf, DWORD flags = CRYPT_STRING_BASE64);

	ustring	Encode(const auto_array<BYTE> & buf, DWORD flags = CRYPT_STRING_BASE64);

	astring EncodeA(PCVOID buf, DWORD size, DWORD flags = CRYPT_STRING_BASE64);

	ustring	Encode(PCVOID buf, DWORD size, DWORD flags = CRYPT_STRING_BASE64);
}

namespace Crypt {
	///==================================================================================== DataBlob
	struct	DataBlob: public _CRYPTOAPI_BLOB {
		typedef DataBlob this_type;
		using _CRYPTOAPI_BLOB::cbData;
		using _CRYPTOAPI_BLOB::pbData;

		~DataBlob();

		DataBlob();

		DataBlob(size_t size);

		DataBlob(const ustring & in);

		bool reserve();

		bool reserve(size_t size);

		void swap(this_type & rhs);
	};

	struct NameBlob: public DataBlob {
		NameBlob(const ustring & in, DWORD enc = X509_ASN_ENCODING | PKCS_7_ASN_ENCODING);
	};

	///==================================================================================== Provider
	struct Key {
		~Key();

		operator HCRYPTKEY() const {
			return m_key;
		}

	private:
		Key(HCRYPTKEY key);

		HCRYPTKEY m_key;

		friend class Provider;
	};

	struct Provider: private Uncopyable {
		~Provider();

		// type = (PROV_RSA_FULL, PROV_RSA_AES) flags = CRYPT_MACHINE_KEYSET
		Provider(PCWSTR name = nullptr, DWORD flags = CRYPT_VERIFYCONTEXT, PCWSTR prov = nullptr, DWORD type = PROV_RSA_FULL);

		bool is_exist_key(DWORD type) const;

		Key create_key(DWORD type, DWORD flags = CRYPT_EXPORTABLE) const;

		Key get_key(DWORD type, DWORD flags = CRYPT_EXPORTABLE) const;

		operator HCRYPTPROV() const {
			return m_hnd;
		}

	private:
		HCRYPTPROV	m_hnd;
	};

	///======================================================================================== Hash
	struct Hash {
		~Hash();

		// alg = (CALG_MD5, CALG_SHA1, CALG_SHA_512)
		Hash(HCRYPTPROV prov, ALG_ID alg);

		Hash(HCRYPTHASH hash);

		Hash(const PBYTE buf, size_t size);

		Hash(const WinFile & wf, uint64_t size = (uint64_t) - 1);

		Hash(const File_map & file);

		Hash(const Hash & in);

		Hash & operator=(const Hash & in);

		size_t get_size() const;

		ALG_ID get_algorithm() const;

		void get_hash(PBYTE buf, DWORD size) const;

		operator HCRYPTHASH() const {
			return m_handle;
		}

		void swap(Hash & rhs);

	private:
		HCRYPTHASH	m_handle;
	};

	///============================================================================ CertificateStore
	struct CertificateStore: private Uncopyable {
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

		ustring get_name() const {
			return m_name;
		}

		ustring import_pfx(const ustring & path, const ustring & pass, const ustring & friendly_name = ustring()) const;

	private:
		HCERTSTORE	m_hnd;
		ustring		m_name;
	};

	///================================================================================= Certificate
	struct Certificate {
		~Certificate();

		explicit Certificate(PCCERT_CONTEXT in);
		Certificate(const ustring & in, const ustring & guid, PSYSTEMTIME until = nullptr);

		Certificate(const Certificate & in);

		Certificate & operator=(const Certificate & in);

		void del();

		void export_to_file(const ustring & path) const;

		void add_key(const ustring & in);
		void add_to_store(HCERTSTORE in);

		ustring get_name() const {
			return get_attr(m_cert, CERT_NAME_SIMPLE_DISPLAY_TYPE);
		}
		ustring get_dns() const {
			return get_attr(m_cert, CERT_NAME_DNS_TYPE);
		}
		ustring get_url() const {
			return get_attr(m_cert, CERT_NAME_URL_TYPE);
		}
		ustring get_upn() const {
			return get_attr(m_cert, CERT_NAME_UPN_TYPE);
		}
		ustring get_mail() const {
			return get_attr(m_cert, CERT_NAME_EMAIL_TYPE);
		}
		ustring get_rdn() const {
			return get_attr(m_cert, CERT_NAME_RDN_TYPE);
		}
		FILETIME get_start() const {
			return m_cert->pCertInfo->NotBefore;
		}
		FILETIME get_end() const {
			return m_cert->pCertInfo->NotBefore;
		}

		size_t get_hash_size() const {
			return get_hash_size(m_cert);
		}
		void get_hash(PVOID hash, DWORD size) const;
		auto_array<BYTE> get_hash() const;

		astring get_hash_string() const {
			return get_hash_string(m_cert);
		}
		ustring get_friendly_name() const {
			//		return GetAttr(CERT_NAME_FRIENDLY_DISPLAY_TYPE);
			return get_property(m_cert, CERT_FRIENDLY_NAME_PROP_ID);
		}
		void set_friendly_name(const ustring & in) const {
			set_friendly_name(m_cert, in);
		}

		operator PCCERT_CONTEXT() {
			return m_cert;
		}

		void swap(Certificate & rhs);

		static ustring get_attr(PCCERT_CONTEXT pctx, DWORD in);
		static ustring get_property(PCCERT_CONTEXT pctx, DWORD in);
		static ustring get_friendly_name(PCCERT_CONTEXT pctx) {
			return get_property(pctx, CERT_FRIENDLY_NAME_PROP_ID);
		}
		static void set_friendly_name(PCCERT_CONTEXT pctx, const ustring & in);
		static size_t get_hash_size(PCCERT_CONTEXT pctx);
		static ustring get_hash_string(PCCERT_CONTEXT pctx);

	private:
		PCCERT_CONTEXT m_cert;
	};

	///============================================================================= WinCertificates
	struct Certificates: private std::map<ustring, Certificate> {
		typedef std::map<ustring, Certificate> this_type;
		typedef this_type::value_type value_type;
		typedef this_type::iterator iterator;
		typedef this_type::const_iterator const_iterator;
		using map::begin;
		using map::end;
		using map::size;

	public:
		~Certificates() {
		}

		Certificates(const CertificateStore & in) {
			cache(in);
		}

		bool cache(const CertificateStore & in);

		iterator find(const ustring & name);
		iterator find_by_name(const ustring & name);
		iterator find_by_friendly(const ustring & name);

		//	void	add(const ustring & name, const ustring & pass = ustring());
		void del(const astring & hash);
		void del(iterator it);
	};
}

#endif
