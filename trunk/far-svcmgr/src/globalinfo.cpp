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
#include <guid.hpp>
#include <lang.hpp>

#include <libfar3/helper.hpp>
#include <libfar3/DlgBuilder.hpp>

#include <libbase/pcstr.hpp>
#include <libbase/logger.hpp>

#include <version.h>


Base::shared_ptr<FarGlobalInfo> globalInfo;


FarGlobalInfo::FarGlobalInfo():
	m_settings(nullptr)
{
	LogTrace();
	AddToPluginsMenu = 1;
	AddToDisksMenu = 0;
	TimeOut = 30;
	Base::copy_str(Prefix, L"svcmgr");
}

FarGlobalInfo::~FarGlobalInfo() {
	delete m_settings;
}

GUID FarGlobalInfo::get_guid() const {
	return PluginGuid;
}

PCWSTR FarGlobalInfo::get_name() const {
	return L"svcmgr";
}

PCWSTR FarGlobalInfo::get_description() const {
	return L"Windows services manager. FAR3 plugin";
}

PCWSTR FarGlobalInfo::get_author() const {
	return L"© 2012 Andrew Grechkin";
}

VersionInfo FarGlobalInfo::get_version() const {
	using namespace AutoVersion;
	return MAKEFARVERSION(MAJOR, MINOR, BUILD, 0, VS_RC);
}

int FarGlobalInfo::Configure(const ConfigureInfo * /*Info*/) {
	Far::DialogBuilder builder = Far::get_dialog_builder(ConfigDialogGuid, Far::get_msg(Far::DlgTitle), nullptr);
	builder->add_checkbox(Far::get_msg(txtAddToPluginsMenu), &AddToPluginsMenu);
	builder->add_checkbox(Far::get_msg(txtAddToDiskMenu), &AddToDisksMenu);
	builder->add_text_before(Far::get_msg(txtPluginPrefix),
		builder->add_editfield(Prefix, Base::lengthof(Prefix)));
	builder->add_text_before(Far::get_msg(txtTimeout),
		builder->add_inteditfield(&TimeOut, 2));
	builder->add_OKCancel(Far::get_msg(Far::txtBtnOk), Far::get_msg(Far::txtBtnCancel));

	if (builder->show()) {
		save_settings();
		return true;
	}
	return false;
}


void FarGlobalInfo::load_settings() {
	if (!m_settings)
		m_settings = new Far::Settings_t(get_guid());
	AddToPluginsMenu = m_settings->get(L"AddToPluginsMenu", AddToPluginsMenu);
	AddToDisksMenu = m_settings->get(L"AddToDisksMenu", AddToDisksMenu);
	TimeOut = m_settings->get(L"wait_timeout", TimeOut);
	Base::copy_str(Prefix, m_settings->get(L"Prefix", L"svcmgr"));
}

void FarGlobalInfo::save_settings() const {
	m_settings->set(L"AddToPluginsMenu", AddToPluginsMenu);
	m_settings->set(L"AddToDisksMenu", AddToDisksMenu);
	m_settings->set(L"wait_timeout", TimeOut);
	m_settings->set(L"Prefix", Prefix);
}


///=================================================================================================
FarGlobalInfo * create_GlobalInfo() {
	return new FarGlobalInfo;
}

void destroy(FarGlobalInfo * info) {
	delete info;
}
