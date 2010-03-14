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

/*
class		cShowWaitState {
public:
	void		operator()(DWORD state, uintmax_t elapsed, PVOID param) {
		static	DWORD	time;
//		static	FarDlg	*hDlg;
		if (elapsed / 1000 != time) {
			time = elapsed / 1000;
//			psi.SendDlgMessage(hDlg->Handle(), DM_SETTEXT, 0, (LONG_PTR)L"ddddddddddddddddddddd");
		}
	}
} ShowWaitState;
*/

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

	static PCWSTR	MenuStrings[1];
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
	Options.Read();
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

	Panel*	panel = new Panel;
	return	(HANDLE)panel;
}
void	WINAPI	EXP_NAME(ClosePlugin)(HANDLE hPlugin) {
	delete(Panel*)hPlugin;
}

void	WINAPI	EXP_NAME(GetOpenPluginInfo)(HANDLE hPlugin, OpenPluginInfo *Info) {
	Panel&	panel = *static_cast<Panel*>(hPlugin);
	return	panel.GetOpenPluginInfo(Info);
}

int		WINAPI	EXP_NAME(GetFindData)(HANDLE hPlugin, PluginPanelItem **pPanelItem, int *pItemsNumber, int OpMode) {
	Panel&	panel = *static_cast<Panel*>(hPlugin);
	return	panel.GetFindData(pPanelItem, pItemsNumber, OpMode);
}
void	WINAPI	EXP_NAME(FreeFindData)(HANDLE hPlugin, PluginPanelItem *PanelItem, int ItemsNumber) {
	Panel&	panel = *static_cast<Panel*>(hPlugin);
	panel.FreeFindData(PanelItem, ItemsNumber);
}

int		WINAPI	EXP_NAME(Compare)(HANDLE hPlugin, const PluginPanelItem *Item1, const PluginPanelItem *Item2, unsigned int Mode) {
	Panel&	panel = *static_cast<Panel*>(hPlugin);
	return	panel.Compare(Item1, Item2, Mode);
}
int		WINAPI	EXP_NAME(ProcessEvent)(HANDLE hPlugin, int Event, void *Param) {
	Panel&	panel = *static_cast<Panel*>(hPlugin);
	return	panel.ProcessEvent(Event, Param);
}
int		WINAPI	EXP_NAME(ProcessKey)(HANDLE hPlugin, int Key, unsigned int ControlState) {
	Panel&	panel = *static_cast<Panel*>(hPlugin);
	return	panel.ProcessKey(Key, ControlState);
}
int		WINAPI	EXP_NAME(SetDirectory)(HANDLE hPlugin, const WCHAR *Dir, int OpMode) {
	Panel&	panel = *static_cast<Panel*>(hPlugin);
	return	panel.SetDirectory(Dir, OpMode);
}
