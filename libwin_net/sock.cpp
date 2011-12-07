#include "win_net.h"
///======================================================================================== WSockLib
WSockLib::~WSockLib() {
	::WSACleanup();
}

WSockLib & WSockLib::Init() {
	static WSockLib init;
	return init;
}

WSockLib::WSockLib() {
	CheckWSock(::WSAStartup(MAKEWORD(2, 2), &wsaData));
}

///=========================================================================================== WSock
WSock::~WSock() {
	::closesocket(m_sock);
}

WSock::WSock(int fam):
	m_sock(INVALID_SOCKET),
	m_fam(fam) {
	WSockLib::Init();
	m_sock = ::socket(m_fam, SOCK_STREAM, 0);
	CheckWSock(m_sock == INVALID_SOCKET);
}

void WSock::Connect(const ustring & ip, DWORD port) {
	INT		size = 128;
	auto_buf<PSOCKADDR>	addr(size);
	int err = ::WSAStringToAddressW((PWSTR)ip.c_str(), m_fam, nullptr, addr, &size);
	if (err == WSAEFAULT) {
		addr.reserve(size);
		CheckWSock(::WSAStringToAddressW((PWSTR)ip.c_str(), m_fam, nullptr, addr, &size));
	} else {
		CheckWSock(err);
	}
	if (m_fam == AF_INET || m_fam == AF_INET6) {
		sockaddr_in* tmp = (sockaddr_in*)addr.data();
		tmp->sin_port = htons(port);
	}
	CheckWSock(::connect(m_sock, addr.data(), size));
}

void WSock::Send(void * buf, size_t len) {
	::send(m_sock, (const char*)buf, len, 0);
}
