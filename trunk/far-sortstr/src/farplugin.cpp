#include "farplugin.hpp"

#ifndef FAR2
#include "guid.hpp"
#endif

#include <tr1/functional>
#include <cmath>
#include <vector>

winstd::shared_ptr<FarPlugin> plugin;

struct SelInfo {
	ssize_t start;
	ssize_t count;
	SelInfo(ssize_t s = -1, ssize_t c = -1):
		start(s),
		count(c) {
	};
};

struct SortInfo {
	size_t line;
	long double num;
	SortInfo(size_t l):
		line(l),
		num(0) {
	};
};

typedef std::pair<ustring, SelInfo> datapair;
typedef std::pair<ustring, SortInfo> sortpair;
typedef std::vector<datapair> data_vector;
typedef std::vector<sortpair> sort_vector;

enum {
	DEL_NO = 0,
	DEL_BLOCK,
	DEL_SPARSE,
};

enum {
	cbInvert = 5,
	cbSensitive,
	cbNumeric,
	txWhitespace,
	cbSelected,
	cbAsEmpty,
	txOperation,
	lbSort,
	lbDelBlock,
	lbDelSparse,
};

inline ustring make_path(const ustring & path, const ustring & name) {
	return path + PATH_SEPARATOR + name;
}

bool PairEqCI(const sortpair & lhs, const sortpair & rhs) {
	return Cmpi(lhs.first.c_str(), rhs.first.c_str()) == 0;
}

bool PairEqCS(const sortpair & lhs, const sortpair & rhs) {
	return Cmp(lhs.first.c_str(), rhs.first.c_str()) == 0;
}

bool PairEqCScode(const sortpair & lhs, const sortpair & rhs) {
	return CmpCode(lhs.first.c_str(), rhs.first.c_str()) == 0;
}

bool PairEqNum(const sortpair & lhs, const sortpair & rhs) {
	return lhs.second.num == rhs.second.num;
}

bool PairLessLine(const sortpair & lhs, const sortpair & rhs) {
	return lhs.second.line < rhs.second.line;
}

bool PairLessCI(const sortpair & lhs, const sortpair & rhs) {
	int ret = Cmpi(lhs.first.c_str(), rhs.first.c_str());
	if (ret < 0)
		return true;
	else if (ret == 0)
		return lhs.second.line < rhs.second.line;
	return false;
}

bool PairLessCS(const sortpair & lhs, const sortpair & rhs) {
	int ret = Cmp(lhs.first.c_str(), rhs.first.c_str());
	if (ret < 0)
		return true;
	else if (ret == 0)
		return lhs.second.line < rhs.second.line;
	return false;
}

bool PairLessCScode(const sortpair & lhs, const sortpair & rhs) {
	int ret = CmpCode(lhs.first.c_str(), rhs.first.c_str());
	if (ret < 0)
		return true;
	else if (ret == 0)
		return lhs.second.line < rhs.second.line;
	return false;
}

bool PairLessNum(const sortpair & lhs, const sortpair & rhs) {
	if (lhs.second.num < rhs.second.num)
		return true;
	else if (lhs.second.num == rhs.second.num)
		return lhs.second.line < rhs.second.line;
	return false;
}

