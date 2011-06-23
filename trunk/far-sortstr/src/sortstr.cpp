/**
	sortstr: Sort strings in editor
	FAR2 plugin

	© 2011 Andrew Grechkin

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
**/

#include <libwin_def/std.h>
#include <libwin_def/reg.h>

#include <math.h>

#include <far/helper.h>

#include <tr1/functional>
#include <vector>
using std::vector;
using std::pair;

//#include <fstream>
//#include <iomanip>
//using namespace std;

PCWSTR const plug_name = L"sortstr";

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

typedef pair<AutoUTF, SelInfo> datapair;
typedef pair<AutoUTF, SortInfo> sortpair;
typedef vector<datapair> data_vector;
typedef vector<sortpair> sort_vector;

enum {
	DEL_NO = 0,
	DEL_BLOCK,
	DEL_SPARSE,
};

///======================================================================================= implement
PluginStartupInfo		psi;
FarStandardFunctions	fsf;

Register reg;

EditorInfo ei;
size_t	lineFirst;
int inv, cs, ns, sel, emp, op;
WCHAR	whsp[32] = L" ";

inline AutoUTF make_path(const AutoUTF &path, const AutoUTF &name) {
	return path + PATH_SEPARATOR + name;
}

bool is_whsp(WCHAR ch) {
	return Find(whsp, ch);
}

