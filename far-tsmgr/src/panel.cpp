/**
	tsmgr: Terminal sessions manager FAR plugin
	Allow to manage sessions on Terminal server

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

#include "panel.hpp"
#include "options.hpp"

#include "../../far/farkeys.hpp"

///======================================================================================= implement
PluginStartupInfo		psi;
FarStandardFunctions 	fsf;

///=========================================================================================== Panel
bool		Panel::DlgConnection() {
	enum {
		HEIGHT = 13,
		WIDTH = 48,

		indHost = 2,
		indUser = 7,
		indPass = 8,
	};
	InitDialogItemF Items[] = {
		{DI_DOUBLEBOX, 3,  1,  WIDTH - 4, HEIGHT - 2, 0, (PCWSTR)txtSelectComputer},
		{DI_TEXT,      5,  2,  0,  0,  0, (PCWSTR)txtHost},
		{DI_EDIT,      5,  3,  42, 0,  DIF_HISTORY,     L""},
		{DI_TEXT,      5,  4,  0,  0,  0, (PCWSTR)txtEmptyForLocal},
		{DI_TEXT,      0,  5,  0,  0,  DIF_SEPARATOR,   L""},
		{DI_TEXT,      5,  6,  0,  0,  0, (PCWSTR)txtLogin},
		{DI_TEXT,      26, 6,  0,  0,  0, (PCWSTR)txtPass},
		{DI_EDIT,      5,  7,  22, 0,  DIF_HISTORY,     L""},
		{DI_PSWEDIT,   26, 7,  42, 0,  0,               L""},
		{DI_TEXT,      5,  8,  0,  0,  0, (PCWSTR)txtEmptyForCurrent},
		{DI_TEXT,      0,  HEIGHT - 4, 0,  0,  DIF_SEPARATOR,   L""},
		{DI_BUTTON,    0,  HEIGHT - 3, 0,  0,  DIF_CENTERGROUP, (PCWSTR)txtBtnOk},
		{DI_BUTTON,    0,  HEIGHT - 3, 0,  0,  DIF_CENTERGROUP, (PCWSTR)txtBtnCancel},
	};
	size_t	size = sizeofa(Items);
	FarDialogItem FarItems[size];
	InitDialogItemsF(Items, FarItems, size);
	FarItems[size - 2].DefaultButton = 1;
	FarItems[indHost].History = L"Connect.Host";
	FarItems[indUser].History = L"Connect.Login";

	FarDlg hDlg;
	if (hDlg.Init(psi.ModuleNumber, -1, -1, WIDTH, HEIGHT, L"dlgRemoteConnection", FarItems, size)) {
		while (true) {
			int	ret = hDlg.Run();
			if (ret > 0 && Items[ret].Data == (PCWSTR)txtBtnOk) {
				try {
					Connect(hDlg.Str(indHost), hDlg.Str(indUser), hDlg.Str(indPass));
				} catch (WinError &e) {
					farebox(e.code());
					continue;
				}
				return	true;
			} else {
				break;
			}
		}
	}
	return	false;
}
bool		Panel::DlgMessage() {
	enum {
		HEIGHT = 10,
		WIDTH = 56,

		indTitle = 2,
		indMessage = 4,
	};
	InitDialogItemF Items[] = {
		{DI_DOUBLEBOX, 3,  1,  WIDTH - 4, HEIGHT - 2, 0, (PCWSTR)txtSendMessage},
		{DI_TEXT,    5, 2, 0, 0,            0, (PCWSTR)txtTitle},
		{DI_EDIT,    5, 3, 49, 0,           DIF_HISTORY, L""},
		{DI_TEXT,    5, 4, 0, 0,            0, (PCWSTR)txtMessage},
		{DI_EDIT,    5, 5, 49, 0,           DIF_HISTORY, L""},
		{DI_TEXT,    0, HEIGHT - 4, 0,  0,  DIF_SEPARATOR,   L""},
		{DI_BUTTON,  0, HEIGHT - 3, 0,  0,  DIF_CENTERGROUP, (PCWSTR)txtBtnOk},
		{DI_BUTTON,  0, HEIGHT - 3, 0,  0,  DIF_CENTERGROUP, (PCWSTR)txtBtnCancel},
	};
	size_t	size = sizeofa(Items);
	FarDialogItem FarItems[size];
	InitDialogItemsF(Items, FarItems, size);
	FarItems[size - 2].DefaultButton = 1;
	FarItems[indTitle].History = L"tsmgr.Title";
	FarItems[indMessage].History = L"tsmgr.Message";

	FarDlg hDlg;
	if (hDlg.Init(psi.ModuleNumber, -1, -1, WIDTH, HEIGHT, L"dlgRemoteMessage", FarItems, size)) {
		int	ret = hDlg.Run();
		if (ret > 0 && Items[ret].Data == (PCWSTR)txtBtnOk) {
			try {
				WinTSession::Message(id(), GetDataPtr(hDlg, 2), GetDataPtr(hDlg, 4), 60, false, conn());
			} catch (WinError &e) {
				farebox(e.code());
			}
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
	Info->HostFile	= null_ptr;
	Info->CurDir	= null_ptr;
	Info->Format	= Options.Prefix.c_str();
	Info->PanelTitle = PanelTitle;
	if (!host().empty()) {
		_snwprintf(PanelTitle, sizeofa(PanelTitle), L"%s: %s", Options.Prefix.c_str(), host().c_str());
	} else {
		_snwprintf(PanelTitle, sizeofa(PanelTitle), L"%s", Options.Prefix.c_str());
	}

	static PCWSTR ColumnTitles0[] = {null_ptr, GetMsg(txtStatus), GetMsg(txtSession)};
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
		{ null_ptr, null_ptr, null_ptr, (PWSTR)L"", (PWSTR)L"", (PWSTR)L"", (PWSTR)L"", (PWSTR)L""},
		{ null_ptr, },
		{ null_ptr, },
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
	*pPanelItem = null_ptr;
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
			CustomColumnData[0] = info.client.c_str();
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
			AutoUTF	out(m_ts.Info());
			AutoUTF	tempfile(TempFile(Options.Prefix.c_str()));
			HANDLE	hdata = ::CreateFile(tempfile.c_str(), GENERIC_WRITE, FILE_SHARE_READ, null_ptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, null_ptr);
			if (hdata != INVALID_HANDLE_VALUE) {
				DWORD	dWritten;
				WriteFile(hdata, (PWSTR)out.c_str(), Len(out.c_str())* sizeof(WCHAR), &dWritten, null_ptr);
				::CloseHandle(hdata);
				psi.Viewer(tempfile.c_str(), null_ptr, 0, 0, -1, -1,
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
			psi.Control(this, FCTL_UPDATEPANEL, 0, null_ptr);
			psi.Control(this, FCTL_REDRAWPANEL, 0, null_ptr);
			return	true;
		}
	}
	if (ControlState == 0 && Key == VK_F6) {
		if (DlgConnection()) {
			psi.Control(this, FCTL_UPDATEPANEL, 0, null_ptr);
			psi.Control(this, FCTL_REDRAWPANEL, 0, null_ptr);
		}
		return	true;
	}
	if (ControlState == PKF_SHIFT && Key == VK_F6) {
		try {
			Connect(null_ptr);
		} catch (WinError &e) {
			farebox(e.code());
		}
		psi.Control(this, FCTL_UPDATEPANEL, 0, null_ptr);
		psi.Control(this, FCTL_REDRAWPANEL, 0, null_ptr);
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
