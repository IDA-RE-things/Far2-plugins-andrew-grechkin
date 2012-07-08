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

ustring make_path(const ustring & path, const ustring & name) {
	return path + Base::PATH_SEPARATOR + name;
}

Options::Options() {
	AddToPluginsMenu = 1;
	AddToDisksMenu = 0;
	TimeOut = 0;
	Base::copy_str(Prefix, L"svcmgr");
	Timeout[0] = L'\0';
}

void Options::load() {
	m_settings.reset(new Far::Settings_t(FarPlugin::get_guid()));
//	op = m_settings->get(L"operation", 1);
//	opm = m_settings->get(L"is_text_mask", 0);
}

//void Options::get_parameters(const Far::Dialog & /*dlg*/) {
////	opm = dlg.Check(indIsMask);
//}

void Options::save() const {
//	m_settings->set(L"operation", op);
//	m_settings->set(L"is_text_mask", opm);

}
