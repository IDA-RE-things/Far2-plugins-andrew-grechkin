//============================================================================
// Name        : 7-zip.cpp
// Author      : Andrew Grechkin
// Version     :
// Copyright   : Your copyright notice
// Description : 7-zip example
//============================================================================

#include <libwin_com/crypt.h>
#include <libwin_net/exception.h>
#include <libwin_net/httpmgr.h>
#include <libwin_def/console.h>

struct ListCert: public Command_p {
	ListCert(PCWSTR st):
		m_st(st) {
	}

	bool execute() const {
		Crypt::CertificateStore store(m_st.c_str(), Crypt::CertificateStore::stMachine);
		printf(L"Store opend: %s\n", store.get_name().c_str());

		Crypt::Certificates crts(store);
		printf(L"Number of crts: %Id\n", crts.size());
		for (Crypt::Certificates::iterator it = crts.begin(); it != crts.end(); ++it) {
			printf(L"%s %s \n",
			       it->first.c_str(),
			       it->second.get_friendly_name().c_str());
			printf(L"\tname: %s\n", it->second.get_name().c_str());
			printf(L"\tdns: %s\n", it->second.get_dns().c_str());
			printf(L"\turl: %s\n", it->second.get_url().c_str());
			printf(L"\tupn: %s\n", it->second.get_upn().c_str());
			printf(L"\tmail: %s\n", it->second.get_mail().c_str());
			printf(L"\trdn: %s\n", it->second.get_rdn().c_str());
		}
		return true;
	}

private:
	ustring m_st;
};

struct BindCert: public Command_p {
	BindCert(PCWSTR hash, PCWSTR ip):
		m_hash(hash),
		m_ip(ip) {
	}

	bool execute() const {
		Http::SslSet info(m_ip, m_hash);
		Http::Server httphelper;
		httphelper.set(info);
//		cout << "Bind: " << ip_port.AsStr() << endl << "Hash: " << ustring(szHash) << endl;
		return true;
	}

private:
	ustring m_hash, m_ip;
};

struct ListHttpBinds: public Command_p {
	ListHttpBinds() {
	}

	bool execute() const {
		Http::Server httphelper;
		printf(L"Https bindings:\n");
		Http::HttpSslQuery httpQuery;
		auto_buf<PHTTP_SERVICE_CONFIG_SSL_SET> info;
		while (httphelper.get_ssl(httpQuery, info)) {
			Http::HttpBindIP tmp(info->KeyDesc);
			printf(L"%s\n", tmp.as_str().c_str());
//			cout << tmp.AsStr();
//	//				sockaddr_in *ip = (sockaddr_in*)ipinfo;
//	//				cout << "IP addr: " << inet_ntoa(ip->sin_addr) << ":" << ntohs(ip->sin_port) << endl;
//	//			}
//
//			string	hashstr;
//			Hash2Str(hashstr, (const char *)info->ParamDesc.pSslHash, info->ParamDesc.SslHashLength);
//			cout << "\t" << hashstr;
//			cout << " (Store name: " << oem(info->ParamDesc.pSslCertStoreName);
//
//	//		if (allcert.Find(hashstr)) {
//	//			cout << " CERT: " << allcert.Value().FriendlyName();
//	//			ustring dee = "c:\\Temp\\" + hashstr + ".pfx";
//	//			allcert.Value().ToFile(dee);
//	//		}
//			cout << ") " << endl;
		}
		return true;
	}
};

struct ShowHelp: public Command_p {
	ShowHelp(PCWSTR prg):
		m_prg(only_file(prg)) {
	}

	bool execute() const {
		printf(L"Простой пример работы с Cert\nAndrew Grechkin, 2011\n");
//		printf(L"\nИнфо:\n");
//		printf(L"\t%s /i\n", m_prg.c_str());
//		printf(L"Лист:\n");
//		printf(L"\t%s /l arc.name\n", m_prg.c_str());
//		printf(L"Тест:\n");
//		printf(L"\t%s /t arc.name\n", m_prg.c_str());
//		printf(L"Распаковка:\n");
//		printf(L"\t%s /e arc.name dest.path\n", m_prg.c_str());
//		printf(L"Запаковка:\n");
//		printf(L"\t%s /a arc.name src.path tar\n", m_prg.c_str());
		return true;
	}
private:
	ustring m_prg;
};

struct CmdParser: public Command_p {
	CmdParser(PWSTR cmd_line):
		argv(::CommandLineToArgvW(cmd_line, &argc), LocalFree),
		action(new ShowHelp(argv[0])) {
		for (int i = 1; i < argc; ++i) {
			if (Eqi(argv[i], L"/?")) {
				action.reset(new ShowHelp(argv[0]));
				break;
			}
//
//			if (Eqi(argv[i], L"/i")) {
//				action.reset(new ArcInfo(arc_lib));
//				break;
//			}
//
			if (Eqi(argv[i], L"/lc") && i < (argc - 1)) {
				action.reset(new ListCert(argv[i + 1]));
				break;
			}

			if (Eqi(argv[i], L"/lh")) {
				action.reset(new ListHttpBinds);
				break;
			}

			if (Eqi(argv[i], L"/bc") && i < (argc - 2)) {
				action.reset(new BindCert(argv[i + 1], argv[i + 2]));
				break;
			}

//			if (Eqi(argv[i], L"/e") && i < (argc - 2)) {
//				action.reset(new ArcExtract(arc_lib, argv[i + 1], argv[i + 2]));
//				continue;
//			}
//
//			if (Eqi(argv[i], L"/a") && i < (argc - 3)) {
//				action.reset(new ArcCompress(arc_lib, argv[i + 1], argv[i + 2], argv[i + 3]));
//				continue;
//			}
		}
	}

	bool execute() const {
		return action->execute();
	}

private:
	auto_close<PWSTR*> argv;
	std::tr1::shared_ptr<Command_p> action;
	int argc;
};

int main() try {

	CmdParser(::GetCommandLineW()).execute();

	return 0;
} catch (WinError & e) {
	return e.format_error();
} catch (std::exception & e) {
	printf("std::exception [%s]:\n", typeid(e).name());
	printf("What: %s\n", e.what());
	return 1;
}
