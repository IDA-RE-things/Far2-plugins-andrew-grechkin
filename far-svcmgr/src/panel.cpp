#include "farplugin.hpp"
#include "lang.hpp"
#include "guid.hpp"

#include <libwin_net/exception.h>
#include <libwin_net/file.h>
#include <libwin_net/sid.h>
#include <libwin_def/win_def.h>

#include <API_far3/DlgBuilder.hpp>

///======================================================================================= implement
//struct		WinSvcAction {
//	ServicePanel * panel;
//	UINT	ControlState;
//	int		Key;
//
//	WinSvcAction(ServicePanel * s, UINT cs, int k):
//		panel(s),
//		ControlState(cs),
//		Key(k) {
//	}
//};

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
							Copy(startParagraphs, GetDataPtr(hDlg, i + 1), lengthof(startParagraphs));
							break;
						case -2:
							Copy(templateName, GetDataPtr(hDlg, i), lengthof(templateName));
							break;
					}
				}
}
*/

void ToEditDialog(const ServiceInfo &svc, const Far::InitDialogItemF *dialog, FarDialogItem * farDialog, size_t i) {
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
				farDialog[i].Selected = (svc.svc_type() == SERVICE_KERNEL_DRIVER);
				break;
			case txtFileSystemDriver:
				farDialog[i].Selected = (svc.svc_type() == SERVICE_FILE_SYSTEM_DRIVER);
				break;
			case txtOwnProcess:
				farDialog[i].Selected = (svc.svc_type() == SERVICE_WIN32_OWN_PROCESS);
				break;
			case txtSharedProcess:
				farDialog[i].Selected = (svc.svc_type() == SERVICE_WIN32_SHARE_PROCESS);
				break;

			case txtDlgBoot:
				farDialog[i].Selected = (svc.start_type() == SERVICE_BOOT_START);
				break;
			case txtDlgSystem:
				farDialog[i].Selected = (svc.start_type() == SERVICE_SYSTEM_START);
				break;
			case txtDlgAuto:
				farDialog[i].Selected = (svc.start_type() == SERVICE_AUTO_START);
				break;
			case txtDlgManual:
				farDialog[i].Selected = (svc.start_type() == SERVICE_DEMAND_START);
				break;
			case txtDlgDisbld:
				farDialog[i].Selected = (svc.start_type() == SERVICE_DISABLED);
				break;

			case txtDlgIgnore:
				farDialog[i].Selected = (svc.error_control() == SERVICE_ERROR_IGNORE);
				break;
			case txtDlgNormal:
				farDialog[i].Selected = (svc.error_control() == SERVICE_ERROR_NORMAL);
				break;
			case txtDlgSevere:
				farDialog[i].Selected = (svc.error_control() == SERVICE_ERROR_SEVERE);
				break;
			case txtDlgCritical:
				farDialog[i].Selected = (svc.error_control() == SERVICE_ERROR_CRITICAL);
				break;
		}
	}
}

void SetSvcFlags(DWORD &flag, HANDLE hDlg, size_t i, DWORD val) {
//	if (GetCheck(hDlg, i))
//		flag = val;
}

void FromEditDialog(ServiceInfo & svc, const Far::InitDialogItemF *dialog, HANDLE hDlg, size_t i) {
//	while (--i) {
//		switch ((ssize_t)dialog[i].Data) {
//			case txtDlgDisplayName:
//				svc.DName = GetDataPtr(hDlg, i + 1);
//				break;
////			case txtDlgBinaryPath:
////				data.path = GetDataPtr(hDlg, i + 1);
//				break;
//			case txtDlgGroup:
//				svc.OrderGroup = GetDataPtr(hDlg, i + 1);
//				break;
//
//			case txtDriver:
//				SetSvcFlags(svc.Status.dwServiceType, hDlg, i, SERVICE_KERNEL_DRIVER);
//				break;
//			case txtFileSystemDriver:
//				SetSvcFlags(svc.Status.dwServiceType, hDlg, i, SERVICE_FILE_SYSTEM_DRIVER);
//				break;
//			case txtOwnProcess:
//				SetSvcFlags(svc.Status.dwServiceType, hDlg, i, SERVICE_WIN32_OWN_PROCESS);
//				break;
//			case txtSharedProcess:
//				SetSvcFlags(svc.Status.dwServiceType, hDlg, i, SERVICE_WIN32_SHARE_PROCESS);
//				break;
//
//			case txtDlgBoot:
//				SetSvcFlags(svc.StartType, hDlg, i, SERVICE_BOOT_START);
//				break;
//			case txtDlgSystem:
//				SetSvcFlags(svc.StartType, hDlg, i, SERVICE_SYSTEM_START);
//				break;
//			case txtDlgAuto:
//				SetSvcFlags(svc.StartType, hDlg, i, SERVICE_AUTO_START);
//				break;
//			case txtDlgManual:
//				SetSvcFlags(svc.StartType, hDlg, i, SERVICE_DEMAND_START);
//				break;
//			case txtDlgDisbld:
//				SetSvcFlags(svc.StartType, hDlg, i, SERVICE_DISABLED);
//				break;
//
//			case txtDlgIgnore:
//				SetSvcFlags(svc.ErrorControl, hDlg, i, SERVICE_ERROR_IGNORE);
//				break;
//			case txtDlgNormal:
//				SetSvcFlags(svc.ErrorControl, hDlg, i, SERVICE_ERROR_NORMAL);
//				break;
//			case txtDlgSevere:
//				SetSvcFlags(svc.ErrorControl, hDlg, i, SERVICE_ERROR_SEVERE);
//				break;
//			case txtDlgCritical:
//				SetSvcFlags(svc.ErrorControl, hDlg, i, SERVICE_ERROR_CRITICAL);
//				break;
//		}
//	}
}