template <typename Type>
void InsertFromVector(const data_vector & data, Type it, Type end, const Options & options) {
	size_t i = options.get_first_line(), j = 0;
	for (; it != end; ++i, ++j) {
		if (data[j].second.count == -2 && !options.emp) {
			continue;
		}
		if (j == it->second.line) {
			++it;
			continue;
		}
		switch (options.op) {
			case DEL_NO:
				if (options.get_block_type() == BTYPE_STREAM) {
					Far::Editor::set_string(i, data[it->second.line].first, nullptr);
				} else {
					if (options.sel) {
						if (data[j].first.size() <= (size_t)data[j].second.start) {
							ustring tmp(data[j].second.start, SPACE_C);
							tmp.replace(0, data[j].first.size(), data[j].first);
							tmp += it->first;
							Far::Editor::set_string(i, tmp, nullptr);
						} else {
							ustring tmp(data[j].first);
							tmp.replace(data[j].second.start, data[j].second.count, it->first);
							Far::Editor::set_string(i, tmp, nullptr);
						}
					} else {
						Far::Editor::set_string(i, data[it->second.line].first, nullptr);
					}
				}
				++it;
				break;
			case DEL_BLOCK:
				Far::Editor::set_string(i, data[it->second.line].first, nullptr);
				++it;
				break;
			case DEL_SPARSE: {
				if (!data[j].first.empty()) {
					Far::Editor::set_string(i, EMPTY_STR, nullptr);
				}
				break;
			}
		}
	}
	switch (options.op) {
		case DEL_BLOCK:
			for (; j < data.size(); ++i, ++j)
				Far::Editor::set_string(i, EMPTY_STR, nullptr);
			break;
		case DEL_SPARSE: {
			for (; j < data.size(); ++i, ++j)
				if (!data[j].first.empty())
					Far::Editor::set_string(i, EMPTY_STR, nullptr);
		}
		break;
	}
}

bool FarPlugin::Execute() const {
	using namespace Far;
	if (!options.get_total_lines())
		return false;

	data_vector	data;
	sort_vector	sortdata;
//	data.reserve(ei.TotalLines - lineFirst);
//	sortdata.reserve(data.capacity());

//	ofstream file("sel.log");
	static EditorGetString	egs;
	for (size_t i = options.get_first_line(); i < options.get_total_lines(); ++i) {
		Editor::get_string(i, egs);

		if (i == options.get_total_lines() - 1) {
			if (Empty(egs.StringText))
				break;
		}
		if (options.get_block_type() != BTYPE_NONE && (egs.SelStart == -1 || egs.SelStart == egs.SelEnd))
			break;

		ustring	tmp(egs.StringText, egs.StringLength);

		ssize_t	SelLen = -2;
		switch (options.get_block_type()) {
			case BTYPE_COLUMN: {
				if (egs.SelStart < egs.StringLength) {
					SelLen = std::min(egs.SelEnd, egs.StringLength) - std::min(egs.SelStart, egs.StringLength);
				}
				if (SelLen != -2) {
					sortdata.push_back(sortpair(ustring(egs.StringText + egs.SelStart, SelLen), i - options.get_first_line()));
					if (options.ns) {
						sortdata.back().second.num = FindNum(sortdata.back().first.c_str());
					}
				} else if (options.emp) {
					sortdata.push_back(sortpair(ustring(), i - options.get_first_line()));
				}
				data.push_back(data_vector::value_type(tmp, SelInfo(egs.SelStart, SelLen)));
				break;
			}
			case BTYPE_STREAM:
			default: {
				data.push_back(data_vector::value_type(tmp, SelInfo(0, egs.StringLength)));
				sortdata.push_back(sortpair(tmp, i - options.get_first_line()));
				if (options.ns) {
					sortdata.back().second.num = FindNum(sortdata.back().first.c_str());
				}
			}
			break;
		}
//		file << "line: " << i << " sta: " << data[i].second.start << " cnt: " << data[i].second.count << endl;
	}

//	ofstream file1("sortdata1.log");
//	for (sort_vector::iterator it = sortdata.begin(); it != sortdata.end(); ++it) {
//		file1 << "line: " << it->second.line << " num: " << setprecision(16) << it->second.num << " str: '" << oem(it->first).c_str() << "'" << endl;
//	}

	std::pointer_to_binary_function<const sortpair &, const sortpair &, bool>
	pfLe(options.ns ? ptr_fun(PairLessNum) : ptr_fun(PairLessCScode)),
	pfEq(options.ns ? ptr_fun(PairEqNum) : ptr_fun(PairEqCScode));

	switch (options.cs) {
		case 0:
			pfLe = options.ns ? ptr_fun(PairLessNum) : ptr_fun(PairLessCI);
			pfEq = options.ns ? ptr_fun(PairEqNum) : ptr_fun(PairEqCI);
			break;
		case 1:
			pfLe = options.ns ? ptr_fun(PairLessNum) : ptr_fun(PairLessCS);
			pfEq = options.ns ? ptr_fun(PairEqNum) : ptr_fun(PairEqCS);
			break;
	}

	std::sort(sortdata.begin(), sortdata.end(), pfLe);

	if (options.op) {
		sort_vector::iterator it = std::unique(sortdata.begin(), sortdata.end(), pfEq);
		sortdata.erase(it, sortdata.end());
		std::sort(sortdata.begin(), sortdata.end(), ptr_fun(PairLessLine));
	}

//	ofstream file4("sortdata4.log");
//	for (sort_vector::iterator it = sortdata.begin(); it != sortdata.end(); ++it) {
//		file4 << "line: " << it->second.line << " num: " << setprecision(16) << it->second.num << " str: '" << oem(it->first).c_str() << "'" << endl;
//	}

	Far::Editor::start_undo();
	if (options.inv && !options.op) {
		InsertFromVector(data, sortdata.rbegin(), sortdata.rend(), options);
	} else {
		InsertFromVector(data, sortdata.begin(), sortdata.end(), options);
	}
	Far::Editor::stop_undo();

	Far::Editor::redraw();

	return true;
}

