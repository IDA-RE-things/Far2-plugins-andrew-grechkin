/**
	svcmgr: Manage services
	Allow to manage windows services

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

struct		WinSvcAction {
	Panel	*panel;
	UINT	ControlState;
	int		Key;

	WinSvcAction(Panel *s, UINT cs, int k): panel(s), ControlState(cs), Key(k) {
	}
};

///========================================================================================= dialogs
LONG_PTR WINAPI		DlgProc(HANDLE hDlg, int Msg, int Param1, LONG_PTR Param2) {
	static WinSvcAction	*action;
	static	PCWSTR		txt = GetMsg(txtActionInProcess);
	switch (Msg) {
		case DN_INITDIALOG: {
			action = (WinSvcAction*)Param2;
			if ((action->ControlState == 0 && action->Key == VK_F5)) {
				txt = GetMsg(txtStartingService);
			}
			if ((action->ControlState == 0 && action->Key == VK_F7)) {
				txt = GetMsg(txtPausingService);
			}
			if ((action->ControlState == 0 && action->Key == VK_F8)) {
				txt = GetMsg(txtStoppingService);
			}
			if ((action->ControlState == PKF_ALT && action->Key == VK_F5)) {
				txt = GetMsg(txtRestartingService);
			}
			if ((action->ControlState == PKF_SHIFT && action->Key == VK_F7)) {
				txt = GetMsg(txtContinueService);
			}
			psi.SendDlgMessage(hDlg, DM_SETTEXTPTR, 1, (LONG_PTR)txt);
			return	true;
		}
		case DN_KEY: {
			if (Param2 == KEY_ESC) {
				psi.SendDlgMessage(hDlg, DM_CLOSE, -1, 0);
				return	true;
			}
			return	false;
		}
		case DN_ENTERIDLE: {
			try {
				WinSvc	svc(action->panel->name().c_str(), GENERIC_EXECUTE | SERVICE_QUERY_STATUS, action->panel->conn());
				if ((action->ControlState == 0 && action->Key == VK_F5)) {
					if (action->panel->state() == SERVICE_PAUSED) {
						svc.Continue();
						svc.WaitForState(SERVICE_RUNNING, 30000);
					} else {
						svc.Start();
						svc.WaitForState(SERVICE_RUNNING, 30000);
//						svc.WaitForState(SERVICE_RUNNING, 30000, ShowWaitState, &dlg);
					}
				}
				if ((action->ControlState == 0 && action->Key == VK_F7)) {
					svc.Pause();
					svc.WaitForState(SERVICE_PAUSED, 30000);
				}
				if ((action->ControlState == 0 && action->Key == VK_F8)) {
					try {
						svc.Stop();
						svc.WaitForState(SERVICE_STOPPED, 30000);
					} catch (WinError e) {
						if (e.code() != ERROR_SERVICE_NOT_ACTIVE)
							throw;
					}
				}
				if ((action->ControlState == PKF_ALT && action->Key == VK_F5)) {
					try {
						svc.Stop();
						svc.WaitForState(SERVICE_STOPPED, 30000);
					} catch (WinError e) {
						if (e.code() != ERROR_SERVICE_NOT_ACTIVE)
							throw;
					}
					svc.Start();
					svc.WaitForState(SERVICE_RUNNING, 30000);
				}
				if ((action->ControlState == PKF_SHIFT && action->Key == VK_F7)) {
					svc.Continue();
					svc.WaitForState(SERVICE_RUNNING, 30000);
//					svc.WaitForState(SERVICE_RUNNING, 30000, ShowWaitState, &dlg);
				}
				psi.SendDlgMessage(hDlg, DM_CLOSE, -1, 0);
			} catch (WinError e) {
				farebox(e.code());
				psi.SendDlgMessage(hDlg, DM_CLOSE, -1, 0);
				return	false;
			}
		}
	}
	return	psi.DefDlgProc(hDlg, Msg, Param1, Param2);
}

///=========================================================================================== Panel
bool		Panel::DlgConnection() {
	WCHAR	HostName[64] = {0};

	InitDialogItem Items[] = {
		{DI_DOUBLEBOX, 3, 1, 44, 11, 0, 0, 0, 0, GetMsg(txtSelectComputer)},
		{DI_TEXT, 5, 2, 0, 0, 0, 0, 0, 0, GetMsg(txtHost)},
		{DI_EDIT, 5, 3, 42, 0, 1, (DWORD_PTR)L"svcmgr.Host", DIF_HISTORY, 1, HostName},
		{DI_TEXT, 5, 4, 0, 0, 0, 0, 0, 0, GetMsg(txtEmptyForLocal)},
		{DI_TEXT, 0, 5, 0, 0, 0, 0, DIF_SEPARATOR, 0, L""},
		{DI_TEXT, 5, 6, 0, 0, 0, 0, 0, 0, GetMsg(txtLogin)},
		{DI_TEXT, 26, 6, 0, 0, 0, 0, 0, 0, GetMsg(txtPaswd)},
		{DI_EDIT, 5, 7, 22, 0, 0, (DWORD_PTR)L"svcmgr.Login", DIF_HISTORY, 0, L""},
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
bool		Panel::DlgCreateService() {
	InitDialogItem Items[] = {
		{DI_DOUBLEBOX, 3, 1, 44, 10, 0, 0, 0, 0, GetMsg(txtDlgCreateService)},
		{DI_TEXT, 5, 2, 0, 0, 0, 0, 0, 0, GetMsg(txtDlgName)},
		{DI_EDIT, 5, 3, 42, 0, 1, (DWORD_PTR)L"svcmgr.Name", DIF_HISTORY, 1, L""},
		{DI_TEXT, 5, 4, 0, 0, 0, 0, 0, 0, GetMsg(txtDlgDisplayName)},
		{DI_EDIT, 5, 5, 42, 0, 0, (DWORD_PTR)L"svcmgr.DName", DIF_HISTORY, 0, L""},
		{DI_TEXT, 5, 6, 0, 0, 0, 0, 0, 0, GetMsg(txtDlgBinaryPath)},
		{DI_EDIT, 5, 7, 42, 0, 0, (DWORD_PTR)L"svcmgr.Path", DIF_HISTORY, 0, L""},
		{DI_TEXT, 5, 8, 0,  0,  0, 0, DIF_BOXCOLOR | DIF_SEPARATOR, 0, L""},
		{DI_BUTTON, 0, 9, 0, 0, 0, 0, DIF_CENTERGROUP, 1, GetMsg(txtBtnOk)},
		{DI_BUTTON, 0, 9, 0, 0, 0, 0, DIF_CENTERGROUP, 0, GetMsg(txtBtnCancel)},
	};
	size_t	size = sizeofa(Items);
	FarDialogItem FarItems[size];
	InitDialogItems(Items, FarItems, size);
	HANDLE hDlg = psi.DialogInit(psi.ModuleNumber, -1, -1, 48, 12, L"dlgCreateService",
								 FarItems, size, 0, 0, NULL, 0);
	if (hDlg != INVALID_HANDLE_VALUE) {
		int		ret = psi.DialogRun(hDlg);
		if (ret == (int)(size - 2)) {
			CStrW	name(GetDataPtr(hDlg, 2));
			CStrW	dname(GetDataPtr(hDlg, 4));
			CStrW	path(GetDataPtr(hDlg, 6));
			try {
				WinScm	scm(SC_MANAGER_CREATE_SERVICE, conn());
				scm.Create(name, path, SERVICE_DEMAND_START, dname);
			} catch (WinError e) {
				farebox(e.code());
			}
		}
		psi.DialogFree(hDlg);
	}
	return	true;
}
bool		Panel::DlgEditSvc() {
	bool	isSvc = WinFlag<DWORD>::Check(m_sm.Value().ServiceType, SERVICE_WIN32_OWN_PROCESS) ||
				 WinFlag<DWORD>::Check(m_sm.Value().ServiceType, SERVICE_WIN32_SHARE_PROCESS);
	DWORD	fl4Svc = isSvc ? DIF_DISABLE : 0;
	DWORD	fl4Dev = isSvc ? 0 : DIF_DISABLE;
	InitDialogItem Items[] = {
		{DI_DOUBLEBOX, 3, 1, 64, 18, 0, 0, 0, 0, GetMsg(txtDlgServiceProperties)},
		{DI_TEXT,      5, 2, 15,  0, 0, 0, 0, 0, GetMsg(txtDlgName)},
		{DI_EDIT,      16, 2, 62, 0,  1, 0, DIF_READONLY, 0, name()},
		{DI_TEXT,      5, 4, 0, 0, 0, 0, 0, 0, GetMsg(txtDlgDisplayName)},
		{DI_EDIT,      5, 5, 42, 0,  0, 0, 0, 0, m_sm.Value().dname.c_str()},
		{DI_TEXT,      5, 6, 0, 0, 0, 0, 0, 0, GetMsg(txtDlgBinaryPath)},
		{DI_EDIT,      5, 7, 42, 0,  0, 0, 0, 0, m_sm.Value().path.c_str()},
//		{DI_TEXT,      5, 9, 0, 0, 0, 0, 0, 0, L"Load ordering group:"},
//		{DI_COMBOBOX,  5, 10, 42, 0,  0, NULL, DIF_SELECTONENTRY, 1, L""},
		{DI_SINGLEBOX,   5, 13, 42, 16, 0, 0, DIF_LEFTTEXT | fl4Dev, 0, GetMsg(txtDlgServiceType)},
		{DI_RADIOBUTTON, 7, 14, 40, 0, 0, WinFlag<DWORD>::Check(m_sm.Value().ServiceType, SERVICE_WIN32_OWN_PROCESS), fl4Dev, 0, GetMsg(txtOwnProcess)},
		{DI_RADIOBUTTON, 7, 15, 40, 0, 0, WinFlag<DWORD>::Check(m_sm.Value().ServiceType, SERVICE_WIN32_SHARE_PROCESS), fl4Dev, 0, GetMsg(txtSharedProcess)},

		{DI_SINGLEBOX,   44, 4, 62, 10, 0, 0, DIF_LEFTTEXT, 0, GetMsg(txtDlgStartupType)},
		{DI_RADIOBUTTON, 46, 5, 60, 0, 0, m_sm.Value().StartType == SERVICE_BOOT_START, fl4Svc, 0, GetMsg(txtDlgBoot)},
		{DI_RADIOBUTTON, 46, 6, 60, 0, 0, m_sm.Value().StartType == SERVICE_SYSTEM_START, fl4Svc, 0, GetMsg(txtDlgSystem)},
		{DI_RADIOBUTTON, 46, 7, 60, 0, 0, m_sm.Value().StartType == SERVICE_AUTO_START, 0, 0, GetMsg(txtDlgAuto)},
		{DI_RADIOBUTTON, 46, 8, 60, 0, 0, m_sm.Value().StartType == SERVICE_DEMAND_START, 0, 0, GetMsg(txtDlgManual)},
		{DI_RADIOBUTTON, 46, 9, 60, 0, 0, m_sm.Value().StartType == SERVICE_DISABLED, 0, 0, GetMsg(txtDlgDisbld)},
		{DI_SINGLEBOX,   44, 11, 62, 16, 0, 0, DIF_LEFTTEXT, 0, GetMsg(txtDlgErrorControl)},
		{DI_RADIOBUTTON, 46, 12, 60, 0, 0, m_sm.Value().ErrorControl == SERVICE_ERROR_IGNORE, 0, 0, GetErrorControl(SERVICE_ERROR_IGNORE)},
		{DI_RADIOBUTTON, 46, 13, 60, 0, 0, m_sm.Value().ErrorControl == SERVICE_ERROR_NORMAL, 0, 0, GetErrorControl(SERVICE_ERROR_NORMAL)},
		{DI_RADIOBUTTON, 46, 14, 60, 0, 0, m_sm.Value().ErrorControl == SERVICE_ERROR_SEVERE, 0, 0, GetErrorControl(SERVICE_ERROR_SEVERE)},
		{DI_RADIOBUTTON, 46, 15, 60, 0, 0, m_sm.Value().ErrorControl == SERVICE_ERROR_CRITICAL, 0, 0, GetErrorControl(SERVICE_ERROR_CRITICAL)},

//		{DI_BUTTON, 0, 17, 0, 0, 0, 0, DIF_CENTERGROUP, 0, L"Logon as"},
		{DI_BUTTON, 0, 17, 0, 0, 0, 0, DIF_CENTERGROUP, 1, GetMsg(txtBtnOk)},
		{DI_BUTTON, 0, 17, 0, 0, 0, 0, DIF_CENTERGROUP, 0, GetMsg(txtBtnCancel)},
	};
	size_t	size = sizeofa(Items);
	FarDialogItem FarItems[size];
	InitDialogItems(Items, FarItems, size);
	HANDLE hDlg = psi.DialogInit(psi.ModuleNumber, -1, -1, 68, 20, L"dlgEditService", FarItems, size, 0, 0, NULL, 0);
	if (hDlg) {
		if (psi.DialogRun(hDlg) == (int)(size - 2)) {
			try {
				WinSvc	svc(name(), SERVICE_CHANGE_CONFIG, conn());
				if (GetCheck(hDlg, 13))
					m_sm.Value().StartType = SERVICE_AUTO_START;
				else if (GetCheck(hDlg, 14))
					m_sm.Value().StartType = SERVICE_DEMAND_START;
				else if (GetCheck(hDlg, 15))
					m_sm.Value().StartType = SERVICE_DISABLED;
				if (GetCheck(hDlg, 17))
					m_sm.Value().ErrorControl = SERVICE_ERROR_IGNORE;
				else if (GetCheck(hDlg, 18))
					m_sm.Value().ErrorControl = SERVICE_ERROR_NORMAL;
				else if (GetCheck(hDlg, 19))
					m_sm.Value().ErrorControl = SERVICE_ERROR_SEVERE;
				else if (GetCheck(hDlg, 20))
					m_sm.Value().ErrorControl = SERVICE_ERROR_CRITICAL;
				CStrW	path = GetDataPtr(hDlg, 6);
				if (isSvc) {
					if (GetCheck(hDlg, 8))
						m_sm.Value().ServiceType = SERVICE_WIN32_OWN_PROCESS;
					else if (GetCheck(hDlg, 9))
						m_sm.Value().ServiceType = SERVICE_WIN32_SHARE_PROCESS;
					CheckAPI(::ChangeServiceConfig(
								 svc,						// handle of service
								 m_sm.Value().ServiceType,	// service type
								 m_sm.Value().StartType,	// service start type
								 m_sm.Value().ErrorControl,	// error control
								 (path == m_sm.Value().path) ? NULL : path.c_str(),
								 NULL,						// load order group: no change
								 NULL,						// tag ID: no change
								 NULL,						// dependencies: no change
								 NULL,						// account name: no change
								 NULL,						// password: no change
								 GetDataPtr(hDlg, 4)));		// display name
				} else {
					if (GetCheck(hDlg, 11))
						m_sm.Value().StartType = SERVICE_BOOT_START;
					else if (GetCheck(hDlg, 12))
						m_sm.Value().StartType = SERVICE_SYSTEM_START;
					CheckAPI(::ChangeServiceConfig(
								 svc,						// handle of service
								 SERVICE_NO_CHANGE,			// service type
								 m_sm.Value().StartType,	// service start type
								 m_sm.Value().ErrorControl,	// error control
								 (path == m_sm.Value().path) ? NULL : path.c_str(),
								 NULL,						// load order group: no change
								 NULL,						// tag ID: no change
								 NULL,						// dependencies: no change
								 NULL,						// account name: no change
								 NULL,						// password: no change
								 GetDataPtr(hDlg, 4)));		// display name
				}
			} catch (WinError e) {
				farebox(e.code());
			}
			psi.Control(this, FCTL_UPDATEPANEL, TRUE, NULL);
			psi.Control(this, FCTL_REDRAWPANEL, 0, NULL);
		}
		psi.DialogFree(hDlg);
		return	true;
	}
	return	false;
}

void		Panel::GetOpenPluginInfo(OpenPluginInfo *Info) {
//	mbox(L"GetOpenPluginInfo");
	static WCHAR PanelTitle[64];
	Info->StructSize = sizeof(*Info);
	Info->Flags		= OPIF_ADDDOTS | OPIF_SHOWNAMESONLY | OPIF_USEATTRHIGHLIGHTING | OPIF_USEFILTER;
	Info->HostFile	= NULL;
	if (m_sm.drivers())
		Info->CurDir	= GetMsg(txtDevices);
	else
		Info->CurDir	= NULL;
//	Info->Format	= Options.Prefix.c_str();
	Info->PanelTitle = PanelTitle;
	if (!host().empty()) {
		_snwprintf(PanelTitle, sizeofa(PanelTitle), L"%s: %s", Options.Prefix.c_str(), host().c_str());
	} else {
		_snwprintf(PanelTitle, sizeofa(PanelTitle), L"%s", Options.Prefix.c_str());
	}

	Info->StartSortMode = SM_NAME;

//	PanelModes
	static PCWSTR ColumnTitles0[] = {GetMsg(txtClmDisplayName), L"Info", L"Info"};
	static PCWSTR ColumnTitles3[] = {GetMsg(txtClmDisplayName), GetMsg(txtClmStatus), GetMsg(txtClmStart)};
	static PCWSTR ColumnTitles4[] = {GetMsg(txtClmDisplayName), GetMsg(txtClmStatus)};
	static PCWSTR ColumnTitles5[] = {NULL, GetMsg(txtClmDisplayName), GetMsg(txtClmStatus), GetMsg(txtClmStart), NULL};
	static PCWSTR ColumnTitles6[] = {GetMsg(txtClmName), GetMsg(txtClmDisplayName)};
	static PCWSTR ColumnTitles7[] = {GetMsg(txtClmDisplayName), GetMsg(txtClmStatus), NULL};
	static PCWSTR ColumnTitles8[] = {GetMsg(txtClmDisplayName), GetMsg(txtClmLogon)};
	static PCWSTR ColumnTitles9[] = {GetMsg(txtClmName), GetMsg(txtClmStatus), GetMsg(txtClmDep)};
	static PanelMode CustomPanelModes[] = {
		{L"NM,C6,C7", L"0,8,8", ColumnTitles0, false, TRUE, TRUE, TRUE, L"N", L"0", {0, 0}},
		{L"N,N,N", L"0,0,0", NULL, false, TRUE, TRUE, TRUE, L"N", L"0", {0, 0}},
		{L"N,N", L"0,0", NULL, false, TRUE, TRUE, TRUE, L"N", L"0", {0, 0}},
		{L"N,C2,C3", L"0,7,6", ColumnTitles3, false, TRUE, TRUE, TRUE, L"C0", L"0", {0, 0}},
		{L"N,C2", L"0,7", ColumnTitles4, false, TRUE, TRUE, TRUE, L"C0,C2", L"0,6", {0, 0}},
		{L"N,C1,C2,C3,DM", L"0,0,7,6,11", ColumnTitles5, true, TRUE, TRUE, TRUE, L"C3", L"0", {0, 0}},
		{L"N,C1", L"40%,0", ColumnTitles6, false, TRUE, TRUE, TRUE, L"C1,C2", L"0,0", {0, 0}},
		{L"N,C2,Z", L"40%,1,0", ColumnTitles7, true, TRUE, TRUE, TRUE, L"C3", L"0", {0, 0}},
		{L"N,O", L"0,30%", ColumnTitles8, false, TRUE, TRUE, TRUE, L"C0", L"0", {0, 0}},
		{L"N,C2,LN", L"0,7,3", ColumnTitles9, false, TRUE, TRUE, TRUE, L"N", L"0", {0, 0}},
	};
	Info->PanelModesArray = CustomPanelModes;
	Info->PanelModesNumber = sizeofa(CustomPanelModes);
	Info->StartPanelMode = L'3';
	/*
	name;				// C0
	dname;				// N
	dwCurrentState;		// C1
	StartType;			// C2
	path;				// C3
	descr;				// Z
	Dependencies;		// C4
	OrderGroup;			// C5
	ServiceStartName;	// C6
	*/