///========================================================================================= dialogs
LONG_PTR WINAPI DlgProc(HANDLE hDlg, int Msg, int Param1, void * Param2) {
//	static WinSvcAction * action = nullptr;
//	static PCWSTR txt(Far::get_msg(txtActionInProcess));
//	switch (Msg) {
//		case DN_INITDIALOG: {
//			action = (WinSvcAction*)Param2;
//			if ((action->ControlState == 0 && action->Key == VK_F5)) {
//				txt = Far::get_msg(txtStartingService);
//			}
//			if ((action->ControlState == 0 && action->Key == VK_F7)) {
//				txt = Far::get_msg(txtPausingService);
//			}
//			if ((action->ControlState == 0 && action->Key == VK_F8)) {
//				txt = Far::get_msg(txtStoppingService);
//			}
//			if ((action->ControlState == PKF_ALT && action->Key == VK_F5)) {
//				txt = Far::get_msg(txtRestartingService);
//			}
//			if ((action->ControlState == PKF_SHIFT && action->Key == VK_F7)) {
//				txt = Far::get_msg(txtContinueService);
//			}
//			Far::psi().SendDlgMessage(hDlg, DM_SETTEXTPTR, 1, (LONG_PTR)txt);
//			return true;
//		}
//		case DN_KEY: {
//			if (Param2 == KEY_ESC) {
//				Far::psi().SendDlgMessage(hDlg, DM_CLOSE, -1, 0);
//				return true;
//			}
//			return false;
//		}
//		case DN_ENTERIDLE: {
//			try {
//				WinSvc svc(action->panel->name().c_str(), GENERIC_EXECUTE | SERVICE_QUERY_STATUS, action->panel->conn());
//				if ((action->ControlState == 0 && action->Key == VK_F5)) {
//					if (action->panel->state() == SERVICE_PAUSED) {
//						svc.Continue();
//						svc.WaitForState(SERVICE_RUNNING, options.TimeOut);
//					} else {
//						svc.Start();
//						svc.WaitForState(SERVICE_RUNNING, options.TimeOut);
////						svc.WaitForState(SERVICE_RUNNING, Options.TimeOut, ShowWaitState, &dlg);
//					}
//				}
//				if ((action->ControlState == 0 && action->Key == VK_F7)) {
//					svc.Pause();
//					svc.WaitForState(SERVICE_PAUSED, options.TimeOut);
//				}
//				if ((action->ControlState == 0 && action->Key == VK_F8)) {
//					try {
//						svc.Stop();
//						svc.WaitForState(SERVICE_STOPPED, options.TimeOut);
//					} catch (WinError &e) {
//						if (e.code() != ERROR_SERVICE_NOT_ACTIVE)
//							throw;
//					}
//				}
//				if ((action->ControlState == PKF_ALT && action->Key == VK_F5)) {
//					try {
//						svc.Stop();
//						svc.WaitForState(SERVICE_STOPPED, options.TimeOut);
//					} catch (WinError &e) {
//						if (e.code() != ERROR_SERVICE_NOT_ACTIVE)
//							throw;
//					}
//					svc.Start();
//					svc.WaitForState(SERVICE_RUNNING, options.TimeOut);
//				}
//				if ((action->ControlState == PKF_SHIFT && action->Key == VK_F7)) {
//					svc.Continue();
//					svc.WaitForState(SERVICE_RUNNING, options.TimeOut);
////					svc.WaitForState(SERVICE_RUNNING, Options.TimeOut, ShowWaitState, &dlg);
//				}
//				Far::psi().SendDlgMessage(hDlg, DM_CLOSE, -1, 0);
//			} catch (WinError & e) {
//				Far::farebox_code(e.code());
//				Far::psi().SendDlgMessage(hDlg, DM_CLOSE, -1, 0);
//				return false;
//			}
//			break;
//		}
//	}
	return Far::psi().DefDlgProc(hDlg, Msg, Param1, Param2);
}

///==================================================================================== PanelActions
void PanelActions::add(WORD Key, DWORD Control, PCWSTR text, ptrToFunc func, PCWSTR long_text) {
	if (text) {
		FarKey key = {Key, Control};
		KeyBarLabel lab = {key, text, long_text};
		labels.push_back(lab);
	}
	if (func) {
		FarKey key = {Key, Control};
		KeyAction act = {key, func};
		actions.push_back(act);
	}
}

size_t PanelActions::size() const {
	return labels.size();
}

KeyBarLabel * PanelActions::get_labels() {
	return &labels[0];
}

bool PanelActions::exec_func(ServicePanel * panel, WORD Key, DWORD Control) const {
	try {
		for (size_t i = 0; i < actions.size(); ++i) {
			if (Control == actions[i].Key.ControlKeyState && Key == actions[i].Key.VirtualKeyCode) {
				return (panel->*(actions[i].Action))();
			}
		}
	} catch (WinError & e) {
		Far::ebox_code(e.code(), e.where().c_str());
	}
	return false;
}


///==================================================================================== ServicePanel
Far::IPanel * ServicePanel::create(const OpenInfo * /*Info*/) {
	return new ServicePanel;
}

void ServicePanel::destroy() {
	delete this;
}

