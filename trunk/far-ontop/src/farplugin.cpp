/**
	ontop: Always on top FAR3 plugin
	Switch between "always on top" state on/off

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
#include "guid.hpp"

#include <libbase/memory.hpp>

Base::shared_ptr<FarPlugin> plugin;

GUID FarPlugin::get_guid() {
	return PluginGuid;
}

PCWSTR FarPlugin::get_prefix() const {
	static PCWSTR ret = L"ontop";
	return ret;
}

PCWSTR FarPlugin::get_name() {
	return L"ontop";
}

PCWSTR FarPlugin::get_description() {
	return L"Always on top FAR3 plugin";
}

PCWSTR FarPlugin::get_author() {
	return L"© 2012 Andrew Grechkin";
}

FarPlugin::FarPlugin(const PluginStartupInfo * psi) {
	Far::helper_t::inst().init(FarPlugin::get_guid(), psi);
}

void FarPlugin::get_info(PluginInfo * pi) const {
	pi->StructSize = sizeof(*pi);
	pi->Flags = 0;
	static PCWSTR PluginMenuStrings[] = {Far::get_msg(Far::MenuTitle)};
	pi->PluginMenu.Guids = &MenuGuid;
	pi->PluginMenu.Strings = PluginMenuStrings;
	pi->PluginMenu.Count = Base::lengthof(PluginMenuStrings);
	pi->CommandPrefix = get_prefix();
}

HANDLE FarPlugin::open(const OpenInfo * /*Info*/) {
	return nullptr;
}
