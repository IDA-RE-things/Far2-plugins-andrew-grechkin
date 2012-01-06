/**
	svcmgr: Manage services
	Allow to manage windows services
	FAR2, FAR3 plugin

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

#include "farplugin.hpp"
#include "version.h"

///========================================================================================== Export
int WINAPI GetMinFarVersionW() {
	return	MAKEFARVERSION(MIN_FAR_VERMAJOR, MIN_FAR_VERMINOR, MIN_FAR_BUILD);
}

void WINAPI SetStartupInfoW(const PluginStartupInfo * psi) {
	plugin.reset(new FarPlugin(psi));
}

int WINAPI ConfigureW(int) {
	return plugin->configure();
}

void WINAPI GetPluginInfoW(PluginInfo * pi) {
	plugin->get_info(pi);
}

#ifndef FAR2
void WINAPI GetGlobalInfoW(GlobalInfo * info)
{
	using namespace AutoVersion;
	info->StructSize = sizeof(GlobalInfo);
	info->MinFarVersion = FARMANAGERVERSION;
	info->Version = MAKEFARVERSION(MAJOR,MINOR,BUILD,REVISION,VS_RELEASE);
	info->Guid = FarPlugin::get_guid();
	info->Title = FarPlugin::get_name();
	info->Description = FarPlugin::get_description();
	info->Author = FarPlugin::get_author();
}

HANDLE WINAPI OpenW(const OpenInfo * Info) {
	return plugin->open(Info);
}
#else
HANDLE WINAPI OpenPluginW(int OpenFrom, INT_PTR Item) {
	return plugin->open(OpenFrom, Item);
}
#endif

void WINAPI ClosePluginW(HANDLE hndl) {
	plugin->close(hndl);
}

void WINAPI GetOpenPluginInfoW(HANDLE hndl, OpenPluginInfo * Info) {
	return	static_cast<IFarPanel*>(hndl)->GetOpenPluginInfo(Info);
}

int WINAPI GetFindDataW(HANDLE hndl, PluginPanelItem ** pPanelItem, int * pItemsNumber, int OpMode) {
	return	static_cast<IFarPanel*>(hndl)->GetFindData(pPanelItem, pItemsNumber, OpMode);
}

void WINAPI FreeFindDataW(HANDLE hndl, PluginPanelItem * PanelItem, int ItemsNumber) {
	static_cast<IFarPanel*>(hndl)->FreeFindData(PanelItem, ItemsNumber);
}

int WINAPI CompareW(HANDLE hndl, const PluginPanelItem * Item1, const PluginPanelItem * Item2, unsigned int Mode) {
	return	static_cast<IFarPanel*>(hndl)->Compare(Item1, Item2, Mode);
}

int WINAPI ProcessEventW(HANDLE hndl, int Event, void * Param) {
	return	static_cast<IFarPanel*>(hndl)->ProcessEvent(Event, Param);
}

int WINAPI ProcessKeyW(HANDLE hndl, int Key, unsigned int ControlState) {
	return	static_cast<IFarPanel*>(hndl)->ProcessKey(Key, ControlState);
}

int WINAPI SetDirectoryW(HANDLE hndl, const WCHAR * Dir, int OpMode) {
	return	static_cast<IFarPanel*>(hndl)->SetDirectory(Dir, OpMode);
}