ServicePanel::ServicePanel():
	m_svcs(&m_conn, false),
	actions(new PanelActions),
	need_recashe(true),
	ViewMode(L'3') {
	actions->add(VK_F1, SHIFT_PRESSED, L"");
	actions->add(VK_F2, SHIFT_PRESSED, L"");
	actions->add(VK_F3, 0, nullptr, &ServicePanel::view);
	actions->add(VK_F3, SHIFT_PRESSED, L"");
	actions->add(VK_F3, LEFT_ALT_PRESSED, L"");
	actions->add(VK_F4, 0, nullptr, &ServicePanel::edit);
	actions->add(VK_F4, SHIFT_PRESSED, Far::get_msg(txtBtnCreate), &ServicePanel::dlg_create_service);
	actions->add(VK_F4, LEFT_ALT_PRESSED, Far::get_msg(txtBtnLogon), &ServicePanel::change_logon);
	actions->add(VK_F5, 0, Far::get_msg(txtBtnStart), &ServicePanel::start);
	actions->add(VK_F5, SHIFT_PRESSED, Far::get_msg(txtBtnStartP), &ServicePanel::restart);
	actions->add(VK_F5, LEFT_ALT_PRESSED, Far::get_msg(txtBtnRestrt), &ServicePanel::restart);
	actions->add(VK_F6, 0, Far::get_msg(txtBtnConnct), &ServicePanel::dlg_connection);
	actions->add(VK_F6, SHIFT_PRESSED, Far::get_msg(txtBtnLocal), &ServicePanel::dlg_local_connection);
	actions->add(VK_F6, LEFT_ALT_PRESSED, L"");
	actions->add(VK_F7, 0, Far::get_msg(txtBtnPause), &ServicePanel::pause);
	actions->add(VK_F7, SHIFT_PRESSED, Far::get_msg(txtBtnContin), &ServicePanel::contin);
	actions->add(VK_F8, 0, Far::get_msg(txtBtnStop), &ServicePanel::stop);
	actions->add(VK_F8, SHIFT_PRESSED, Far::get_msg(txtBtnDelete), &ServicePanel::del);
}

ServicePanel::~ServicePanel() {
}

bool ServicePanel::dlg_connection() {
	WCHAR host[MAX_PATH] = {0};
	WCHAR user[MAX_PATH] = {0};
	WCHAR pass[MAX_PATH] = {0};
	PluginDialogBuilder Builder(Far::psi(), plugin->get_guid(), ConnectionDialogGuid, txtSelectComputer);
	Builder.AddText(txtHost);
	Builder.AddEditField(host, lengthof(host), 32, L"Connect.Host");
	Builder.AddText(txtEmptyForLocal);
	Builder.AddSeparator();
	Builder.AddText(txtLogin);
	Builder.AddEditField(user, lengthof(user), 32, L"Connect.Login");
	Builder.AddText(txtPass);
	Builder.AddPasswordField(pass, lengthof(pass), 32);
	Builder.AddText(txtEmptyForCurrent);
	Builder.AddOKCancel(Far::txtBtnOk, Far::txtBtnCancel);

	while (Builder.ShowDialog()) {
		try {
			m_conn.Open(host, user, pass);
		} catch (WinError & e) {
			Far::ebox_code(e.code());
			continue;
		}
		update();
		redraw();
		return true;
	}
	return false;
}

bool ServicePanel::dlg_local_connection() {
	try {
		m_conn.Open(nullptr);
	} catch (WinError & e) {
		Far::ebox_code(e.code());
		return false;
	}
	update();
	redraw();
	return true;
}

bool ServicePanel::dlg_create_service() {
	WCHAR name[MAX_PATH] = {0};
	WCHAR dname[MAX_PATH] = {0};
	WCHAR path[MAX_PATH_LEN] = {0};
	PluginDialogBuilder Builder(Far::psi(), plugin->get_guid(), CreateServiceDialogGuid, txtDlgCreateService, nullptr);
	Builder.AddText(txtDlgName);
	Builder.AddEditField(name, lengthof(name), 32, L"svcmgr.Name");
	Builder.AddText(txtDlgDisplayName);
	Builder.AddEditField(dname, lengthof(dname), 32, L"svcmgr.DName");
	Builder.AddText(txtDlgBinaryPath);
	Builder.AddEditField(path, lengthof(path), 32, L"svcmgr.Path");
	Builder.AddOKCancel(Far::txtBtnOk, Far::txtBtnCancel);

	while (Builder.ShowDialog()) {
		try {
			WinScm(SC_MANAGER_CREATE_SERVICE, &m_conn).create_service(name, path, SERVICE_DEMAND_START, dname);
		} catch (WinError & e) {
			Far::ebox_code(e.code());
			continue;
		}
		update();
		redraw();
		return true;
	}
	return false;
}

