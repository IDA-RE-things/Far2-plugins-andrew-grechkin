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

enum {
	rbDelAll = 5,
	rbDelRepeated,
	rbDelWithText,
};

///======================================================================================= implement
PluginStartupInfo		psi;
FarStandardFunctions	fsf;

Register reg;

EditorInfo ei;
size_t	lineFirst;
int op;
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
				if (Find(egs.StringText, text.c_str())) {
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
	psi.EditorControl(ECTL_GETINFO, &ei);

	enum {
		HEIGHT = 10,
		WIDTH = 60,

		indDelAll = 1,
		indDelRep,
		indDelText,
		indText,
	};
	InitDialogItemF	Items[] = {
		{DI_DOUBLEBOX, 3,  1,  WIDTH - 4, HEIGHT - 2, 0, (PCWSTR)DlgTitle},
		{DI_RADIOBUTTON,  5, 2, 28,  0,               0, (PCWSTR)rbDelAll},
		{DI_RADIOBUTTON,  5, 3, 28,  0,               0, (PCWSTR)rbDelRepeated},
		{DI_RADIOBUTTON,  5, 4, 28,  0,               0, (PCWSTR)rbDelWithText},
		{DI_EDIT,         9, 5, 40,  0,               0, (PCWSTR)text.c_str()},
		{DI_TEXT,      0,  HEIGHT - 4, 0,  0,         DIF_SEPARATOR,   EMPTY},
		{DI_BUTTON,    0,  HEIGHT - 3, 0,  0,         DIF_CENTERGROUP, (PCWSTR)txtBtnOk},
		{DI_BUTTON,    0,  HEIGHT - 3, 0,  0,         DIF_CENTERGROUP, (PCWSTR)txtBtnCancel},
	};
	size_t	size = sizeofa(Items);
	FarDialogItem	FarItems[size];
	InitDialogItemsF(Items, FarItems, size);
	FarItems[size - 2].DefaultButton = 1;
	FarItems[op].Selected = 1;

	FarDlg hDlg;
	if (hDlg.Init(psi.ModuleNumber, -1, -1, WIDTH, HEIGHT, nullptr, FarItems, size)) {
		int	ret = hDlg.Run();
		if (ret > 0 && Items[ret].Data == (PCWSTR)txtBtnOk) {
			for (int i = indDelAll; i <= indDelText; ++i) {
				if (hDlg.Check(i)) {
					op = i;
					break;
				}
			}
			if (op == indDelText)
				text = hDlg.Str(indText);

			reg.Set(L"operation", op);

			ProcessEditor();
		}
	}
	return INVALID_HANDLE_VALUE;
}

void WINAPI		EXP_NAME(SetStartupInfo)(const PluginStartupInfo *psi) {
	InitFSF(psi);

	reg.Open(KEY_READ | KEY_WRITE, make_path(psi->RootKey, plug_name).c_str());

	reg.Get(L"operation", op, 1);
}
