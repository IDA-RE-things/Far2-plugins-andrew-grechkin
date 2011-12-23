//============================================================================
// Name        : 7-zip.cpp
// Author      : Andrew Grechkin
// Version     :
// Copyright   : Your copyright notice
// Description : 7-zip example
//============================================================================

#include <libwin_com/crypt.h>
#include <libwin_net/exception.h>

struct ListCert: public CommandPattern {
	ListCert(PCWSTR st):
		m_st(st) {
	}

	bool Execute() const {
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

struct ShowHelp: public CommandPattern {
	ShowHelp(PCWSTR prg):
		m_prg(only_file(prg)) {
	}

	bool Execute() const {
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

struct CmdParser: public CommandPattern {
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
			if (Eqi(argv[i], L"/cl") && i < (argc - 1)) {
				action.reset(new ListCert(argv[i + 1]));
				break;
			}

//			if (Eqi(argv[i], L"/t") && i < (argc - 1)) {
//				action.reset(new ArcTest(arc_lib, argv[i + 1]));
//				break;
//			}
//
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

	bool Execute() const {
		return action->Execute();
	}

private:
	auto_close<PWSTR*> argv;
	std::tr1::shared_ptr<CommandPattern> action;
	int argc;
};

int main() try {

	CmdParser(::GetCommandLineW()).Execute();

	return 0;
} catch (WinError & e) {
	return e.format_error();
} catch (std::exception & e) {
	printf("std::exception [%s]:\n", typeid(e).name());
	printf("What: %s\n", e.what());
	return 1;
}
