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

#include "panel.hpp"

#include <libext/exception.hpp>


///==================================================================================== PanelActions
void PanelActions::add(WORD Key, DWORD Control, PCWSTR text, ptrToFunc func, PCWSTR long_text) {
	if (text) {
		FarKey key = {Key, Control};
		KeyBarLabel lab = {key, text, long_text};
		labels.push_back(lab);
	}
	if (func) {
		FarKey key = {Key, Control};
		KeyAction act = {key, func};
		actions.push_back(act);
	}
}

size_t PanelActions::size() const {
	return labels.size();
}

KeyBarLabel * PanelActions::get_labels() {
	return &labels[0];
}

bool PanelActions::exec_func(ServicePanel * panel, WORD Key, DWORD Control) const {
//	LogDebug(L"panel = %p, key = %d, Control = %d\n", panel, (int32_t) Key, Control);
	try {
		for (size_t i = 0; i < actions.size(); ++i) {
			if (Control == actions[i].Key.ControlKeyState && Key == actions[i].Key.VirtualKeyCode) {
				return (panel->*(actions[i].Action))();
			}
		}
	} catch (Ext::AbstractError & e) {
		Base::mstring msg;
		e.format_error(msg);
		Far::ebox(msg);
	}
	return false;
}
