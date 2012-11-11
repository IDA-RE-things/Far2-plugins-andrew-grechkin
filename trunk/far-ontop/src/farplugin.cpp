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

#include <farplugin.hpp>

#include <globalinfo.hpp>
#include <guid.hpp>
#include <lang.hpp>

#include <libfar3/helper.hpp>

#include <libbase/std.hpp>
#include <libbase/logger.hpp>


///======================================================================================= FarPlugin
struct FarPlugin: public Far::Plugin_i {
	FarPlugin(Far::GlobalInfo_i * gi, const PluginStartupInfo * Info);

	~FarPlugin() override;

	void GetInfo(PluginInfo * Info) override;

	Far::PanelController_i * Open(const OpenInfo * Info) override;

private:
	mutable WCHAR menu_item[64];
	HWND m_hwnd;
	bool m_state;
};


FarPlugin::FarPlugin(Far::GlobalInfo_i * gi, const PluginStartupInfo * Info):
	Far::Plugin_i(gi, Info),
	m_hwnd(::GetForegroundWindow()),
	m_state(false)
{
	LogTrace();
}

FarPlugin::~FarPlugin() {
	LogTrace();
}

void FarPlugin::GetInfo(PluginInfo * Info) {
	LogTrace();
	Info->Flags = PF_EDITOR | PF_VIEWER | PF_DIALOG;

	static GUID PluginMenuGuids[] = {MenuGuid,};
	static PCWSTR PluginMenuStrings[] = {menu_item,};

	::wcsncpy(menu_item, Far::get_msg(Far::MenuTitle), Base::lengthof(menu_item));
	::wcsncat(menu_item, Far::get_msg(m_state ? MsgOff : MsgOn), Base::lengthof(menu_item));

	Info->PluginMenu.Guids = PluginMenuGuids;
	Info->PluginMenu.Strings = PluginMenuStrings;
	Info->PluginMenu.Count = Base::lengthof(PluginMenuStrings);

	Info->CommandPrefix = FarGlobalInfo::inst().Prefix;
}

Far::PanelController_i * FarPlugin::Open(const OpenInfo * /*Info*/) {
	if (!m_state)
		m_state = ::SetWindowPos(m_hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	else
		m_state = !::SetWindowPos(m_hwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	return nullptr;
}


///=================================================================================================
Far::Plugin_i * create_FarPlugin(Far::GlobalInfo_i * gi, const PluginStartupInfo * psi) {
	return new FarPlugin(gi, psi);
}

void destroy(Far::Plugin_i * plugin) {
	delete plugin;
}
