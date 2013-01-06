﻿/**
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

namespace Far {

	DialogItemBinding_i::~DialogItemBinding_i()
	{
	}

	void DialogItemBinding_i::set_dlg(HANDLE * hndl)
	{
		if (this)
			m_dlg = hndl;
	}

	void DialogItemBinding_i::set_index(ssize_t index)
	{
		if (this)
			m_index = index;
	}

	void DialogItemBinding_i::save() const
	{
		if (this)
			save_();
	}

	ssize_t DialogItemBinding_i::get_height() const
	{
		return (this) ? get_height_() : 1;
	}

	ssize_t DialogItemBinding_i::get_width() const
	{
		return (this) ? get_width_() : 0;
	}

	ssize_t DialogItemBinding_i::get_height_() const
	{
		return 1;
	}

}
