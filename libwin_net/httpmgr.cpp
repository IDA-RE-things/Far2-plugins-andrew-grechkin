/**
	win_httpmgr
	Interface to IIS
	@classes	()
	@author		Copyright � 2009 Andrew Grechkin
	@link		(httpapi)
	@link		(wsock32)
**/

#include "httpmgr.h"

///====================================================================================== HttpBindIP
bool		HttpBindIP::CreateData() {
	DestroyData();
	m_data.pIpPort = new SOCKADDR;
	return m_data.pIpPort;
}
bool		HttpBindIP::DestroyData() {
	if (m_data.pIpPort != NULL) {
		delete	m_data.pIpPort;
		m_data.pIpPort = NULL;
		return true;
	}
	return false;
}
bool		HttpBindIP::IsValidIP(const AutoUTF &inout) {
	in_addr	addr;
	addr.s_addr = inet_addr(inout.utf8().c_str());
	if (addr.s_addr == INADDR_NONE)
		return false;
//		inout = inet_ntoa(addr);
	return true;
}
bool		HttpBindIP::Assign(const AutoUTF &ip, const AutoUTF &port) {
	u_short	prt = htons(AsUInt(port.c_str()));
	if (IsValidIP(ip) && prt) {
		CreateData();
		sockaddr_in *tmp = (sockaddr_in*)m_data.pIpPort;
		tmp->sin_port		= prt;
		tmp->sin_addr.s_addr = inet_addr(ip.utf8().c_str());
		tmp->sin_family		= AF_INET;
		return true;
	}
	return false;
}

HttpBindIP::~HttpBindIP() {
	DestroyData();
}
HttpBindIP::HttpBindIP(const AutoUTF &ipport) {
	m_data.pIpPort = NULL;
	AutoUTF	port = ipport;
	AutoUTF	ip = CutWord(port, L":");
	Assign(ip, port);
}
HttpBindIP::HttpBindIP(const AutoUTF &ip, const AutoUTF &port) {
	m_data.pIpPort = NULL;
	Assign(ip, port);
}
HttpBindIP::HttpBindIP(const HTTP_SERVICE_CONFIG_SSL_KEY &in) {
	m_data.pIpPort = NULL;
	if (in.pIpPort) {
		CreateData();
		WinMem::Copy(m_data.pIpPort, in.pIpPort, sizeof(sockaddr));
	}
}
HttpBindIP&	HttpBindIP::operator=(const HTTP_SERVICE_CONFIG_SSL_KEY & in) {
	DestroyData();
	if (in.pIpPort) {
		CreateData();
		WinMem::Copy(m_data.pIpPort, in.pIpPort, sizeof(sockaddr));
	}
	return *this;
}
AutoUTF		HttpBindIP::GetIP() const {
	AutoUTF Result;
	if (m_data.pIpPort) {
		sockaddr_in *tmp = (sockaddr_in*)m_data.pIpPort;
		Result += AutoUTF(inet_ntoa(tmp->sin_addr));
	}
	return Result;
}
AutoUTF		HttpBindIP::GetPort() const {
	AutoUTF	Result;
	if (m_data.pIpPort) {
		sockaddr_in *tmp = (sockaddr_in*)m_data.pIpPort;
		u_short port = ntohs(tmp->sin_port);
		if (port) {
			Result += Num2Str(port);
		}
	}
	return Result;
}

///=================================================================================== HttpBindParam
HttpBindParam::HttpBindParam(const astring &hash) {
	WinMem::Zero(m_data);
	Str2Hash(hash, m_data.pSslHash, m_data.SslHashLength);
	m_data.pSslCertStoreName = (PWSTR)L"MY";
}
HttpBindParam::operator	HTTP_SERVICE_CONFIG_SSL_PARAM() const {
	return m_data;
}
astring		AsStr(const HTTP_SERVICE_CONFIG_SSL_PARAM &m_data) {
	astring	Result;
	Result = Hash2Str((PBYTE)m_data.pSslHash, m_data.SslHashLength);
	return Result;
}

///=================================================================================== WinHttpServer
HttpServer::~HttpServer() {
	::HttpTerminate(HTTP_INITIALIZE_CONFIG, NULL);
}
HttpServer::HttpServer() {
	HTTPAPI_VERSION	httpVer = HTTPAPI_VERSION_1;
	err(::HttpInitialize(httpVer, HTTP_INITIALIZE_CONFIG, NULL));
	if (!IsOK())
		throw	"Can't initialise HttpHelper";
}
bool		HttpServer::Get(const HttpBindIP &ip, auto_buf<PHTTP_SERVICE_CONFIG_SSL_SET> &info) const {
	ULONG	ReturnLength = 0;
	HttpSslQuery	query(HttpServiceConfigQueryExact);
	query.KeyDesc = ip;
	err(::HttpQueryServiceConfiguration(NULL, HttpServiceConfigSSLCertInfo, &query, sizeof(query),
										info.data(), info.size(), &ReturnLength, NULL));
	if (err() == ERROR_MORE_DATA) {
		info.reserve(ReturnLength);
		err(::HttpQueryServiceConfiguration(NULL, HttpServiceConfigSSLCertInfo, &query, sizeof(query),
											info.data(), info.size(), &ReturnLength, NULL));
	}
	return IsOK();
}
bool		HttpServer::Get(HttpSslQuery &query, auto_buf<PHTTP_SERVICE_CONFIG_SSL_SET> &info) const {
	ULONG	ReturnLength = 0;
	err(::HttpQueryServiceConfiguration(NULL, HttpServiceConfigSSLCertInfo, &query, sizeof(query),
										info.data(), info.size(), &ReturnLength, NULL));
	if (err() == ERROR_MORE_DATA) {
		info.reserve(ReturnLength);
		err(::HttpQueryServiceConfiguration(NULL, HttpServiceConfigSSLCertInfo, &query, sizeof(query),
											info.data(), info.size(), &ReturnLength, NULL));
	}
	if (IsOK()) {
		++query;
		return true;
	}
	return false;
}
bool		HttpServer::Set(const HttpSslSet &info) const {
	err(::HttpSetServiceConfiguration(NULL, HttpServiceConfigSSLCertInfo, (PVOID)&info, sizeof(info), NULL));
	return IsOK();;

}
bool		HttpServer::Del(const HttpBindIP &ip) const {
	HTTP_SERVICE_CONFIG_SSL_SET	info;
	WinMem::Zero(info);

	info.KeyDesc = ip;
	err(::HttpDeleteServiceConfiguration(NULL, HttpServiceConfigSSLCertInfo, (PVOID) &info, sizeof(info), NULL));
	return IsOK();
}
bool		HttpServer::Find(const astring &hash) const {
	HttpSslQuery	query;
	auto_buf<PHTTP_SERVICE_CONFIG_SSL_SET>	info(500);
	HttpServer	httphelper;
	while (httphelper.Get(query, info)) {
		astring	tmp = AsStr(info->ParamDesc);
		if (hash == tmp)
			return true;
	}
	return false;
}
bool		HttpServer::IsExist(const AutoUTF &ip, const AutoUTF &port) {
	auto_buf<PHTTP_SERVICE_CONFIG_SSL_SET>	info(500);
	if (!Get(HttpBindIP(ip, port), info)) {
		return Get(HttpBindIP(L"0.0.0.0", port), info);
	}
	return true;

}
