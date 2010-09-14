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

#include "../../far/farkeys.hpp"

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

/*
	void		FromDialog(InitDialogItemF * dlg, HANDLE hDlg, size_t i) {
				while (--i) {
					if ((dlg[i].Type == DI_RADIOBUTTON) && GetCheck(hDlg, i)) {
						formattingType = (size_t)dlg[i].Data;
					}

					switch ((size_t)dlg[i].Data) {
						case msgLeftMargin:
							leftMargin         = fsf.atoi(GetDataPtr(hDlg, i + 1));
							break;
						case msgRightMargin:
							rightMargin        = fsf.atoi(GetDataPtr(hDlg, i + 1));
							break;
						case msgParagraph:
							paragraphIndent    = fsf.atoi(GetDataPtr(hDlg, i + 1));
							break;
						case msgParagraphPerLine:
							EachLineAsPara   = GetCheck(hDlg, i);
							break;
						case msgSeparateParagraphs:
							EmptyLineAfterPara = GetCheck(hDlg, i);
							break;
						case msgKeepEmpty:
							KeepEmptyLines          = GetCheck(hDlg, i);
							break;
						case msgCatchParagraphs:
							CatchPara    = GetCheck(hDlg, i);
							Copy(startParagraphs, GetDataPtr(hDlg, i + 1), sizeofa(startParagraphs));
							break;
						case -2:
							Copy(templateName, GetDataPtr(hDlg, i), sizeofa(templateName));
							break;
					}
				}
}
*/
void	ToEditDialog(const s_ServiceInfo &data, const InitDialogItemF *dialog, FarDialogItem * farDialog, size_t i) {
	while (--i) {
		switch ((ssize_t)dialog[i].Data) {
			case txtDlgDisplayName:
				farDialog[i+1].History = L"svcmgr.Dname";
				break;
			case txtDlgBinaryPath:
				farDialog[i+1].History = L"svcmgr.Path";
				break;
			case txtDlgGroup:
				farDialog[i+1].History = L"svcmgr.Group";
				break;

			case txtDriver:
				farDialog[i].Selected = (data.dwServiceType == SERVICE_KERNEL_DRIVER);
				break;
			case txtFileSystemDriver:
				farDialog[i].Selected = (data.dwServiceType == SERVICE_FILE_SYSTEM_DRIVER);
				break;
			case txtOwnProcess:
				farDialog[i].Selected = (data.dwServiceType == SERVICE_WIN32_OWN_PROCESS);
				break;
			case txtSharedProcess:
				farDialog[i].Selected = (data.dwServiceType == SERVICE_WIN32_SHARE_PROCESS);
				break;

			case txtDlgBoot:
				farDialog[i].Selected = (data.StartType == SERVICE_BOOT_START);
				break;
			case txtDlgSystem:
				farDialog[i].Selected = (data.StartType == SERVICE_SYSTEM_START);
				break;
			case txtDlgAuto:
				farDialog[i].Selected = (data.StartType == SERVICE_AUTO_START);
				break;
			case txtDlgManual:
				farDialog[i].Selected = (data.StartType == SERVICE_DEMAND_START);
				break;
			case txtDlgDisbld:
				farDialog[i].Selected = (data.StartType == SERVICE_DISABLED);
				break;

			case txtDlgIgnore:
				farDialog[i].Selected = (data.ErrorControl == SERVICE_ERROR_IGNORE);
				break;
			case txtDlgNormal:
				farDialog[i].Selected = (data.ErrorControl == SERVICE_ERROR_NORMAL);
				break;
			case txtDlgSevere:
				farDialog[i].Selected = (data.ErrorControl == SERVICE_ERROR_SEVERE);
				break;
			case txtDlgCritical:
				farDialog[i].Selected = (data.ErrorControl == SERVICE_ERROR_CRITICAL);
				break;
		}
	}
}

