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

#include "guid.hpp"
#include "farplugin.hpp"
#include "globalinfo.hpp"
#include "options.hpp"
#include "panel.hpp"
#include "lang.hpp"

#include <libbase/logger.hpp>

#include <libfar3/DlgBuilder.hpp>

using namespace Base;
using namespace Ext;

Base::shared_ptr<Far::Plugin_i> plugin;


FarPlugin * FarPlugin::create(const PluginStartupInfo * psi) {
	return new FarPlugin(psi);
}

void FarPlugin::destroy() {
	delete this;
}

void FarPlugin::GetPluginInfo(PluginInfo * pi) {
	LogTrace();
	pi->StructSize = sizeof(*pi);
	pi->Flags = PF_NONE;

	static GUID PluginMenuGuids[] = {MenuGuid,};
	static PCWSTR PluginMenuStrings[] = {Far::get_msg(Far::MenuTitle),};

	if (options->AddToPluginsMenu) {
		pi->PluginMenu.Guids = PluginMenuGuids;
		pi->PluginMenu.Strings = PluginMenuStrings;
		pi->PluginMenu.Count = lengthof(PluginMenuStrings);
	}

	static PCWSTR DiskStrings[] = {Far::get_msg(Far::DiskTitle),};
	if (options->AddToDisksMenu) {
		pi->DiskMenu.Guids = PluginMenuGuids;
		pi->DiskMenu.Strings = DiskStrings;
		pi->DiskMenu.Count = lengthof(DiskStrings);
	}

	pi->PluginConfig.Guids = PluginMenuGuids;
	pi->PluginConfig.Strings = PluginMenuStrings;
	pi->PluginConfig.Count = lengthof(PluginMenuStrings);
	pi->CommandPrefix = options->Prefix;
}

HANDLE FarPlugin::Open(const OpenInfo * Info)
{
	LogTrace();
	HANDLE ret = nullptr;
	try {
		LogTrace();
		ret = ServicePanel::create(Info);
		LogTrace();
	} catch (AbstractError & e) {
		LogDebug(L"%s\n", e.what().c_str());
		Far::ebox(e.format_error());
		LogTrace();
	}
	LogTrace();
	return ret;
}

void FarPlugin::Close(const ClosePanelInfo * Info) {
	LogTrace();
	if (Info->hPanel)
		static_cast<Far::IPanel*>(Info->hPanel)->destroy();
}

int FarPlugin::Configure(const ConfigureInfo * /*Info*/) {
	return options->configure();
}

FarPlugin::~FarPlugin() {
}

FarPlugin::FarPlugin(const PluginStartupInfo * psi) {
	LogTrace();
	Far::helper_t::inst().init(globalInfo->get_guid(), psi);
	options->load();
}