#ifndef FAR2
GUID FarPlugin::get_guid() {
	return PluginGuid;
}
#endif

PCWSTR FarPlugin::get_name() {
	return L"sortstr";
}

PCWSTR FarPlugin::get_description() {
	return L"Sort strings in editor. FAR2, FAR3 plugin";
}

PCWSTR FarPlugin::get_author() {
	return L"© 2012 Andrew Grechkin";
}

FarPlugin::FarPlugin(const PluginStartupInfo * psi) {
#ifndef FAR2
	Far::helper_t::inst().init(PluginGuid, psi);
	options.load();
#else
	Far::helper_t::inst().init(psi);
	options.load(make_path(psi->RootKey, plugin->get_name()));
#endif
}

void FarPlugin::get_info(PluginInfo * pi) const {
	pi->StructSize = sizeof(PluginInfo);
	pi->Flags = PF_DISABLEPANELS | PF_EDITOR;
	static PCWSTR PluginMenuStrings[1];
	PluginMenuStrings[0] = Far::get_msg(Far::MenuTitle);
#ifndef FAR2
	pi->PluginMenu.Guids = &MenuGuid;
	pi->PluginMenu.Strings = PluginMenuStrings;
	pi->PluginMenu.Count = lengthof(PluginMenuStrings);
#else
	pi->PluginMenuStrings = PluginMenuStrings;
	pi->PluginMenuStringsNumber = 1;
#endif
}

