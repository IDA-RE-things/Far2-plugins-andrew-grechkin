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

#include <libfar3/dialog_builder.hpp>
#include <libfar3/helper.hpp>

#include <libbase/std.hpp>
#include <libbase/logger.hpp>

namespace Far {

	struct PluginComboBoxBinding: public DialogItemBinding_i {
		PluginComboBoxBinding(ssize_t * value, FarList * items);

		void save_() const override;

		ssize_t get_width_() const override;

	private:
		ssize_t * Value;
		FarList * m_items;
	};

	PluginComboBoxBinding::PluginComboBoxBinding(ssize_t * value, FarList * items) :
		Value(value),
		m_items(items)
	{
	}

	void PluginComboBoxBinding::save_() const
	{
		*Value = psi().SendDlgMessage(get_dlg(), DM_LISTGETCURPOS, get_index(), 0);
		LogDebug(L"value: %Id\n", *Value);
	}

	ssize_t PluginComboBoxBinding::get_width_() const
	{
		ssize_t width = 0;
		for (size_t i = 0; i < m_items->ItemsNumber; ++i)
			width = std::max(width, (ssize_t)lstrlenW(m_items->Items[i].Text));
		return width + 9;
	}

	FarDialogItem_t * create_combobox(ssize_t * value, FarList * items, FARDIALOGITEMFLAGS flags)
	{
		LogTrace();
		items->Items[*value].Flags |= LIF_SELECTED;
		auto ret = new FarDialogItem_t(new PluginComboBoxBinding(value, items), DI_COMBOBOX, nullptr, flags);
		ret->ListItems = items;
		ret->X2 = ret->X1 + ret->get_width();

		return ret;
	}

}
