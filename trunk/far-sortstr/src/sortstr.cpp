/**
	sortstr: Sort strings in editor
	FAR2 plugin

	© 2010 Andrew Grechkin

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

#include <win_std.h>

#include <far/helper.hpp>

#include <vector>
using std::vector;
using std::pair;

//#include <fstream>
//using namespace std;

struct SelInfo {
	intmax_t start;
	intmax_t count;
	SelInfo(intmax_t s = -1, intmax_t c = -1): start(s), count(c) {
	};
};

typedef pair<AutoUTF, SelInfo> datapair;
typedef pair<AutoUTF, size_t> sortpair;
typedef vector<datapair>	data_vector;
typedef vector<sortpair>	sort_vector;

enum {
	DEL_NO = 0,
	DEL_BLOCK,
	DEL_SPARSE,
};

enum {
	cbSelected = 5,
	cbInvert,
	cbSensitive,
	txOperation,
	lbSort,
	lbDelBlock,
	lbDelSparse,
};

///======================================================================================= implement
PluginStartupInfo		psi;
FarStandardFunctions	fsf;

EditorInfo ei;
size_t	lineFirst;

bool sel;
int inv, cs, op;

bool	PairEqCI(const sortpair &lhs, const sortpair &rhs) {
	return	Cmpi(lhs.first.c_str(), rhs.first.c_str()) == 0;
}

bool	PairEqCS(const sortpair &lhs, const sortpair &rhs) {
	return	Cmp(lhs.first.c_str(), rhs.first.c_str()) == 0;
}

bool	PairEqCScode(const sortpair &lhs, const sortpair &rhs) {
	return	CmpCode(lhs.first.c_str(), rhs.first.c_str()) == 0;
}

bool	PairLessNum(const sortpair &lhs, const sortpair &rhs) {
	return	lhs.second < rhs.second;
}

bool	PairLessCI(const sortpair &lhs, const sortpair &rhs) {
	int ret = Cmpi(lhs.first.c_str(), rhs.first.c_str());
	if (ret < 0)
		return	true;
	else if (ret == 0)
		return lhs.second < rhs.second;
	return false;
}

bool	PairLessCS(const sortpair &lhs, const sortpair &rhs) {
	int ret = Cmp(lhs.first.c_str(), rhs.first.c_str());
	if (ret < 0)
		return	true;
	else if (ret == 0)
		return lhs.second < rhs.second;
	return false;
}

bool	PairLessCScode(const sortpair &lhs, const sortpair &rhs) {
	int ret = CmpCode(lhs.first.c_str(), rhs.first.c_str());
	if (ret < 0)
		return	true;
	else if (ret == 0)
		return lhs.second < rhs.second;
	return false;
}

template <typename Type>
void	InsertFromVector(const data_vector &data, Type it, Type end) {
	size_t i = lineFirst, j = 0;
	switch (op) {
		case DEL_NO:
			for (; it != end; ++i, ++j) {
				if (data[j].second.count == -2) {
					continue;
				}
				if (j == it->second) {
					++it;
					continue;
				}
				if (ei.BlockType == BTYPE_STREAM) {
					Editor::SetString(i, data[it->second].first);
				} else {
					if (sel) {
						AutoUTF tmp(data[j].first);
						tmp.replace(data[j].second.start, data[j].second.count, it->first);
						Editor::SetString(i, tmp);
					} else {
						Editor::SetString(i, data[it->second].first);
					}
				}
				++it;
			}
			break;
		case DEL_BLOCK:
			for (; it != end; ++i, ++j, ++it) {
				if (j == it->second) {
					continue;
				}
				Editor::SetString(i, data[it->second].first);
			}
			for (; j < data.size(); ++i, ++j)
				Editor::SetString(i, L"");
			break;
		case DEL_SPARSE: {
			for (; it != end; ++i, ++j) {
				if (j == it->second) {
					++it;
					continue;
				}
				if (data[j].second.count != -2 && !data[j].first.empty()) {
					Editor::SetString(i, L"");
				}
			}
			for (; j < data.size(); ++i, ++j)
				if (!data[j].first.empty())
					Editor::SetString(i, L"");
			break;
		}
	}
}

bool	ProcessEditor() {
	data_vector	data;
	sort_vector	sortdata;

	if (ei.BlockType == BTYPE_STREAM || ei.BlockType == BTYPE_COLUMN)
		lineFirst = ei.BlockStartLine;
	else
		lineFirst = 0;

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

		intmax_t	SelLen = -2;
		switch (ei.BlockType) {
			case BTYPE_COLUMN: {
				if (egs.SelStart < egs.StringLength) {
					SelLen = std::min(egs.SelEnd, egs.StringLength) - std::min(egs.SelStart, egs.StringLength);
				}

				data.push_back(data_vector::value_type(tmp, SelInfo(egs.SelStart, SelLen)));
				if (SelLen != -2) {
					sortdata.push_back(sortpair(AutoUTF(egs.StringText + egs.SelStart, SelLen), i - lineFirst));
				}
				break;
			}
			case BTYPE_STREAM:
			default:
				data.push_back(data_vector::value_type(tmp, SelInfo(0, egs.StringLength)));
				sortdata.push_back(sortpair(tmp, i - lineFirst));
		}
//		file << "num: " << i << " sta: " << data[i].second.start << " cnt: " << data[i].second.count << endl;
	}

	std::pointer_to_binary_function<const sortpair&, const sortpair&, bool>
		pfLe(ptr_fun(PairLessCScode)),
		pfEq(ptr_fun(PairEqCScode));

	switch (cs) {
		case 0:
			pfLe = ptr_fun(PairLessCI);
			pfEq = ptr_fun(PairEqCI);
			break;
		case 1:
			pfLe = ptr_fun(PairLessCS);
			pfEq = ptr_fun(PairEqCS);
			break;
	}

	std::sort(sortdata.begin(), sortdata.end(), pfLe);

	if (op) {
		sort_vector::iterator it = std::unique(sortdata.begin(), sortdata.end(), pfEq);
		sortdata.erase(it, sortdata.end());
		std::sort(sortdata.begin(), sortdata.end(), ptr_fun(PairLessNum));
	}

//	ofstream file1("sortdata1.log");
//	for (sort_vector::iterator it = sortdata.begin(); it != sortdata.end(); ++it) {
//		file1 << "num: " << it->second << " str: '" << oem(it->first).c_str() << "'" << endl;
//	}

	Editor::StartUndo();
	if (inv && !op) {
		InsertFromVector(data, sortdata.rbegin(), sortdata.rend());
	} else {
		InsertFromVector(data, sortdata.begin(), sortdata.end());
	}
	Editor::StopUndo();

	Editor::Redraw();

	return	true;
}

//▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓
//▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓
//▓▓▓▓▓▓▓▓▓▓▓▓                                                            ▓▓▓▓▓▓▓▓▓▓▓▓▓▓
//▓▓▓▓▓▓▓▓▓▓▓▓   ╔═══════════════════ Sort strings ═══════════════════╗   ░░▓▓▓▓▓▓▓▓▓▓▓▓
//▓▓▓▓▓▓▓▓▓▓▓▓   ║ [ ] s Selection only                               ║   ░░▓▓▓▓▓▓▓▓▓▓▓▓
//▓▓▓▓▓▓▓▓▓▓▓▓   ║ [ ] i Invert sort                                  ║   ░░▓▓▓▓▓▓▓▓▓▓▓▓
//▓▓▓▓▓▓▓▓▓▓▓▓   ║ [ ] a Case sensitive                               ║   ░░▓▓▓▓▓▓▓▓▓▓▓▓
//▓▓▓▓▓▓▓▓▓▓▓▓   ╟────────────────────────────────────────────────────╢   ░░▓▓▓▓▓▓▓▓▓▓▓▓
//▓▓▓▓▓▓▓▓▓▓▓▓   ║ p Operation:                                       ║   ░░▓▓▓▓▓▓▓▓▓▓▓▓
//▓▓▓▓▓▓▓▓▓▓▓▓   ║ Sort                                              ↓║   ░░▓▓▓▓▓▓▓▓▓▓▓▓
//▓▓▓▓▓▓▓▓▓▓▓▓   ╟────────────────────────────────────────────────────╢   ░░▓▓▓▓▓▓▓▓▓▓▓▓
//▓▓▓▓▓▓▓▓▓▓▓▓   ║               { o ok } [ c cancel ]                ║   ░░▓▓▓▓▓▓▓▓▓▓▓▓
//▓▓▓▓▓▓▓▓▓▓▓▓   ╚════════════════════════════════════════════════════╝   ░░▓▓▓▓▓▓▓▓▓▓▓▓
//▓▓▓▓▓▓▓▓▓▓▓▓                                                            ░░▓▓▓▓▓▓▓▓▓▓▓▓
//▓▓▓▓▓▓▓▓▓▓▓▓▓▓░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░▓▓▓▓▓▓▓▓▓▓▓▓
//▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓
//▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓

///========================================================================================== Export
void WINAPI		EXP_NAME(GetPluginInfo)(PluginInfo *psi) {
	psi->StructSize = sizeof(PluginInfo);
	psi->Flags = PF_DISABLEPANELS | PF_EDITOR;
	static PCWSTR	PluginMenuStrings[1];
	PluginMenuStrings[0] = GetMsg(MenuTitle);
	psi->PluginMenuStrings = PluginMenuStrings;
	psi->PluginMenuStringsNumber = 1;

	sel = false;
	inv = cs = op = 0;
}

HANDLE WINAPI	EXP_NAME(OpenPlugin)(int OpenFrom, INT_PTR Item) {
	psi.EditorControl(ECTL_GETINFO, &ei);
	static FarListItem litems[] = {
		{0, GetMsg(lbSort), {0}},
		{0, GetMsg(lbDelBlock), {0}},
		{0, GetMsg(lbDelSparse), {0}},
	};
	static FarList flist = {sizeofa(litems), litems};

	enum {
		HEIGHT = 12,
		WIDTH = 60,

		indSelected = 1,
		indInv = 2,
		indCS = 3,
		indList = 6,
	};
	InitDialogItemF	Items[] = {
		{DI_DOUBLEBOX, 3,  1,  WIDTH - 4, HEIGHT - 2, 0, (PCWSTR)DlgTitle},
		{DI_CHECKBOX,  5, 2, 0,  0,                   0, (PCWSTR)cbSelected},
		{DI_CHECKBOX,  5, 3, 0,  0,                   0, (PCWSTR)cbInvert},
		{DI_CHECKBOX,  5, 4, 0,  0,                   DIF_3STATE, (PCWSTR)cbSensitive},
		{DI_TEXT,      0, 5, 0,  0,                   DIF_SEPARATOR,   L""},
		{DI_TEXT,      5, 6, 0,  0,                   0, (PCWSTR)txOperation},
		{DI_COMBOBOX,  5, 7, 54,  0,                  DIF_DROPDOWNLIST | DIF_LISTNOAMPERSAND, (PCWSTR)lbSort},
		{DI_TEXT,      0,  HEIGHT - 4, 0,  0,         DIF_SEPARATOR,   L""},
		{DI_BUTTON,    0,  HEIGHT - 3, 0,  0,         DIF_CENTERGROUP, (PCWSTR)txtBtnOk},
		{DI_BUTTON,    0,  HEIGHT - 3, 0,  0,         DIF_CENTERGROUP, (PCWSTR)txtBtnCancel},
	};
	size_t	size = sizeofa(Items);
	FarDialogItem	FarItems[size];
	InitDialogItemsF(Items, FarItems, size);
	FarItems[size - 2].DefaultButton = 1;
	FarItems[indSelected].Selected = sel;
	if (ei.BlockType != BTYPE_COLUMN)
		FarItems[indSelected].Flags |= DIF_DISABLE;
	FarItems[indSelected].Selected = sel;
	FarItems[indInv].Selected = inv;
	FarItems[indCS].Selected = cs;
	FarItems[indList].ListItems = &flist;

	FarDlg hDlg;
	if (hDlg.Init(psi.ModuleNumber, -1, -1, WIDTH, HEIGHT, nullptr, FarItems, size)) {
		int	ret = hDlg.Run();
		if (ret > 0 && Items[ret].Data == (PCWSTR)txtBtnOk) {
			sel = hDlg.Check(1);
			inv = hDlg.Check(2);
			cs = hDlg.Check(3);
			op = psi.SendDlgMessage(hDlg, DM_LISTGETCURPOS, indList, nullptr);
			ProcessEditor();
		}
	}
	return	INVALID_HANDLE_VALUE;
}

void WINAPI		EXP_NAME(SetStartupInfo)(const PluginStartupInfo *psi) {
	InitFSF(psi);
}
