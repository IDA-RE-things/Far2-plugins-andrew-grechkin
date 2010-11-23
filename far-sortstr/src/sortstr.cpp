/**
	sortstr: Sort strings in editor FAR2 plugin

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

typedef pair<AutoUTF, size_t> sortpair;

enum {
	cbSelected = 5,
	cbInvert,
	cbSensitive,
};

///======================================================================================= implement
PluginStartupInfo		psi;
FarStandardFunctions	fsf;

struct	PairsLessCI: public std::binary_function<const sortpair&, const sortpair&, bool> {
	bool	operator()(const sortpair &lhs, const sortpair &rhs) const {
		return	Cmpi(lhs.first.c_str(), rhs.first.c_str()) < 0;
	}
};

struct	PairsLessCS: public std::binary_function<const sortpair&, const sortpair&, bool> {
	bool	operator()(const sortpair &lhs, const sortpair &rhs) const {
		return	Cmp(lhs.first.c_str(), rhs.first.c_str()) < 0;
	}
};

struct	PairsLessCScode: public std::binary_function<const sortpair&, const sortpair&, bool> {
	bool	operator()(const sortpair &lhs, const sortpair &rhs) const {
		return	CmpCode(lhs.first.c_str(), rhs.first.c_str()) < 0;
	}
};

template <typename Type>
void	InsertFromVector(vector<AutoUTF> &data, Type it, Type end, size_t lineFirst) {
	for (size_t i = lineFirst; it != end; ++i, ++it) {
		if ((lineFirst + it->second) == i) {
			continue;
		}
		Editor::SetString(i, data[it->second]);
	}
}

bool	ProcessEditor(bool sel, bool inv, int cs) {
	EditorInfo ei;
	psi.EditorControl(ECTL_GETINFO, &ei);
	size_t	lineFirst = 0;
	if (sel) {
		if (ei.BlockType != BTYPE_STREAM && ei.BlockType != BTYPE_COLUMN) {
			return	false;
		} else {
			lineFirst = ei.BlockStartLine;
		}
	}

	vector<AutoUTF>		data;
	vector<sortpair>	sortdata;
//	data.reserve(ei.TotalLines - lineFirst);
//	sortdata.reserve(data.capacity());

	for (size_t i = lineFirst; i < (size_t)ei.TotalLines; ++i) {
		static EditorGetString	egs;
		egs.StringNumber = i;
		psi.EditorControl(ECTL_GETSTRING, &egs);

		if (sel && (egs.SelStart == -1 || egs.SelStart == egs.SelEnd))
			break;
		if (i == (ei.TotalLines - 1) && Empty(egs.StringText))
			break;
		AutoUTF	tmp(egs.StringText, egs.StringLength);
		data.push_back(tmp);

		if (sel) {
			size_t	SelLen = ((egs.SelEnd - egs.SelStart) <= 0) ? egs.StringLength : egs.SelEnd - egs.SelStart;
			sortdata.push_back(sortpair(AutoUTF(egs.StringText + egs.SelStart, SelLen), i - lineFirst));
		} else {
			sortdata.push_back(sortpair(tmp, i - lineFirst));
		}
	}

	switch (cs) {
		case 0:
			std::sort(sortdata.begin(), sortdata.end(), PairsLessCI());
			break;
		case 1:
			std::sort(sortdata.begin(), sortdata.end(), PairsLessCS());
			break;
		default:
			std::sort(sortdata.begin(), sortdata.end(), PairsLessCScode());
	}

	EditorUndoRedo eur = {EUR_BEGIN, {0}};
	psi.EditorControl(ECTL_UNDOREDO, &eur);
	if (inv) {
		InsertFromVector(data, sortdata.rbegin(), sortdata.rend(), lineFirst);
	} else {
		InsertFromVector(data, sortdata.begin(), sortdata.end(), lineFirst);
	}
	eur.Command = EUR_END;
	psi.EditorControl(ECTL_UNDOREDO, &eur);

	// Editor::UnselectBlock();
	Editor::Redraw();

	return	true;
}

//▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓
//▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓
//▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓                                             ▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓
//▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓   ╔═══════════ Sort strings ════════════╗   ░░▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓
//▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓   ║ [ ] s Selected only strings         ║   ░░▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓
//▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓   ║ [ ] i Invert sort                   ║   ░░▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓
//▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓   ║ [ ] a Case sensitive                ║   ░░▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓
//▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓   ╟─────────────────────────────────────╢   ░░▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓
//▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓   ║        { o ok } [ c cancel ]        ║   ░░▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓
//▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓   ╚═════════════════════════════════════╝   ░░▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓
//▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓                                             ░░▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓
//▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓
//▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓
//▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓

///========================================================================================== Export
void WINAPI		EXP_NAME(GetPluginInfo)(PluginInfo *psi) {
	psi->StructSize = sizeof(PluginInfo);
	psi->Flags = PF_DISABLEPANELS | PF_EDITOR;
	static PCWSTR	PluginMenuStrings[1];
	PluginMenuStrings[0] = GetMsg(MenuTitle);
	psi->PluginMenuStrings = PluginMenuStrings;
	psi->PluginMenuStringsNumber = 1;
}

HANDLE WINAPI	EXP_NAME(OpenPlugin)(int OpenFrom, INT_PTR Item) {
	EditorInfo ei;
	psi.EditorControl(ECTL_GETINFO, &ei);
	static int inv = 0, cs = 0;
	enum {
		HEIGHT = 9,
		WIDTH = 45,

		indSelected = 1,
		indInv = 2,
		indCS = 3,
	};
	InitDialogItemF	Items[] = {
		{DI_DOUBLEBOX, 3,  1,  WIDTH - 4, HEIGHT - 2, 0, (PCWSTR)DlgTitle},
		{DI_CHECKBOX,  5, 2, 0,  0,                   0, (PCWSTR)cbSelected},
		{DI_CHECKBOX,  5, 3, 0,  0,                   0, (PCWSTR)cbInvert},
		{DI_CHECKBOX,  5, 4, 0,  0,                   DIF_3STATE, (PCWSTR)cbSensitive},
		{DI_TEXT,      0,  HEIGHT - 4, 0,  0,         DIF_SEPARATOR,   L""},
		{DI_BUTTON,    0,  HEIGHT - 3, 0,  0,         DIF_CENTERGROUP, (PCWSTR)txtBtnOk},
		{DI_BUTTON,    0,  HEIGHT - 3, 0,  0,         DIF_CENTERGROUP, (PCWSTR)txtBtnCancel},
	};
	size_t	size = sizeofa(Items);
	FarDialogItem	FarItems[size];
	InitDialogItemsF(Items, FarItems, size);
	FarItems[size - 2].DefaultButton = 1;
	FarItems[indSelected].Selected = (ei.BlockType != BTYPE_NONE);
	FarItems[indInv].Selected = inv;
	FarItems[indCS].Selected = cs;

	FarDlg hDlg;
	if (hDlg.Init(psi.ModuleNumber, -1, -1, WIDTH, HEIGHT, nullptr, FarItems, size)) {
		int	ret = hDlg.Run();
		if (ret > 0 && Items[ret].Data == (PCWSTR)txtBtnOk) {
			inv = hDlg.Check(2);
			cs = hDlg.Check(3);
			ProcessEditor(hDlg.Check(1), inv, cs);
		}
	}
	return	INVALID_HANDLE_VALUE;
}

void WINAPI		EXP_NAME(SetStartupInfo)(const PluginStartupInfo *psi) {
	InitFSF(psi);
}
