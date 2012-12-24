/**
 © 2012 Andrew Grechkin
 Source code: <http://code.google.com/p/andrew-grechkin>

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program. If not, see <http://www.gnu.org/licenses/>.
 **/

#include "DlgBuilder_pvt.hpp"

namespace Far {

	PluginRadioButtonBinding::PluginRadioButtonBinding(HANDLE & aHandle, ssize_t aId, ssize_t * aValue, ssize_t RadioGroupIndex) :
		DialogItemBinding_i(&aHandle, aId),
		Value(aValue),
		m_rg_index(RadioGroupIndex)
	{
	}

	void PluginRadioButtonBinding::save_() const
	{
		if (psi().SendDlgMessage(get_dlg(), DM_GETCHECK, get_index(), nullptr)) {
			*Value = m_rg_index;
		}
	}

	ssize_t PluginRadioButtonBinding::get_width_() const
	{
//		return ::lstrlenW(get_item()->Data) + 4;
		return 0;
	}

}
