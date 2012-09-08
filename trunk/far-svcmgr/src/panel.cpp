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

///========================================================================================= dialogs
//LONG_PTR WINAPI DlgProc(HANDLE hDlg, int Msg, int Param1, void * Param2) {
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
//	return Far::psi().DefDlgProc(hDlg, Msg, Param1, Param2);
//}


///============================================================================== ProgressWindow
//template <typename Type, intptr_t (Type::*mem_func)(HANDLE, int, int)>
//intptr_t WINAPI dlg_proc_thunk(HANDLE dlg, int msg, int param1, void * param2) {
//	return (((Type*)(param2))->*mem_func)(dlg, msg, param1);
//}
//
//
//struct ProgressWindow {
//	~ProgressWindow();
//
//	ProgressWindow(const GUID & guid, const ServicePanel * panel, Far::Panel * info, PCWSTR title, WinSvcFunc func);
//
//	void set_name(size_t num, PCWSTR name);
//
//	intptr_t dlg_proc(HANDLE dlg, int msg, int param1);
//
//private:
//	const ServicePanel * m_panel;
//	Far::Panel * m_info;
//	WinSvcFunc m_func;
//	HANDLE m_dlg;
//};


///==================================================================================== ServicePanel
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


static void TrunCopy(PWSTR cpDest, PCWSTR cpSrc, size_t size, size_t max_len)
{
	Base::copy_str(cpDest, cpSrc, size);
	Far::fsf().TruncStr(cpDest, max_len);

	size_t iLen = Base::get_str_len(cpDest);
	if (iLen < max_len)
	{
		::wmemset(&cpDest[iLen], L' ', max_len - iLen);
		cpDest[max_len] = L'\0';
	}
}

static void ShowMessage(PCWSTR title, PCWSTR name)
{
	LogTrace();
	WCHAR TruncName[MAX_PATH];
	TrunCopy(TruncName, name, lengthof(TruncName), 60);
	PCWSTR MsgItems[] =
	{
		title,
		TruncName,
	};
	Far::psi().Message(Far::get_plugin_guid(), nullptr, 0, NULL, MsgItems, Base::lengthof(MsgItems), 0);
}

bool ServicePanel::action_process(WinSvcFunc func, PCWSTR title) {
	Far::Panel panel(this, FCTL_GETPANELINFO);
	if (panel.size()) {
		HANDLE hScreen = Far::psi().SaveScreen(0, 0, -1, -1);
		panel.StartSelection();
		try {
			for (size_t i = panel.selected(); i; --i) {
				Ext::WinServices::const_iterator it = m_svcs.find(panel.get_selected(0)->CustomColumnData[0]);
				if (it != m_svcs.end()) {
					ShowMessage(title, panel.get_selected(0)->CustomColumnData[1]);
					func(it->Name, m_conn.get());
					panel.clear_selection(0);
				}
			}
		} catch (Ext::AbstractError & e) {
			Far::ebox(e.format_error());
		}
		panel.CommitSelection();
		Far::psi().RestoreScreen(hScreen);
		update();
		redraw();
	}
	return true;
}
