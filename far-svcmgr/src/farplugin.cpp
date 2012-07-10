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
#include "lang.hpp"

#include <libbase/logger.hpp>

#include <libfar3/DlgBuilder.hpp>

using namespace Base;
using namespace Ext;

Base::shared_ptr<FarPlugin> plugin;

FarPlugin::FarPlugin(const PluginStartupInfo * psi) {
	LogTrace();
	Far::helper_t::inst().init(FarPlugin::get_guid(), psi);
	options.load();
}

bool FarPlugin::execute() const {
	return true;
}

PCWSTR FarPlugin::get_prefix() const {
	static PCWSTR ret = L"svcmgr";
	return ret;
}

GUID FarPlugin::get_guid() {
	return PluginGuid;
}

PCWSTR FarPlugin::get_name() {
	return L"svcmgr";
}

PCWSTR FarPlugin::get_description() {
	return L"Windows services manager. FAR3 plugin";
}

PCWSTR FarPlugin::get_author() {
	return L"© 2012 Andrew Grechkin";
}

void FarPlugin::get_info(PluginInfo * pi) const {
	LogTrace();
	pi->StructSize = sizeof(*pi);
	pi->Flags = PF_NONE;

	static GUID PluginMenuGuids[] = {MenuGuid,};
	static PCWSTR PluginMenuStrings[] = {Far::get_msg(Far::MenuTitle),};

	if (options.AddToPluginsMenu) {
		pi->PluginMenu.Guids = PluginMenuGuids;
		pi->PluginMenu.Strings = PluginMenuStrings;
		pi->PluginMenu.Count = lengthof(PluginMenuStrings);
	}

	static PCWSTR DiskStrings[] = {Far::get_msg(Far::DiskTitle),};
	if (options.AddToDisksMenu) {
		pi->DiskMenu.Guids = PluginMenuGuids;
		pi->DiskMenu.Strings = DiskStrings;
		pi->DiskMenu.Count = lengthof(DiskStrings);
	}

	pi->PluginConfig.Guids = PluginMenuGuids;
	pi->PluginConfig.Strings = PluginMenuStrings;
	pi->PluginConfig.Count = lengthof(PluginMenuStrings);
	pi->CommandPrefix = options.Prefix;
}

HANDLE FarPlugin::open(const OpenInfo * Info)
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

void FarPlugin::close(HANDLE hndl) {
	LogTrace();
	static_cast<Far::IPanel*>(hndl)->destroy();
}

int FarPlugin::configure() {
	LogTrace();
	Far::DialogBuilder builder = Far::get_dialog_builder(ConfigDialogGuid, Far::get_msg(Far::DlgTitle), nullptr);
	builder->add_checkbox(Far::get_msg(txtAddToPluginsMenu), &options.AddToPluginsMenu);
	builder->add_checkbox(Far::get_msg(txtAddToDiskMenu), &options.AddToDisksMenu);
	builder->add_text_before(Far::get_msg(txtPluginPrefix),
		builder->add_editfield(options.Prefix, lengthof(options.Prefix)));
	builder->add_text_before(Far::get_msg(txtTimeout),
		builder->add_fixeditfield(options.Timeout, lengthof(options.Timeout), -1, L"99"));
	builder->add_OKCancel(Far::get_msg(Far::txtBtnOk), Far::get_msg(Far::txtBtnCancel));

	if (builder->show()) {
		options.save();
		return true;
	}
	return false;
}
