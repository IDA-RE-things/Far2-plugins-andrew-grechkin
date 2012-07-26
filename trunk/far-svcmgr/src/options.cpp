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

#include "farplugin.hpp"

#include "guid.hpp"
#include "lang.hpp"

#include <libfar3/DlgBuilder.hpp>

ustring make_path(const ustring & path, const ustring & name) {
	return path + Base::PATH_SEPARATOR + name;
}

Options::Options() {
	AddToPluginsMenu = 1;
	AddToDisksMenu = 0;
	TimeOut = 0;
	Base::copy_str(Prefix, L"svcmgr");
	Base::copy_str(Timeout, Base::EMPTY_STR);
}

void Options::load() {
	m_settings.reset(new Far::Settings_t(FarPlugin::get_guid()));
	AddToPluginsMenu = m_settings->get(L"AddToPluginsMenu", AddToPluginsMenu);
	AddToDisksMenu = m_settings->get(L"AddToDisksMenu", AddToDisksMenu);
	Base::copy_str(Prefix, m_settings->get(L"Prefix", L"svcmgr"));
	Base::copy_str(Timeout, m_settings->get(L"Timeout", L"30"));
	TimeOut = Far::fsf().atoi64(Timeout);
}

void Options::save() const {
	m_settings->set(L"AddToPluginsMenu", AddToPluginsMenu);
	m_settings->set(L"AddToDisksMenu", AddToDisksMenu);
	m_settings->set(L"Prefix", Prefix);
	m_settings->set(L"Timeout", Timeout);
}

int Options::configure() {
	LogTrace();
	Far::DialogBuilder builder = Far::get_dialog_builder(ConfigDialogGuid, Far::get_msg(Far::DlgTitle), nullptr);
	builder->add_checkbox(Far::get_msg(txtAddToPluginsMenu), &AddToPluginsMenu);
	builder->add_checkbox(Far::get_msg(txtAddToDiskMenu), &AddToDisksMenu);
	builder->add_text_before(Far::get_msg(txtPluginPrefix),
		builder->add_editfield(Prefix, Base::lengthof(Prefix)));
	builder->add_text_before(Far::get_msg(txtTimeout),
		builder->add_fixeditfield(Timeout, Base::lengthof(Timeout), -1, L"99"));
	builder->add_OKCancel(Far::get_msg(Far::txtBtnOk), Far::get_msg(Far::txtBtnCancel));

	if (builder->show()) {
		save();
		TimeOut = Far::fsf().atoi64(Timeout);
		return true;
	}
	return false;
}
