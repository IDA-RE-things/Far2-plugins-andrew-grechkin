/**
	delstr: Delete strings in editor
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

#include <far/helper.h>

PCWSTR plug_name = L"delstr";
PCWSTR PathHistoryName = L"delstr.text";

enum {
	rbDelAll = 5,
	rbDelRepeated,
	rbDelWithText,
	rbDelWithoutText,
	rbMask,
};

///======================================================================================= implement
PluginStartupInfo		psi;
FarStandardFunctions	fsf;

Register reg;

EditorInfo ei;
size_t	lineFirst;
int op, opm;
AutoUTF text;

inline AutoUTF make_path(const AutoUTF &path, const AutoUTF &name) {
	return path + PATH_SEPARATOR + name;
}

bool	ProcessEditor() {
	if (ei.BlockType == BTYPE_STREAM || ei.BlockType == BTYPE_COLUMN)
		lineFirst = ei.BlockStartLine;
	else
		lineFirst = 0;

	Editor::StartUndo();
	size_t total = ei.TotalLines;
	for (size_t i = lineFirst; i < total; ++i) {
		static EditorGetString	egs;
		egs.StringNumber = i;
		if (!psi.EditorControl(ECTL_GETSTRING, &egs))
			break;

		if (i == (total - 1) && Empty(egs.StringText))
			break;
		if (ei.BlockType != BTYPE_NONE && (egs.SelStart == -1 || egs.SelStart == egs.SelEnd))
			break;

		switch (op) {
			case 1:
				if (Empty(egs.StringText)) {
					Editor::DelString(i--);
					total--;
				}
				break;
			case 2:
				if (i > lineFirst) {
					static EditorGetString	pred;
					pred.StringNumber = i - 1;
					psi.EditorControl(ECTL_GETSTRING, &pred);
					if (Empty(pred.StringText) && Empty(egs.StringText)) {
						Editor::DelString(i--);
						total--;
					}
				}
				break;
			case 3:
			    if (opm) {
    				if (fsf.ProcessName((PWSTR)text.c_str(), (PWSTR)egs.StringText, 0, PN_CMPNAMELIST | PN_SKIPPATH)) {
                        Editor::DelString(i--);
                        total--;
    				}
			    }
			    else if (Find(egs.StringText, text.c_str())) {
                        Editor::DelString(i--);
                        total--;
                     }
				break;
			case 4:
			    if (opm) {
    				if (!fsf.ProcessName((PWSTR)text.c_str(), (PWSTR)egs.StringText, 0, PN_CMPNAMELIST | PN_SKIPPATH)) {
                        Editor::DelString(i--);
                        total--;
    				}
			    } else if (!Find(egs.StringText, text.c_str())) {
					Editor::DelString(i--);
					total--;
				}
		}
	}
//	Editor::SetPos(ei.CurLine, ei.CurPos);
	Editor::StopUndo();

	Editor::Redraw();

	return true;
}

///========================================================================================== Export
void WINAPI		EXP_NAME(GetPluginInfo)(PluginInfo *pi) {
	pi->StructSize = sizeof(PluginInfo);
	pi->Flags = PF_DISABLEPANELS | PF_EDITOR;
	static PCWSTR	PluginMenuStrings[1];
	PluginMenuStrings[0] = GetMsg(MenuTitle);
	pi->PluginMenuStrings = PluginMenuStrings;
	pi->PluginMenuStringsNumber = 1;
}

HANDLE WINAPI	EXP_NAME(OpenPlugin)(int /*OpenFrom*/, INT_PTR /*Item*/) {
	enum {
		HEIGHT = 11,
		WIDTH = 70,

		indDelAll = 1,
		indDelRep,
		indDelWithText,
		indDelWithoutText,
		indText,
		indIsMask,
	};
	static InitDialogItemF	Items[] = {
		{DI_DOUBLEBOX, 3,  1,  WIDTH - 4, HEIGHT - 2, 0, (PCWSTR)DlgTitle},
		{DI_RADIOBUTTON,  5, 2, 54,  0,               0, (PCWSTR)rbDelAll},
		{DI_RADIOBUTTON,  5, 3, 54,  0,               0, (PCWSTR)rbDelRepeated},
		{DI_RADIOBUTTON,  5, 4, 54,  0,               0, (PCWSTR)rbDelWithText},
		{DI_RADIOBUTTON,  5, 5, 54,  0,               0, (PCWSTR)rbDelWithoutText},
		{DI_EDIT,         9, 6, 46,  0,               0, (PCWSTR)text.c_str()},
		{DI_CHECKBOX,     50, 6, 0,  0,               0, (PCWSTR)rbMask},
		{DI_TEXT,      0,  HEIGHT - 4, 0,  0,         DIF_SEPARATOR,   EMPTY_STR},
		{DI_BUTTON,    0,  HEIGHT - 3, 0,  0,         DIF_CENTERGROUP, (PCWSTR)txtBtnOk},
		{DI_BUTTON,    0,  HEIGHT - 3, 0,  0,         DIF_CENTERGROUP, (PCWSTR)txtBtnCancel},
	};
	static size_t	size = sizeofa(Items);

	psi.EditorControl(ECTL_GETINFO, &ei);
	FarDialogItem	FarItems[size];
	InitDialogItemsF(Items, FarItems, size);
	FarItems[size - 2].DefaultButton = 1;
	FarItems[op].Selected = 1;
	FarItems[indIsMask].Selected = opm;
	FarItems[indText].Flags |= DIF_HISTORY;
	FarItems[indText].History = PathHistoryName;

	FarDlg hDlg;
	if (hDlg.Init(psi.ModuleNumber, -1, -1, WIDTH, HEIGHT, nullptr, FarItems, size)) {
		int	ret = hDlg.Run();
		if (ret > 0 && Items[ret].Data == (PCWSTR)txtBtnOk) {
			for (int i = indDelAll; i <= indDelWithoutText; ++i) {
				if (hDlg.Check(i)) {
					op = i;
					break;
				}
			}
			if ((op == indDelWithText) || (op == indDelWithoutText))
				text = hDlg.Str(indText);

            opm = hDlg.Check(indIsMask);

			reg.Set(L"operation", op);
			reg.Set(L"ismask", opm);

			ProcessEditor();
		}
	}
	return INVALID_HANDLE_VALUE;
}

void WINAPI		EXP_NAME(SetStartupInfo)(const PluginStartupInfo *psi) {
	InitFSF(psi);

	reg.Open(KEY_READ | KEY_WRITE, make_path(psi->RootKey, plug_name).c_str());
	reg.Get(L"operation", op, 1);
	reg.Get(L"ismask", opm, 0);
}