bool ServicePanel::dlg_edit_service(WinServices::iterator & /*it*/) {
//	enum {
//		HEIGHT = 22,
//		WIDTH = 68,
//		indDname = 4,
//		indPath = 6,
//	};
//	bool	isSvc = it->is_service();
//	DWORD	fl4Svc = isSvc ? DIF_DISABLE : 0;
//	DWORD	fl4Dev = isSvc ? 0 : DIF_DISABLE;
//	Far::InitDialogItemF Items[] = {
//		{DI_DOUBLEBOX,   3, 1,   WIDTH - 4, HEIGHT - 2, 0, (PCWSTR)txtDlgServiceProperties},
//		{DI_TEXT,        5, 2,   0,  0,  0, (PCWSTR)txtDlgName},
//		{DI_EDIT,        5, 3,   42, 0,  DIF_READONLY,    it->Name.c_str()},
//		{DI_TEXT,        5, 5,   0,  0,  0, (PCWSTR)txtDlgDisplayName},
//		{DI_EDIT,        5, 6,   42, 0,  DIF_HISTORY,     it->DName.c_str()},
//		{DI_TEXT,        5, 7,   0,  0,  0, (PCWSTR)txtDlgBinaryPath},
//		{DI_EDIT,        5, 8,   42, 0,  DIF_HISTORY,     it->Path.c_str()},
//		{DI_TEXT,        5, 9,   0,  0,  0, (PCWSTR)txtDlgGroup},
//		{DI_EDIT,        5, 10,   42, 0,  DIF_HISTORY,     it->OrderGroup.c_str()},
//		{DI_SINGLEBOX,   5, 12,  42, 17, DIF_LEFTTEXT, (PCWSTR)txtDlgServiceType},
//		{DI_RADIOBUTTON, 7, 13,  40, 0,  fl4Svc, (PCWSTR)txtDriver},
//		{DI_RADIOBUTTON, 7, 14,  40, 0,  fl4Svc, (PCWSTR)txtFileSystemDriver},
//		{DI_RADIOBUTTON, 7, 15,  40, 0,  fl4Dev, (PCWSTR)txtOwnProcess},
//		{DI_RADIOBUTTON, 7, 16,  40, 0,  fl4Dev, (PCWSTR)txtSharedProcess},
//		{DI_BUTTON,     44, 3,   0,  0,  0, (PCWSTR)txtBtnDepends},
//		{DI_SINGLEBOX,   44, 5,  62, 11, DIF_LEFTTEXT, (PCWSTR)txtDlgStartupType},
//		{DI_RADIOBUTTON, 46, 6,  60, 0,  fl4Svc, (PCWSTR)txtDlgBoot},
//		{DI_RADIOBUTTON, 46, 7,  60, 0,  fl4Svc, (PCWSTR)txtDlgSystem},
//		{DI_RADIOBUTTON, 46, 8,  60, 0,  0, (PCWSTR)txtDlgAuto},
//		{DI_RADIOBUTTON, 46, 9,  60, 0,  0, (PCWSTR)txtDlgManual},
//		{DI_RADIOBUTTON, 46, 10, 60, 0,  0, (PCWSTR)txtDlgDisbld},
//		{DI_SINGLEBOX,   44, 12, 62, 17, DIF_LEFTTEXT, (PCWSTR)txtDlgErrorControl},
//		{DI_RADIOBUTTON, 46, 13, 60, 0,  0, (PCWSTR)txtDlgIgnore},
//		{DI_RADIOBUTTON, 46, 14, 60, 0,  0, (PCWSTR)txtDlgNormal},
//		{DI_RADIOBUTTON, 46, 15, 60, 0,  0, (PCWSTR)txtDlgSevere},
//		{DI_RADIOBUTTON, 46, 16, 60, 0,  0, (PCWSTR)txtDlgCritical},
//		{DI_TEXT,      0,  HEIGHT - 4, 0,  0,  DIF_SEPARATOR,   L""},
//		{DI_BUTTON,    0,  HEIGHT - 3, 0,  0,  DIF_CENTERGROUP, (PCWSTR)Far::txtBtnOk},
//		{DI_BUTTON,    0,  HEIGHT - 3, 0,  0,  DIF_CENTERGROUP, (PCWSTR)Far::txtBtnCancel},
//	};
//	size_t	size = lengthof(Items);
//	FarDialogItem FarItems[size];
//	InitDialogItemsF(Items, FarItems, size);
//	ToEditDialog(*it, Items, FarItems, size);
//	FarItems[size - 2].DefaultButton = 1;
//
//	Far::Dialog hDlg;
//	if (hDlg.Init(Far::psi().ModuleNumber, -1, -1, WIDTH, HEIGHT, L"dlgEditService", FarItems, size)) {
//		while (true) {
//			int	ret = hDlg.Run();
//			if (ret > 0 && Items[ret].Data == (PCWSTR)Far::txtBtnOk) {
//				try {
//					WinSvc	svc(it->Name.c_str(), SERVICE_CHANGE_CONFIG, &m_conn);
//					ServiceInfo	&info = *it;
//					PCWSTR	newPath = hDlg.Str(indPath);
//					PCWSTR	newDname = hDlg.Str(indDname);
//					FromEditDialog(info, Items, hDlg, size);
//					CheckApi(::ChangeServiceConfigW(
//								 svc,				// handle of service
//								 info.svc_type(),	// service type
//								 info.StartType,	// service start type
//								 info.ErrorControl,	// error control
//								 Eqi(info.Path.c_str(), newPath) ? nullptr : newPath,
//								 info.OrderGroup.c_str(),
//								 nullptr,				// tag ID: no change
//								 nullptr,				// dependencies: no change
//								 nullptr,				// account name: no change
//								 nullptr,				// password: no change
//								 Eqi(info.DName.c_str(), newDname) ? nullptr : newDname));	    // display name
//				} catch (WinError & e) {
//					Far::farebox_code(e.code());
//					continue;
//				}
//				Far::psi().Control(this, FCTL_UPDATEPANEL, TRUE, nullptr);
//				Far::psi().Control(this, FCTL_REDRAWPANEL, 0, nullptr);
//				return true;
//			} else if (ret > 0 && Items[ret].Data == (PCWSTR)txtBtnDepends) {
//				MenuSelectNewDepend();
//			} else {
//				break;
//			}
//		}
//	}
	return false;
}

