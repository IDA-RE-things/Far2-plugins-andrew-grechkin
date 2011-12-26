//============================================================================
// Name        : main.cpp
// Author      : Andrew Grechkin
// Version     :
// Copyright   : Your copyright notice
// Description : hMailServer example
//============================================================================

#include <API_hMailServer/hMailServer_uid.h>
#include <API_hMailServer/hMailServer.h>
#include <libwin_com/win_com.h>
#include <libwin_net/exception.h>
#include <libwin_def/console.h>

struct hmail_server {
	hmail_server() {
		WinCOM::init();

		CheckApiError(CoCreateInstance(CLSID_Application, nullptr, CLSCTX_LOCAL_SERVER,
		                               IID_IInterfaceApplication, (PVOID*)&m_app));
		CheckHMailError(m_app->Connect());
		CheckHMailError(m_app->Authenticate(BStr(L"Administrator"), BStr(L"qwerty1"), &m_acc));
		CheckHMailError(m_app->Stop());
	}
private:
	ComObject<IInterfaceApplication> m_app;
	ComObject<IInterfaceAccount> m_acc;
};

int main() try {
	hmail_server srv;

	return 0;
} catch (WinError & e) {
	return e.format_error();
} catch (std::exception & e) {
	printf("std::exception [%s]:\n", typeid(e).name());
	printf("What: %s\n", e.what());
	return 1;
}