void	SetSvcFlags(DWORD &flag, HANDLE hDlg, size_t i, DWORD val) {
	if (GetCheck(hDlg, i))
		flag = val;
}
void	FromEditDialog(s_ServiceInfo &data, const InitDialogItemF *dialog, HANDLE hDlg, size_t i) {
	while (--i) {
		switch ((ssize_t)dialog[i].Data) {
			case txtDlgDisplayName:
				data.dname = GetDataPtr(hDlg, i + 1);
				break;
//			case txtDlgBinaryPath:
//				data.path = GetDataPtr(hDlg, i + 1);
				break;
			case txtDlgGroup:
				data.OrderGroup = GetDataPtr(hDlg, i + 1);
				break;

			case txtDriver:
				SetSvcFlags(data.dwServiceType, hDlg, i, SERVICE_KERNEL_DRIVER);
				break;
			case txtFileSystemDriver:
				SetSvcFlags(data.dwServiceType, hDlg, i, SERVICE_FILE_SYSTEM_DRIVER);
				break;
			case txtOwnProcess:
				SetSvcFlags(data.dwServiceType, hDlg, i, SERVICE_WIN32_OWN_PROCESS);
				break;
			case txtSharedProcess:
				SetSvcFlags(data.dwServiceType, hDlg, i, SERVICE_WIN32_SHARE_PROCESS);
				break;

			case txtDlgBoot:
				SetSvcFlags(data.StartType, hDlg, i, SERVICE_BOOT_START);
				break;
			case txtDlgSystem:
				SetSvcFlags(data.StartType, hDlg, i, SERVICE_SYSTEM_START);
				break;
			case txtDlgAuto:
				SetSvcFlags(data.StartType, hDlg, i, SERVICE_AUTO_START);
				break;
			case txtDlgManual:
				SetSvcFlags(data.StartType, hDlg, i, SERVICE_DEMAND_START);
				break;
			case txtDlgDisbld:
				SetSvcFlags(data.StartType, hDlg, i, SERVICE_DISABLED);
				break;

			case txtDlgIgnore:
				SetSvcFlags(data.ErrorControl, hDlg, i, SERVICE_ERROR_IGNORE);
				break;
			case txtDlgNormal:
				SetSvcFlags(data.ErrorControl, hDlg, i, SERVICE_ERROR_NORMAL);
				break;
			case txtDlgSevere:
				SetSvcFlags(data.ErrorControl, hDlg, i, SERVICE_ERROR_SEVERE);
				break;
			case txtDlgCritical:
				SetSvcFlags(data.ErrorControl, hDlg, i, SERVICE_ERROR_CRITICAL);
				break;
		}
	}
}