//	KeyBar
	static KeyBarTitles keybartitles = {
		{ 0, 0, 0, 0, (PWSTR)L"", (PWSTR)L"", (PWSTR)L"", (PWSTR)L""},
		{ 0, },
		{ 0, 0, 0, (PWSTR)L"", (PWSTR)L"", (PWSTR)L"", },
		{(PWSTR)L"", (PWSTR)L"", (PWSTR)L"", (PWSTR)L"", (PWSTR)L"", (PWSTR)L"", (PWSTR)L"", (PWSTR)L"", }
	};
	keybartitles.Titles[4] = (PWSTR)GetMsg(txtBtnStart);
	keybartitles.Titles[5] = (PWSTR)GetMsg(txtBtnConnct);
	keybartitles.Titles[6] = (PWSTR)GetMsg(txtBtnPause);
	keybartitles.Titles[7] = (PWSTR)GetMsg(txtBtnStop);
	keybartitles.AltTitles[3] = (PWSTR)L""; //HWProf
	keybartitles.AltTitles[4] = (PWSTR)GetMsg(txtBtnRestrt);
	keybartitles.ShiftTitles[3] = (PWSTR)GetMsg(txtBtnCreate);
	keybartitles.ShiftTitles[4] = (PWSTR)GetMsg(txtBtnStartP);
	keybartitles.ShiftTitles[5] = (PWSTR)GetMsg(txtBtnLocal);
	keybartitles.ShiftTitles[6] = (PWSTR)GetMsg(txtBtnContin);
	keybartitles.ShiftTitles[7] = (PWSTR)GetMsg(txtBtnDelete);
	Info->KeyBar = &keybartitles;
}

