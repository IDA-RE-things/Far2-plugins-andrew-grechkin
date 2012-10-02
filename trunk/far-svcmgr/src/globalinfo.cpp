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
#include <guid.hpp>
#include <lang.hpp>

#include <libfar3/helper.hpp>
#include <libfar3/DlgBuilder.hpp>
#include <libfar3/settings.hpp>

#include <libbase/pcstr.hpp>
#include <libbase/logger.hpp>

#include <version.h>


FarGlobalInfo & FarGlobalInfo::inst() {
	static FarGlobalInfo ret;
	return ret;
}

FarGlobalInfo::FarGlobalInfo():
	m_settings(nullptr)
{
	LogTrace();
	addToPluginsMenu = 1;
	addToDisksMenu = 0;
	waitForState = 0;
	waitTimeout = 10 * 1000;
	Base::copy_str(Prefix, L"svcmgr");
}

FarGlobalInfo::~FarGlobalInfo() {
	delete m_settings;
}

PCWSTR FarGlobalInfo::get_author() const {
	return L"© 2012 Andrew Grechkin";
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
	return MAKEFARVERSION(MAJOR, MINOR, BUILD, 0, VS_RC);
}

int FarGlobalInfo::Configure(const ConfigureInfo * /*Info*/) {
	Far::DialogBuilder builder = Far::get_dialog_builder(ConfigDialogGuid, Far::get_msg(Far::DlgTitle), nullptr);
	builder->add_checkbox(Far::get_msg(txtAddToPluginsMenu), &addToPluginsMenu);
	builder->add_checkbox(Far::get_msg(txtAddToDiskMenu), &addToDisksMenu);
	builder->add_checkbox(Far::get_msg(txtWaitForState), &waitForState);
	builder->add_text_before(Far::get_msg(txtPluginPrefix),
		builder->add_editfield(Prefix, Base::lengthof(Prefix)));
	builder->add_OKCancel(Far::get_msg(Far::txtBtnOk), Far::get_msg(Far::txtBtnCancel));

	if (builder->show()) {
		save_settings();
		return true;
	}
	return false;
}

Far::Plugin_i * FarGlobalInfo::CreatePlugin(const PluginStartupInfo * Info) {
	Far::Plugin_i * plugin = create_FarPlugin(this, Info);
	FarGlobalInfo::inst().load_settings();
	return plugin;
}

void FarGlobalInfo::load_settings() {
	if (!m_settings)
		m_settings = new Far::Settings_t(*get_guid());
	addToPluginsMenu = m_settings->get(L"AddToPluginsMenu", addToPluginsMenu);
	addToDisksMenu = m_settings->get(L"AddToDisksMenu", addToDisksMenu);
	waitForState = m_settings->get(L"waitForState", waitForState);
	waitTimeout = m_settings->get(L"waitTimeout", waitTimeout);
	Base::copy_str(Prefix, m_settings->get(L"Prefix", L"svcmgr"));

	LogTrace();
	set_changed(true);
	Base::Event event;
	event.userData = this;
	notify_all(event);
}

void FarGlobalInfo::save_settings() const {
	m_settings->set(L"AddToPluginsMenu", addToPluginsMenu);
	m_settings->set(L"AddToDisksMenu", addToDisksMenu);
	m_settings->set(L"waitForState", waitForState);
	m_settings->set(L"waitTimeout", waitTimeout);
	m_settings->set(L"Prefix", Prefix);

	LogTrace();
	set_changed(true);
	Base::Event event;
	event.userData = (void*)this;
	notify_all(event);
}
