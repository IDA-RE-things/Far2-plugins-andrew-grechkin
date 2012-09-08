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

#include "globalinfo.hpp"
#include "plugin.hpp"

#include <libbase/logger.hpp>

#include <libbase/shared_ptr.hpp>


Base::shared_ptr<Far::Plugin_i> plugin;


///========================================================================================== Export
/// GlobalInfo
void WINAPI GetGlobalInfoW(GlobalInfo * info) {
	Base::Logger::set_target(Base::Logger::get_TargetToFile(L"c:/FAR3/svcmgr.log"));
	Base::Logger::set_level(Base::Logger::LVL_TRACE);

	LogTrace();
	globalInfo.reset(create_GlobalInfo());
	globalInfo->GetGlobalInfo(info);
}

int WINAPI ConfigureW(const ConfigureInfo * Info) {
	return globalInfo->Configure(Info);
}


/// Plugin
void WINAPI SetStartupInfoW(const PluginStartupInfo * psi) {
	plugin.reset(create_FarPlugin(psi));
	globalInfo->load_settings();
}

void WINAPI GetPluginInfoW(PluginInfo * pi) {
	plugin->GetPluginInfo(pi);
}

HANDLE WINAPI OpenW(const OpenInfo * Info) {
	LogTrace();
	return plugin->Open(Info);
}

void WINAPI ClosePanelW(const ClosePanelInfo * Info) {
	LogTrace();
	plugin->ClosePanel(Info);
}


/// Panel
void WINAPI GetOpenPanelInfoW(OpenPanelInfo * Info) {
	static_cast<Far::Panel_i*>(Info->hPanel)->GetOpenPanelInfo(Info);
}

int WINAPI GetFindDataW(GetFindDataInfo * Info) {
	return static_cast<Far::Panel_i*>(Info->hPanel)->GetFindData(Info);
}

void WINAPI FreeFindDataW(const FreeFindDataInfo * Info) {
	static_cast<Far::Panel_i*>(Info->hPanel)->FreeFindData(Info);
}

int WINAPI CompareW(const CompareInfo * Info) {
	return static_cast<Far::Panel_i*>(Info->hPanel)->Compare(Info);
}

int WINAPI SetDirectoryW(const SetDirectoryInfo * Info) {
	return static_cast<Far::Panel_i*>(Info->hPanel)->SetDirectory(Info);
}

int WINAPI ProcessPanelEventW(const ProcessPanelEventInfo * Info) {
	return static_cast<Far::Panel_i*>(Info->hPanel)->ProcessEvent(Info);
}

int WINAPI ProcessPanelInputW(const ProcessPanelInputInfo * Info) {
	return static_cast<Far::Panel_i*>(Info->hPanel)->ProcessKey(Info->Rec);
}

//int WINAPI ProcessEventW(HANDLE hndl, int Event, void * Param) {
//	return static_cast<Far::Panel_i*>(hndl)->ProcessEvent(Event, Param);
//}
//
//int WINAPI ProcessKeyW(HANDLE hndl, int Key, unsigned int ControlState) {
//	return static_cast<Far::Panel_i*>(hndl)->ProcessKey(Key, ControlState);
//}
