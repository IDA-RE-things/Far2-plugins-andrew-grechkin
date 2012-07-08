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

#ifndef FAR_LANG_HPP
#define FAR_LANG_HPP

enum {
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

	infoServiceName,
	infoDisplayName,
	infoDescription,
	infoPath,
	infoState,
	infoStartupType,
	infoErrorControl,
	infoOrderGroup,
	infoStartName,
	infoTag,
	infoDependencies,
};

#endif
