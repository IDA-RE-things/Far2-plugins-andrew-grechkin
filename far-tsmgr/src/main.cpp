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

///========================================================================================== export
int		WINAPI	EXP_NAME(GetMinFarVersion)() {
	return	MAKEFARVERSION(MIN_FAR_VERMAJOR, MIN_FAR_VERMINOR, MIN_FAR_BUILD);
}
void	WINAPI	EXP_NAME(SetStartupInfo)(const PluginStartupInfo *psi) {
	InitFSF(psi);
	Options.reg.path(AutoUTF(psi->RootKey) + L"\\" + Options.Prefix);
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
	Options.Read();
	DiskDigit[0] = Options.DiskMenuDigit;
	enum {
		HEIGHT = 10,
		WIDTH = 56,

		indAddPlug = 1,
		indAddDisk = 2,
		indDigit = 3,
	};
	InitDialogItemF Items[] = {
		{DI_DOUBLEBOX, 3,  1,  WIDTH - 4, HEIGHT - 2, 0, (PCWSTR)DlgTitle},
		{DI_CHECKBOX,  5, 2, 0,  0, 0, (PCWSTR)txtAddToPluginsMenu},
		{DI_CHECKBOX,  5, 3, 0,  0, 0, (PCWSTR)txtAddToDiskMenu},
		{DI_FIXEDIT,   7, 4, 7,  4, DIF_MASKEDIT, DiskDigit},
		{DI_TEXT,     10, 4, 0,  0, 0, (PCWSTR)txtDisksMenuHotkey},
		{DI_EDIT,      5, 5, 12, 5, 0, Options.Prefix.c_str()},
		{DI_TEXT,     14, 5, 0,  0, 0, (PCWSTR)txtPluginPrefix},
		{DI_TEXT,      0,  HEIGHT - 4, 0,  0,  DIF_SEPARATOR,   L""},
		{DI_BUTTON,    0,  HEIGHT - 3, 0,  0,  DIF_CENTERGROUP, (PCWSTR)txtBtnOk},
		{DI_BUTTON,    0,  HEIGHT - 3, 0,  0,  DIF_CENTERGROUP, (PCWSTR)txtBtnCancel},
	};
	size_t	size = sizeofa(Items);
	FarDialogItem FarItems[size];
	InitDialogItemsF(Items, FarItems, size);
	FarItems[indAddPlug].Selected = Options.AddToPluginsMenu;
	FarItems[indAddDisk].Selected = Options.AddToDisksMenu;
	FarItems[indDigit].Mask = L"9";
	FarItems[size-2].DefaultButton = 1;

	FarDlg hDlg;
	if (hDlg.Init(psi.ModuleNumber, -1, -1, WIDTH, HEIGHT, L"dlgConfigure", FarItems, size)) {
		int	ret = hDlg.Run();
		if (ret > 0 && Items[ret].Data == (PCWSTR)txtBtnOk) {
			Options.AddToPluginsMenu = GetCheck(hDlg, 1);
			Options.AddToDisksMenu = GetCheck(hDlg, 2);
			Options.DiskMenuDigit = GetDataPtr(hDlg, 3)[0];
			Options.Prefix = GetDataPtr(hDlg, 5);
			Options.Write();
		}
	}
	return	true;
}

HANDLE	WINAPI	EXP_NAME(OpenPlugin)(int OpenFrom, INT_PTR Item) {
	Options.Read();
	AutoUTF	cline;
	/*
		if (OpenFrom == OPEN_PLUGINSMENU) {
			PanelInfo	pi;
			if (psi.Control(PANEL_ACTIVE, FCTL_GETPANELINFO, sizeof(pi), (LONG_PTR)&pi)) {
				WCHAR	buf[MAX_PATH_LENGTH + MAX_PATH + 1];
				fsf.GetCurrentDirectory(sizeofa(buf), buf);
				if (!Empty(buf))
					fsf.AddEndSlash(buf);

				PluginPanelItem PPI;
				psi.Control(PANEL_ACTIVE, FCTL_GETPANELITEM, pi.CurrentItem, (LONG_PTR)&PPI);
				Cat(buf, PPI.FindData.lpwszFileName);
				cline = buf;
			}
		} else if (OpenFrom == OPEN_COMMANDLINE) {
			cline = (PCWSTR)Item;
			fsf.Trim(cline.buffer());
			fsf.Unquote(cline.buffer());
		}
	*/
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
int		WINAPI	EXP_NAME(ProcessKey)(HANDLE hPlugin, int Key, unsigned int ControlState) {
	Panel&	panel = *static_cast<Panel*>(hPlugin);
	return	panel.ProcessKey(Key, ControlState);
}