long double	FindNum(PCWSTR str) {
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
			if (!is_whsp(*l)) {
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

bool	PairEqCI(const sortpair &lhs, const sortpair &rhs) {
	return Cmpi(lhs.first.c_str(), rhs.first.c_str()) == 0;
}

bool	PairEqCS(const sortpair &lhs, const sortpair &rhs) {
	return Cmp(lhs.first.c_str(), rhs.first.c_str()) == 0;
}

bool	PairEqCScode(const sortpair &lhs, const sortpair &rhs) {
	return CmpCode(lhs.first.c_str(), rhs.first.c_str()) == 0;
}

bool	PairEqNum(const sortpair &lhs, const sortpair &rhs) {
	return lhs.second.num == rhs.second.num;
}

bool	PairLessLine(const sortpair &lhs, const sortpair &rhs) {
	return lhs.second.line < rhs.second.line;
}

bool	PairLessCI(const sortpair &lhs, const sortpair &rhs) {
	int ret = Cmpi(lhs.first.c_str(), rhs.first.c_str());
	if (ret < 0)
		return true;
	else if (ret == 0)
		return lhs.second.line < rhs.second.line;
	return false;
}

bool	PairLessCS(const sortpair &lhs, const sortpair &rhs) {
	int ret = Cmp(lhs.first.c_str(), rhs.first.c_str());
	if (ret < 0)
		return true;
	else if (ret == 0)
		return lhs.second.line < rhs.second.line;
	return false;
}

bool	PairLessCScode(const sortpair &lhs, const sortpair &rhs) {
	int ret = CmpCode(lhs.first.c_str(), rhs.first.c_str());
	if (ret < 0)
		return true;
	else if (ret == 0)
		return lhs.second.line < rhs.second.line;
	return false;
}

bool	PairLessNum(const sortpair &lhs, const sortpair &rhs) {
	if (lhs.second.num < rhs.second.num)
		return true;
	else if (lhs.second.num == rhs.second.num)
		return lhs.second.line < rhs.second.line;
	return false;
}

template <typename Type>
void	InsertFromVector(const data_vector &data, Type it, Type end) {
	size_t i = lineFirst, j = 0;
	for (; it != end; ++i, ++j) {
		if (data[j].second.count == -2 && !emp) {
			continue;
		}
		if (j == it->second.line) {
			++it;
			continue;
		}
		switch (op) {
			case DEL_NO:
				if (ei.BlockType == BTYPE_STREAM) {
					Editor::SetString(i, data[it->second.line].first);
				} else {
					if (sel) {
						if (data[j].first.size() <= (size_t)data[j].second.start) {
							AutoUTF tmp(data[j].second.start, SPACE_C);
							tmp.replace(0, data[j].first.size(), data[j].first);
							tmp += it->first;
							Editor::SetString(i, tmp);
						} else {
							AutoUTF tmp(data[j].first);
							tmp.replace(data[j].second.start, data[j].second.count, it->first);
							Editor::SetString(i, tmp);
						}
					} else {
						Editor::SetString(i, data[it->second.line].first);
					}
				}
				++it;
				break;
			case DEL_BLOCK:
				Editor::SetString(i, data[it->second.line].first);
				++it;
				break;
			case DEL_SPARSE: {
				if (!data[j].first.empty()) {
					Editor::SetString(i, EMPTY_STR);
				}
				break;
			}
		}
	}
	switch (op) {
		case DEL_BLOCK:
			for (; j < data.size(); ++i, ++j)
				Editor::SetString(i, EMPTY_STR);
			break;
		case DEL_SPARSE: {
			for (; j < data.size(); ++i, ++j)
				if (!data[j].first.empty())
					Editor::SetString(i, EMPTY_STR);
		}
	}
}

bool	ProcessEditor() {
	if (ei.BlockType == BTYPE_STREAM || ei.BlockType == BTYPE_COLUMN)
		lineFirst = ei.BlockStartLine;
	else
		lineFirst = 0;

	data_vector	data;
	sort_vector	sortdata;
//	data.reserve(ei.TotalLines - lineFirst);
//	sortdata.reserve(data.capacity());

//	ofstream file("sel.log");
	for (size_t i = lineFirst; i < (size_t)ei.TotalLines; ++i) {
		static EditorGetString	egs;
		egs.StringNumber = i;
		psi.EditorControl(ECTL_GETSTRING, &egs);

		if (i == (size_t)(ei.TotalLines - 1) && Empty(egs.StringText))
			break;
		if (ei.BlockType != BTYPE_NONE && (egs.SelStart == -1 || egs.SelStart == egs.SelEnd))
			break;

		AutoUTF	tmp(egs.StringText, egs.StringLength);

		ssize_t	SelLen = -2;
		switch (ei.BlockType) {
			case BTYPE_COLUMN: {
				if (egs.SelStart < egs.StringLength) {
					SelLen = std::min(egs.SelEnd, egs.StringLength) - std::min(egs.SelStart, egs.StringLength);
				}
				if (SelLen != -2) {
					sortdata.push_back(sortpair(AutoUTF(egs.StringText + egs.SelStart, SelLen), i - lineFirst));
					if (ns) {
						sortdata.back().second.num = FindNum(sortdata.back().first.c_str());
					}
				} else if (emp) {
					sortdata.push_back(sortpair(AutoUTF(), i - lineFirst));
				}
				data.push_back(data_vector::value_type(tmp, SelInfo(egs.SelStart, SelLen)));
				break;
			}
			case BTYPE_STREAM:
			default: {
				data.push_back(data_vector::value_type(tmp, SelInfo(0, egs.StringLength)));
				sortdata.push_back(sortpair(tmp, i - lineFirst));
				if (ns) {
					sortdata.back().second.num = FindNum(sortdata.back().first.c_str());
				}
			}
		}
//		file << "line: " << i << " sta: " << data[i].second.start << " cnt: " << data[i].second.count << endl;
	}

//	ofstream file1("sortdata1.log");
//	for (sort_vector::iterator it = sortdata.begin(); it != sortdata.end(); ++it) {
//		file1 << "line: " << it->second.line << " num: " << setprecision(16) << it->second.num << " str: '" << oem(it->first).c_str() << "'" << endl;
//	}

	std::pointer_to_binary_function<const sortpair&, const sortpair&, bool>
	pfLe((ns) ? ptr_fun(PairLessNum) : ptr_fun(PairLessCScode)),
	pfEq((ns) ? ptr_fun(PairEqNum) : ptr_fun(PairEqCScode));

	switch (cs) {
		case 0:
			pfLe = (ns) ? ptr_fun(PairLessNum) : ptr_fun(PairLessCI);
			pfEq = (ns) ? ptr_fun(PairEqNum) : ptr_fun(PairEqCI);
			break;
		case 1:
			pfLe = (ns) ? ptr_fun(PairLessNum) : ptr_fun(PairLessCS);
			pfEq = (ns) ? ptr_fun(PairEqNum) : ptr_fun(PairEqCS);
			break;
	}

	std::sort(sortdata.begin(), sortdata.end(), pfLe);

	if (op) {
		sort_vector::iterator it = std::unique(sortdata.begin(), sortdata.end(), pfEq);
		sortdata.erase(it, sortdata.end());
		std::sort(sortdata.begin(), sortdata.end(), ptr_fun(PairLessLine));
	}

//	ofstream file4("sortdata4.log");
//	for (sort_vector::iterator it = sortdata.begin(); it != sortdata.end(); ++it) {
//		file4 << "line: " << it->second.line << " num: " << setprecision(16) << it->second.num << " str: '" << oem(it->first).c_str() << "'" << endl;
//	}

	Editor::StartUndo();
	if (inv && !op) {
		InsertFromVector(data, sortdata.rbegin(), sortdata.rend());
	} else {
		InsertFromVector(data, sortdata.begin(), sortdata.end());
	}
	Editor::StopUndo();

	Editor::Redraw();

	return true;
}

///========================================================================================== Export
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

void WINAPI		EXP_NAME(GetPluginInfo)(PluginInfo *pi) {
	pi->StructSize = sizeof(PluginInfo);
	pi->Flags = PF_DISABLEPANELS | PF_EDITOR;
	static PCWSTR	PluginMenuStrings[1];
	PluginMenuStrings[0] = GetMsg(MenuTitle);
	pi->PluginMenuStrings = PluginMenuStrings;
	pi->PluginMenuStringsNumber = 1;
}

HANDLE WINAPI	EXP_NAME(OpenPlugin)(int /*OpenFrom*/, INT_PTR /*Item*/) {
	static FarListItem litems[] = {
		{0, GetMsg(lbSort), {0}},
		{0, GetMsg(lbDelBlock), {0}},
		{0, GetMsg(lbDelSparse), {0}},
	};
	static FarList flist = {sizeofa(litems), litems};

	enum {
		HEIGHT = 15,
		WIDTH = 60,

		indInv = 1,
		indCS,
		indNS,
		indWhsp = 5,
		indSelected = 7,
		indAsEmpty,
		indList = 11,
	};
	static InitDialogItemF Items[] = {
		{DI_DOUBLEBOX, 3,  1,  WIDTH - 4, HEIGHT - 2, 0, (PCWSTR)DlgTitle},
		{DI_CHECKBOX,  5, 2, 0,  0,                   0, (PCWSTR)cbInvert},
		{DI_CHECKBOX,  5, 3, 0,  0,                   DIF_3STATE, (PCWSTR)cbSensitive},
		{DI_CHECKBOX,  5, 4, 30, 0,                   0, (PCWSTR)cbNumeric},
		{DI_TEXT,      34, 4, WIDTH - 10,0,           0, (PCWSTR)txWhitespace},
		{DI_EDIT,      WIDTH - 8, 4, WIDTH - 5, 0,    0, whsp},
		{DI_TEXT,      0, 5, 0,  0,                   DIF_SEPARATOR,   EMPTY_STR},
		{DI_CHECKBOX,  5, 6, 0,  0,                   0, (PCWSTR)cbSelected},
		{DI_CHECKBOX,  5, 7, 0,  0,                   0, (PCWSTR)cbAsEmpty},
		{DI_TEXT,      0, HEIGHT - 7, 0,  0,          DIF_SEPARATOR,   EMPTY_STR},
		{DI_TEXT,      5, HEIGHT - 6, 0,  0,          0, (PCWSTR)txOperation},
		{DI_COMBOBOX,  5, HEIGHT - 5, 54,  0,         DIF_DROPDOWNLIST | DIF_LISTNOAMPERSAND, (PCWSTR)lbSort},
		{DI_TEXT,      0,  HEIGHT - 4, 0,  0,         DIF_SEPARATOR,   EMPTY_STR},
		{DI_BUTTON,    0,  HEIGHT - 3, 0,  0,         DIF_CENTERGROUP, (PCWSTR)txtBtnOk},
		{DI_BUTTON,    0,  HEIGHT - 3, 0,  0,         DIF_CENTERGROUP, (PCWSTR)txtBtnCancel},
	};
	static size_t size = sizeofa(Items);

	psi.EditorControl(ECTL_GETINFO, &ei);
	FarDialogItem	FarItems[size];
	InitDialogItemsF(Items, FarItems, size);
	FarItems[size - 2].DefaultButton = 1;
	FarItems[indInv].Selected = inv;
	FarItems[indCS].Selected = cs;
	FarItems[indNS].Selected = ns;
	FarItems[indWhsp].MaxLen = sizeofa(whsp) - 1;
	if (ei.BlockType != BTYPE_COLUMN) {
		FarItems[indSelected].Flags |= DIF_DISABLE;
		FarItems[indAsEmpty].Flags |= DIF_DISABLE;
	}
	FarItems[indSelected].Selected = sel;
	FarItems[indAsEmpty].Selected = emp;
	FarItems[indList].ListItems = &flist;

	FarDlg hDlg;
	if (hDlg.Init(psi.ModuleNumber, -1, -1, WIDTH, HEIGHT, nullptr, FarItems, size)) {
		int	ret = hDlg.Run();
		if (ret > 0 && Items[ret].Data == (PCWSTR)txtBtnOk) {
			inv = hDlg.Check(indInv);
			cs = hDlg.Check(indCS);
			ns = hDlg.Check(indNS);
			sel = hDlg.Check(indSelected);
			emp = hDlg.Check(indAsEmpty);
			op = psi.SendDlgMessage(hDlg, DM_LISTGETCURPOS, indList, nullptr);
			Copy(whsp, hDlg.Str(indWhsp), sizeofa(whsp) - 1);

			reg.Set(L"invert", inv);
			reg.Set(L"case", cs);
			reg.Set(L"numeric", ns);
			reg.Set(L"selection", sel);
			reg.Set(L"asempty", emp);
			reg.Set(L"whitespace", whsp);

			ProcessEditor();
		}
	}
	return INVALID_HANDLE_VALUE;
}

void WINAPI		EXP_NAME(SetStartupInfo)(const PluginStartupInfo *psi) {
	InitFSF(psi);

	reg.Open(KEY_READ | KEY_WRITE, make_path(psi->RootKey, plug_name).c_str());

	reg.Get(L"invert", inv, 0);
	reg.Get(L"case", cs, 0);
	reg.Get(L"numeric", ns, 0);
	reg.Get(L"selection", sel, 0);
	reg.Get(L"asempty", emp, 0);
	reg.GetStr(L"whitespace", whsp, sizeof(whsp)); // size in bytes
	op = 0;
}
