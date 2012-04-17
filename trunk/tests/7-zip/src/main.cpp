﻿//============================================================================
// Name        : 7-zip.cpp
// Author      : Andrew Grechkin
// Version     :
// Copyright   : Your copyright notice
// Description : 7-zip example
//============================================================================

#include <libwin_com/7zip.h>
#include <libwin_net/exception.h>
#include <libwin_def/console.h>

using namespace SevenZip;

struct ArcInfo: public Command_p {
	ArcInfo(const Lib & lib) :
		arc_lib(lib) {
	}

	bool execute() const {
		printf(L"\nSupported methods (%Id):\n", arc_lib.methods().size());
		for (auto it = arc_lib.methods().begin(); it != arc_lib.methods().end(); ++it) {
			printf(L"%I64d\tname: %s\n", it->first, it->second->name.c_str());
		}

		printf(L"\nSupported archive formats (%Id):\n", arc_lib.codecs().size());
		for (auto it = arc_lib.codecs().begin(); it != arc_lib.codecs().end(); ++it) {
			printf(L"%s\tupd: %d\text: %s\t add_ext: %s\n",
			       it->first.c_str(),
			       it->second->updatable,
			       it->second->ext.c_str(),
			       it->second->add_ext.c_str());
			//		"\tguid: " it->second->guid.as_str();
		}
		printf(L"\n");
		return true;
	}

private:
	const Lib & arc_lib;
};

struct ArcCompress: public Command_p {
	ArcCompress(const Lib & lib, const ustring & path, const ustring & what, const ustring & codec) :
		arc_lib(lib),
		m_path(path),
		m_what(what),
		m_codec(codec) {
	}

	bool execute() const {
		ustring full_path(m_path + L"." + m_codec);
		if (!FS::is_exist(full_path)) {
			printf(L"\nCompressing:\n");
			CreateArchive arc(arc_lib, m_codec);
			arc.add(m_what);
			arc.silent = false;
//			arc.level = 5;
			if (m_codec == L"7z") {
				arc.password = L"7z";
				arc.encrypt_header = true;
				arc.solid = true;
				arc.level = 9;
			} else if (m_codec == L"zip") {
				arc.password = L"zip";
			}

			arc.compress(full_path);
		}
		return true;
	}

private:
	const Lib & arc_lib;
	ustring m_path;
	ustring m_what;
	ustring m_codec;
};

struct ArcList: public Command_p {
	ArcList(const Lib & lib, const ustring & path, bool full = false) :
		arc_lib(lib), m_path(path), m_full(full) {
	}

	bool execute() const {
		if (FS::is_exist(m_path)) {
			Archive archive(arc_lib, m_path);
			printf(L"\nArchive information: %s\tCodec: %s\n", m_path.c_str(), archive.codec().name.c_str());
			printf(L"Number of archive properties: %Id\n", archive.props().size());
			for (auto it = archive.props().begin(); it != archive.props().end(); ++it) {
				printf(L"'%s'\t%s %d\t'%s'\n",
				       it->name.c_str(),
				       NamedValues<int>::GetName(ArcItemPropsNames, sizeofa(ArcItemPropsNames), it->id),
				       it->id,
				       it->prop.as_str().c_str());
			}
			printf(L"\n");

			printf(L"\nListing(%Id):\n", archive.size());
			for (size_t i = 0; i < archive.size(); ++i) {
				auto it = archive.at(i);
				printf(L"%Id IsDir: %d\tPath: %s\tSize: %Id\n",
				       i,
				       it.is_dir(),
				       it.path().c_str(),
				       it.size());
				for (size_t j = 0; m_full && j < it.get_props_count(); ++j) {
					ustring name;
					PROPID id;
					if (it.get_prop_info(j, name, id)) {
						printf(L"'%s'\t%s %I64d\t%s\n",
						       name.c_str(),
						       NamedValues<int>::GetName(ArcItemPropsNames, sizeofa(ArcItemPropsNames), id),
						       id,
						       it.get_prop(id).as_str().c_str());
					}
				}
			}
		}
		return true;
	}

private:
	const Lib & arc_lib;
	ustring m_path;
	bool m_full;
};

struct ArcTest: public Command_p {
	ArcTest(const Lib & lib, const ustring & path) :
		arc_lib(lib), m_path(path) {
	}

	bool execute() const {
		if (FS::is_exist(m_path)) {
			Archive archive(arc_lib, m_path);

			printf(L"\nTesting:\n");
			printf(L"Errors: %Id\n", archive.test());
		}
		return true;
	}

private:
	const Lib & arc_lib;
	ustring m_path;
};

struct ArcExtract: public Command_p {
	ArcExtract(const Lib & lib, const ustring & path, const ustring & where) :
		arc_lib(lib), m_path(path), m_where(where) {
	}

	bool execute() const {
		if (FS::is_exist(m_path)) {
			Archive archive(arc_lib, m_path);

			printf(L"\nExtracting:\n");
			archive.extract(m_where);
		}
		return true;
	}

private:
	const Lib & arc_lib;
	ustring m_path;
	ustring m_where;
};

struct ShowHelp: public Command_p {
	ShowHelp(PCWSTR prg):
		m_prg(filename_only(prg)) {
	}

	bool execute() const {
		printf(L"Простой пример работы с 7-zip\nAndrew Grechkin, 2012\n");
		printf(L"\nИнфо:\n");
		printf(L"\t%s /i\n", m_prg.c_str());
		printf(L"Лист:\n");
		printf(L"\t%s /l arc.name\n", m_prg.c_str());
		printf(L"Тест:\n");
		printf(L"\t%s /t arc.name\n", m_prg.c_str());
		printf(L"Распаковка:\n");
		printf(L"\t%s /e arc.name dest.path\n", m_prg.c_str());
		printf(L"Запаковка:\n");
		printf(L"\t%s /a arc.name src.path tar\n", m_prg.c_str());
		return true;
	}
private:
	ustring m_prg;
};

struct CmdParser: public Command_p {
	CmdParser(PWSTR cmd_line, const Lib & arc_lib):
		argv(::CommandLineToArgvW(cmd_line, &argc), LocalFree),
		action(new ShowHelp(argv[0])) {
		for (int i = 1; i < argc; ++i) {
			if (Eqi(argv[i], L"/?")) {
				action.reset(new ShowHelp(argv[0]));
				break;
			}

			if (Eqi(argv[i], L"/i")) {
				action.reset(new ArcInfo(arc_lib));
				break;
			}

			if (Eqi(argv[i], L"/l") && i < (argc - 1)) {
				action.reset(new ArcList(arc_lib, argv[i + 1]));
				break;
			}

			if (Eqi(argv[i], L"/t") && i < (argc - 1)) {
				action.reset(new ArcTest(arc_lib, argv[i + 1]));
				break;
			}

			if (Eqi(argv[i], L"/e") && i < (argc - 2)) {
				action.reset(new ArcExtract(arc_lib, argv[i + 1], argv[i + 2]));
				continue;
			}

			if (Eqi(argv[i], L"/a") && i < (argc - 3)) {
				action.reset(new ArcCompress(arc_lib, argv[i + 1], argv[i + 2], argv[i + 3]));
				continue;
			}
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
	Lib arc_lib(L"7z.dll");
	printf(L"7-zip library version: %s\n", arc_lib.get_version().c_str());

	CmdParser(::GetCommandLineW(), arc_lib).execute();

	return 0;
} catch (WinError & e) {
	return e.format_error();
} catch (std::exception & e) {
	printf("std::exception [%s]:\n", typeid(e).name());
	printf("What: %s\n", e.what());
	return 1;
}
