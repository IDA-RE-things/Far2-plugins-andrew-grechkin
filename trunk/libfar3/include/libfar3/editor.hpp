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

#ifndef _LIBFAR_EDITOR_HPP_
#define _LIBFAR_EDITOR_HPP_

#include <libfar3/plugin.hpp>

namespace Far {

	namespace Editor {

		ssize_t get_filename(PWSTR buf, ssize_t size);

		ssize_t get_string(ssize_t y, PCWSTR & str);

		intptr_t set_position(ssize_t y, ssize_t x = -1);

		intptr_t set_string(ssize_t y, PCWSTR str, size_t size, PCWSTR eol);

		intptr_t insert_string(ssize_t y, PCWSTR str, int size, PCWSTR eol);

		intptr_t del_string(ssize_t y);

		intptr_t unselect_block();

		intptr_t start_undo();

		intptr_t stop_undo();

		intptr_t redraw();

		intptr_t get_info(EditorInfo & info);

		intptr_t get_string(size_t index, EditorGetString & egs);
	}

}

#endif
