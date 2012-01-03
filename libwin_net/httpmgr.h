/**
	win_httpmgr
	Interface to IIS ssl binders
	@classes	()
	@author		Copyright © 2009 Andrew Grechkin
	@link		(httpapi)
	@link		(wsock32)
**/
#ifndef WIN_HTTPMGR_HPP
#define WIN_HTTPMGR_HPP

#include <http.h>

#include <libwin_net/win_net.h>

// {94F0DFD3-0DB2-49d5-B560-34B2B1706F72}
static const GUID GUID_ISPmanager = { 0x94f0dfd3, 0xdb2, 0x49d5, { 0xb5, 0x60, 0x34, 0xb2, 0xb1, 0x70, 0x6f, 0x72 } };

namespace Http {
	///====================================================================================== HttpBindIP
	struct HttpBindIP: public HTTP_SERVICE_CONFIG_SSL_KEY {
		~HttpBindIP();

		HttpBindIP(const ustring & ipport);

		HttpBindIP(const ustring & ip, const ustring & port);

		explicit HttpBindIP(const HTTP_SERVICE_CONFIG_SSL_KEY & in);

		HttpBindIP(const HttpBindIP & in);

		HttpBindIP & operator=(const HttpBindIP & in);

		operator PSOCKADDR() const {
			return pIpPort;
		};

		bool operator==(const HttpBindIP & rhs) const;

		ustring	get_ip() const;

		ustring	get_port() const;

		ustring	as_str() const;

		bool copy(HTTP_SERVICE_CONFIG_SSL_KEY & out) const;

		void swap(HttpBindIP & rhs);

	private:
		bool is_valid(const ustring & ip);
		bool Assign(const ustring & ip, const ustring & port);
	};

	///=================================================================================== HttpBindParam
	struct HttpBindParam : public HTTP_SERVICE_CONFIG_SSL_PARAM, private Uncopyable {
		~HttpBindParam();

		HttpBindParam(const ustring & hash);

		bool copy(HTTP_SERVICE_CONFIG_SSL_PARAM & out) const;
	};

	//astring AsStr(const HTTP_SERVICE_CONFIG_SSL_PARAM &m_data);

	///==================================================================================== SslQuery
	///	replace for HTTP_SERVICE_CONFIG_SSL_QUERY
	struct SslQuery: public HTTP_SERVICE_CONFIG_SSL_QUERY {
		~SslQuery();

		SslQuery();

		explicit SslQuery(const HttpBindIP & ip);

		SslQuery & operator ++();
	};

	///====================================================================================== SslSet
	///	replace for HTTP_SERVICE_CONFIG_SSL_SET
	struct SslSet: public HTTP_SERVICE_CONFIG_SSL_SET {
		~SslSet();

		SslSet(const HttpBindIP & ip, const HttpBindParam & param);
	};

	///====================================================================================== Server
	struct Server {
		~Server();

		Server();

		bool get_ssl(const HttpBindIP & ip, auto_buf<PHTTP_SERVICE_CONFIG_SSL_SET> & info) const;

		bool get_ssl(SslQuery & query, auto_buf<PHTTP_SERVICE_CONFIG_SSL_SET> & info) const;

		void set(const SslSet & info) const;

		void del(const HttpBindIP & ip) const;

		bool find(const ustring & hash) const;

		bool is_exist(const ustring & ip, const ustring & port);
	};
}

#endif
