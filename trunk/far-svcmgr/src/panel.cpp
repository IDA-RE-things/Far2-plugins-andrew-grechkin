/**
	svcmgr: Manage services
	Allow to manage windows services
	FAR3 plugin

	© 2012 Andrew Grechkin

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

#include "guid.hpp"
#include "farplugin.hpp"
#include "lang.hpp"

#include <libbase/std.hpp>
#include <libbase/path.hpp>
#include <libext/exception.hpp>
#include <libext/file.hpp>
#include <libext/sid.hpp>

#include <libfar3/DlgBuilder.hpp>

using namespace Base;
using namespace Ext;

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

ssize_t svc_type_to_radio_button(DWORD svc_type) {
	ssize_t ret = 0;
	if (svc_type == SERVICE_KERNEL_DRIVER)
		ret = 0;
	else if (svc_type == SERVICE_FILE_SYSTEM_DRIVER)
		ret = 1;
	else if (svc_type == SERVICE_WIN32_OWN_PROCESS)
		ret = 2;
	else if (svc_type == SERVICE_WIN32_SHARE_PROCESS)
		ret = 3;
	return ret;
}

ssize_t svc_start_type_to_radio_button(DWORD svc_start_type) {
	return svc_start_type;
}

ssize_t svc_error_control_to_radio_button(DWORD svc_error_control) {
	return svc_error_control;
}

DWORD radio_button_to_svc_type(ssize_t btn_index) {
	DWORD ret = 0;
	switch (btn_index) {
		case 0:
			ret = SERVICE_KERNEL_DRIVER;
			break;
		case 1:
			ret = SERVICE_FILE_SYSTEM_DRIVER;
			break;
		case 2:
			ret = SERVICE_WIN32_OWN_PROCESS;
			break;
		case 3:
			ret = SERVICE_WIN32_SHARE_PROCESS;
			break;
	}
	return ret;
}

///==================================================================================== ServicePanel
Far::IPanel * ServicePanel::create(const OpenInfo * /*Info*/) {
	return new ServicePanel;
}

void ServicePanel::destroy() {
	delete this;
}

ServicePanel::ServicePanel():
	m_conn(new RemoteConnection),
	m_svcs(m_conn.get()),
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
	Far::DialogBuilder Builder = Far::get_dialog_builder(ConnectionDialogGuid, Far::get_msg(txtSelectComputer), nullptr);
	Builder->add_text(Far::get_msg(txtHost));
	Builder->add_editfield(host, lengthof(host), 32, L"Connect.Host");
	Builder->add_text(Far::get_msg(txtEmptyForLocal));
	Builder->add_separator();
	Builder->add_text(Far::get_msg(txtLogin));
	Builder->add_editfield(user, lengthof(user), 32, L"Connect.Login");
	Builder->add_text(Far::get_msg(txtPass));
	Builder->add_passwordfield(pass, lengthof(pass), 32);
	Builder->add_text(Far::get_msg(txtEmptyForCurrent));
	Builder->add_OKCancel(Far::get_msg(Far::txtBtnOk), Far::get_msg(Far::txtBtnCancel));

	while (Builder->show()) {
		try {
			LogTrace();
			Base::shared_ptr<RemoteConnection> conn(new RemoteConnection(host, user, pass));
			m_svcs.cache(conn.get());
			m_conn.swap(conn);
			need_recashe = false;
			update();
			redraw();
			break;
		} catch (AbstractError & e) {
			LogTrace();
			vector<ustring> msg;
			e.format_error(msg);
			Far::ebox(msg);
			continue;
		}
	}
	return true;
}

bool ServicePanel::dlg_local_connection() {
	try {
		Base::shared_ptr<RemoteConnection> conn(new RemoteConnection);
		m_svcs.cache(conn.get());
		m_conn.swap(conn);
		need_recashe = false;
		update();
		redraw();
	} catch (AbstractError & e) {
		vector<ustring> msg;
		e.format_error(msg);
		Far::ebox(msg);
		return false;
	}
	return true;
}

