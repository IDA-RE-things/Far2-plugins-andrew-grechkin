/**
	win_httpmgr
	Interface to IIS
	@classes	()
	@author		Copyright � 2009 Andrew Grechkin
	@link		(httpapi)
	@link		(wsock32)
**/
#ifndef WIN_HTTPMGR_HPP
#define WIN_HTTPMGR_HPP

#include <libwin_net/win_net.h>

#include <http.h>

///===================================================================================== definitions


///====================================================================================== HttpBindIP
class		HttpBindIP {
	HTTP_SERVICE_CONFIG_SSL_KEY	m_data;

	bool CreateData();
	bool DestroyData();
	bool IsValidIP(const AutoUTF &inout);
	bool Assign(const AutoUTF &ip, const AutoUTF &port);
public:
	~HttpBindIP();
	HttpBindIP(const AutoUTF &ipport);
	explicit	HttpBindIP(const AutoUTF &ip, const AutoUTF &port);
	explicit	HttpBindIP(const HTTP_SERVICE_CONFIG_SSL_KEY &in);

	HttpBindIP&	operator=(const HTTP_SERVICE_CONFIG_SSL_KEY &in);
	operator	HTTP_SERVICE_CONFIG_SSL_KEY() const {
		return	m_data;
	}
	operator	PSOCKADDR() const {
		return	m_data.pIpPort;
	};

	bool		operator==(const HttpBindIP &rhs) const {
		sockaddr_in *sa1 = (sockaddr_in*)m_data.pIpPort;
		sockaddr_in *sa2 = (sockaddr_in*)rhs.m_data.pIpPort;
		return	(sa1->sin_port == sa2->sin_port) && (sa1->sin_addr.s_addr == sa2->sin_addr.s_addr);
	}

	AutoUTF		GetIP() const;
	AutoUTF		GetPort() const;
	AutoUTF		AsStr() const {
		return	GetIP() + L":" + GetPort();
	}
	bool		CopySelf(HTTP_SERVICE_CONFIG_SSL_KEY &out) const {
		WinMem::Zero(out);
		if (m_data.pIpPort) {
			out.pIpPort = new SOCKADDR;
			WinMem::Copy(out.pIpPort, m_data.pIpPort, sizeof(*m_data.pIpPort));
			return	true;
		}
		return	false;
	}
};

///=================================================================================== HttpBindParam
class		HttpBindParam {
	HTTP_SERVICE_CONFIG_SSL_PARAM m_data;
public:
	HttpBindParam(const astring &hash);
	operator	HTTP_SERVICE_CONFIG_SSL_PARAM() const;
	bool		CopySelf(HTTP_SERVICE_CONFIG_SSL_PARAM &out) const {
		WinMem::Zero(out);

		// {94F0DFD3-0DB2-49d5-B560-34B2B1706F72}
		static const GUID GUID_ISPmanager = { 0x94f0dfd3, 0xdb2, 0x49d5, { 0xb5, 0x60, 0x34, 0xb2, 0xb1, 0x70, 0x6f, 0x72 } };
		out.AppId = GUID_ISPmanager;
		out.DefaultFlags = HTTP_SERVICE_CONFIG_SSL_FLAG_NEGOTIATE_CLIENT_CERT;
		out.pSslCertStoreName = (PWSTR)L"MY";

		out.SslHashLength = m_data.SslHashLength;
		out.pSslHash = new BYTE[out.SslHashLength];
		WinMem::Copy(out.pSslHash, m_data.pSslHash, out.SslHashLength);
		return	true;
	}
};
astring			AsStr(const HTTP_SERVICE_CONFIG_SSL_PARAM &m_data);

///==================================================================================== HttpSslQuery
///	replace for HTTP_SERVICE_CONFIG_SSL_QUERY
struct		HttpSslQuery: public HTTP_SERVICE_CONFIG_SSL_QUERY {
	explicit	HttpSslQuery() {
		QueryDesc = HttpServiceConfigQueryNext;
		dwToken = 0;
	}
	explicit	HttpSslQuery(const HTTP_SERVICE_CONFIG_QUERY_TYPE &type) {
		QueryDesc = type;
		dwToken = 0;
	}
	HttpSslQuery&		operator++() {
		++dwToken;
		return	*this;
	}
};

///====================================================================================== HttpSslSet
///	replace for HTTP_SERVICE_CONFIG_SSL_SET
struct		HttpSslSet: public HTTP_SERVICE_CONFIG_SSL_SET {
	~HttpSslSet() {
		if (KeyDesc.pIpPort) {
			delete	KeyDesc.pIpPort;
		}
	}
	explicit	HttpSslSet(const HttpBindIP &ip, const HttpBindParam &param) {
		WinMem::Zero(KeyDesc);
		WinMem::Zero(ParamDesc);
		ip.CopySelf(KeyDesc);
		param.CopySelf(ParamDesc);
	}
};

///====================================================================================== HttpServer
class		HttpServer: private Uncopyable, public WinErrorCheck {
public:
	~HttpServer();
	HttpServer();

	bool		Get(const HttpBindIP &ip, auto_buf<PHTTP_SERVICE_CONFIG_SSL_SET> &info) const;
	bool		Get(HttpSslQuery &query, auto_buf<PHTTP_SERVICE_CONFIG_SSL_SET> &info) const;
	bool		Set(const HttpSslSet &info) const;
	bool		Del(const HttpBindIP &ip) const;
	bool		Find(const astring &hash) const;
	bool		IsExist(const AutoUTF &ip, const AutoUTF &port);
};

#endif // WIN_HTTPMGR_HPP