bool ServicePanel::dlg_logon_as(Far::Panel & panel) {
	static PCWSTR const NetworkService = L"NT AUTHORITY\\NetworkService";
	static PCWSTR const LocalService = L"NT AUTHORITY\\LocalService";
	static PCWSTR const LocalSystem = L"LocalSystem";

	WCHAR user[MAX_PATH] = {0};
	WCHAR pass[MAX_PATH] = {0};
	static const int logon_types[] = {
		txtDlgNetworkService,
		txtDlgLocalService,
		txtDlgLocalSystem,
		txtDlgUserDefined,
	};
	size_t logonType = 3, allowDesk = 0;

	for (size_t i = 0; i < panel.selected(); ++i) {
		try {
			WinSvc svc(panel.get_selected(i)->CustomColumnData[0], SERVICE_QUERY_CONFIG /*| SERVICE_CHANGE_CONFIG*/, &m_conn);
			Copy(user, svc.get_user().c_str(), lengthof(user));
			if (Eqi(user, NetworkService) || Eqi(user, Sid(WinNetworkServiceSid).get_full_name().c_str()))
				logonType = 0;
			else if (Eqi(user, LocalService) || Eqi(user, Sid(WinLocalServiceSid).get_full_name().c_str()))
				logonType = 1;
			else if (Eqi(user, LocalSystem) || Eqi(user, Sid(WinLocalSystemSid).get_full_name().c_str()))
				logonType = 2;
			allowDesk = WinFlag::Check(svc.get_type(), (DWORD)SERVICE_INTERACTIVE_PROCESS);
			break;
		} catch (WinError & e) {
			// just try to get info from next service
		}
	}
	PluginDialogBuilder Builder(Far::psi(), plugin->get_guid(), LogonAsDialogGuid, txtDlgLogonAs);
	Builder.AddRadioButtons(&logonType, lengthof(logon_types), logon_types);
	Builder.StartSingleBox();
	Builder.AddText(txtLogin);
	Builder.AddEditField(user, lengthof(user), 32);
	Builder.AddText(txtPass);
	Builder.AddPasswordField(pass, lengthof(pass), 32);
	Builder.AddCheckbox(txtDlgAllowDesktop, &allowDesk);
	Builder.EndSingleBox();
	Builder.AddOKCancel(Far::txtBtnOk, Far::txtBtnCancel);
	if (Builder.ShowDialog()) {
		try {
			for (size_t i = panel.selected(); i; --i) {
				WinSvc svc(panel.get_selected(0)->CustomColumnData[0], SERVICE_QUERY_CONFIG | SERVICE_CHANGE_CONFIG, &m_conn);
				ustring	username;
				switch (logonType) {
					case 0:
						username = NetworkService;
						break;
					case 1:
						username = LocalService;
						break;
					case 2:
						username = LocalSystem;
						break;
					case 3:
						username = Find(user, PATH_SEPARATOR) ? ustring(user) : ustring(L".\\") + user;
						break;
				}
				svc.set_logon(username, pass, allowDesk);
				panel.StartSelection();
				panel.clear_selection(0);
				panel.CommitSelection();
			}
		} catch (WinError & e) {
			Far::ebox_code(e.code());
		}
		update();
		redraw();
		return true;
	}
	return false;
}

bool ServicePanel::menu_depends() {
//	return true;
//	enum {
//		CMD_INSERT = 0,
//		CMD_EDIT,
//		CMD_DELETE,
//		CMD_DOWN,
//		CMD_UP,
//		CMD_TEMPSELECT,
//		CMD_SELECT,
//		CMD_CONFIG,
//	};
//	int BreakCode;
//	static const int BreakKeys[] = {
//		VK_INSERT,
//		VK_DELETE,
//		0
//	};
////	int		Result = 0;
//	int		ret = -1;
//	while (true) {
//		if (ret != -1)
//			break;
//		int i = Far::psi().Menu(Far::psi().ModuleNumber, -1, -1, 0,
//						 FMENU_WRAPMODE | FMENU_AUTOHIGHLIGHT,
//						 Far::get_msg(txtMnuTitle), Far::get_msg(txtMnuCommands), L"svcmgr.Depends", BreakKeys, &BreakCode,
//						 nullptr, 0);
//		if (i >= 0) {
//			switch (BreakCode) {
//				case CMD_INSERT:
//				case CMD_EDIT: {
//					Far::farmbox(L"Insert || Edit");
//					break;
//				}
//
//				case CMD_DELETE: {
//					break;
//				}
//
//				default: {
//					Far::farmbox(L"Default");
//					break;
//				}
//			}
//			continue;
//		}
//		break;
//	}
	return true;
}

bool ServicePanel::menu_select_new_depend() {
	return true;
//	enum {
//		CMD_CHANGETYPE = 0,
//	};
//	static const int BreakKeys[] = {
//		VK_F2,
//		0
//	};
//	int		BreakCode;
////	int		Result = 0;
//	int		ret = -1;
//	DWORD	type = m_sm.type();
//
//	WinScm	scm(SC_MANAGER_CONNECT | SC_MANAGER_ENUMERATE_SERVICE, &m_conn);
//	DWORD	BufNeed = 0, NumberOfService = 0;
//	::EnumServicesStatusW(scm, type, SERVICE_STATE_ALL, nullptr,
//						  0, &BufNeed, &NumberOfService, nullptr);
//	CheckApi(::GetLastError() == ERROR_MORE_DATA);
//
//	auto_buf<ENUM_SERVICE_STATUSW*> buf(BufNeed);
//	CheckApi(::EnumServicesStatusW(scm, type, SERVICE_STATE_ALL, buf.data(), buf.size(),
//								   &BufNeed, &NumberOfService, nullptr));
//
//	Far::farebox_code(m_sm.size());
//
//	while (true) {
//		FarMenuItemEx items[] = {
//			{0, L"Qwee", 0, 0, nullptr},
//		};
//		if (ret != -1)
//			break;
//		int i = Far::psi().Menu(Far::psi().ModuleNumber, -1, -1, 0,
//						 FMENU_USEEXT | FMENU_SHOWAMPERSAND | FMENU_WRAPMODE,
//						 Far::get_msg(txtMnuSelectNewTitle), Far::get_msg(txtMnuSelectNewCommands), L"svcmgr.SelectDepend", BreakKeys, &BreakCode,
//						 (const FarMenuItem*)items, lengthof(items));
//		if (i >= 0) {
//			switch (BreakCode) {
//				case CMD_CHANGETYPE: {
//					Far::farmbox(L"F2");
//					break;
//				}
//			}
//			continue;
//		}
//		break;
//	}
	return true;
}

