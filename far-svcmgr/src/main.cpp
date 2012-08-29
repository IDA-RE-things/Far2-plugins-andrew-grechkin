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

#include "farplugin.hpp"
#include "globalinfo.hpp"

#include <libbase/logger.hpp>

///========================================================================================== Export
void WINAPI GetGlobalInfoW(GlobalInfo * info) {
	Base::Logger::set_target(Base::Logger::get_TargetToFile(L"c:/svcmgr.log"));
	Base::Logger::set_level(Base::Logger::LVL_TRACE);

	globalInfo.reset(FarGlobalInfo::create());
	globalInfo->GetGlobalInfo(info);
}

void WINAPI SetStartupInfoW(const PluginStartupInfo * psi) {
	plugin.reset(FarPlugin::create(psi));
}

void WINAPI GetPluginInfoW(PluginInfo * pi) {
	plugin->GetPluginInfo(pi);
}

HANDLE WINAPI OpenW(const OpenInfo * Info) {
	return plugin->Open(Info);
}

void WINAPI ClosePanelW(const ClosePanelInfo * Info) {
	plugin->Close(Info);
}

int WINAPI ConfigureW(const ConfigureInfo * Info) {
	return plugin->Configure(Info);
}

void WINAPI GetOpenPanelInfoW(OpenPanelInfo * Info) {
	static_cast<Far::IPanel*>(Info->hPanel)->GetOpenPanelInfo(Info);
}

int WINAPI GetFindDataW(GetFindDataInfo * Info) {
	return static_cast<Far::IPanel*>(Info->hPanel)->GetFindData(Info);
}

void WINAPI FreeFindDataW(const FreeFindDataInfo * Info) {
	static_cast<Far::IPanel*>(Info->hPanel)->FreeFindData(Info);
}

int WINAPI CompareW(const CompareInfo * Info) {
	return static_cast<Far::IPanel*>(Info->hPanel)->Compare(Info);
}

int WINAPI SetDirectoryW(const SetDirectoryInfo * Info) {
	return static_cast<Far::IPanel*>(Info->hPanel)->SetDirectory(Info);
}

int WINAPI ProcessPanelEventW(const ProcessPanelEventInfo * Info) {
	return static_cast<Far::IPanel*>(Info->hPanel)->ProcessEvent(Info);
}

int WINAPI ProcessPanelInputW(const ProcessPanelInputInfo * Info) {
	return static_cast<Far::IPanel*>(Info->hPanel)->ProcessKey(Info->Rec);
}

//int WINAPI ProcessEventW(HANDLE hndl, int Event, void * Param) {
//	return static_cast<Far::IPanel*>(hndl)->ProcessEvent(Event, Param);
//}
//
//int WINAPI ProcessKeyW(HANDLE hndl, int Key, unsigned int ControlState) {
//	return static_cast<Far::IPanel*>(hndl)->ProcessKey(Key, ControlState);
//}
