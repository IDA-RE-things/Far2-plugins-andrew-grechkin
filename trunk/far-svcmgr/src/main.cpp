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

#include <globalinfo.hpp>
#include <farplugin.hpp>

#include <libfar3/plugin_i.hpp>
#include <libfar3/panelcontroller_i.hpp>

#include <libbase/logger.hpp>

#include <libbase/shared_ptr.hpp>


///========================================================================================== Export
/// GlobalInfo
void WINAPI GetGlobalInfoW(GlobalInfo * Info) {
	Base::Logger::set_target(Base::Logger::get_TargetToFile(L"c:/FAR3/svcmgr.log"));
	Base::Logger::set_level(Base::Logger::LVL_TRACE);

	LogTrace();
	FarGlobalInfo::inst().GetGlobalInfoW(Info);
}

int WINAPI ConfigureW(const ConfigureInfo * Info) {
	LogTrace();
	return FarGlobalInfo::inst().ConfigureW(Info);
}

void WINAPI SetStartupInfoW(const PluginStartupInfo * Info) {
	LogTrace();
	FarGlobalInfo::inst().SetStartupInfoW(Info);
}


/// Plugin
void WINAPI GetPluginInfoW(PluginInfo * Info) {
	FarGlobalInfo::inst().get_plugin()->GetPluginInfoW(Info);
}

HANDLE WINAPI OpenW(const OpenInfo * Info) {
	LogTrace();
	return FarGlobalInfo::inst().get_plugin()->OpenW(Info);
}

void WINAPI ExitFARW(const struct ExitInfo *Info) {
	LogTrace();
	FarGlobalInfo::inst().get_plugin()->ExitFARW(Info);
}


/// Panel
void WINAPI ClosePanelW(const ClosePanelInfo * Info) {
	LogTrace();
	static_cast<Far::PanelController_i*>(Info->hPanel)->ClosePanelW(Info);
}

int WINAPI CompareW(const CompareInfo * Info) {
	return static_cast<Far::PanelController_i*>(Info->hPanel)->CompareW(Info);
}

int WINAPI DeleteFilesW(const DeleteFilesInfo * Info) {
	LogTrace();
	return static_cast<Far::PanelController_i*>(Info->hPanel)->DeleteFilesW(Info);
}

void WINAPI FreeFindDataW(const FreeFindDataInfo * Info) {
	LogTrace();
	static_cast<Far::PanelController_i*>(Info->hPanel)->FreeFindDataW(Info);
}

void WINAPI FreeVirtualFindDataW(const FreeFindDataInfo * Info) {
	LogTrace();
	static_cast<Far::PanelController_i*>(Info->hPanel)->FreeVirtualFindDataW(Info);
}

int WINAPI GetFilesW(GetFilesInfo * Info) {
	LogTrace();
	return static_cast<Far::PanelController_i*>(Info->hPanel)->GetFilesW(Info);
}

int WINAPI GetFindDataW(GetFindDataInfo * Info) {
	LogTrace();
	return static_cast<Far::PanelController_i*>(Info->hPanel)->GetFindDataW(Info);
}

void WINAPI GetOpenPanelInfoW(OpenPanelInfo * Info) {
//	LogTrace();
	static_cast<Far::PanelController_i*>(Info->hPanel)->GetOpenPanelInfoW(Info);
}

int WINAPI GetVirtualFindDataW(GetVirtualFindDataInfo * Info) {
	LogTrace();
	return static_cast<Far::PanelController_i*>(Info->hPanel)->GetVirtualFindDataW(Info);
}

int WINAPI MakeDirectoryW(MakeDirectoryInfo * Info) {
	LogTrace();
	return static_cast<Far::PanelController_i*>(Info->hPanel)->MakeDirectoryW(Info);
}

int WINAPI ProcessPanelEventW(const ProcessPanelEventInfo * Info) {
//	LogTrace();
	return static_cast<Far::PanelController_i*>(Info->hPanel)->ProcessPanelEventW(Info);
}

int WINAPI ProcessHostFileW(const ProcessHostFileInfo * Info) {
	LogTrace();
	return static_cast<Far::PanelController_i*>(Info->hPanel)->ProcessHostFileW(Info);
}

int WINAPI ProcessPanelInputW(const ProcessPanelInputInfo * Info) {
//	LogTrace();
	return static_cast<Far::PanelController_i*>(Info->hPanel)->ProcessPanelInputW(Info);
}

int WINAPI PutFilesW(const PutFilesInfo * Info) {
	LogTrace();
	return static_cast<Far::PanelController_i*>(Info->hPanel)->PutFilesW(Info);
}

int WINAPI SetDirectoryW(const SetDirectoryInfo * Info) {
	LogTrace();
	return static_cast<Far::PanelController_i*>(Info->hPanel)->SetDirectoryW(Info);
}

int WINAPI SetFindListW(const SetFindListInfo * Info) {
	LogTrace();
	return static_cast<Far::PanelController_i*>(Info->hPanel)->SetFindListW(Info);
}
