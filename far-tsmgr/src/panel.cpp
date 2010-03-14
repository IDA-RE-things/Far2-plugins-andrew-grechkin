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

#include "panel.hpp"
#include "options.hpp"

#include "far/farkeys.hpp"

///======================================================================================= implement
PluginStartupInfo		psi;
FarStandardFunctions 	fsf;

///=========================================================================================== Panel
bool		Panel::DlgConnection() {
	WCHAR HostName[64] = {0};

	InitDialogItem Items[] = {
		{DI_DOUBLEBOX, 3, 1, 44, 11, 0, 0, 0, 0, GetMsg(txtSelectComputer)},
		{DI_TEXT, 5, 2, 0, 0, 0, 0, 0, 0, GetMsg(txtHost)},
		{DI_EDIT, 5, 3, 42, 0, 1, (DWORD_PTR)L"tsmgr.Host", DIF_HISTORY, 1, HostName},
		{DI_TEXT, 5, 4, 0, 0, 0, 0, 0, 0, GetMsg(txtEmptyForLocal)},
		{DI_TEXT, 0, 5, 0, 0, 0, 0, DIF_SEPARATOR, 0, L""},
		{DI_TEXT, 5, 6, 0, 0, 0, 0, 0, 0, GetMsg(txtLogin)},
		{DI_TEXT, 26, 6, 0, 0, 0, 0, 0, 0, GetMsg(txtPaswd)},
		{DI_EDIT, 5, 7, 22, 0, 0, (DWORD_PTR)L"tsmgr.Login", DIF_HISTORY, 0, L""},
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
				Connect(GetDataPtr(hDlg, 2), GetDataPtr(hDlg, 7), GetDataPtr(hDlg, 8));
			} catch (WinError e) {
				farebox(e.code());
			}
			psi.DialogFree(hDlg);
			return	true;
		}
	}
	return	false;
}
bool		Panel::DlgMessage() {
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
				WinTSession::Message(id(), GetDataPtr(hDlg, 2), GetDataPtr(hDlg, 4), 60, false, conn());
			} catch (WinError e) {
				farebox(e.code());
			}
			psi.DialogFree(hDlg);
			return	true;
		}
	}
	return	false;
}

void		Panel::GetOpenPluginInfo(OpenPluginInfo *Info) {
//	mbox(L"GetOpenPluginInfo");
	static WCHAR PanelTitle[64];
	Info->StructSize = sizeof(*Info);
	Info->Flags = OPIF_ADDDOTS | OPIF_SHOWNAMESONLY | OPIF_USEHIGHLIGHTING | OPIF_USEFILTER;
	Info->HostFile	= NULL;
	Info->CurDir	= NULL;
	Info->Format	= Options.Prefix.c_str();
	Info->PanelTitle = PanelTitle;
	if (!host().empty()) {
		_snwprintf(PanelTitle, sizeofa(PanelTitle), L"%s: %s", Options.Prefix.c_str(), host().c_str());
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

//	KeyBar
	static KeyBarTitles keybartitles = {
		{ NULL, NULL, NULL, (PWSTR)L"", (PWSTR)L"", (PWSTR)L"", (PWSTR)L"", (PWSTR)L""},
		{ NULL, },
		{ NULL, },
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

int			Panel::GetFindData(PluginPanelItem **pPanelItem, int *pItemsNumber, int OpMode) {
//	farmbox(L"GetFindData");
	*pPanelItem = NULL;
	*pItemsNumber = 0;

	m_ts.Cache();

	WinMem::Alloc(*pPanelItem, sizeof(**pPanelItem) * m_ts.Size());
	*pItemsNumber = m_ts.Size();

	int		i = 0;
	ForEachInO(m_ts) {
		WinTSInfo	&info = m_ts.Value();
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
void		Panel::FreeFindData(PluginPanelItem *PanelItem, int ItemsNumber) {
//	farmbox(L"FreeFindData");
	for (int i = 0; i < ItemsNumber; ++i) {
		//	for (int j = 0; j < PanelItem[i].CustomColumnNumber; ++j)
		//		free (PanelItem[i].CustomColumnData[j]);
		WinMem::Free(PanelItem[i].CustomColumnData);
	}
	WinMem::Free(PanelItem);
}

int			Panel::Compare(const PluginPanelItem *Item1, const PluginPanelItem *Item2, unsigned int Mode) {
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
int			Panel::ProcessEvent(int Event, void *Param) {
	if (Event == FE_CHANGEVIEWMODE) {
	}
	return	false;
}
int			Panel::ProcessKey(int Key, unsigned int ControlState) {
	if (ControlState == 0 && Key == VK_F3) {
		FarPnl pInfo(this, FCTL_GETPANELINFO);
		if (pInfo.ItemsNumber() && pInfo.CurrentItem() && m_ts.Find(pInfo[pInfo.CurrentItem()].FindData.nFileSize)) {
			CStrW	out(m_ts.Info());
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
		FarPnl pInfo(this, FCTL_GETPANELINFO);
		if (pInfo.ItemsNumber() && pInfo.CurrentItem() && m_ts.Find(pInfo[pInfo.CurrentItem()].FindData.nFileSize)) {
			if (ControlState == 0 && Key == VK_F8) {
				if (farquestion(GetMsg(txtAreYouSure), GetMsg(txtDisconnectSession)))
					WinTSession::Disconnect(id(), conn());
			} else if (ControlState == PKF_SHIFT && Key == VK_F8) {
				if (farquestion(GetMsg(txtAreYouSure), GetMsg(txtLogoffSession)))
					WinTSession::LogOff(id(), conn());
			} else if (ControlState == 0 && Key == VK_F5) {
				DlgMessage();
			}
			psi.Control(this, FCTL_UPDATEPANEL, 0, NULL);
			psi.Control(this, FCTL_REDRAWPANEL, 0, NULL);
			return	true;
		}
	}
	if (ControlState == 0 && Key == VK_F6) {
		if (DlgConnection()) {
			psi.Control(this, FCTL_UPDATEPANEL, 0, NULL);
			psi.Control(this, FCTL_REDRAWPANEL, 0, NULL);
		}
		return	true;
	}
	if (ControlState == PKF_SHIFT && Key == VK_F6) {
		try {
			Connect(NULL);
		} catch (WinError e) {
			farebox(e.code());
		}
		psi.Control(this, FCTL_UPDATEPANEL, 0, NULL);
		psi.Control(this, FCTL_REDRAWPANEL, 0, NULL);
		return	true;
	}
	if ((ControlState == PKF_SHIFT && Key == VK_F3) ||
			(ControlState == PKF_SHIFT && Key == VK_F4)) {
		return	true;
	}
	return	false;
}
int			Panel::SetDirectory(const WCHAR *Dir, int OpMode) {
	return	true;
}
