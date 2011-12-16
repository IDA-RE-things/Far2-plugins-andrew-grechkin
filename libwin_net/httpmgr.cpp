/**
	win_httpmgr
	Interface to IIS
	@classes	()
	@author		Copyright � 2009 Andrew Grechkin
	@link		(httpapi)
	@link		(wsock32)
**/

#include "httpmgr.h"
#include "exception.h"

///====================================================================================== HttpBindIP
HttpBindIP::~HttpBindIP() {
	delete pIpPort;
}

HttpBindIP::HttpBindIP(const ustring & ipport) {
	pIpPort = new (sockaddr);
	ustring	port = ipport;
	ustring	ip = CutWord(port, L":");
	Assign(ip, port);
}

HttpBindIP::HttpBindIP(const ustring & ip, const ustring & port) {
	pIpPort = new (sockaddr);
	Assign(ip, port);
}

HttpBindIP::HttpBindIP(const HttpBindIP & in) {
	pIpPort = new (sockaddr);
	WinMem::Copy(pIpPort, in.pIpPort, sizeof(sockaddr));
}

HttpBindIP & HttpBindIP::operator=(const HttpBindIP & in) {
	if (this != &in) {
		HttpBindIP(in).swap(*this);
	}
	return *this;
}

bool HttpBindIP::operator==(const HttpBindIP & rhs) const {
	sockaddr_in *sa1 = (sockaddr_in*)pIpPort;
	sockaddr_in *sa2 = (sockaddr_in*)rhs.pIpPort;
	return (sa1->sin_port == sa2->sin_port) && (sa1->sin_addr.s_addr == sa2->sin_addr.s_addr);
}

ustring HttpBindIP::get_ip() const {
	sockaddr_in * tmp = (sockaddr_in*)pIpPort;
	return ustring(inet_ntoa(tmp->sin_addr));
}

ustring HttpBindIP::get_port() const {
	sockaddr_in * tmp = (sockaddr_in*)pIpPort;
	return Num2Str(ntohs(tmp->sin_port));
}

ustring	HttpBindIP::as_str() const {
	return get_ip() + L":" + get_port();
}

bool HttpBindIP::copy(HTTP_SERVICE_CONFIG_SSL_KEY & out) const {
	out.pIpPort = new SOCKADDR;
	WinMem::Copy(out.pIpPort, pIpPort, sizeof(*pIpPort));
	return true;
}

void HttpBindIP::swap(HttpBindIP & rhs) {
	using std::swap;
	swap(pIpPort, rhs.pIpPort);
}

bool HttpBindIP::is_valid(const ustring & ip) {
	in_addr	addr;
	addr.s_addr = inet_addr(ip.utf8().c_str());
	return addr.s_addr != INADDR_NONE;
}

bool HttpBindIP::Assign(const ustring & ip, const ustring & port) {
	u_short	prt = htons(AsUInt(port.c_str()));
	if (is_valid(ip) && prt) {
		sockaddr_in *tmp = (sockaddr_in*)pIpPort;
		tmp->sin_port		= prt;
		tmp->sin_addr.s_addr = inet_addr(ip.utf8().c_str());
		tmp->sin_family		= AF_INET;
		return true;
	}
	return false;
}

///=================================================================================== HttpBindParam
HttpBindParam::HttpBindParam(const astring & hash) {
	WinMem::Zero(*this);
	Str2Hash(hash, pSslHash, SslHashLength);
	pSslCertStoreName = (PWSTR)L"MY";
}

bool HttpBindParam::copy(HTTP_SERVICE_CONFIG_SSL_PARAM & out) const {
	WinMem::Zero(out);

	out.AppId = GUID_ISPmanager;
	out.DefaultFlags = HTTP_SERVICE_CONFIG_SSL_FLAG_NEGOTIATE_CLIENT_CERT;
	out.pSslCertStoreName = (PWSTR)L"MY";

	out.SslHashLength = SslHashLength;
	out.pSslHash = new BYTE[out.SslHashLength];
	WinMem::Copy(out.pSslHash, pSslHash, out.SslHashLength);
	return true;
}

astring as_str(const HTTP_SERVICE_CONFIG_SSL_PARAM & m_data) {
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
	CheckApiError(::HttpInitialize(httpVer, HTTP_INITIALIZE_CONFIG, NULL));
}

bool HttpServer::get_ssl(const HttpBindIP & ip, auto_buf<PHTTP_SERVICE_CONFIG_SSL_SET> & info) const {
	HttpSslQuery query(ip);
	return get_ssl(query, info);
}

bool HttpServer::get_ssl(HttpSslQuery & query, auto_buf<PHTTP_SERVICE_CONFIG_SSL_SET> & info) const {
	ULONG ReturnLength = 0;
	ULONG err =::HttpQueryServiceConfiguration(NULL, HttpServiceConfigSSLCertInfo, &query, sizeof(query),
	                                           info.data(), info.size(), &ReturnLength, NULL);
	if (err == ERROR_MORE_DATA) {
		info.reserve(ReturnLength);
		err = ::HttpQueryServiceConfiguration(NULL, HttpServiceConfigSSLCertInfo, &query, sizeof(query),
		                                      info.data(), info.size(), &ReturnLength, NULL);
		if (err == NO_ERROR) {
			++query;
			return true;
		}
	}
	return false;
}

void HttpServer::set(const HttpSslSet & info) const {
	CheckApiError(::HttpSetServiceConfiguration(NULL, HttpServiceConfigSSLCertInfo, (PVOID)&info, sizeof(info), NULL));
}

void HttpServer::del(const HttpBindIP &ip) const {
	HTTP_SERVICE_CONFIG_SSL_SET	info;
	WinMem::Zero(info);

	info.KeyDesc = ip;
	CheckApiError(::HttpDeleteServiceConfiguration(NULL, HttpServiceConfigSSLCertInfo, (PVOID) &info, sizeof(info), NULL));
}

bool HttpServer::find(const astring & hash) const {
	HttpSslQuery query;
	auto_buf<PHTTP_SERVICE_CONFIG_SSL_SET> info(512);
	while (get_ssl(query, info)) {
		if (hash == as_str(info->ParamDesc))
			return true;
	}
	return false;
}

bool HttpServer::is_exist(const ustring & ip, const ustring & port) {
	auto_buf<PHTTP_SERVICE_CONFIG_SSL_SET> info(512);
	return get_ssl(HttpBindIP(ip, port), info) || get_ssl(HttpBindIP(L"0.0.0.0", port), info);
}