#ifndef FAR2
HANDLE FarPlugin::open(const OpenInfo * /*Info*/)
#else
HANDLE FarPlugin::open(int /*OpenFrom*/, INT_PTR /*Item*/)
#endif
{	static FarListItem litems[] = {
		{0, Far::get_msg(lbSort), {0}},
		{0, Far::get_msg(lbDelBlock), {0}},
		{0, Far::get_msg(lbDelSparse), {0}},
	};
	static FarList flist = {sizeofa(litems), litems};

	static Far::InitDialogItemF Items[] = {
		{DI_DOUBLEBOX, 3,  1,  WIDTH - 4, HEIGHT - 2, 0, (PCWSTR)Far::DlgTitle},
		{DI_CHECKBOX,  5, 2, 0,  0,                   0, (PCWSTR)cbInvert},
		{DI_CHECKBOX,  5, 3, 0,  0,                   DIF_3STATE, (PCWSTR)cbSensitive},
		{DI_CHECKBOX,  5, 4, 30, 0,                   0, (PCWSTR)cbNumeric},
		{DI_TEXT,      34, 4, WIDTH - 10,0,           0, (PCWSTR)txWhitespace},
		{DI_EDIT,      WIDTH - 8, 4, WIDTH - 5, 0,    0, options.get_whitespaces().c_str()},
		{DI_TEXT,      0, 5, 0,  0,                   DIF_SEPARATOR,   EMPTY_STR},
		{DI_CHECKBOX,  5, 6, 0,  0,                   0, (PCWSTR)cbSelected},
		{DI_CHECKBOX,  5, 7, 0,  0,                   0, (PCWSTR)cbAsEmpty},
		{DI_TEXT,      0, HEIGHT - 7, 0,  0,          DIF_SEPARATOR,   EMPTY_STR},
		{DI_TEXT,      5, HEIGHT - 6, 0,  0,          0, (PCWSTR)txOperation},
		{DI_COMBOBOX,  5, HEIGHT - 5, 54,  0,         DIF_DROPDOWNLIST | DIF_LISTNOAMPERSAND, (PCWSTR)lbSort},
		{DI_TEXT,      0,  HEIGHT - 4, 0,  0,         DIF_SEPARATOR,   EMPTY_STR},
		{DI_BUTTON,    0,  HEIGHT - 3, 0,  0,         DIF_CENTERGROUP, (PCWSTR)Far::txtBtnOk},
		{DI_BUTTON,    0,  HEIGHT - 3, 0,  0,         DIF_CENTERGROUP, (PCWSTR)Far::txtBtnCancel},
	};
	static size_t size = lengthof(Items);

	options.load_editor_info();
	FarDialogItem	FarItems[size];
	InitDialogItemsF(Items, FarItems, size);
	FarItems[indInv].Selected = options.inv;
	FarItems[indCS].Selected = options.cs;
	FarItems[indNS].Selected = options.ns;
#ifndef FAR2
#else
	FarItems[size - 2].DefaultButton = 1;
	FarItems[indWhsp].MaxLen = options.WHITESPACES_LEN;
#endif
	if (options.get_block_type() != BTYPE_COLUMN) {
		FarItems[indSelected].Flags |= DIF_DISABLE;
		FarItems[indAsEmpty].Flags |= DIF_DISABLE;
	}
	FarItems[indSelected].Selected = options.sel;
	FarItems[indAsEmpty].Selected = options.emp;
	FarItems[indList].ListItems = &flist;

	Far::Dialog hDlg;
#ifndef FAR2
	if (hDlg.Init(DialogGuid, -1, -1, WIDTH, HEIGHT, nullptr, FarItems, size))
#else
	if (hDlg.Init(Far::psi().ModuleNumber, -1, -1, WIDTH, HEIGHT, nullptr, FarItems, size))
#endif
	{
		int	ret = hDlg.Run();
		if (ret > 0 && Items[ret].Data == (PCWSTR)Far::txtBtnOk) {
			options.get_parameters(hDlg);
			options.save();
			Execute();
		}
	}
	return INVALID_HANDLE_VALUE;
}

long double FarPlugin::FindNum(PCWSTR str) const {
	long double ret = HUGE_VAL;
	PCWSTR num = (PCWSTR)(str + ::wcscspn(str, L"0123456789"));

	if (*num) {
		WCHAR buf[132]; buf[sizeofa(buf) - 1] = 0;
		Copy(buf, num, sizeofa(buf) - 1);
		for (PWSTR k = buf; *k; ++k) {
			if (*k == L',') {
				*k = L'.';
				break;
			}
		}
		PWSTR k = buf;
		for (PWSTR l = buf; *k && *l ; ++l) {
			if (!options.is_whitespace(*l)) {
				*k++ = *l;
			}
		}
		*k = 0;
		ret = wcstold(buf, nullptr);
		if (num > str && num[-1] == L'-') {
			ret = -ret;
		}
	}
	return ret;
}
