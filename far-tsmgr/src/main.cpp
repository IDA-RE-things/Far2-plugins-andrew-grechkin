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
	Options.Init(psi->RootKey);
}

void	WINAPI	EXP_NAME(GetPluginInfo)(PluginInfo *pi) {
	Options.Read();
	pi->StructSize = sizeof(PluginInfo);
	pi->Flags = 0;

	static PCWSTR	DiskStrings[1];
	if (Options.AddToDisksMenu) {
		DiskStrings[0] = GetMsg(DiskTitle);
		pi->DiskMenuStrings = DiskStrings;
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
	Options.Read();
	enum {
		HEIGHT = 9,
		WIDTH = 56,

		indAddPlug = 1,
		indAddDisk = 2,
		indPrefix = 3,
	};
	InitDialogItemF Items[] = {
		{DI_DOUBLEBOX, 3,  1,  WIDTH - 4, HEIGHT - 2, 0, (PCWSTR)DlgTitle},
		{DI_CHECKBOX,  5, 2, 0,  0, 0, (PCWSTR)txtAddToPluginsMenu},
		{DI_CHECKBOX,  5, 3, 0,  0, 0, (PCWSTR)txtAddToDiskMenu},
		{DI_EDIT,      5, 4, 12, 5, 0, Options.Prefix.c_str()},
		{DI_TEXT,     14, 4, 0,  0, 0, (PCWSTR)txtPluginPrefix},
		{DI_TEXT,      0,  HEIGHT - 4, 0,  0,  DIF_SEPARATOR,   L""},
		{DI_BUTTON,    0,  HEIGHT - 3, 0,  0,  DIF_CENTERGROUP, (PCWSTR)txtBtnOk},
		{DI_BUTTON,    0,  HEIGHT - 3, 0,  0,  DIF_CENTERGROUP, (PCWSTR)txtBtnCancel},
	};
	size_t	size = sizeofa(Items);
	FarDialogItem FarItems[size];
	InitDialogItemsF(Items, FarItems, size);
	FarItems[indAddPlug].Selected = Options.AddToPluginsMenu;
	FarItems[indAddDisk].Selected = Options.AddToDisksMenu;
	FarItems[size-2].DefaultButton = 1;

	FarDlg hDlg;
	if (hDlg.Init(psi.ModuleNumber, -1, -1, WIDTH, HEIGHT, L"dlgConfigure", FarItems, size)) {
		int	ret = hDlg.Run();
		if (ret > 0 && Items[ret].Data == (PCWSTR)txtBtnOk) {
			Options.AddToPluginsMenu = hDlg.Check(indAddPlug);
			Options.AddToDisksMenu = hDlg.Check(indAddDisk);
			Options.Prefix = hDlg.Str(indPrefix);
			Options.Write();
		}
	}
	return	true;
}

HANDLE	WINAPI	EXP_NAME(OpenPlugin)(int /*OpenFrom*/, INT_PTR /*Item*/) {
	Options.Read();
	return	(HANDLE)(new Panel);
}

void	WINAPI	EXP_NAME(ClosePlugin)(HANDLE hPlugin) {
	delete (Panel*)hPlugin;
}

void	WINAPI	EXP_NAME(GetOpenPluginInfo)(HANDLE hPlugin, OpenPluginInfo *Info) {
	Panel*	panel = static_cast<Panel*>(hPlugin);
	return	panel->GetOpenPluginInfo(Info);
}

int		WINAPI	EXP_NAME(GetFindData)(HANDLE hPlugin, PluginPanelItem **pPanelItem, int *pItemsNumber, int OpMode) {
	Panel*	panel = static_cast<Panel*>(hPlugin);
	return	panel->GetFindData(pPanelItem, pItemsNumber, OpMode);
}

void	WINAPI	EXP_NAME(FreeFindData)(HANDLE hPlugin, PluginPanelItem *PanelItem, int ItemsNumber) {
	Panel*	panel = static_cast<Panel*>(hPlugin);
	panel->FreeFindData(PanelItem, ItemsNumber);
}

int		WINAPI	EXP_NAME(Compare)(HANDLE hPlugin, const PluginPanelItem *Item1, const PluginPanelItem *Item2, unsigned int Mode) {
	Panel*	panel = static_cast<Panel*>(hPlugin);
	return	panel->Compare(Item1, Item2, Mode);
}

int		WINAPI	EXP_NAME(ProcessEvent)(HANDLE hPlugin, int Event, void *Param) {
	Panel*	panel = static_cast<Panel*>(hPlugin);
	return	panel->ProcessEvent(Event, Param);
}

int		WINAPI	EXP_NAME(ProcessKey)(HANDLE hPlugin, int Key, unsigned int ControlState) {
	Panel*	panel = static_cast<Panel*>(hPlugin);
	return	panel->ProcessKey(Key, ControlState);
}
