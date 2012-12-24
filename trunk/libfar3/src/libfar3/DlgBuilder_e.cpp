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

	PluginIntEditFieldBinding::PluginIntEditFieldBinding(HANDLE & aHandle, ssize_t aId, ssize_t * aValue, ssize_t Width) :
		DialogItemBinding_i(&aHandle, aId),
		Value(aValue),
		m_width(Width < 31 ? Width : 31)
	{
		fsf().sprintf(Buffer, L"%u", *aValue);
		for (ssize_t i = 1; i < m_width; ++i)
			Mask[i] = L'9';
		Mask[0] = L'#';
		Mask[m_width] = L'\0';
	}

	void PluginIntEditFieldBinding::save_() const
	{
		PCWSTR DataPtr = (PCWSTR)psi().SendDlgMessage(get_dlg(), DM_GETCONSTTEXTPTR, get_index(), 0);
		*Value = fsf().atoi(DataPtr);
	}

	ssize_t PluginIntEditFieldBinding::get_width_() const
	{
		return m_width;
	}

	PWSTR PluginIntEditFieldBinding::GetBuffer()
	{
		return Buffer;
	}

	PCWSTR PluginIntEditFieldBinding::GetMask() const
	{
		return Mask;
	}

}
