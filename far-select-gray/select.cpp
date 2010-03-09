#include "win_def.h"

#include "far/far_helper.hpp"
#include "far/farkeys.hpp"

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
						if (fsf.ProcessName(Mask, (PWSTR)ppi.FindData.lpwszFileName, WinStr::Len(ppi.FindData.lpwszFileName), PN_CMPNAMELIST)) {
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
	InitDialogItem	InitItems[] = {
		{DI_DOUBLEBOX, 3, 1, 41, 6, 0, 0, 0, 0, L"Select"},
		{DI_EDIT,      5, 2, 39, 0, 1, (DWORD_PTR)L"Masks", DIF_HISTORY, 0, L"*.*"},
		{DI_CHECKBOX,  5, 3, 0,  0, 0, 0, 0, 0, L"Select folders"},
		{DI_TEXT,      5, 4, 0,  0,  0, 0, DIF_BOXCOLOR | DIF_SEPARATOR, 0, L""},
		{DI_BUTTON,    0, 5, 0,  0,  0, 0, DIF_CENTERGROUP, 1, GetMsg(txtBtnOk)},
		{DI_BUTTON,    0, 5, 0,  0,  0, 0, DIF_CENTERGROUP, 0, GetMsg(txtBtnCancel)},
	};
	FarDialogItem	Items[sizeofa(InitItems)];
	InitDialogItems(InitItems, Items, sizeofa(InitItems));
	FarDlg	dlg;
	dlg.Execute(psi.ModuleNumber, -1, -1, 45, 8, NULL, Items, sizeofa(Items), 0, 0, DlgProc, Code);
	return	INVALID_HANDLE_VALUE;
}

///========================================================================================== Export
void WINAPI			EXP_NAME(GetPluginInfo)(PluginInfo *psi) {
	psi->StructSize = sizeof(PluginInfo);
	static PCWSTR	PluginMenuStrings[1];
	PluginMenuStrings[0] = GetMsg(MenuTitle);
	psi->PluginMenuStrings = PluginMenuStrings;
	psi->PluginMenuStringsNumber = 1;
}
HANDLE WINAPI		EXP_NAME(OpenPlugin)(int OpenFrom, INT_PTR Item) {
	const FarMenuItemEx Items[] = {
		{0, L"&+", 0, 0, NULL},
		{0, L"&-", 0, 0, NULL},
	};

	int		BreakKeys[] = {
		VK_ADD,
		VK_SUBTRACT,
		0,
	};
	int		BreakCode;

	int		Code = psi.Menu(psi.ModuleNumber, -1, -1, 0, FMENU_USEEXT | FMENU_WRAPMODE,
						  GetMsg(DlgTitle), NULL, NULL, BreakKeys, &BreakCode,
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
