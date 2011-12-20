//============================================================================
// Name        : 7-zip.cpp
// Author      : Andrew Grechkin
// Version     :
// Copyright   : Your copyright notice
// Description : 7-zip example
//============================================================================

#include <libwin_com/7zip.h>
#include <libwin_net/exception.h>
#include <libwin_net/wcout.h>

using winstd::wcout;
using winstd::wendl;
using namespace SevenZip;

struct ArcInfo: public CommandPattern {
	ArcInfo(const Lib & lib) :
		arc_lib(lib) {
	}

	bool Execute() const {
		wcout << "\nSupported methods (" << arc_lib.methods().size() << "): " << wendl;
		for (Methods::iterator it = arc_lib.methods().begin(); it != arc_lib.methods().end(); ++it) {
			wcout << it->second->id;
			wcout << "\tname: " << it->second->name;
			wcout << wendl;
		}

		wcout << "\nSupported archive formats (" << arc_lib.codecs().size() << "): " << wendl;
		for (Codecs::iterator it = arc_lib.codecs().begin(); it != arc_lib.codecs().end(); ++it) {
			wcout << it->first;
			wcout << "\tupd: " << it->second->updatable;
			//		wcout << "\tguid: " << it->second->guid.as_str();
			wcout << "\t ext: " << it->second->ext;
			wcout << "\t add_ext: " << it->second->add_ext << wendl;
		}
		wcout << wendl;
		return true;
	}

private:
	const Lib & arc_lib;
};

struct ArcCompress: public CommandPattern {
	ArcCompress(const Lib & lib, const ustring & path, const ustring & what, const ustring & codec) :
		arc_lib(lib), m_path(path), m_what(what), m_codec(codec) {
	}

	bool Execute() const {
		if (!FS::is_exist(m_path + L"." + m_codec)) {
			wcout << L"\nCompressing: " << wendl;
			CreateArchive arc(arc_lib, m_path, m_codec);
			arc.add(m_what);
			arc.level = 5;
			arc.compress();

			//			DirStructure items(L"C:\\Temp\\pthreads");
			//			DirStructure items(L"C:\\Temp\\Pontifex");

			//			for (DirStructure::iterator it = items.begin(); it != items.end(); ++it) {
			//				wcout << L"path: " << it->path << L" name: " << it->name << wendl;
			//				wcout << L"\tis_dir: " << it->is_dir_or_link() << L" size: " << it->size() << wendl;
			//			}

			//			for (size_t i = 0; i < updateCallbackSpec->FailedFiles.size(); i++) {
			//				printf(L"\n");
			//				printf(L"Error for file: %s", updateCallbackSpec->FailedFiles[i].c_str());
			//			}
			//			if (updateCallbackSpec->FailedFiles.size() != 0)
			//				return 1;
		}
		return true;
	}

private:
	const Lib & arc_lib;
	ustring m_path;
	ustring m_what;
	ustring m_codec;
};

struct ArcList: public CommandPattern {
	ArcList(const Lib & lib, const ustring & path, bool full = false) :
		arc_lib(lib), m_path(path), m_full(full) {
	}

	bool Execute() const {
		if (FS::is_exist(m_path)) {
			Archive archive(arc_lib, m_path);
			wcout << L"\nArchive information: " << m_path << L"\tCodec: " << archive.codec().name << wendl;
			wcout << L"Number of archive properties: " << archive.props().size() << wendl;
			for (Props::iterator it = archive.props().begin(); it != archive.props().end(); ++it) {
				wcout << L"'" << it->name << L"'\t" << NamedValues<int>::GetName(ArcItemPropsNames, sizeofa(ArcItemPropsNames), it->id) << L"  " << it->id;
				wcout << "\t" << it->prop << wendl;
			}
			wcout << wendl;

			wcout << L"\nListing(" << archive.size() << "): " << wendl;
			for (size_t i = 0; i < archive.size(); ++i) {
				Archive::iterator it(archive[i]);
				wcout << i << L" IsDir: " << it.is_dir() << L"\tPath: " << it.path() << L"\tSize: " << it.size() << wendl;
				for (size_t j = 0; m_full && j < it.get_props_count(); ++j) {
					ustring name;
					PROPID id;
					if (it.get_prop_info(j, name, id)) {
						wcout << L"'" << name << L"'\t" << NamedValues<int>::GetName(ArcItemPropsNames, sizeofa(ArcItemPropsNames), id) << L"  " << id;
						PropVariant prop = it.get_prop(id);
						wcout << "\t" << prop << wendl;
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

struct ArcTest: public CommandPattern {
	ArcTest(const Lib & lib, const ustring & path) :
		arc_lib(lib), m_path(path) {
	}

	bool Execute() const {
		if (FS::is_exist(m_path)) {
			Archive archive(arc_lib, m_path);

			wcout << L"\nTesting:" << wendl;
			wcout << L"Errors: " << archive.test() << wendl;
		}
		return true;
	}

private:
	const Lib & arc_lib;
	ustring m_path;
};

struct ArcExtract: public CommandPattern {
	ArcExtract(const Lib & lib, const ustring & path, const ustring & where) :
		arc_lib(lib), m_path(path), m_where(where) {
	}

	bool Execute() const {
		if (FS::is_exist(m_path)) {
			Archive archive(arc_lib, m_path);

			wcout << L"\nExtracting:" << wendl;
			archive.extract(m_where);
		}
		return true;
	}

private:
	const Lib & arc_lib;
	ustring m_path;
	ustring m_where;
};

struct ArcShowHelp: public CommandPattern {
	ArcShowHelp() {
	}

	bool Execute() const {
		printf(L"Простой пример работы с 7-zip\nAndrew Grechkin, 2011\n");
		printf(L"\nИнфо:\n");
		printf(L"\t7-zip.exe /i\n");
		printf(L"Лист:\n");
		printf(L"\t7-zip.exe /l arc.name\n");
		printf(L"Тест:\n");
		printf(L"\t7-zip.exe /t arc.name\n");
		printf(L"Распаковка:\n");
		printf(L"\t7-zip.exe /e arc.name dest.path\n");
		printf(L"Запаковка:\n");
		printf(L"\t7-zip.exe /a arc.name src.path tar\n");
		return true;
	}
};

void parse_command_line(size_t argc, PWSTR argv[], const Lib & arc_lib) {
	std::tr1::shared_ptr<CommandPattern> action(new ArcShowHelp);
	for (size_t i = 1; i < argc; ++i) {
		if (Eqi(argv[i], L"/?")) {
			action.reset(new ArcShowHelp);
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
	action->Execute();
}

int main() try {
	Lib arc_lib(L"7z.dll");
	wcout << L"7-zip library version: " << arc_lib.get_version() << wendl;

	int argc = 0;
	PWSTR * argv = ::CommandLineToArgvW(::GetCommandLineW(), &argc);
	parse_command_line(argc, argv, arc_lib);
	::LocalFree(argv);

	return 0;
} catch (WinError & e) {
	printf(L"Error: %s\n", e.what().c_str());
	printf(L"Where: %s\n", e.where().c_str());
	return e.code();
} catch (std::exception & e) {
	printf(L"What: %s\n", e.what());
	return 1;
}
