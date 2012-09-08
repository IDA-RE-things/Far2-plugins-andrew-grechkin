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

#include <panelactions.hpp>

#include <panelcontroller.hpp>

#include <libext/exception.hpp>


PanelActions::PanelActions():
	m_titles({0})
{
}

void PanelActions::add(WORD Key, DWORD Control, PCWSTR text, ptrToFunc func, PCWSTR long_text) {
	if (text) {
		FarKey key = {Key, Control};
		KeyBarLabel lab = {key, text, long_text};
		m_labels.push_back(lab);
	}
	if (func) {
		FarKey key = {Key, Control};
		KeyAction act = {key, func};
		m_actions.push_back(act);
	}
	m_titles.CountLabels = m_labels.size();
	m_titles.Labels = &m_labels[0];
}

bool PanelActions::exec_func(PanelController * panel, WORD Key, DWORD Control) const {
//	LogDebug(L"panel = %p, key = %d, Control = %d\n", panel, (int32_t) Key, Control);
	try {
		for (size_t i = 0; i < m_actions.size(); ++i) {
			if (Control == m_actions[i].Key.ControlKeyState && Key == m_actions[i].Key.VirtualKeyCode) {
				return (panel->*(m_actions[i].Action))();
			}
		}
	} catch (Ext::AbstractError & e) {
		Base::mstring msg;
		e.format_error(msg);
		Far::ebox(msg);
	}
	return false;
}

const KeyBarTitles * PanelActions::get_titles() const {
	return &m_titles;
}

