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

#include <libfar3/editor.hpp>
#include <libfar3/helper.hpp>

namespace Far {

	///====================================================================================== Editor
	namespace Editor {
		ssize_t get_filename(PWSTR buf, ssize_t size)
		{
			if (size)
				return psi().EditorControl(-1, ECTL_GETFILENAME, 0, (void*)buf);
			return psi().EditorControl(-1, ECTL_GETFILENAME, 0, nullptr) + 1;
		}

		ssize_t get_string(ssize_t y, PCWSTR & str)
		{
			EditorGetString egs = {sizeof(egs), y};
			psi().EditorControl(-1, ECTL_GETSTRING, 0, &egs);
			str = egs.StringText;
			return egs.StringLength;
		}

		intptr_t set_position(ssize_t y, ssize_t x)
		{
			EditorSetPosition esp = {sizeof(esp), y, x, -1, -1, -1, -1};
			return psi().EditorControl(-1, ECTL_SETPOSITION, 0, &esp);
		}

		intptr_t set_string(ssize_t y, PCWSTR str, size_t size, PCWSTR eol)
		{
			EditorSetString ess = {sizeof(ess), y, (intptr_t)size, str, eol};
			return psi().EditorControl(-1, ECTL_SETSTRING, 0, &ess);
		}

		intptr_t insert_string(ssize_t y, PCWSTR str, int size, PCWSTR eol)
		{
			if (set_position(y) && psi().EditorControl(-1, ECTL_INSERTSTRING, 0, 0)) {
				return set_string(y, str, size, eol);
			}
			return 0;
		}

		intptr_t del_string(ssize_t y)
		{
			set_position(y);
			return psi().EditorControl(-1, ECTL_DELETESTRING, 0, nullptr);
		}

		intptr_t unselect_block()
		{
			EditorSelect tmp = {sizeof(tmp), BTYPE_NONE};
			return psi().EditorControl(-1, ECTL_SELECT, 0, &tmp);
		}

		intptr_t start_undo()
		{
			EditorUndoRedo eur = {sizeof(eur), EUR_BEGIN};
			return psi().EditorControl(-1, ECTL_UNDOREDO, 0, &eur);
		}

		intptr_t stop_undo()
		{
			EditorUndoRedo eur = {sizeof(eur), EUR_END};
			return psi().EditorControl(-1, ECTL_UNDOREDO, 0, &eur);
		}

		intptr_t redraw()
		{
			return psi().EditorControl(-1, ECTL_REDRAW, 0, nullptr);
		}

		intptr_t get_info(EditorInfo & info)
		{
			info.StructSize = sizeof(info);
			return psi().EditorControl(-1, ECTL_GETINFO, 0, &info);
		}

		intptr_t get_string(size_t index, EditorGetString & egs)
		{
			egs.StructSize = sizeof(egs);
			egs.StringNumber = index;
			return psi().EditorControl(-1, ECTL_GETSTRING, 0, &egs);
		}
	}

}
