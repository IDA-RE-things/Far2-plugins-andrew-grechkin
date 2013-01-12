/**
	svcmgr: Manage services
	Allow to manage windows services
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

#include <globalinfo.hpp>
#include <farplugin.hpp>
#include <guid.hpp>
#include <lang.hpp>

#include <libfar3/helper.hpp>
#include <libfar3/dialog_builder.hpp>
#include <libfar3/settings.hpp>

#include <libbase/pcstr.hpp>
#include <libbase/logger.hpp>

#include <version.h>

FarGlobalInfo::FarGlobalInfo()
{
	LogTrace();
	addToPluginsMenu = 1;
	addToDisksMenu = 0;
	waitForState = 0;
	waitTimeout = 10 * 1000;
	Base::Str::copy(prefix, L"svcmgr");
}

FarGlobalInfo::~FarGlobalInfo() {
	LogTrace();
}

PCWSTR FarGlobalInfo::get_author() const {
	return L"© 2013 Andrew Grechkin";
}

PCWSTR FarGlobalInfo::get_description() const {
	return L"Windows services manager. FAR3 plugin";
}

const GUID * FarGlobalInfo::get_guid() const {
	return &PluginGuid;
}

PCWSTR FarGlobalInfo::get_title() const {
	return L"svcmgr";
}

VersionInfo FarGlobalInfo::get_version() const {
	using namespace AutoVersion;
	return MAKEFARVERSION(MAJOR, MINOR, BUILD, FARMANAGERVERSION_BUILD, VS_RELEASE);
}

VersionInfo FarGlobalInfo::get_min_version() const {
	return MAKEFARVERSION(3, 0, 0, 3000, VS_RELEASE);
}

intptr_t FarGlobalInfo::Configure(const ConfigureInfo * /*Info*/) {
	using namespace Far;
	auto builder = create_dialog_builder(ConfigDialogGuid, get_msg(DlgTitle));
	builder->add_item(create_checkbox(&addToPluginsMenu, txtAddToPluginsMenu));
	builder->add_item(create_checkbox(&addToDisksMenu, txtAddToDiskMenu));
	builder->add_item(create_checkbox(&waitForState, txtWaitForState));
	builder->add_item(create_label(txtPluginPrefix));
	builder->add_item_after(create_edit(prefix, Base::lengthof(prefix)));
	builder->add_item(create_separator());
	builder->add_OKCancel(get_msg(txtBtnOk), get_msg(txtBtnCancel));

	if (builder->show()) {
		save_settings();
	}
	return true;
}

Far::Plugin_i * FarGlobalInfo::CreatePlugin(const PluginStartupInfo * Info) const
{
	Far::Plugin_i * plugin = create_FarPlugin(Info);
	return plugin;
}

void FarGlobalInfo::load_settings() {
	LogTrace();
	Far::Settings_t m_settings(*get_guid());
	addToPluginsMenu = m_settings.get(L"AddToPluginsMenu", addToPluginsMenu);
	addToDisksMenu = m_settings.get(L"AddToDisksMenu", addToDisksMenu);
	waitForState = m_settings.get(L"waitForState", waitForState);
	waitTimeout = m_settings.get(L"waitTimeout", waitTimeout);
	Base::Str::copy(prefix, m_settings.get(L"Prefix", L"svcmgr"));

	set_changed(true);
	notify_all(Base::Message(0, 0, 0, this));
}

void FarGlobalInfo::save_settings() const {
	LogTrace();
	Far::Settings_t m_settings(*get_guid());
	m_settings.set(L"AddToPluginsMenu", addToPluginsMenu);
	m_settings.set(L"AddToDisksMenu", addToDisksMenu);
	m_settings.set(L"waitForState", waitForState);
	m_settings.set(L"waitTimeout", waitTimeout);
	m_settings.set(L"Prefix", prefix);

	set_changed(true);
	notify_all(Base::Message(0, 0, 0, (void*)this));
}

FarGlobalInfo * get_global_info()
{
	return (FarGlobalInfo*)Far::helper_t::inst().get_global_info();
}
