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
#include "version.h"

///========================================================================================== Export
void WINAPI SetStartupInfoW(const PluginStartupInfo * psi) {
	plugin.reset(new FarPlugin(psi));
	Far::mbox(L"end", ustring(__PRETTY_FUNCTION__).c_str());
}

void WINAPI GetPluginInfoW(PluginInfo * pi) {
	Far::mbox(L"begin", ustring(__PRETTY_FUNCTION__).c_str());
	plugin->get_info(pi);
	Far::mbox(L"end", ustring(__PRETTY_FUNCTION__).c_str());
}

int WINAPI ConfigureW(const ConfigureInfo * /*Info*/) {
	Far::mbox(L"begin", ustring(__PRETTY_FUNCTION__).c_str());
	return plugin->configure();
}

void WINAPI GetGlobalInfoW(GlobalInfo * info)
{
	using namespace AutoVersion;
	info->StructSize = sizeof(*info);
	info->MinFarVersion = FARMANAGERVERSION;
	info->Version = MAKEFARVERSION(MAJOR, MINOR, BUILD, REVISION, VS_RELEASE);
	info->Guid = FarPlugin::get_guid();
	info->Title = FarPlugin::get_name();
	info->Description = FarPlugin::get_description();
	info->Author = FarPlugin::get_author();
}

HANDLE WINAPI OpenW(const OpenInfo * Info) {
	Far::mbox(L"begin", ustring(__PRETTY_FUNCTION__).c_str());
	return plugin->open(Info);
}

void WINAPI GetOpenPanelInfoW(OpenPanelInfo * Info) {
	static size_t ctr;
	Far::mbox(as_str(++ctr).c_str(), ustring(__PRETTY_FUNCTION__).c_str());
	static_cast<Far::IPanel*>(Info->hPanel)->GetOpenPanelInfo(Info);
}

int WINAPI GetFindDataW(GetFindDataInfo * Info) {
//	Far::mbox(L"begin", ustring(__PRETTY_FUNCTION__).c_str());
	return static_cast<Far::IPanel*>(Info->hPanel)->GetFindData(Info);
}

void WINAPI FreeFindDataW(const FreeFindDataInfo * Info) {
//	Far::mbox(L"begin", ustring(__PRETTY_FUNCTION__).c_str());
	static_cast<Far::IPanel*>(Info->hPanel)->FreeFindData(Info);
}

int WINAPI CompareW(const CompareInfo * Info) {
//	Far::mbox(L"begin", ustring(__PRETTY_FUNCTION__).c_str());
	return static_cast<Far::IPanel*>(Info->hPanel)->Compare(Info);
}

int WINAPI SetDirectoryW(const SetDirectoryInfo * Info) {
	Far::mbox(L"begin", ustring(__PRETTY_FUNCTION__).c_str());
	return static_cast<Far::IPanel*>(Info->hPanel)->SetDirectory(Info);
}

int WINAPI ProcessPanelEventW(const ProcessPanelEventInfo * Info) {
//	Far::mbox(L"begin", ustring(__PRETTY_FUNCTION__).c_str());
	return static_cast<Far::IPanel*>(Info->hPanel)->ProcessEvent(Info);
}

int WINAPI ProcessPanelInputW(const ProcessPanelInputInfo * Info) {
//	Far::mbox(L"begin", ustring(__PRETTY_FUNCTION__).c_str());
	return static_cast<Far::IPanel*>(Info->hPanel)->ProcessKey(Info->Rec);
}

//int WINAPI ProcessEventW(HANDLE hndl, int Event, void * Param) {
//	return static_cast<Far::IPanel*>(hndl)->ProcessEvent(Event, Param);
//}
//
//int WINAPI ProcessKeyW(HANDLE hndl, int Key, unsigned int ControlState) {
//	return static_cast<Far::IPanel*>(hndl)->ProcessKey(Key, ControlState);
//}
