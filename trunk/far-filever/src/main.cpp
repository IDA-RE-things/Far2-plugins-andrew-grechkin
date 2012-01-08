/**
	filever: File Version FAR plugin
	Displays version information from file resource in dialog
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
void WINAPI SetStartupInfoW(const PluginStartupInfo * psi) {
	plugin.reset(new FarPlugin(psi));
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
int WINAPI GetMinFarVersionW() {
	return MAKEFARVERSION(MIN_FAR_VERMAJOR, MIN_FAR_VERMINOR, MIN_FAR_BUILD);
}

HANDLE WINAPI OpenPluginW(int OpenFrom, INT_PTR Item) {
	return plugin->open(OpenFrom, Item);
}
#endif
