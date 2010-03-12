/**
	tsmgr: Terminal sessions manager FAR plugin
	Allow to manage sessions on Terminal server

	© 2010  Andrew Grechkin

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
#include "win_kernel.h"
#include "win_net.h"

#include "far/far_helper.hpp"
#include "far/farkeys.hpp"

///========================================================================================== define
#define MIN_FAR_VERMAJOR  2
#define MIN_FAR_VERMINOR  0
#define MIN_FAR_BUILD     0

struct		PluginOptions {
	WinReg	reg;
	int		AddToPluginsMenu;
	int		AddToDisksMenu;
	int		DiskMenuDigit;
	CStrW	Prefix;
	PluginOptions():Prefix(L"tsmgr") {
		AddToPluginsMenu = true;
		AddToDisksMenu = false;
		DiskMenuDigit = L' ';
	}
	void		Read() {
		reg.Get(L"AddToPluginsMenu", AddToPluginsMenu, true);
		reg.Get(L"AddToDisksMenu", AddToDisksMenu, false);
		reg.Get(L"DiskMenuDigit", DiskMenuDigit, L' ');
		reg.Get(L"Prefix", Prefix, Prefix);
	}
	void		Write() {
		reg.Set(L"AddToPluginsMenu", AddToPluginsMenu);
		reg.Set(L"AddToDisksMenu", AddToDisksMenu);
		reg.Set(L"DiskMenuDigit", DiskMenuDigit);
		reg.Set(L"Prefix", Prefix);
	}
};

///======================================================================================= implement
PluginStartupInfo		psi;
FarStandardFunctions 	fsf;
PluginOptions			Options;

enum	{
	txtSelectComputer = 5,
	txtHost,
	txtEmptyForLocal,
	txtLogin,
	txtPaswd,
	txtEmptyForCurrent,
	txtConnecting,

	txtSendMessage,
	txtTitle,
	txtMessage,

	txtStatus,
	txtSession,

	txtMessg,
	txtComp,
	txtDisconn,
	txtLocal,
	txtLogOff,

	txtAreYouSure,
	txtDisconnectSession,
	txtLogoffSession,

	txtAddToPluginsMenu,
	txtAddToDiskMenu,
	txtDisksMenuHotkey,
	txtPluginPrefix,
};

///========================================================================================= dialogs
bool				DlgConnection(TsMgr* tsmgr) {
	WCHAR HostName[64] = {0};

	InitDialogItem Items[] = {
		{DI_DOUBLEBOX, 3, 1, 44, 11, 0, 0, 0, 0, GetMsg(txtSelectComputer)},
		{DI_TEXT, 5, 2, 0, 0, 0, 0, 0, 0, GetMsg(txtHost)},
		{DI_EDIT, 5, 3, 42, 0, 1, (DWORD_PTR)L"TsMgr.Host", DIF_HISTORY, 1, HostName},
		{DI_TEXT, 5, 4, 0, 0, 0, 0, 0, 0, GetMsg(txtEmptyForLocal)},
		{DI_TEXT, 0, 5, 0, 0, 0, 0, DIF_SEPARATOR, 0, L""},
		{DI_TEXT, 5, 6, 0, 0, 0, 0, 0, 0, GetMsg(txtLogin)},
		{DI_TEXT, 26, 6, 0, 0, 0, 0, 0, 0, GetMsg(txtPaswd)},
		{DI_EDIT, 5, 7, 22, 0, 0, (DWORD_PTR)L"TsMgr.Login", DIF_HISTORY, 0, L""},
		{DI_PSWEDIT, 26, 7, 42, 0, 0, 0, 0, 0, L""},
		{DI_TEXT, 5, 8, 0, 0, 0, 0, 0, 0, GetMsg(txtEmptyForCurrent)},
		{DI_TEXT, 5, 9, 0, 0, 0, 0, DIF_BOXCOLOR | DIF_SEPARATOR, 0, L""},
		{DI_BUTTON, 0, 10, 0, 0, 0, 0, DIF_CENTERGROUP, 1, GetMsg(txtBtnOk)},
		{DI_BUTTON, 0, 10, 0, 0, 0, 0, DIF_CENTERGROUP, 0, GetMsg(txtBtnCancel)},
	};
	size_t	size = sizeofa(Items);
	FarDialogItem FarItems[size];
	InitDialogItems(Items, FarItems, size);
	HANDLE hDlg = psi.DialogInit(psi.ModuleNumber, -1, -1, 48, 13, L"dlgRemoteConnection", FarItems, size, 0, 0, NULL, 0);
	if (hDlg) {
		if (psi.DialogRun(hDlg) == (int)(size - 2)) {
			try {
				tsmgr->Connect(GetDataPtr(hDlg, 2), GetDataPtr(hDlg, 7), GetDataPtr(hDlg, 8));
			} catch (WinError e) {
				farebox(e.code());
			}
			psi.DialogFree(hDlg);
			return	true;
		}
	}
	return	false;
}
bool				DlgMessage(TsMgr* tsmgr) {
	InitDialogItem Items[] = {
		{DI_DOUBLEBOX, 3, 1, 52, 8, 0, 0, 0, 0, GetMsg(txtSendMessage)},
		{DI_TEXT,    5, 2, 0, 0, 0, 0, 0, 0, GetMsg(txtTitle)},
		{DI_EDIT,    5, 3, 49, 0, 1, (DWORD_PTR)L"tsmgr.Title", DIF_HISTORY, 1, L""},
		{DI_TEXT,    5, 4, 0, 0, 0, 0, 0, 0, GetMsg(txtMessage)},
		{DI_EDIT,    5, 5, 49, 0, 0, (DWORD_PTR)L"tsmgr.Message", DIF_HISTORY, 0, L""},
		{DI_TEXT,    5, 6, 0, 0, 0, 0, DIF_BOXCOLOR | DIF_SEPARATOR, 0, L""},
		{DI_BUTTON,  0, 7, 0, 0, 0, 0, DIF_CENTERGROUP, 1, GetMsg(txtBtnOk)},
		{DI_BUTTON,  0, 7, 0, 0, 0, 0, DIF_CENTERGROUP, 0, GetMsg(txtBtnCancel)},
	};
	size_t	size = sizeofa(Items);
	FarDialogItem FarItems[size];
	InitDialogItems(Items, FarItems, size);
	HANDLE hDlg = psi.DialogInit(psi.ModuleNumber, -1, -1, 56, 10, L"dlgRemoteMessage", FarItems, size, 0, 0, NULL, 0);
	if (hDlg) {
		if (psi.DialogRun(hDlg) == (int)(size - 2)) {
			try {
				WinTSession::Message(tsmgr->id(), GetDataPtr(hDlg, 2), GetDataPtr(hDlg, 4), 60, false, tsmgr->conn());
			} catch (WinError e) {
				farebox(e.code());
			}
			psi.DialogFree(hDlg);
			return	true;
		}
	}
	return	false;
}

///========================================================================================== export
int		WINAPI	EXP_NAME(GetMinFarVersion)() {
	return	MAKEFARVERSION(MIN_FAR_VERMAJOR, MIN_FAR_VERMINOR, MIN_FAR_BUILD);
}
void	WINAPI	EXP_NAME(SetStartupInfo)(const PluginStartupInfo *psi) {
	InitFSF(psi);
	Options.reg.path(CStrW(psi->RootKey) + L"\\" + Options.Prefix);
}
void	WINAPI	EXP_NAME(GetPluginInfo)(PluginInfo *pi) {
	Options.Read();
	pi->StructSize = sizeof(PluginInfo);
	pi->Flags = 0;

	static PCWSTR	DiskStrings[1];
	static int		DiskNumbers[1];
	if (Options.AddToDisksMenu) {
		DiskStrings[0] = GetMsg(DiskTitle);
		DiskNumbers[0] = Options.DiskMenuDigit - L'0';
		pi->DiskMenuStrings = DiskStrings;
		pi->DiskMenuNumbers = DiskNumbers;
		pi->DiskMenuStringsNumber = sizeofa(DiskStrings);
	}

	static const TCHAR	*MenuStrings[1];
	MenuStrings[0] = GetMsg(MenuTitle);
	if (Options.AddToPluginsMenu) {
		pi->PluginMenuStrings = MenuStrings;
		pi->PluginMenuStringsNumber = sizeofa(MenuStrings);
	}

	pi->PluginConfigStrings = MenuStrings;
	pi->PluginConfigStringsNumber = sizeofa(MenuStrings);
	pi->CommandPrefix = Options.Prefix.c_str();
}
int		WINAPI	EXP_NAME(Configure)(int) {
	static WCHAR	DiskDigit[2] = {0};
	InitDialogItem Items[] = {
		{DI_DOUBLEBOX, 3, 1, 52, 8, 0, 0, 0, 0, GetMsg(DlgTitle)},
		{DI_CHECKBOX, 5, 2, 0, 0, 1, 0, 0, 0, GetMsg(txtAddToPluginsMenu)},
		{DI_CHECKBOX, 5, 3, 0, 0, 0, 0, 0, 0, GetMsg(txtAddToDiskMenu)},
		{DI_FIXEDIT, 7, 4, 7, 4, 0, 0, 0, 0, DiskDigit},
		{DI_TEXT, 10, 4, 0, 0, 0, 0, 0, 0, GetMsg(txtDisksMenuHotkey)},
		{DI_FIXEDIT, 5, 5, 12, 5, 0, 0, 0, 0, L""},
		{DI_TEXT, 14, 5, 0, 0, 0, 0, 0, 0, GetMsg(txtPluginPrefix)},
		{DI_TEXT, 5, 6, 0, 0, 0, 0, DIF_BOXCOLOR | DIF_SEPARATOR, 0, L""},
		{DI_BUTTON, 0, 7, 0, 0, 0, 0, DIF_CENTERGROUP, 1, GetMsg(txtBtnOk)},
		{DI_BUTTON, 0, 7, 0, 0, 0, 0, DIF_CENTERGROUP, 0, GetMsg(txtBtnCancel)},
	};
	size_t	size = sizeofa(Items);
	FarDialogItem FarItems[size];
	InitDialogItems(Items, FarItems, size);
	Options.Read();
	DiskDigit[0] = Options.DiskMenuDigit;
	FarItems[1].Selected = Options.AddToPluginsMenu;
	FarItems[2].Selected = Options.AddToDisksMenu;
	FarItems[3].PtrData = DiskDigit;
	FarItems[5].PtrData = Options.Prefix.c_str();

	HANDLE hDlg = psi.DialogInit(psi.ModuleNumber, -1, -1, 56, 10, L"dlgConfigure", FarItems, size, 0, 0, NULL, 0);
	if (hDlg) {
		if (psi.DialogRun(hDlg) == (int)(size - 2)) {
			Options.AddToPluginsMenu = GetCheck(hDlg, 1);
			Options.AddToDisksMenu = GetCheck(hDlg, 2);
			Options.DiskMenuDigit = GetDataPtr(hDlg, 3)[0];
			Options.Prefix = GetDataPtr(hDlg, 5);
			Options.Write();
			psi.DialogFree(hDlg);
			return	true;
		}
	}
	return	true;
}


HANDLE	WINAPI	EXP_NAME(OpenPlugin)(int OpenFrom, INT_PTR Item) {
//	Options.Read();
	CStrW	cline;
	if (OpenFrom == OPEN_PLUGINSMENU) {
		PanelInfo	pi;
		if (psi.Control(PANEL_ACTIVE, FCTL_GETPANELINFO, sizeof(pi), (LONG_PTR)&pi)) {
			CStrW	buf(MAX_PATH_LENGTH + MAX_PATH + 1);
			fsf.GetCurrentDirectory(buf.capacity(), buf.buffer());
			if (!buf.empty())
				fsf.AddEndSlash(buf.buffer());

			PluginPanelItem PPI;
			psi.Control(PANEL_ACTIVE, FCTL_GETPANELITEM, pi.CurrentItem, (LONG_PTR)&PPI);
			buf += PPI.FindData.lpwszFileName;
			cline = buf;
		}
	} else if (OpenFrom == OPEN_COMMANDLINE) {
		cline = (PCWSTR)Item;
		fsf.Trim(cline.buffer());
		fsf.Unquote(cline.buffer());
	}

	TsMgr*	hPlugin = new TsMgr;
	return	(HANDLE)hPlugin;
}
void	WINAPI	EXP_NAME(ClosePlugin)(HANDLE hPlugin) {
	delete(TsMgr*)hPlugin;
}
void	WINAPI	EXP_NAME(GetOpenPluginInfo)(HANDLE hPlugin, OpenPluginInfo *Info) {
	static WCHAR PanelTitle[64];
	TsMgr	*tsmgr = (TsMgr*)hPlugin;
	Info->StructSize = sizeof(*Info);
	Info->Flags = OPIF_ADDDOTS | OPIF_SHOWNAMESONLY | OPIF_USEHIGHLIGHTING | OPIF_USEFILTER;
	Info->HostFile	= NULL;
	Info->CurDir	= NULL;
	Info->Format	= Options.Prefix.c_str();
	Info->PanelTitle = PanelTitle;
	if (!tsmgr->host().empty()) {
		_snwprintf(PanelTitle, sizeofa(PanelTitle), L"%s: %s", Options.Prefix.c_str(), tsmgr->host().c_str());
	} else {
		_snwprintf(PanelTitle, sizeofa(PanelTitle), L"%s", Options.Prefix.c_str());
	}

	static PCWSTR ColumnTitles0[] = {NULL, GetMsg(txtStatus), GetMsg(txtSession)};
	static PanelMode CustomPanelModes[] = {
		{L"N,Z,O", L"0,10,10", ColumnTitles0, FALSE, TRUE, TRUE, TRUE, L"C0, S", L"0, 6", {0, 0}},
		{L"N,Z,O", L"0,10,10", ColumnTitles0, FALSE, TRUE, TRUE, TRUE, L"C0, S", L"0, 6", {0, 0}},
		{L"N,Z,O", L"0,10,10", ColumnTitles0, FALSE, TRUE, TRUE, TRUE, L"C0, S", L"0, 6", {0, 0}},
		{L"N,Z,O", L"0,10,10", ColumnTitles0, FALSE, TRUE, TRUE, TRUE, L"C0, S", L"0, 6", {0, 0}},
		{L"N,Z,O", L"0,10,10", ColumnTitles0, FALSE, TRUE, TRUE, TRUE, L"C0, S", L"0, 6", {0, 0}},
		{L"N,Z,O", L"0,10,10", ColumnTitles0, FALSE, TRUE, TRUE, TRUE, L"C0, S", L"0, 6", {0, 0}},
		{L"N,Z,O", L"0,10,10", ColumnTitles0, FALSE, TRUE, TRUE, TRUE, L"C0, S", L"0, 6", {0, 0}},
		{L"N,Z,O", L"0,10,10", ColumnTitles0, FALSE, TRUE, TRUE, TRUE, L"C0, S", L"0, 6", {0, 0}},
		{L"N,Z,O", L"0,10,10", ColumnTitles0, FALSE, TRUE, TRUE, TRUE, L"C0, S", L"0, 6", {0, 0}},
		{L"N,Z,O", L"0,10,10", ColumnTitles0, FALSE, TRUE, TRUE, TRUE, L"C0, S", L"0, 6", {0, 0}},
	};
	Info->PanelModesArray = CustomPanelModes;
	Info->PanelModesNumber = sizeofa(CustomPanelModes);
	Info->StartPanelMode = L'0';

	static KeyBarTitles keybartitles = {
		{(PWSTR)L"", (PWSTR)L"", (PWSTR)L"", (PWSTR)L"", (PWSTR)L"", (PWSTR)L"", (PWSTR)L"", (PWSTR)L""},
		{ 0, },
		{ 0, },
		{(PWSTR)L"", (PWSTR)L"", (PWSTR)L"", (PWSTR)L"", (PWSTR)L"", (PWSTR)L"", (PWSTR)L"", (PWSTR)L"", }
	};
	keybartitles.Titles[4] = (PWSTR)GetMsg(txtMessg);
	keybartitles.Titles[5] = (PWSTR)GetMsg(txtComp);
	keybartitles.Titles[6] = (PWSTR)L"";
	keybartitles.Titles[7] = (PWSTR)GetMsg(txtDisconn);
	keybartitles.ShiftTitles[3] = (PWSTR)L"";
	keybartitles.ShiftTitles[4] = (PWSTR)L"";
	keybartitles.ShiftTitles[5] = (PWSTR)GetMsg(txtLocal);
	keybartitles.ShiftTitles[6] = (PWSTR)L"";
	keybartitles.ShiftTitles[7] = (PWSTR)GetMsg(txtLogOff);
	Info->KeyBar = &keybartitles;
}

int		WINAPI	EXP_NAME(GetFindData)(HANDLE hPlugin, PluginPanelItem **pPanelItem, int *pItemsNumber, int OpMode) {
	TsMgr	*tsmgr = (TsMgr*)hPlugin;

	*pPanelItem = NULL;
	*pItemsNumber = 0;

	tsmgr->ts()->Cache();

	WinMem::Alloc(*pPanelItem, sizeof(**pPanelItem) * tsmgr->ts()->Size());
	*pItemsNumber = tsmgr->ts()->Size();

	int		i = 0;
	ForEachIn(tsmgr->ts()) {
		WinTSInfo	&info = tsmgr->ts()->Value();
		(*pPanelItem)[i].FindData.lpwszFileName = info.user.c_str();
		(*pPanelItem)[i].FindData.lpwszAlternateFileName = info.user.c_str();
		(*pPanelItem)[i].FindData.nFileSize = info.id;
		(*pPanelItem)[i].Description = info.GetState();
		(*pPanelItem)[i].Owner = info.sess.c_str();

		if (info.state == WTSDisconnected)
			(*pPanelItem)[i].FindData.dwFileAttributes = FILE_ATTRIBUTE_HIDDEN;
		PCWSTR	*CustomColumnData;// = (const wchar_t**)calloc(2, sizeof(const wchar_t*));
		WinMem::Alloc(CustomColumnData, 1 * sizeof(PCWSTR));
		if (CustomColumnData) {
			CustomColumnData[0] = info.client;
			(*pPanelItem)[i].CustomColumnData = CustomColumnData;
			(*pPanelItem)[i].CustomColumnNumber = 1;
		}
		(*pPanelItem)[i].UserData = i;
		++i;
	}

	return	true;
}
void	WINAPI	EXP_NAME(FreeFindData)(HANDLE hPlugin, PluginPanelItem *PanelItem, int ItemsNumber) {
	for (int i = 0; i < ItemsNumber; ++i) {
		//	for (int j = 0; j < PanelItem[i].CustomColumnNumber; ++j)
		//		free (PanelItem[i].CustomColumnData[j]);
		WinMem::Free(PanelItem[i].CustomColumnData);
	}
	WinMem::Free(PanelItem);
}

int		WINAPI	EXP_NAME(Compare)(HANDLE hPlugin, const PluginPanelItem *Item1, const PluginPanelItem *Item2, unsigned int Mode) {
//	TsMgr* tsmgr = (TsMgr*)hPlugin;
	/*
		if (((Mode == SM_NAME) || (Mode == SM_EXT))) {
			return	WinStr::Cmpi(Item1->FindData.lpwszFileName, Item2->FindData.lpwszFileName);
		}
		if (Mode == SM_UNSORTED) {
			return	WinStr::Cmpi(Item1->FindData.lpwszAlternateFileName, Item2->FindData.lpwszAlternateFileName);
		}
		WinTSInfo	&info1 = tsmgr->ts()->operator[](Item1->FindData.lpwszAlternateFileName);
		WinTSInfo	&info2 = tsmgr->ts()->operator[](Item2->FindData.lpwszAlternateFileName);
		if (Mode == SM_MTIME) {
			if (info1.ssp.dwCurrentState == info2.ssp.dwCurrentState)
				return	WinStr::Cmpi(Item1->FindData.lpwszFileName, Item2->FindData.lpwszFileName);
			if (info1.ssp.dwCurrentState < info2.ssp.dwCurrentState)
				return	1;
			return	-1;
		}
		if (Mode == SM_SIZE) {
			if (info1.StartType == info2.StartType)
				return	WinStr::Cmpi(Item1->FindData.lpwszFileName, Item2->FindData.lpwszFileName);
			if (info1.StartType < info2.StartType)
				return	-1;
			return	1;
		}
	*/
	return	-2;
}
int		WINAPI	EXP_NAME(ProcessKey)(HANDLE hPlugin, int Key, unsigned int ControlState) {
	TsMgr	*tsmgr = (TsMgr*)hPlugin;

	if (ControlState == 0 && Key == VK_F3) {
		FarPnl pInfo(hPlugin, FCTL_GETPANELINFO);
		if (pInfo.ItemsNumber() && pInfo.CurrentItem() && tsmgr->ts()->Find(pInfo[pInfo.CurrentItem()].FindData.nFileSize)) {
			CStrW	out(tsmgr->ts()->Info());
			CStrW	tempfile(TempFile(Options.Prefix));
			HANDLE	hdata = ::CreateFile(tempfile, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			if (hdata != INVALID_HANDLE_VALUE) {
				DWORD	dWritten;
				WriteFile(hdata, (PWSTR)out.c_str(), out.data_length(), &dWritten, NULL);
				::CloseHandle(hdata);
				psi.Viewer(tempfile, NULL, 0, 0, -1, -1,
						   VF_DELETEONLYFILEONCLOSE | VF_ENABLE_F6 | VF_DISABLEHISTORY |
						   VF_NONMODAL | VF_IMMEDIATERETURN, CP_AUTODETECT);
			}
		}
		return	true;
	}
	if ((ControlState == 0 && Key == VK_F8) ||
			(ControlState == PKF_SHIFT && Key == VK_F8) ||
			(ControlState == 0 && Key == VK_F5)) {
		FarPnl pInfo(hPlugin, FCTL_GETPANELINFO);
		if (pInfo.ItemsNumber() && pInfo.CurrentItem() && tsmgr->ts()->Find(pInfo[pInfo.CurrentItem()].FindData.nFileSize)) {
			if (ControlState == 0 && Key == VK_F8) {
				if (farquestion(GetMsg(txtAreYouSure), GetMsg(txtDisconnectSession)))
					WinTSession::Disconnect(tsmgr->id(), tsmgr->conn());
			} else if (ControlState == PKF_SHIFT && Key == VK_F8) {
				if (farquestion(GetMsg(txtAreYouSure), GetMsg(txtLogoffSession)))
					WinTSession::LogOff(tsmgr->id(), tsmgr->conn());
			} else if (ControlState == 0 && Key == VK_F5) {
				DlgMessage(tsmgr);
			}
			psi.Control(hPlugin, FCTL_UPDATEPANEL, 0, NULL);
			psi.Control(hPlugin, FCTL_REDRAWPANEL, 0, NULL);
			return	true;
		}
	}
	if (ControlState == 0 && Key == VK_F6) {
		if (DlgConnection(tsmgr)) {
			psi.Control(hPlugin, FCTL_UPDATEPANEL, 0, NULL);
			psi.Control(hPlugin, FCTL_REDRAWPANEL, 0, NULL);
		}
		return	true;
	}
	if (ControlState == PKF_SHIFT && Key == VK_F6) {
		try {
			tsmgr->Connect(NULL);
		} catch (WinError e) {
			farebox(e.code());
		}
		psi.Control(hPlugin, FCTL_UPDATEPANEL, 0, NULL);
		psi.Control(hPlugin, FCTL_REDRAWPANEL, 0, NULL);
		return	true;
	}
	if ((ControlState == PKF_SHIFT && Key == VK_F3) ||
			(ControlState == PKF_SHIFT && Key == VK_F4)) {
		return	true;
	}
	return	false;
}

///========================================================================================= WinMain
/*
extern		"C" {
	BOOL		WINAPI	DllMainCRTStartup(HANDLE, DWORD, PVOID) {
		return	true;
	}
}
*/
