/**
	select-gray: Select Gray FAR plugin
	Allow fast mark/unmark files on FAR panel

	© 1999 Aleksandr Arefiev
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
#include "win_def.h"

#include <far/helper.hpp>
#include <far/farkeys.hpp>

///======================================================================================= implement
PluginStartupInfo		psi;
FarStandardFunctions	fsf;

///========================================================================================= DlgProc
LONG_PTR WINAPI		DlgProc(HANDLE hDlg, int Msg, int Param1, LONG_PTR Param2) {
	static int		Key, KeyCount;
	static DWORD	TickCount;
	switch (Msg) {
		case DN_INITDIALOG: {
			int SelectFolders = FPS_SELECTFOLDERS & psi.AdvControl(psi.ModuleNumber, ACTL_GETPANELSETTINGS, 0);
			psi.SendDlgMessage(hDlg, DM_SETCHECK, 2, SelectFolders ? BSTATE_CHECKED : BSTATE_UNCHECKED);
			TickCount = GetTickCount();
			Key = Param2;
			KeyCount = 0;
			return TRUE;
		}
		case DN_KEY:
			if (!KeyCount) {
				KeyCount++;
				//?? используй параметр времени двойного клика мышью
				if (Key == Param2 && GetTickCount() - TickCount < 500) {
					psi.SendDlgMessage(hDlg, DM_USER, 0, Param2);
					return TRUE;
				}
			}
			if (Param2 == KEY_ENTER || Param2 == KEY_NUMENTER) {
				psi.SendDlgMessage(hDlg, DM_USER, 0, Param2);
				return TRUE;
			}
			return FALSE;
		case DM_USER: {
			FarPnl	pi(PANEL_ACTIVE);
			if (pi.IsOK()) {
				if (Param2 == KEY_ADD)
					for (int i = 0; i < pi.ItemsNumber(); ++i) {
						pi.Select(i, true);
					}
				else if (Param2 == KEY_SUBTRACT)
					for (int i = 0; i < pi.ItemsNumber(); ++i) {
						pi.Select(i, false);
					}
				else if (Param2 == KEY_ENTER || Param2 == KEY_NUMENTER) {
					PCWSTR	Mask = GetDataPtr(hDlg, 1);
					for (int i = 0; i < pi.ItemsNumber(); ++i) {
						PluginPanelItem	&ppi = pi[i];
						if (ppi.FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY
								&& !GetCheck(hDlg, 2)) {
							continue;
						}
						if (fsf.ProcessName(Mask, (PWSTR)ppi.FindData.lpwszFileName, Len(ppi.FindData.lpwszFileName), PN_CMPNAMELIST)) {
							if (Key == KEY_ADD) {
								pi.Select(i, true);
							} else if (Key == KEY_SUBTRACT) {
								pi.Select(i, false);
							}
						}
					}
				}
				psi.SendDlgMessage(hDlg, DM_CLOSE, -1, 0);
				return	TRUE;
			}
			return	FALSE;
		}
	}
	return	psi.DefDlgProc(hDlg, Msg, Param1, Param2);
}

//▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓
//▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓                                             ▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓
//▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓   ╔══════════════ Select ═══════════════╗   ░░▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓
//▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓   ║ ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░║   ░░▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓
//▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓   ║ [ ] Select folders                  ║   ░░▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓
//▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓   ╚═════════════════════════════════════╝   ░░▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓
//▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓                                             ░░▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓
//▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓
//▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓

HANDLE				Select(int Code) {
	enum {
		HEIGHT = 8,
		WIDTH = 45,

		indMask = 1,
	};
	InitDialogItemF		Items[] = {
		{DI_DOUBLEBOX, 3, 1, 41, 6, 	0,           L"Select"},
		{DI_EDIT,      5, 2, 39, 0, 	DIF_HISTORY, L"*.*"},
		{DI_CHECKBOX,  5, 3, 0,  0,     0,           L"Select folders"},
		{DI_TEXT,      0,  HEIGHT - 4, 0,  0,  DIF_SEPARATOR,   L""},
		{DI_BUTTON,    0,  HEIGHT - 3, 0,  0,  DIF_CENTERGROUP, (PCWSTR)txtBtnOk},
		{DI_BUTTON,    0,  HEIGHT - 3, 0,  0,  DIF_CENTERGROUP, (PCWSTR)txtBtnCancel},
	};
	size_t	size = sizeofa(Items);
	FarDialogItem	FarItems[size];
	InitDialogItemsF(Items, FarItems, size);
	FarItems[size - 2].DefaultButton = 1;
	FarItems[indMask].History = L"SelectGray.Masks";

	FarDlg	hDlg;
	hDlg.Init(psi.ModuleNumber, -1, -1, 45, 8, nullptr, FarItems, size, 0, 0, DlgProc, Code);
	hDlg.Run();
	return	INVALID_HANDLE_VALUE;
}

///========================================================================================== Export
void WINAPI			EXP_NAME(GetPluginInfo)(PluginInfo *pi) {
	pi->StructSize = sizeof(*pi);
	static PCWSTR	PluginMenuStrings[1];
	PluginMenuStrings[0] = GetMsg(MenuTitle);
	pi->PluginMenuStrings = PluginMenuStrings;
	pi->PluginMenuStringsNumber = 1;
}
HANDLE WINAPI		EXP_NAME(OpenPlugin)(int OpenFrom, INT_PTR Item) {
	const FarMenuItemEx Items[] = {
		{0, L"&+", 0, 0, nullptr},
		{0, L"&-", 0, 0, nullptr},
	};

	int		BreakKeys[] = {
		VK_ADD,
		VK_SUBTRACT,
		0,
	};
	int		BreakCode;

	int		Code = psi.Menu(psi.ModuleNumber, -1, -1, 0, FMENU_USEEXT | FMENU_WRAPMODE,
						 GetMsg(DlgTitle), nullptr, nullptr, BreakKeys, &BreakCode,
						 (FarMenuItem *)Items, sizeofa(Items));
	Code = ((BreakCode == -1) ? Code : BreakCode);

	switch (Code) {
		case 0:
			Select(KEY_ADD);
			break;
		case 1:
			Select(KEY_SUBTRACT);
			break;
	}
	return	INVALID_HANDLE_VALUE;
}
void WINAPI			EXP_NAME(SetStartupInfo)(const PluginStartupInfo *psi) {
	InitFSF(psi);
}

///========================================================================================= WinMain
extern "C" {
	BOOL WINAPI		DllMainCRTStartup(HANDLE hDll, DWORD dwReason, LPVOID lpReserved) {
		return	TRUE;
	}
}
