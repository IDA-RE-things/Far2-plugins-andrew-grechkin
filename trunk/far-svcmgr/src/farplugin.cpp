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

#include <farplugin.hpp>

#include <libext/exception.hpp>

#include <libfar3/helper.hpp>
#include <libfar3/panelcontroller_i.hpp>
#include <libfar3/DlgBuilder.hpp>

#include <libbase/logger.hpp>

#include "globalinfo.hpp"
#include "guid.hpp"
#include "panel.hpp"
#include "lang.hpp"


///======================================================================================= FarPlugin
struct FarPlugin: public Far::Plugin_i {
	FarPlugin(Far::GlobalInfo_i * gi, const PluginStartupInfo * Info);

	~FarPlugin() override;

	void GetInfo(PluginInfo * Info) override;

	Far::PanelController_i * Open(const OpenInfo * Info) override;
};


FarPlugin::FarPlugin(Far::GlobalInfo_i * gi, const PluginStartupInfo * Info):
	Far::Plugin_i(gi, Info)
{
	LogTrace();
}

FarPlugin::~FarPlugin() {
	LogTrace();
}

void FarPlugin::GetInfo(PluginInfo * Info) {
	LogTrace();
	Info->Flags = PF_NONE;

	static GUID PluginMenuGuids[] = {MenuGuid,};
	static PCWSTR PluginMenuStrings[] = {Far::get_msg(Far::MenuTitle),};

	if (FarGlobalInfo::inst().addToPluginsMenu) {
		Info->PluginMenu.Guids = PluginMenuGuids;
		Info->PluginMenu.Strings = PluginMenuStrings;
		Info->PluginMenu.Count = Base::lengthof(PluginMenuStrings);
	}

	static PCWSTR DiskStrings[] = {Far::get_msg(Far::DiskTitle),};
	if (FarGlobalInfo::inst().addToDisksMenu) {
		Info->DiskMenu.Guids = PluginMenuGuids;
		Info->DiskMenu.Strings = DiskStrings;
		Info->DiskMenu.Count = Base::lengthof(DiskStrings);
	}

	Info->PluginConfig.Guids = PluginMenuGuids;
	Info->PluginConfig.Strings = PluginMenuStrings;
	Info->PluginConfig.Count = Base::lengthof(PluginMenuStrings);
	Info->CommandPrefix = FarGlobalInfo::inst().Prefix;
}

Far::PanelController_i * FarPlugin::Open(const OpenInfo * Info)
{
	LogTrace();
	try {
		return create_FarPanel(Info);
	} catch (Ext::AbstractError & e) {
		LogDebug(L"%s\n", e.what().c_str());
		Far::ebox(e.format_error());
	}
	return nullptr;
}


///=================================================================================================
Far::Plugin_i * create_FarPlugin(Far::GlobalInfo_i * gi, const PluginStartupInfo * psi) {
	return new FarPlugin(gi, psi);
}

void destroy(Far::Plugin_i * plugin) {
	delete plugin;
}