/*
BOOL __stdcall StopDependentServices() {
	DWORD i;
	DWORD dwBytesNeeded;
	DWORD dwCount;

	LPENUM_SERVICE_STATUS   lpDependencies = nullptr;
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

void ServicePanel::GetOpenPanelInfo(OpenPanelInfo * Info) {
//	Far::mbox(L"ServicePanel::GetOpenPanelInfo()");
	Info->StructSize = sizeof(*Info);
	Info->Flags = OPIF_ADDDOTS | OPIF_SHOWNAMESONLY | OPIF_USEATTRHIGHLIGHTING;
	Info->HostFile = nullptr;
	if (m_svcs.is_drivers())
		Info->CurDir = Far::get_msg(txtDevices);
	else
		Info->CurDir = L"";
	Info->Format = plugin->options.Prefix;
	Info->PanelTitle = PanelTitle;
	if (Empty(m_conn.host())) {
		_snwprintf(PanelTitle, lengthof(PanelTitle), L"%s: %s", plugin->options.Prefix, m_conn.host());
	} else {
		_snwprintf(PanelTitle, lengthof(PanelTitle), L"%s", plugin->options.Prefix);
	}
//	Info->StartPanelMode = ViewMode;
	Info->StartSortMode = SM_DEFAULT;
/// PanelModes
	static PCWSTR colTitles3[] = {Far::get_msg(txtClmDisplayName), Far::get_msg(txtClmStatus), Far::get_msg(txtClmStart)};
	static PCWSTR colTitles4[] = {Far::get_msg(txtClmDisplayName), Far::get_msg(txtClmStatus)};
	static PCWSTR colTitles5[] = {nullptr, Far::get_msg(txtClmDisplayName), Far::get_msg(txtClmStatus), Far::get_msg(txtClmStart), nullptr};
	static PCWSTR colTitles6[] = {Far::get_msg(txtClmName), Far::get_msg(txtClmDisplayName)};
	static PCWSTR colTitles7[] = {Far::get_msg(txtClmDisplayName), Far::get_msg(txtClmStatus), nullptr};
	static PCWSTR colTitles8[] = {Far::get_msg(txtClmDisplayName), Far::get_msg(txtClmLogon)};
	static PCWSTR colTitles9[] = {Far::get_msg(txtClmName), Far::get_msg(txtClmStatus), Far::get_msg(txtClmDep)};
	static PCWSTR colTitles0[] = {Far::get_msg(txtClmDisplayName), L"Info", L"Info"};
	static PanelMode CustomPanelModes[] = {
		{sizeof(PanelMode), L"NM,C6,C7", L"0,8,8", colTitles0, L"N", L"0", 0},
		{sizeof(PanelMode), L"N,N,N", L"0,0,0", nullptr, L"N", L"0", 0},
		{sizeof(PanelMode), L"N,N", L"0,0", nullptr, L"N", L"0", 0},
		{sizeof(PanelMode), L"N,C2,C3", L"0,7,6", colTitles3, L"C0", L"0", 0},
		{sizeof(PanelMode), L"N,C2", L"0,7", colTitles4, L"C0,C2", L"0,6", 0},
		{sizeof(PanelMode), L"N,C1,C2,C3,DM", L"0,0,7,6,11", colTitles5, L"C3", L"0", PMFLAGS_FULLSCREEN},
		{sizeof(PanelMode), L"N,C1", L"40%,0", colTitles6, L"C1,C2", L"0,0", 0},
		{sizeof(PanelMode), L"N,C2,Z", L"40%,1,0", colTitles7, L"C3", L"0", PMFLAGS_FULLSCREEN},
		{sizeof(PanelMode), L"N,O", L"0,40%", colTitles8, L"C0", L"0", 0},
		{sizeof(PanelMode), L"N,C2,LN", L"0,7,3", colTitles9, L"N", L"0", 0},
	};
	Info->PanelModesArray = CustomPanelModes;
	Info->PanelModesNumber = lengthof(CustomPanelModes);
////	name;				// C0
////	dname;				// N
////	dwCurrentState;		// C1
////	StartType;			// C2
////	path;				// C3
////	descr;				// Z
////	Dependencies;		// C4
////	OrderGroup;			// C5
////	ServiceStartName;	// C6

	static KeyBarTitles titles = {
		actions->size(),
		actions->get_labels(),
	};
	Info->KeyBar = &titles;
}

int ServicePanel::GetFindData(GetFindDataInfo * Info) try {
	Info->ItemsNumber = 0;
	Info->PanelItem = nullptr;

	cache();
	int i = 0;
	if (m_svcs.is_services()) {
		++i;
		Info->ItemsNumber = m_svcs.size() + 1;
		WinMem::Alloc(Info->PanelItem, sizeof(*Info->PanelItem) * Info->ItemsNumber);
		Info->PanelItem[0].FileName = Far::get_msg(txtDevices);
		Info->PanelItem[0].FileAttributes = FILE_ATTRIBUTE_DIRECTORY;
	} else {
		Info->ItemsNumber = m_svcs.size();
		WinMem::Alloc(Info->PanelItem, sizeof(*Info->PanelItem) * Info->ItemsNumber);
	}

	for (WinServices::iterator it = m_svcs.begin(); it != m_svcs.end(); ++it, ++i) {
		if (is_name_mode()) {
			Info->PanelItem[i].FileName = it->Name.c_str();
			Info->PanelItem[i].AlternateFileName = it->DName.c_str();
		} else {
			Info->PanelItem[i].FileName = it->DName.c_str();
			Info->PanelItem[i].AlternateFileName = it->Name.c_str();
		}
		Info->PanelItem[i].Description = it->Descr.c_str();
		Info->PanelItem[i].Owner = it->ServiceStartName.c_str();
		Info->PanelItem[i].NumberOfLinks = it->Dependencies.size();
		Info->PanelItem[i].FileSize = it->TagId;

		if (it->is_disabled()) {
			Info->PanelItem[i].FileAttributes = FILE_ATTRIBUTE_HIDDEN;
		}
		PCWSTR * CustomColumnData;
		if (WinMem::Alloc(CustomColumnData, 5 * sizeof(PCWSTR))) {
			CustomColumnData[0] = it->Name.c_str();
			CustomColumnData[1] = it->DName.c_str();
			CustomColumnData[2] = state_as_str(it->Status.dwCurrentState);
			CustomColumnData[3] = start_type_as_str(it->StartType);
			CustomColumnData[4] = it->Path.c_str();
			Info->PanelItem[i].CustomColumnData = CustomColumnData;
			Info->PanelItem[i].CustomColumnNumber = 5;
		}
	}
	return true;
} catch (WinError & e) {
	Far::ebox_code(e.code(), e.where().c_str());
	return false;
}

void ServicePanel::FreeFindData(const FreeFindDataInfo * Info) {
//	Far::mbox(L"ServicePanel::FreeFindData()");
	for (size_t i = 0; i < Info->ItemsNumber; ++i) {
		WinMem::Free(Info->PanelItem[i].CustomColumnData);
	}
	WinMem::Free_(Info->PanelItem);
}

int ServicePanel::Compare(const CompareInfo * Info) {
//	Far::mbox(L"ServicePanel::Compare()");
	WinServices::const_iterator it1 = m_svcs.find(Info->Item1->CustomColumnData[0]);
	WinServices::const_iterator it2 = m_svcs.find(Info->Item2->CustomColumnData[0]);
	if (it1 != m_svcs.end() && it2 != m_svcs.end()) {
		if (Info->Mode == SM_NAME || Info->Mode == SM_EXT) {
			if (is_name_mode())
				return Cmpi(it1->Name.c_str(), it2->Name.c_str());
			else
				return Cmpi(it1->DName.c_str(), it2->DName.c_str());
		}
		if (Info->Mode == SM_MTIME) {
			if (it1->svc_state() == it2->svc_state())
				return Cmpi(it1->DName.c_str(), it2->DName.c_str());
			if (it1->svc_state() < it2->svc_state())
				return 1;
			return -1;
		}
		if (Info->Mode == SM_SIZE) {
			if (it1->StartType == it2->StartType)
				return Cmpi(it1->DName.c_str(), it2->DName.c_str());
			if (it1->StartType < it2->StartType)
				return -1;
			return 1;
		}
		return -2;
	}
	return -2;
}

int ServicePanel::SetDirectory(const SetDirectoryInfo * Info) try {
//	Far::mbox(L"1", ustring(__PRETTY_FUNCTION__).c_str());
	if (Eqi(Info->Dir, Far::get_msg(txtDevices))) {
		m_svcs.cache_by_type(WinServices::type_drv);
		need_recashe = false;
	} else if (Eqi(Info->Dir, L"..")) {
		m_svcs.cache_by_type(WinServices::type_svc);
		need_recashe = false;
	}
	return true;
} catch (WinError & e) {
	Far::ebox_code(e.code(), e.where().c_str());
	return false;
}

int ServicePanel::ProcessEvent(const ProcessPanelEventInfo * Info) {
	if (Info->Event == FE_CHANGEVIEWMODE) {
		Far::Panel panel(this);
		if (ViewMode != panel.view_mode()) {
			ViewMode = panel.view_mode();
			need_recashe = false;
			update();
			redraw();
		}
	} else if (Info->Event != FE_IDLE) {
//		Far::mbox(L"Some event", as_str(Info->Event).c_str());
	}
	return false;
}

int ServicePanel::ProcessKey(INPUT_RECORD rec) {
	if (rec.EventType != KEY_EVENT && rec.EventType != FARMACRO_KEY_EVENT)
		return false;

	return actions->exec_func(this, rec.Event.KeyEvent.wVirtualKeyCode, rec.Event.KeyEvent.dwControlKeyState);

//	if (Control == SHIFT_PRESSED && Key == VK_F8) {
//		if (Far::question(Far::get_msg(txtAreYouSure), Far::get_msg(txtDeleteService))) {
//			Control = 0;
//		} else {
//			return false;
//		}
//	}

//	if ((Control == 0 && (Key == VK_F5 || Key == VK_F7 || Key == VK_F8)) ||
//		(Control == LEFT_ALT_PRESSED && Key == VK_F5) ||
//		(Control == SHIFT_PRESSED && (Key == VK_F7 || Key == VK_F8))
//	   ) {
//		Far::Panel info(this, FCTL_GETPANELINFO);
//		if (info.size() && info.selected()) {
//			if (m_svcs.find(pInfo.get_current()->FindData.lpwszAlternateFileName)) {
//				try {
//					WinSvcAction	action(this, ControlState, Key);
//
//					Far::InitDialogItemF InitItems[] = {
//						{DI_DOUBLEBOX, 3, 1, 41, 3, 0, (PCWSTR)txtWaitAMoment},
//						{DI_TEXT,      5, 2, 0,  0, 0, (PCWSTR)txtActionInProcess},
//					};
//					FarDialogItem	Items[lengthof(InitItems)];
//					Far::InitDialogItemsF(InitItems, Items, lengthof(InitItems));
//					Far::Dialog dlg;
//					dlg.Init(Far::psi().ModuleNumber, -1, -1, 45, 5, nullptr, Items, lengthof(Items), 0, 0, DlgProc, (LONG_PTR)&action);
//					dlg.Run();
//				} catch (WinError & e) {
//					Far::ebox_code(e.code());
//					return false;
//				}
//				try {
//					if (tcs == PKF_SHIFT && Key == VK_F8) {
//						WinSvc svc(name().c_str(), GENERIC_EXECUTE | DELETE, conn());
//						svc.Del();
//					}
//				} catch (WinError &e) {
//					Far::ebox_code(e.code());
//					return false;
//				}
//			}
//			update();
//			redraw();
//		}
//		return true;
//	}
	return false;
}

bool ServicePanel::del() {
	Far::Panel info(this, FCTL_GETPANELINFO);
	if (info.size()) {
		for (size_t i = 0; i < info.selected(); ++i) {
			WinServices::const_iterator it = m_svcs.find(info.get_selected(i)->CustomColumnData[0]);
			if (it != m_svcs.end())
				WinSvc::Del(it->Name);
		}
		return true;
	}
	return false;
}

bool ServicePanel::view() {
	Far::Panel info(this, FCTL_GETPANELINFO);
	if (info.size() && info.selected()) {
		WinServices::const_iterator it = m_svcs.find(info.get_selected(0)->CustomColumnData[0]);
		if (it != m_svcs.end()) {
			ustring tmp(TempFile(TempDir()));
			HANDLE hfile = ::CreateFileW(tmp.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
			if (hfile != INVALID_HANDLE_VALUE) {
				File::write(hfile, get_info(it));
				::CloseHandle(hfile);
				Far::psi().Viewer(tmp.c_str(), nullptr, 0, 0, -1, -1,
				           VF_DELETEONLYFILEONCLOSE | VF_ENABLE_F6 | VF_DISABLEHISTORY |
				           VF_NONMODAL | VF_IMMEDIATERETURN, CP_AUTODETECT);
			}
		}
		return true;
	}
	return false;
}

bool ServicePanel::edit() {
	Far::Panel info(this, FCTL_GETPANELINFO);
	if (info.size() && info.selected()) {
		WinServices::iterator it = m_svcs.find(info.get_selected(0)->CustomColumnData[0]);
		if (it != m_svcs.end()) {
			return dlg_edit_service(it);
		}
	}
	return false;
}

bool ServicePanel::change_logon() {
	Far::Panel info(this, FCTL_GETPANELINFO);
	if (m_svcs.is_services() && info.size() && info.selected()) {
		return dlg_logon_as(info);
	}
	return false;
}

bool ServicePanel::pause() {
	Far::Panel info(this, FCTL_GETPANELINFO);
	if (info.size()) {
		for (size_t i = 0; i < info.selected(); ++i) {
			WinServices::const_iterator it = m_svcs.find(info.get_selected(i)->CustomColumnData[0]);
			if (it != m_svcs.end())
				WinSvc::Pause(it->Name);
		}
		return true;
	}
	return false;
}

bool ServicePanel::contin() {
	Far::Panel info(this, FCTL_GETPANELINFO);
	if (info.size()) {
		for (size_t i = 0; i < info.selected(); ++i) {
			WinServices::const_iterator it = m_svcs.find(info.get_selected(i)->CustomColumnData[0]);
			if (it != m_svcs.end())
				WinSvc::Continue(it->Name);
		}
		return true;
	}
	return false;
}

bool ServicePanel::start() {
//	Far::mbox(L"1", ustring(__PRETTY_FUNCTION__).c_str());
	Far::Panel info(this, FCTL_GETPANELINFO);
	if (info.size()) {
		for (size_t i = 0; i < info.selected(); ++i) {
			WinServices::const_iterator it = m_svcs.find(info.get_selected(i)->CustomColumnData[0]);
			if (it != m_svcs.end()) {
				WinSvc::Start(it->Name);
			}
		}
		update();
		redraw();
		return true;
	}
	return false;
}

bool ServicePanel::stop() {
//	Far::mbox(ustring(__PRETTY_FUNCTION__).c_str());
	Far::Panel info(this, FCTL_GETPANELINFO);
	if (info.size()) {
		for (size_t i = 0; i < info.selected(); ++i) {
			WinServices::const_iterator it = m_svcs.find(info.get_selected(i)->CustomColumnData[0]);
			if (it != m_svcs.end())
				WinSvc::Stop(it->Name);
		}
		update();
		redraw();
		return true;
	}
	return false;
}

bool ServicePanel::restart() {
	Far::Panel info(this, FCTL_GETPANELINFO);
	if (info.size()) {
		for (size_t i = 0; i < info.selected(); ++i) {
			WinServices::const_iterator it = m_svcs.find(info.get_selected(i)->CustomColumnData[0]);
			if (it != m_svcs.end())
				WinSvc::Restart(it->Name);
		}
		return true;
	}
	return false;
}

ustring	ServicePanel::get_info(WinServices::const_iterator it) const {
	ustring	Result;
	Result += L"Service name:  ";
		Result += it->Name;
		Result += L"\n\n";
		Result += L"Display name:  ";
		Result += it->DName;
		Result += L"\n\n";
		Result += L"Description:   ";
		Result += it->Descr;
		Result += L"\n\n";
		Result += L"Path:          ";
		Result += it->Path;
		Result += L"\n\n";
//		Result += L"State:         ";
//		Result += GetState(m_sm.Value().dwCurrentState);
//		Result += L"\n\n";
//		Result += L"Startup type:  ";
//		Result += GetStartType(m_sm.Value().StartType);
//		Result += L"\n\n";
//		Result += L"Error control: ";
//		Result += GetErrorControl(m_sm.Value().ErrorControl);
//		Result += L"\n\n";
//		Result += L"Dependencies:  ";
//		for (size_t i = 0; i < m_sm.Value().Dependencies.size(); ++i) {
//			Result += m_sm.Value().Dependencies[i];
//			Result += L"\n               ";
//		}
	return Result;
}

bool ServicePanel::is_name_mode() const {
	return ViewMode == 1 || ViewMode == 2 || ViewMode == 5 || ViewMode == 9;
}

PCWSTR ServicePanel::state_as_str(DWORD state) const {
	return Far::get_msg(state + txtStopped - SERVICE_STOPPED);
}

PCWSTR ServicePanel::start_type_as_str(DWORD start) const {
	return Far::get_msg(start + txtBoot - SERVICE_BOOT_START);
}

PCWSTR ServicePanel::error_control_as_str(DWORD err) const {
	return Far::get_msg(err + txtIgnore - SERVICE_ERROR_IGNORE);
}

void ServicePanel::cache() {
	if (need_recashe) {
//		farmbox(L"Recache");
		m_svcs.cache();
	}
	need_recashe = true;
}