bool ServicePanel::dlg_create_service() {
	LogTrace();
	WCHAR name[MAX_PATH] = {0};
	WCHAR dname[MAX_PATH] = {0};
	WCHAR path[MAX_PATH_LEN] = {0};
	Far::DialogBuilder Builder = Far::get_dialog_builder(CreateServiceDialogGuid, Far::get_msg(txtDlgCreateService), nullptr);
	Builder->add_text(Far::get_msg(txtDlgName));
	Builder->add_editfield(name, lengthof(name), 32, L"svcmgr.Name");
	Builder->add_text(Far::get_msg(txtDlgDisplayName));
	Builder->add_editfield(dname, lengthof(dname), 32, L"svcmgr.DName");
	Builder->add_text(Far::get_msg(txtDlgBinaryPath));
	Builder->add_editfield(path, lengthof(path), 32, L"svcmgr.Path");
	Builder->add_OKCancel(Far::get_msg(Far::txtBtnOk), Far::get_msg(Far::txtBtnCancel));

	while (Builder->show()) {
		try {
			WinScm(SC_MANAGER_CREATE_SERVICE, m_conn.get()).create_service(name, path, SERVICE_DEMAND_START, dname);
			update();
			redraw();
			break;
		} catch (AbstractError & e) {
			Far::ebox_code(e.code());
			continue;
		}
	}
	return true;
}

bool ServicePanel::dlg_edit_service(WinServices::iterator & it) {
	LogTrace();
	WCHAR name[MAX_PATH] = {0};
	WCHAR dname[MAX_PATH] = {0};
	WCHAR path[MAX_PATH_LEN] = {0};
	WCHAR group[MAX_PATH] = {0};

	copy_str(name, it->Name.c_str(), lengthof(name));
	copy_str(dname, it->DName.c_str(), lengthof(dname));
	copy_str(path, it->Path.c_str(), lengthof(path));
	copy_str(group, it->OrderGroup.c_str(), lengthof(group));

	Far::DialogBuilder Builder = Far::get_dialog_builder(EditServiceDialogGuid, Far::get_msg(txtDlgServiceProperties), nullptr);
	Builder->start_column();
	Builder->add_text(Far::get_msg(txtDlgName));
	Builder->add_editfield(name, lengthof(name), 32, L"svcmgr.Name")->Flags |= DIF_READONLY;
	Builder->add_empty_line();
	Builder->add_text(Far::get_msg(txtDlgDisplayName));
	Builder->add_editfield(dname, lengthof(dname), 32, L"svcmgr.DName");
	Builder->add_text(Far::get_msg(txtDlgBinaryPath));
	Builder->add_editfield(path, lengthof(path), 32, L"svcmgr.Path");
	Builder->add_text(Far::get_msg(txtDlgGroup));
	Builder->add_editfield(group, lengthof(group), 32, L"svcmgr.Group");
	Builder->add_empty_line();
	Builder->start_singlebox(32, Far::get_msg(txtDlgServiceType), true);
	FARDIALOGITEMFLAGS fl4Dev = it->is_service() ? DIF_DISABLE : DIF_NONE;
	FARDIALOGITEMFLAGS fl4Svc = it->is_service() ? DIF_NONE : DIF_DISABLE;
	Far::AddRadioButton_t svc_types[] = {
		{txtDriver, fl4Dev},
		{txtFileSystemDriver, fl4Dev},
		{txtOwnProcess, fl4Svc},
		{txtSharedProcess, fl4Svc},
	};
	ssize_t svc_type = svc_type_to_radio_button(it->Status.dwServiceType);
	Builder->add_radiobuttons(&svc_type, lengthof(svc_types), svc_types);
	Builder->end_singlebox();
	Builder->break_column();
	Builder->add_empty_line();
	Builder->add_empty_line();
	Builder->add_empty_line();
	Builder->start_singlebox(20, Far::get_msg(txtDlgStartupType), true);
	Far::AddRadioButton_t start_types[] = {
		{txtDlgBoot, fl4Dev},
		{txtDlgSystem, DIF_NONE},
		{txtDlgAuto, DIF_NONE},
		{txtDlgManual, DIF_NONE},
		{txtDlgDisbld, DIF_NONE},
	};
	ssize_t start_type = svc_start_type_to_radio_button(it->StartType);
	Builder->add_radiobuttons(&start_type, lengthof(start_types), start_types);
	Builder->end_singlebox();

	Builder->start_singlebox(20, Far::get_msg(txtDlgErrorControl), true);
	Far::AddRadioButton_t error_controls[] = {
		{txtDlgIgnore, DIF_NONE},
		{txtDlgNormal, DIF_NONE},
		{txtDlgSevere, DIF_NONE},
		{txtDlgCritical, DIF_NONE},
	};
	ssize_t error_control = svc_error_control_to_radio_button(it->ErrorControl);
	Builder->add_radiobuttons(&error_control, lengthof(error_controls), error_controls);
	Builder->end_singlebox();
	Builder->end_column();

	Builder->add_OKCancel(Far::get_msg(Far::txtBtnOk), Far::get_msg(Far::txtBtnCancel));

	while (Builder->show()) {
		LogTrace();
		try {
			WinSvc svc(it->Name.c_str(), SERVICE_CHANGE_CONFIG, m_conn.get());
			CheckApi(::ChangeServiceConfigW(
				svc,			// handle of service
				radio_button_to_svc_type(svc_type),		// service type
				start_type,		// service start type
				error_control,	// error control
				compare_str_cs(it->Path.c_str(), path) == 0 ? nullptr : path,
				compare_str_cs(it->OrderGroup.c_str(), group) == 0 ? nullptr : group,
				nullptr,			// tag ID: no change
				nullptr,			// dependencies: no change
				nullptr,			// account name: no change
				nullptr,			// password: no change
				compare_str_cs(it->DName.c_str(), dname) == 0 ? nullptr : dname));	// display name
			update();
			redraw();
			break;
		} catch (AbstractError & e) {
			LogTrace();
			Far::ebox_code(e.code());
			continue;
		}
	}
	return true;
}