int			Panel::GetFindData(PluginPanelItem **pPanelItem, int *pItemsNumber, int OpMode) {
//	farmbox(L"GetFindData");
	*pPanelItem = NULL;
	*pItemsNumber = 0;

	Cache();
	int i = 0;
	if (m_sm.services()) {
		i = 1;
		WinMem::Alloc(*pPanelItem, sizeof(**pPanelItem) * (m_sm.Size() + 1));
		*pItemsNumber = m_sm.Size() + 1;
		(*pPanelItem)[0].FindData.lpwszFileName = GetMsg(txtDevices);
		(*pPanelItem)[0].FindData.dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;
	} else {
		WinMem::Alloc(*pPanelItem, sizeof(**pPanelItem) * m_sm.Size());
		*pItemsNumber = m_sm.Size();
	}

	ForEachInO(m_sm) {
		s_ServiceInfo	&info = m_sm.Value();
		if (ViewMode == 1 || ViewMode == 2 || ViewMode == 5 || ViewMode == 9) {
			(*pPanelItem)[i].FindData.lpwszAlternateFileName = info.name.c_str();
			(*pPanelItem)[i].FindData.lpwszFileName = info.name.c_str();
		} else {
			(*pPanelItem)[i].FindData.lpwszAlternateFileName = info.name.c_str();
			(*pPanelItem)[i].FindData.lpwszFileName = info.dname.c_str();
		}
		(*pPanelItem)[i].Description = info.descr.c_str();
		(*pPanelItem)[i].NumberOfLinks = info.Dependencies.size();
		(*pPanelItem)[i].Owner = info.ServiceStartName.c_str();

		if (info.StartType == SERVICE_DISABLED)
			(*pPanelItem)[i].FindData.dwFileAttributes = FILE_ATTRIBUTE_HIDDEN;
		PCWSTR	*CustomColumnData;
		if (WinMem::Alloc(CustomColumnData, 5 * sizeof(PCWSTR))) {
			CustomColumnData[0] = info.name.c_str();
			CustomColumnData[1] = info.dname.c_str();
			CustomColumnData[2] = GetState(info.dwCurrentState);
			CustomColumnData[3] = GetStartType(info.StartType);
			CustomColumnData[4] = info.path.c_str();
			(*pPanelItem)[i].CustomColumnData = CustomColumnData;
			(*pPanelItem)[i].CustomColumnNumber = 5;
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
	if (m_sm.Find(Item1->FindData.lpwszAlternateFileName) && (Item2->FindData.lpwszAlternateFileName)) {
		s_ServiceInfo	&info1 = m_sm[Item1->FindData.lpwszAlternateFileName];
		s_ServiceInfo	&info2 = m_sm[Item2->FindData.lpwszAlternateFileName];
		if (Mode == SM_NAME) {
			return	Cmpi(info1.dname.c_str(), info2.dname.c_str());
		}
		if (Mode == SM_EXT) {
			return	Cmpi(info1.name.c_str(), info2.name.c_str());
		}
		if (Mode == SM_MTIME) {
			if (info1.dwCurrentState == info2.dwCurrentState)
				return	Cmpi(info1.dname.c_str(), info2.dname.c_str());
			if (info1.dwCurrentState < info2.dwCurrentState)
				return	1;
			return	-1;
		}
		if (Mode == SM_SIZE) {
			if (info1.StartType == info2.StartType)
				return	Cmpi(info1.dname.c_str(), info2.dname.c_str());
			if (info1.StartType < info2.StartType)
				return	-1;
			return	1;
		}
		return	-2;
	}
	return	-2;
}
int			Panel::ProcessEvent(int Event, void *Param) {
	if (Event == FE_CHANGEVIEWMODE) {
		FarPnl	panel(this);
		if (ViewMode != panel.ViewMode()) {
			ViewMode = panel.ViewMode();
//			farmbox(Num2Str(ViewMode), L"Event == FE_CHANGEVIEWMODE");
			need_recashe = false;
			psi.Control(this, FCTL_UPDATEPANEL, TRUE, NULL);
			psi.Control(this, FCTL_REDRAWPANEL, 0, NULL);
		}
	}
	return	FALSE;
}
int			Panel::ProcessKey(int Key, unsigned int ControlState) {
	if (ControlState == 0 && Key == VK_F3) {
		FarPnl pInfo(this, FCTL_GETPANELINFO);
		if (pInfo.ItemsNumber() && m_sm.Find(pInfo[pInfo.CurrentItem()].FindData.lpwszAlternateFileName)) {
			CStrW	out(Info());
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
	if (ControlState == 0 && Key == VK_F4) {
		FarPnl pInfo(this, FCTL_GETPANELINFO);
		if (pInfo.ItemsNumber()) {
			if (m_sm.Find(pInfo[pInfo.CurrentItem()].FindData.lpwszAlternateFileName)) {
				DlgEditSvc();
			}
		}
		return	true;
	}
	if (ControlState == 0 && Key == VK_F6) {
		if (DlgConnection()) {
			psi.Control(this, FCTL_UPDATEPANEL, 0, NULL);
			psi.Control(this, FCTL_REDRAWPANEL, 0, NULL);
		}
		return	true;
	}
	if (ControlState == PKF_SHIFT && Key == VK_F4) {
		if (DlgCreateService()) {
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

	UINT	tcs = 0;

	if (ControlState == PKF_SHIFT && Key == VK_F8) {
		if (farquestion(GetMsg(txtAreYouSure), GetMsg(txtDeleteService))) {
			tcs = ControlState;
			ControlState = 0;
		} else {
			return	false;
		}
	}

	if ((ControlState == 0 && (Key == VK_F5 || Key == VK_F7 || Key == VK_F8)) ||
			(ControlState == PKF_ALT && Key == VK_F5) ||
			(ControlState == PKF_SHIFT && Key == VK_F7) ||
			(ControlState == PKF_SHIFT && Key == VK_F8)
	   ) {
		FarPnl pInfo(this, FCTL_GETPANELINFO);
		if (pInfo.ItemsNumber()) {
			if (m_sm.Find(pInfo[pInfo.CurrentItem()].FindData.lpwszAlternateFileName)) {
				try {
					//PCWSTR	name = sm->Value().dname.c_str();
					WinSvcAction	action(this, ControlState, Key);

					InitDialogItem	InitItems[] = {
						{DI_DOUBLEBOX, 3, 1, 41, 3, 0, 0, 0, 0, GetMsg(txtWaitAMoment)},
						{DI_TEXT,      5, 2, 0,  0, 0, 0, 0, 0, GetMsg(txtActionInProcess)},
					};
					FarDialogItem	Items[sizeofa(InitItems)];
					InitDialogItems(InitItems, Items, sizeofa(InitItems));
					FarDlg	dlg;
					dlg.Execute(psi.ModuleNumber, -1, -1, 45, 5, NULL, Items, sizeofa(Items), 0, 0, DlgProc, (LONG_PTR)&action);
				} catch (WinError e) {
					farebox(e.code());
					return	false;
				}
				try {
					if (tcs == PKF_SHIFT && Key == VK_F8) {
						WinSvc	svc(name(), GENERIC_EXECUTE | DELETE, conn());
						svc.Del();
					}
				} catch (WinError e) {
					farebox(e.code());
					return	false;
				}
			}
			psi.Control(this, FCTL_UPDATEPANEL, 0, NULL);
			psi.Control(this, FCTL_REDRAWPANEL, 0, NULL);
		}
		return	true;
	}
	return	false;
}
int			Panel::SetDirectory(const WCHAR *Dir, int OpMode) {
	if (Eqi(Dir, GetMsg(txtDevices)))
		m_sm.drivers(true);
	else if (Eqi(Dir, L".."))
		m_sm.services(true);
	return	true;
}

//void		Panel::UpdateItems(int ShowOwners, int ShowLinks) {
//}
