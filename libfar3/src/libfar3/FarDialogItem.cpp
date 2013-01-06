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

#include <libbase/std.hpp>
#include <libbase/logger.hpp>

namespace Far {

	ssize_t inline TextWidth(const FarDialogItem_t * Item)
	{
		return ::lstrlenW(Item->Data);
	}

	FarDialogItem_t::~FarDialogItem_t()
	{
		delete (DialogItemBinding_i*)UserData;
	}

	FarDialogItem_t::FarDialogItem_t(FARDIALOGITEMTYPES Type_, PCWSTR Text_, FARDIALOGITEMFLAGS flags_)
	{
		LogTrace();
		::memset(this, 0, sizeof(*this));
		Type = Type_;
		Data = Text_;
		Flags = flags_;
	}

	FarDialogItem_t::FarDialogItem_t(DialogItemBinding_i * binding, FARDIALOGITEMTYPES Type_, PCWSTR Text_, FARDIALOGITEMFLAGS flags_)
	{
		LogTrace();
		::memset(this, 0, sizeof(*this));
		Type = Type_;
		Data = Text_;
		Flags = flags_;
		UserData = (intptr_t)binding;
	}

	FarDialogItem_t::FarDialogItem_t(FarDialogItem_t && right):
		FarDialogItem(right)
	{
		LogTrace();
		right.UserData = 0;
	}

	FarDialogItem_t & FarDialogItem_t::operator = (FarDialogItem_t && right)
	{
		LogTrace();
		::memcpy(this, &right, sizeof(*this));
		right.UserData = 0;
		return *this;
	}

	ssize_t FarDialogItem_t::get_height() const
	{
		LogTrace();
		return ((DialogItemBinding_i*)UserData)->get_height();
	}

	ssize_t FarDialogItem_t::get_width() const
	{
		switch (Type) {
			case DI_TEXT:
			case DI_DOUBLEBOX:
				return TextWidth(this);

			case DI_CHECKBOX:
			case DI_RADIOBUTTON:
			case DI_BUTTON:
				return TextWidth(this) + 4;

			case DI_EDIT:
			case DI_FIXEDIT:
			case DI_PSWEDIT: {
				int Width = X2 - X1 + 1;
				// стрелка history занимает дополнительное место, но раньше она рисовалась поверх рамки???
				if (Flags & DIF_HISTORY)
					Width++;
				return Width;
				break;
			}

			case DI_SINGLEBOX:
				return X2 - X1 + 1;

			default:
				return ((DialogItemBinding_i*)UserData)->get_width();
				break;
		}
		return 0;
	}

	void FarDialogItem_t::set_dlg(HANDLE * dlg)
	{
		((DialogItemBinding_i*)UserData)->set_dlg(dlg);
	}

	void FarDialogItem_t::set_index(ssize_t ind)
	{
		((DialogItemBinding_i*)UserData)->set_index(ind);
	}

	void FarDialogItem_t::save() const
	{
		((DialogItemBinding_i*)UserData)->save();
	}

}