bool ServicePanel::dlg_logon_as(Far::Panel & panel) {
	LogTrace();
	static PCWSTR const NetworkService = L"NT AUTHORITY\\NetworkService";
	static PCWSTR const LocalService = L"NT AUTHORITY\\LocalService";
	static PCWSTR const LocalSystem = L"LocalSystem";

	WCHAR user[MAX_PATH] = {0};
	WCHAR pass[MAX_PATH] = {0};
	static const Far::AddRadioButton_t logon_types[] = {
		{txtDlgNetworkService, DIF_NONE},
		{txtDlgLocalService, DIF_NONE},
		{txtDlgLocalSystem, DIF_NONE},
		{txtDlgUserDefined, DIF_NONE},
	};
	ssize_t logonType = 3, allowDesk = 0;

	for (size_t i = 0; i < panel.selected(); ++i) {
		try {
			WinSvc svc(panel.get_selected(i)->CustomColumnData[0], SERVICE_QUERY_CONFIG /*| SERVICE_CHANGE_CONFIG*/, m_conn.get());
			copy_str(user, svc.get_user().c_str(), lengthof(user));
			if (compare_str_ic(user, NetworkService) == 0 || compare_str_ic(user, Sid(WinNetworkServiceSid).get_full_name().c_str()) == 0)
				logonType = 0;
			else if (compare_str_ic(user, LocalService) == 0 || compare_str_ic(user, Sid(WinLocalServiceSid).get_full_name().c_str()) == 0)
				logonType = 1;
			else if (compare_str_ic(user, LocalSystem) == 0 || compare_str_ic(user, Sid(WinLocalSystemSid).get_full_name().c_str()) == 0)
				logonType = 2;
			allowDesk = svc.get_type() & SERVICE_INTERACTIVE_PROCESS;
			break;
		} catch (AbstractError & e) {
			// just try to get info from next service
		}
	}
	Far::DialogBuilder Builder = Far::get_dialog_builder(LogonAsDialogGuid, Far::get_msg(txtDlgLogonAs), nullptr);
	Builder->add_radiobuttons(&logonType, lengthof(logon_types), logon_types);
	Builder->start_singlebox(52);
	Builder->add_text(Far::get_msg(txtLogin));
	Builder->add_editfield(user, lengthof(user), 49);
	Builder->add_text(Far::get_msg(txtPass));
	Builder->add_passwordfield(pass, lengthof(pass), 49);
	Builder->add_checkbox(Far::get_msg(txtDlgAllowDesktop), &allowDesk);
	Builder->end_singlebox();
	Builder->add_OKCancel(Far::get_msg(Far::txtBtnOk), Far::get_msg(Far::txtBtnCancel));
	if (Builder->show()) {
		try {
			for (size_t i = panel.selected(); i; --i) {
				WinSvc svc(panel.get_selected(0)->CustomColumnData[0], SERVICE_QUERY_CONFIG | SERVICE_CHANGE_CONFIG, m_conn.get());
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
						username = find_str(user, PATH_SEPARATOR) ? ustring(user) : ustring(L".\\") + user;
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
	}
	return true;
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
	if (is_str_empty(m_conn->get_host())) {
		_snwprintf(PanelTitle, lengthof(PanelTitle), L"%s", plugin->options.Prefix);
	} else {
		_snwprintf(PanelTitle, lengthof(PanelTitle), L"%s: %s", plugin->options.Prefix, m_conn->get_host());
	}
	Info->StartPanelMode = ViewMode;
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
		{sizeof(PanelMode), L"NM,C6,C7", L"0,8,8", colTitles0, L"C1", L"0", 0},
		{sizeof(PanelMode), L"N,N,N", L"0,0,0", nullptr, L"C1", L"0", 0},
		{sizeof(PanelMode), L"N,N", L"0,0", nullptr, L"C1", L"0", 0},
		{sizeof(PanelMode), L"N,C2,C3", L"0,7,6", colTitles3, L"C0", L"0", 0},
		{sizeof(PanelMode), L"N,C2", L"0,7", colTitles4, L"C0,C2", L"0,6", 0},
		{sizeof(PanelMode), L"N,C1,C2,C3,DM", L"0,0,7,6,11", colTitles5, L"C3", L"0", PMFLAGS_FULLSCREEN},
		{sizeof(PanelMode), L"N,C1", L"40%,0", colTitles6, L"C2,C3", L"0,0", 0},
		{sizeof(PanelMode), L"N,C2,Z", L"40%,1,0", colTitles7, L"C3", L"0", PMFLAGS_FULLSCREEN},
		{sizeof(PanelMode), L"N,O", L"0,40%", colTitles8, L"C0", L"0", 0},
		{sizeof(PanelMode), L"N,C2,LN", L"0,7,3", colTitles9, L"N", L"0", 0},
	};
	Info->PanelModesArray = CustomPanelModes;
	Info->PanelModesNumber = lengthof(CustomPanelModes);
////	name;				// C0
////	dname;				// C1
////	dwCurrentState;		// C2
////	StartType;			// C3
////	path;				// C4
////	descr;				// Z
////	Dependencies;		//
////	OrderGroup;			//
////	ServiceStartName;	//

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
		Memory::alloc(Info->PanelItem, sizeof(*Info->PanelItem) * Info->ItemsNumber);
		Info->PanelItem[0].FileName = Far::get_msg(txtDevices);
		Info->PanelItem[0].AlternateFileName = Far::get_msg(txtDevices);
		Info->PanelItem[0].FileAttributes = FILE_ATTRIBUTE_DIRECTORY;
	} else {
		Info->ItemsNumber = m_svcs.size();
		Memory::alloc(Info->PanelItem, sizeof(*Info->PanelItem) * Info->ItemsNumber);
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
		if (Memory::alloc(CustomColumnData, 5 * sizeof(PCWSTR))) {
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
		Memory::free(Info->PanelItem[i].CustomColumnData);
	}
	Memory::free_v(Info->PanelItem);
}

int ServicePanel::Compare(const CompareInfo * Info) {
//	Far::mbox(L"ServicePanel::Compare()");
	WinServices::const_iterator it1 = m_svcs.find(Info->Item1->CustomColumnData[0]);
	WinServices::const_iterator it2 = m_svcs.find(Info->Item2->CustomColumnData[0]);
	if (it1 != m_svcs.end() && it2 != m_svcs.end()) {
		if (Info->Mode == SM_NAME || Info->Mode == SM_EXT) {
			if (is_name_mode())
				return compare_str_ic(it1->Name.c_str(), it2->Name.c_str());
			else
				return compare_str_ic(it1->DName.c_str(), it2->DName.c_str());
		}
		if (Info->Mode == SM_MTIME) {
			if (it1->svc_state() == it2->svc_state())
				return compare_str_ic(it1->DName.c_str(), it2->DName.c_str());
			if (it1->svc_state() < it2->svc_state())
				return 1;
			return -1;
		}
		if (Info->Mode == SM_SIZE) {
			if (it1->StartType == it2->StartType)
				return compare_str_ic(it1->DName.c_str(), it2->DName.c_str());
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
	if (compare_str_ic(Info->Dir, Far::get_msg(txtDevices)) == 0) {
		m_svcs.cache_by_type(WinServices::type_drv);
		need_recashe = false;
	} else if (compare_str_ic(Info->Dir, L"..") == 0) {
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
//			redraw();
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
	if (Far::question(Far::get_msg(txtAreYouSure), Far::get_msg(txtDeleteService))) {
		Far::Panel info(this, FCTL_GETPANELINFO);
		if (info.size()) {
			Far::ProgressWindow pw(info.size(), L"Deleting");
			for (size_t i = 0; i < info.selected(); ++i) {
				pw.set_name(i + 1, info.get_selected(i)->CustomColumnData[0]);
				m_svcs.del(info.get_selected(i)->CustomColumnData[0]);
			}
			update();
			redraw();
			return true;
		}
	}
	return false;
}

bool ServicePanel::view() {
	Far::Panel info(this, FCTL_GETPANELINFO);
	if (info.size() && info.selected()) {
		WinServices::const_iterator it = m_svcs.find(info.get_current()->CustomColumnData[0]);
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
		WinServices::iterator it = m_svcs.find(info.get_current()->CustomColumnData[0]);
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
				WinSvc::Pause(it->Name, m_conn.get());
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
				WinSvc::Continue(it->Name, m_conn.get());
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
				WinSvc::Start(it->Name, m_conn.get());
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
				WinSvc::Stop(it->Name, m_conn.get());
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
				WinSvc::Restart(it->Name, m_conn.get());
		}
		return true;
	}
	return false;
}

ustring	ServicePanel::get_info(WinServices::const_iterator it) const {
	ustring	Result;
	Result += Far::get_msg(infoServiceName);
		Result += it->Name;
		Result += L"\n\n";
		Result += Far::get_msg(infoDisplayName);
		Result += it->DName;
		Result += L"\n\n";
		Result += Far::get_msg(infoDescription);
		Result += it->Descr;
		Result += L"\n\n";
		Result += Far::get_msg(infoPath);
		Result += it->Path;
		Result += L"\n\n";
		Result += Far::get_msg(infoState);
		Result += state_as_str(it->svc_state());
		Result += L"\n\n";
		Result += Far::get_msg(infoStartupType);
		Result += start_type_as_str(it->start_type());
		Result += L"\n\n";
		Result += Far::get_msg(infoErrorControl);
		Result += error_control_as_str(it->error_control());
		Result += L"\n\n";
		Result += Far::get_msg(infoOrderGroup);
		Result += it->OrderGroup;
		Result += L"\n\n";
		Result += Far::get_msg(infoStartName);
		Result += it->ServiceStartName;
		Result += L"\n\n";
		Result += Far::get_msg(infoTag);
		Result += as_str(it->TagId);
		Result += L"\n\n";
		Result += Far::get_msg(infoDependencies);
		for (size_t i = 0; i < it->Dependencies.size(); ++i) {
			Result += it->Dependencies[i];
			Result += L"\n               ";
		}
	return Result;
}

bool ServicePanel::is_name_mode() const {
	return ViewMode == 1 || ViewMode == 2 || ViewMode == 5 || ViewMode == 6 || ViewMode == 9;
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
	LogTrace();
	if (need_recashe) {
//		farmbox(L"Recache");
		m_svcs.cache();
	}
	need_recashe = true;
}