///========================================================================================= dialogs
LONG_PTR WINAPI		DlgProc(HANDLE hDlg, int Msg, int Param1, LONG_PTR Param2) {
	static WinSvcAction	*action;
	static PCWSTR		txt = GetMsg(txtActionInProcess);
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
						svc.WaitForState(SERVICE_RUNNING, Options.TimeOut);
					} else {
						svc.Start();
						svc.WaitForState(SERVICE_RUNNING, Options.TimeOut);
//						svc.WaitForState(SERVICE_RUNNING, Options.TimeOut, ShowWaitState, &dlg);
					}
				}
				if ((action->ControlState == 0 && action->Key == VK_F7)) {
					svc.Pause();
					svc.WaitForState(SERVICE_PAUSED, Options.TimeOut);
				}
				if ((action->ControlState == 0 && action->Key == VK_F8)) {
					try {
						svc.Stop();
						svc.WaitForState(SERVICE_STOPPED, Options.TimeOut);
					} catch (WinError &e) {
						if (e.code() != ERROR_SERVICE_NOT_ACTIVE)
							throw;
					}
				}
				if ((action->ControlState == PKF_ALT && action->Key == VK_F5)) {
					try {
						svc.Stop();
						svc.WaitForState(SERVICE_STOPPED, Options.TimeOut);
					} catch (WinError &e) {
						if (e.code() != ERROR_SERVICE_NOT_ACTIVE)
							throw;
					}
					svc.Start();
					svc.WaitForState(SERVICE_RUNNING, Options.TimeOut);
				}
				if ((action->ControlState == PKF_SHIFT && action->Key == VK_F7)) {
					svc.Continue();
					svc.WaitForState(SERVICE_RUNNING, Options.TimeOut);
//					svc.WaitForState(SERVICE_RUNNING, Options.TimeOut, ShowWaitState, &dlg);
				}
				psi.SendDlgMessage(hDlg, DM_CLOSE, -1, 0);
			} catch (WinError &e) {
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
bool		Panel::DlgCreateService() {
	enum {
		HEIGHT = 12,
		WIDTH = 48,

		indName = 2,
		indDname = 4,
		indPath = 6,
	};
	InitDialogItemF Items[] = {
		{DI_DOUBLEBOX, 3, 1, WIDTH - 4, HEIGHT - 2, 0, (PCWSTR)txtDlgCreateService},
		{DI_TEXT,      5, 2, 0,  0,  0, (PCWSTR)txtDlgName},
		{DI_EDIT,      5, 3, 42, 0,  DIF_HISTORY,     L""},
		{DI_TEXT,      5, 4, 0,  0,  0, (PCWSTR)txtDlgDisplayName},
		{DI_EDIT,      5, 5, 42, 0,  DIF_HISTORY,     L""},
		{DI_TEXT,      5, 6, 0,  0,  0, (PCWSTR)txtDlgBinaryPath},
		{DI_EDIT,      5, 7, 42, 0,  DIF_HISTORY,     L""},
		{DI_TEXT,      0,  HEIGHT - 4, 0,  0,  DIF_SEPARATOR,   L""},
		{DI_BUTTON,    0,  HEIGHT - 3, 0,  0,  DIF_CENTERGROUP, (PCWSTR)txtBtnOk},
		{DI_BUTTON,    0,  HEIGHT - 3, 0,  0,  DIF_CENTERGROUP, (PCWSTR)txtBtnCancel},
	};
	size_t	size = sizeofa(Items);
	FarDialogItem FarItems[size];
	InitDialogItemsF(Items, FarItems, size);
	FarItems[size - 2].DefaultButton = 1;
	FarItems[indName].History = L"svcmgr.Name";
	FarItems[indDname].History = L"svcmgr.DName";
	FarItems[indPath].History = L"svcmgr.Path";

	FarDlg hDlg;
	if (hDlg.Init(psi.ModuleNumber, -1, -1, WIDTH, HEIGHT, L"dlgCreateService", FarItems, size)) {
		while (true) {
			int	ret = hDlg.Run();
			if (ret > 0 && Items[ret].Data == (PCWSTR)txtBtnOk) {
				try {
					WinScm	scm(SC_MANAGER_CREATE_SERVICE, conn());
					scm.Create(hDlg.Str(indName), hDlg.Str(indPath), SERVICE_DEMAND_START, hDlg.Str(indDname));
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
bool		Panel::DlgEditSvc() {
	enum {
		HEIGHT = 22,
		WIDTH = 68,
		indDname = 4,
		indPath = 6,
	};
	bool	isSvc = WinFlag::Check(m_sm.Value().dwServiceType, (DWORD)SERVICE_WIN32_OWN_PROCESS) ||
				 WinFlag::Check(m_sm.Value().dwServiceType, (DWORD)SERVICE_WIN32_SHARE_PROCESS);
	DWORD	fl4Svc = isSvc ? DIF_DISABLE : 0;
	DWORD	fl4Dev = isSvc ? 0 : DIF_DISABLE;
	InitDialogItemF Items[] = {
		{DI_DOUBLEBOX,   3, 1,   WIDTH - 4, HEIGHT - 2, 0, (PCWSTR)txtDlgServiceProperties},
		{DI_TEXT,        5, 2,   0,  0,  0, (PCWSTR)txtDlgName},
		{DI_EDIT,        5, 3,   42, 0,  DIF_READONLY,    name()},
		{DI_TEXT,        5, 5,   0,  0,  0, (PCWSTR)txtDlgDisplayName},
		{DI_EDIT,        5, 6,   42, 0,  DIF_HISTORY,     m_sm.Value().dname.c_str()},
		{DI_TEXT,        5, 7,   0,  0,  0, (PCWSTR)txtDlgBinaryPath},
		{DI_EDIT,        5, 8,   42, 0,  DIF_HISTORY,     m_sm.Value().path.c_str()},
		{DI_TEXT,        5, 9,   0,  0,  0, (PCWSTR)txtDlgGroup},
		{DI_EDIT,        5, 10,   42, 0,  DIF_HISTORY,     m_sm.Value().OrderGroup.c_str()},
		{DI_SINGLEBOX,   5, 12,  42, 17, DIF_LEFTTEXT, (PCWSTR)txtDlgServiceType},
		{DI_RADIOBUTTON, 7, 13,  40, 0,  fl4Svc, (PCWSTR)txtDriver},
		{DI_RADIOBUTTON, 7, 14,  40, 0,  fl4Svc, (PCWSTR)txtFileSystemDriver},
		{DI_RADIOBUTTON, 7, 15,  40, 0,  fl4Dev, (PCWSTR)txtOwnProcess},
		{DI_RADIOBUTTON, 7, 16,  40, 0,  fl4Dev, (PCWSTR)txtSharedProcess},
		{DI_BUTTON,     44, 3,   0,  0,  0, (PCWSTR)txtBtnDepends},
		{DI_SINGLEBOX,   44, 5,  62, 11, DIF_LEFTTEXT, (PCWSTR)txtDlgStartupType},
		{DI_RADIOBUTTON, 46, 6,  60, 0,  fl4Svc, (PCWSTR)txtDlgBoot},
		{DI_RADIOBUTTON, 46, 7,  60, 0,  fl4Svc, (PCWSTR)txtDlgSystem},
		{DI_RADIOBUTTON, 46, 8,  60, 0,  0, (PCWSTR)txtDlgAuto},
		{DI_RADIOBUTTON, 46, 9,  60, 0,  0, (PCWSTR)txtDlgManual},
		{DI_RADIOBUTTON, 46, 10, 60, 0,  0, (PCWSTR)txtDlgDisbld},
		{DI_SINGLEBOX,   44, 12, 62, 17, DIF_LEFTTEXT, (PCWSTR)txtDlgErrorControl},
		{DI_RADIOBUTTON, 46, 13, 60, 0,  0, (PCWSTR)txtDlgIgnore},
		{DI_RADIOBUTTON, 46, 14, 60, 0,  0, (PCWSTR)txtDlgNormal},
		{DI_RADIOBUTTON, 46, 15, 60, 0,  0, (PCWSTR)txtDlgSevere},
		{DI_RADIOBUTTON, 46, 16, 60, 0,  0, (PCWSTR)txtDlgCritical},
		{DI_TEXT,      0,  HEIGHT - 4, 0,  0,  DIF_SEPARATOR,   L""},
		{DI_BUTTON,    0,  HEIGHT - 3, 0,  0,  DIF_CENTERGROUP, (PCWSTR)txtBtnOk},
		{DI_BUTTON,    0,  HEIGHT - 3, 0,  0,  DIF_CENTERGROUP, (PCWSTR)txtBtnCancel},
	};
	size_t	size = sizeofa(Items);
	FarDialogItem FarItems[size];
	InitDialogItemsF(Items, FarItems, size);
	ToEditDialog(m_sm.Value(), Items, FarItems, size);
	FarItems[size - 2].DefaultButton = 1;

	FarDlg hDlg;
	if (hDlg.Init(psi.ModuleNumber, -1, -1, WIDTH, HEIGHT, L"dlgEditService", FarItems, size)) {
		while (true) {
			int	ret = hDlg.Run();
			if (ret > 0 && Items[ret].Data == (PCWSTR)txtBtnOk) {
				try {
					WinSvc	svc(name(), SERVICE_CHANGE_CONFIG, conn());
					s_ServiceInfo	&info = m_sm.Value();
					PCWSTR	newPath = hDlg.Str(indPath);
					PCWSTR	newDname = hDlg.Str(indDname);
					FromEditDialog(info, Items, hDlg, size);
					CheckAPI(::ChangeServiceConfigW(
								 svc,				// handle of service
								 info.dwServiceType,// service type
								 info.StartType,	// service start type
								 info.ErrorControl,	// error control
								 Eqi(info.path.c_str(), newPath) ? NULL : newPath,
								 info.OrderGroup,
								 NULL,				// tag ID: no change
								 NULL,				// dependencies: no change
								 NULL,				// account name: no change
								 NULL,				// password: no change
								 Eqi(info.dname.c_str(), newDname) ? NULL : newDname));	    // display name
				} catch (WinError &e) {
					farebox(e.code());
					continue;
				}
				psi.Control(this, FCTL_UPDATEPANEL, TRUE, NULL);
				psi.Control(this, FCTL_REDRAWPANEL, 0, NULL);
				return	true;
			} else if (ret > 0 && Items[ret].Data == (PCWSTR)txtBtnDepends) {
				MenuSelectNewDepend();
			} else {
				break;
			}
		}
	}
	return	false;
}
bool		Panel::DlgLogonAs() {
	try {
		WinSvc	svc(name(), SERVICE_QUERY_CONFIG | SERVICE_CHANGE_CONFIG, conn());
		DWORD	type(svc.GetType());
		AutoUTF	user(svc.GetUser());
		AutoUTF	pass;
		AutoUTF	NetworkService = L"NT AUTHORITY\\NetworkService";
		AutoUTF	LocalService = L"NT AUTHORITY\\LocalService";
		AutoUTF	LocalSystem = L"LocalSystem";
		int	curb = txtDlgUserDefined;
		if (Eqi(user.c_str(), NetworkService.c_str()) || Eqi(user.c_str(), Sid(WinNetworkServiceSid).AsFullName().c_str()))
			curb = txtDlgNetworkService;
		else if (Eqi(user.c_str(), LocalService.c_str()) || Eqi(user.c_str(), Sid(WinLocalServiceSid).AsFullName().c_str()))
			curb = txtDlgLocalService;
		else if (Eqi(user.c_str(), LocalSystem.c_str()) || Eqi(user.c_str(), Sid(WinLocalSystemSid).AsFullName().c_str()))
			curb = txtDlgLocalSystem;

		enum {
			HEIGHT = 14,
			WIDTH = 64,
		};
		InitDialogItemF Items[] = {
			{DI_DOUBLEBOX,   3, 1,  WIDTH - 4, HEIGHT - 2, 0, (PCWSTR)txtDlgLogonAs},
			{DI_RADIOBUTTON, 5, 2,  0,  0,  DIF_GROUP, (PCWSTR)txtDlgNetworkService},
			{DI_RADIOBUTTON, 5, 3,  0,  0,  0, (PCWSTR)txtDlgLocalService},
			{DI_RADIOBUTTON, 5, 4,  0,  0,  0, (PCWSTR)txtDlgLocalSystem},
			{DI_RADIOBUTTON, 5, 7,  0,  0,  0, (PCWSTR)txtDlgUserDefined},
			{DI_TEXT,       10, 8,  0,  0,  0, (PCWSTR)txtLogin},
			{DI_EDIT,       25, 8,  58, 0,  0, user.c_str()},
			{DI_TEXT,       10, 9,  0,  0,  0, (PCWSTR)txtPass},
			{DI_PSWEDIT,    25, 9,  58, 0,  0, L""},
			{DI_CHECKBOX,   10, 5,  0,  0,  0, (PCWSTR)txtDlgAllowDesktop},
			{DI_TEXT,      0,  HEIGHT - 4, 0,  0,  DIF_SEPARATOR,   L""},
			{DI_BUTTON,    0,  HEIGHT - 3, 0,  0,  DIF_CENTERGROUP, (PCWSTR)txtBtnOk},
			{DI_BUTTON,    0,  HEIGHT - 3, 0,  0,  DIF_CENTERGROUP, (PCWSTR)txtBtnCancel},
		};
		size_t	size = sizeofa(Items);
		FarDialogItem FarItems[size];
		InitDialogItemsF(Items, FarItems, size);
		FarItems[size - 2].DefaultButton = 1;
		FarItems[1].Selected = (curb == txtDlgNetworkService);
		FarItems[2].Selected = (curb == txtDlgLocalService);
		FarItems[3].Selected = (curb == txtDlgLocalSystem);
		FarItems[4].Selected = (curb == txtDlgUserDefined);
		FarItems[9].Selected = WinFlag::Check(type, (DWORD)SERVICE_INTERACTIVE_PROCESS);

		FarDlg hDlg;
		if (hDlg.Init(psi.ModuleNumber, -1, -1, 64, 14, L"dlgLogonAs", FarItems, size)) {
			while (true) {
				int	ret = hDlg.Run();
				if (ret > 0 && Items[ret].Data == (PCWSTR)txtBtnOk) {
					try {
						if (hDlg.Check(1))
							user = NetworkService;
						else if (hDlg.Check(2))
							user = LocalService;
						else if (hDlg.Check(3))
							user = LocalSystem;
						else if (hDlg.Check(4)) {
							user = hDlg.Str(6);
							pass = hDlg.Str(8);
							if (!user.Find(PATH_SEPARATOR)) {
								user = L".\\" + user;
							}
						}
						svc.SetLogon(user, pass, hDlg.Check(9));
					} catch (WinError &e) {
						farebox(e.code());
						continue;
					}
				} else {
					break;
				}
			}
		}
		psi.Control(this, FCTL_UPDATEPANEL, TRUE, NULL);
		psi.Control(this, FCTL_REDRAWPANEL, 0, NULL);
		return	true;
	} catch (WinError &e) {
		farebox(e.code());
	}
	return	false;
}
bool		Panel::MenuDepends() {
	enum {
		CMD_INSERT = 0,
		CMD_EDIT,
		CMD_DELETE,
		CMD_DOWN,
		CMD_UP,
		CMD_TEMPSELECT,
		CMD_SELECT,
		CMD_CONFIG,
	};
	int BreakCode;
	static const int BreakKeys[] = {
		VK_INSERT,
		VK_DELETE,
		0
	};
	int		Result = 0;
	int		ret = -1;
	while (true) {
		if (ret != -1)
			break;
		int i = psi.Menu(psi.ModuleNumber, -1, -1, 0,
						 FMENU_WRAPMODE | FMENU_AUTOHIGHLIGHT,
						 GetMsg(txtMnuTitle), GetMsg(txtMnuCommands), L"svcmgr.Depends", BreakKeys, &BreakCode,
						 NULL, 0);
		if (i >= 0) {
			switch (BreakCode) {
				case CMD_INSERT:
				case CMD_EDIT: {
					farmbox(L"Insert || Edit");
					break;
				}

				case CMD_DELETE: {
					break;
				}

				default: {
					farmbox(L"Default");
					break;
				}
			}
			continue;
		}
		break;
	}
	return	true;
}
bool		Panel::MenuSelectNewDepend() {
	enum {
		CMD_CHANGETYPE = 0,
	};
	static const int BreakKeys[] = {
		VK_F2,
		0
	};
	int		BreakCode;
	int		Result = 0;
	int		ret = -1;
	DWORD	type = m_sm.type();

	WinScm	scm(SC_MANAGER_CONNECT | SC_MANAGER_ENUMERATE_SERVICE, &m_conn);
	DWORD	BufNeed = 0, NumberOfService = 0;
	::EnumServicesStatusW(scm, type, SERVICE_STATE_ALL, NULL,
						  0, &BufNeed, &NumberOfService, NULL);
	CheckAPI(::GetLastError() == ERROR_MORE_DATA);

	WinBuf<ENUM_SERVICE_STATUSW> buf(BufNeed, true);
	CheckAPI(::EnumServicesStatusW(scm, type, SERVICE_STATE_ALL, buf, buf.size(),
								   &BufNeed, &NumberOfService, NULL));

	farebox(m_sm.Size());

	while (true) {
		FarMenuItemEx items[] = {
			{0, L"Qwee", 0, 0, NULL},
		};
		if (ret != -1)
			break;
		int i = psi.Menu(psi.ModuleNumber, -1, -1, 0,
						 FMENU_USEEXT | FMENU_SHOWAMPERSAND | FMENU_WRAPMODE,
						 GetMsg(txtMnuSelectNewTitle), GetMsg(txtMnuSelectNewCommands), L"svcmgr.SelectDepend", BreakKeys, &BreakCode,
						 (const FarMenuItem*)items, sizeofa(items));
		if (i >= 0) {
			switch (BreakCode) {
				case CMD_CHANGETYPE: {
					farmbox(L"F2");
					break;
				}
			}
			continue;
		}
		break;
	}
	return	true;
}

/*
BOOL __stdcall StopDependentServices() {
	DWORD i;
	DWORD dwBytesNeeded;
	DWORD dwCount;

	LPENUM_SERVICE_STATUS   lpDependencies = NULL;
	ENUM_SERVICE_STATUS     ess;
	SC_HANDLE               hDepService;
	SERVICE_STATUS_PROCESS  ssp;

	DWORD dwStartTime = GetTickCount();
	DWORD dwTimeout = 30000; // 30-second time-out

	// Pass a zero-length buffer to get the required buffer size.
	if (EnumDependentServices(schService, SERVICE_ACTIVE,
							  lpDependencies, 0, &dwBytesNeeded, &dwCount)) {
		// If the Enum call succeeds, then there are no dependent
		// services, so do nothing.
		return TRUE;
	} else {
		if (GetLastError() != ERROR_MORE_DATA)
			return FALSE; // Unexpected error

		// Allocate a buffer for the dependencies.
		lpDependencies = (LPENUM_SERVICE_STATUS) HeapAlloc(
							 GetProcessHeap(), HEAP_ZERO_MEMORY, dwBytesNeeded);

		if (!lpDependencies)
			return FALSE;

		__try {
			// Enumerate the dependencies.
			if (!EnumDependentServices(schService, SERVICE_ACTIVE,
			lpDependencies, dwBytesNeeded, &dwBytesNeeded,
			&dwCount))
				return FALSE;

			for (i = 0; i < dwCount; i++) {
				ess = *(lpDependencies + i);
				// Open the service.
				hDepService = OpenService(schSCManager,
										  ess.lpServiceName,
										  SERVICE_STOP | SERVICE_QUERY_STATUS);

				if (!hDepService)
					return FALSE;

				__try {
					// Send a stop code.
					if (!ControlService(hDepService,
					SERVICE_CONTROL_STOP,
					(LPSERVICE_STATUS) &ssp))
						return FALSE;

					// Wait for the service to stop.
					while (ssp.dwCurrentState != SERVICE_STOPPED) {
						Sleep(ssp.dwWaitHint);
						if (!QueryServiceStatusEx(
									hDepService,
									SC_STATUS_PROCESS_INFO,
									(LPBYTE)&ssp,
									sizeof(SERVICE_STATUS_PROCESS),
									&dwBytesNeeded))
							return FALSE;

						if (ssp.dwCurrentState == SERVICE_STOPPED)
							break;

						if (GetTickCount() - dwStartTime > dwTimeout)
							return FALSE;
					}
				}
				__finally {
					// Always release the service handle.
					CloseServiceHandle(hDepService);
				}
			}
		}
		__finally {
			// Always free the enumeration buffer.
			HeapFree(GetProcessHeap(), 0, lpDependencies);
		}
	}
	return TRUE;
}
*/

void		Panel::GetOpenPluginInfo(OpenPluginInfo *Info) {
//	farmbox(L"GetOpenPluginInfo");
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
		{L"N,O", L"0,40%", ColumnTitles8, false, TRUE, TRUE, TRUE, L"C0", L"0", {0, 0}},
		{L"N,C2,LN", L"0,7,3", ColumnTitles9, false, TRUE, TRUE, TRUE, L"N", L"0", {0, 0}},
	};
	Info->PanelModesArray = CustomPanelModes;
	Info->PanelModesNumber = sizeofa(CustomPanelModes);
	Info->StartPanelMode = L'3';
//	name;				// C0
//	dname;				// N
//	dwCurrentState;		// C1
//	StartType;			// C2
//	path;				// C3
//	descr;				// Z
//	Dependencies;		// C4
//	OrderGroup;			// C5
//	ServiceStartName;	// C6

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
	keybartitles.AltTitles[3] = (PWSTR)GetMsg(txtBtnLogon);
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
			AutoUTF	tmp(TempFile(TempDir()));
			HANDLE	hfile = ::CreateFile(tmp.c_str(), GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			if (hfile != INVALID_HANDLE_VALUE) {
				FileWrite(hfile, Info());
				::CloseHandle(hfile);
				psi.Viewer(tmp, NULL, 0, 0, -1, -1,
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
		} catch (WinError &e) {
			farebox(e.code());
		}
		psi.Control(this, FCTL_UPDATEPANEL, 0, NULL);
		psi.Control(this, FCTL_REDRAWPANEL, 0, NULL);
		return	true;
	}
	if (ControlState == PKF_ALT && Key == VK_F4) {
		FarPnl pInfo(this, FCTL_GETPANELINFO);
		if (m_sm.services() && pInfo.ItemsNumber()) {
			if (m_sm.Find(pInfo[pInfo.CurrentItem()].FindData.lpwszAlternateFileName)) {
				DlgLogonAs();
			}
		}
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
					WinSvcAction	action(this, ControlState, Key);

					InitDialogItemF	InitItems[] = {
						{DI_DOUBLEBOX, 3, 1, 41, 3, 0, (PCWSTR)txtWaitAMoment},
						{DI_TEXT,      5, 2, 0,  0, 0, (PCWSTR)txtActionInProcess},
					};
					FarDialogItem	Items[sizeofa(InitItems)];
					InitDialogItemsF(InitItems, Items, sizeofa(InitItems));
					FarDlg	dlg;
					dlg.Init(psi.ModuleNumber, -1, -1, 45, 5, NULL, Items, sizeofa(Items), 0, 0, DlgProc, (LONG_PTR)&action);
					dlg.Run();
				} catch (WinError &e) {
					farebox(e.code());
					return	false;
				}
				try {
					if (tcs == PKF_SHIFT && Key == VK_F8) {
						WinSvc	svc(name(), GENERIC_EXECUTE | DELETE, conn());
						svc.Del();
					}
				} catch (WinError &e) {
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
