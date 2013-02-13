/**
 sortstr: Sort strings in editor
 FAR3 plugin

 © 2013 Andrew Grechkin

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
#include <process.hpp>

#include <libfar3/helper.hpp>

#include <libbase/logger.hpp>

#include <globalinfo.hpp>
#include <guid.hpp>
#include <lang.hpp>

///======================================================================================= FarPlugin
struct FarPlugin: public Far::Plugin_i {
	FarPlugin(const PluginStartupInfo * Info);

	~FarPlugin() override;

	void GetPluginInfo(PluginInfo * Info) override;

	Far::PanelController_i * Open(const OpenInfo * Info) override;
};

FarPlugin::FarPlugin(const PluginStartupInfo * Info) :
	Far::Plugin_i(Info)
{
	LogTrace();
}

FarPlugin::~FarPlugin()
{
	LogTrace();
}

void FarPlugin::GetPluginInfo(PluginInfo * Info)
{
	LogTrace();
	Info->Flags = PF_DISABLEPANELS | PF_EDITOR;

	static GUID PluginMenuGuids[] = {MenuGuid,};
	static PCWSTR PluginMenuStrings[] = {Far::get_msg(Far::MenuTitle),};
	PluginMenuStrings[0] = Far::get_msg(Far::MenuTitle);

	Info->PluginMenu.Guids = PluginMenuGuids;
	Info->PluginMenu.Strings = PluginMenuStrings;
	Info->PluginMenu.Count = Base::lengthof(PluginMenuStrings);

	Info->CommandPrefix = get_global_info()->prefix;
}

Far::PanelController_i * FarPlugin::Open(const OpenInfo * /*Info*/)
{
	LogTrace();
	process();
	return nullptr;
}

///=================================================================================================
Far::Plugin_i * create_FarPlugin(const PluginStartupInfo * psi)
{
	return new FarPlugin(psi);
}

void destroy(Far::Plugin_i * plugin)
{
	delete plugin;
}
