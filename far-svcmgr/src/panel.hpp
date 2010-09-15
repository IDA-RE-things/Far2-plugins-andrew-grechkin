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

#ifndef PANEL_HPP
#define PANEL_HPP

#include "win_net.h"

#include "../../far/far_helper.hpp"

///======================================================================================== Messages
enum	{
	txtBtnLogonAs = 5,
	txtBtnDepends,
	txtAddToPluginsMenu,
	txtAddToDiskMenu,
	txtDisksMenuHotkey,
	txtPluginPrefix,
	txtTimeout,

	txtSelectComputer,
	txtHost,
	txtEmptyForLocal,
	txtLogin,
	txtPass,
	txtEmptyForCurrent,
	txtConnecting,

	txtStopped,
	txtStartPending,
	txtStopPending,
	txtxRunning,
	txtContinuePending,
	txtPausePending,
	txtPaused,

	txtBoot,
	txtSystem,
	txtAuto,
	txtManual,
	txtDisbld,

	txtIgnore,
	txtNormal,
	txtSevere,
	txtCritical,

	txtAreYouSure,
	txtDeleteService,

	txtWaitAMoment,
	txtActionInProcess,
	txtStartingService,
	txtPausingService,
	txtStoppingService,
	txtRestartingService,
	txtContinueService,

	txtServices,
	txtDevices,

	txtDlgCreateService,
	txtDlgServiceProperties,
	txtDlgName,
	txtDlgDisplayName,
	txtDlgBinaryPath,
	txtDlgGroup,
	txtDlgServiceType,
	txtDlgStartupType,
	txtDlgErrorControl,

	txtDlgBoot,
	txtDlgSystem,
	txtDlgAuto,
	txtDlgManual,
	txtDlgDisbld,

	txtDlgIgnore,
	txtDlgNormal,
	txtDlgSevere,
	txtDlgCritical,

	txtDriver,
	txtFileSystemDriver,
	txtOwnProcess,
	txtSharedProcess,

	txtClmName,
	txtClmDisplayName,
	txtClmStatus,
	txtClmStart,
	txtClmLogon,
	txtClmDep,

	txtBtnStart,
	txtBtnConnct,
	txtBtnPause,
	txtBtnStop,
	txtBtnRestrt,
	txtBtnCreate,
	txtBtnStartP,
	txtBtnLocal,
	txtBtnContin,
	txtBtnDelete,
	txtBtnLogon,

	txtDlgLogonAs,
	txtDlgNetworkService,
	txtDlgLocalService,
	txtDlgLocalSystem,
	txtDlgAllowDesktop,
	txtDlgUserDefined,

	txtMnuTitle,
	txtMnuCommands,

	txtMnuSelectNewTitle,
	txtMnuSelectNewCommands,
};

///=========================================================================================== Panel
class		Panel : private Uncopyable {
	RemoteConnection	m_conn;
	WinServices			m_sm;

	bool				need_recashe;
	int					ViewMode;
public:
	Panel(): m_sm(&m_conn, false), need_recashe(true), ViewMode(3) {
	}
	RemoteConnection*	conn() {
		return	&m_conn;
	}
	WinServices*		sm() {
		return	&m_sm;
	}

	AutoUTF				host() const {
		return	m_conn.host();
	}
	AutoUTF				name() const {
		return	m_sm.Key();
	}
	DWORD				state() const {
		return	m_sm.state();
	}
	void				Connect(PCWSTR host, PCWSTR user = null_ptr, PCWSTR pass = null_ptr) {
		m_conn.Open(host, user, pass);
	}

	PCWSTR				GetState(DWORD state) const {
		return	GetMsg(state + txtStopped - SERVICE_STOPPED);
	}
	PCWSTR				GetStartType(DWORD start) const {
		return	GetMsg(start + txtBoot - SERVICE_BOOT_START);
	}
	PCWSTR				GetErrorControl(DWORD err) const {
		return	GetMsg(err + txtIgnore - SERVICE_ERROR_IGNORE);
	}

	AutoUTF				Info() const {
		AutoUTF	Result;
		Result += L"Service name:  ";
		Result += m_sm.Key();
		Result += L"\n\n";
		Result += L"Display name:  ";
		Result += m_sm.Value().dname;
		Result += L"\n\n";
		Result += L"Description:   ";
		Result += m_sm.Value().descr;
		Result += L"\n\n";
		Result += L"Path:          ";
		Result += m_sm.Value().path;
		Result += L"\n\n";
		Result += L"State:         ";
		Result += GetState(m_sm.Value().dwCurrentState);
		Result += L"\n\n";
		Result += L"Startup type:  ";
		Result += GetStartType(m_sm.Value().StartType);
		Result += L"\n\n";
		Result += L"Error control: ";
		Result += GetErrorControl(m_sm.Value().ErrorControl);
		Result += L"\n\n";
		Result += L"Dependencies:  ";
		for (size_t i = 0; i < m_sm.Value().Dependencies.size(); ++i) {
			Result += m_sm.Value().Dependencies[i];
			Result += L"\n              ";
		}

		return	Result;
	}

	void				Cache() {
		if (need_recashe) {
//			farmbox(L"Recache");
			m_sm.Cache();
		}
		need_recashe = true;
	}

	bool				DlgConnection();
	bool				DlgCreateService();
	bool				DlgEditSvc();
	bool				DlgLogonAs();
	bool				MenuDepends();
	bool				MenuSelectNewDepend();

	void				GetOpenPluginInfo(OpenPluginInfo *Info);

	int					GetFindData(PluginPanelItem **pPanelItem, int *pItemsNumber, int OpMode);
	void				FreeFindData(PluginPanelItem *PanelItem, int ItemsNumber);

	int					Compare(const PluginPanelItem *Item1, const PluginPanelItem *Item2, unsigned int Mode);
	int					ProcessEvent(int Event, void *Param);
	int					ProcessKey(int Key, unsigned int ControlState);
	int					SetDirectory(const WCHAR *Dir, int OpMode);
};

#endif
