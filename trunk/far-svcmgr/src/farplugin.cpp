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
	FarPlugin(const PluginStartupInfo * psi);

	virtual ~FarPlugin();

	virtual void GetPluginInfo(PluginInfo * pi);

	virtual HANDLE Open(const OpenInfo * Info);

	virtual void ClosePanel(const ClosePanelInfo * Info);
};


FarPlugin::FarPlugin(const PluginStartupInfo * psi) {
	LogTrace();
	Far::helper_t::inst().init(globalInfo->get_guid(), psi);
}

FarPlugin::~FarPlugin() {
	LogTrace();
}

void FarPlugin::GetPluginInfo(PluginInfo * pi) {
	LogTrace();
	pi->StructSize = sizeof(*pi);
	pi->Flags = PF_NONE;

	static GUID PluginMenuGuids[] = {MenuGuid,};
	static PCWSTR PluginMenuStrings[] = {Far::get_msg(Far::MenuTitle),};

	if (globalInfo->AddToPluginsMenu) {
		pi->PluginMenu.Guids = PluginMenuGuids;
		pi->PluginMenu.Strings = PluginMenuStrings;
		pi->PluginMenu.Count = Base::lengthof(PluginMenuStrings);
	}

	static PCWSTR DiskStrings[] = {Far::get_msg(Far::DiskTitle),};
	if (globalInfo->AddToDisksMenu) {
		pi->DiskMenu.Guids = PluginMenuGuids;
		pi->DiskMenu.Strings = DiskStrings;
		pi->DiskMenu.Count = Base::lengthof(DiskStrings);
	}

	pi->PluginConfig.Guids = PluginMenuGuids;
	pi->PluginConfig.Strings = PluginMenuStrings;
	pi->PluginConfig.Count = Base::lengthof(PluginMenuStrings);
	pi->CommandPrefix = globalInfo->Prefix;
}

HANDLE FarPlugin::Open(const OpenInfo * Info)
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

void FarPlugin::ClosePanel(const ClosePanelInfo * Info) {
	LogTrace();
	destroy(static_cast<Far::PanelController_i*>(Info->hPanel));
}


///=================================================================================================
Far::Plugin_i * create_FarPlugin(const PluginStartupInfo * psi) {
	return new FarPlugin(psi);
}

void destroy(Far::Plugin_i * plugin) {
	delete plugin;
}
