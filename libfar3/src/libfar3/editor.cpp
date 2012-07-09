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

#include <libfar3/helper.hpp>

namespace Far {

	///====================================================================================== Editor
	namespace Editor {
		ssize_t get_filename(PWSTR buf, ssize_t size) {
			if (size)
				return psi().EditorControl(-1, ECTL_GETFILENAME, 0, (void*)buf);
			return psi().EditorControl(-1, ECTL_GETFILENAME, 0, nullptr) + 1;
		}

		ssize_t get_string(ssize_t y, PCWSTR & str) {
			EditorGetString egs = {(int)y};
			psi().EditorControl(-1, ECTL_GETSTRING, 0, &egs);
			str = egs.StringText;
			return egs.StringLength;
		}

		INT_PTR set_position(ssize_t y, ssize_t x) {
			EditorSetPosition esp = {(int)y, (int)x, -1, -1, -1, -1};
			return psi().EditorControl(-1, ECTL_SETPOSITION, 0, &esp);
		}

		INT_PTR set_string(ssize_t y, PCWSTR str, size_t size, PCWSTR eol) {
			EditorSetString ess = {(int)y, (int)size, str, eol};
			return psi().EditorControl(-1, ECTL_SETSTRING, 0, &ess);
		}

		INT_PTR insert_string(ssize_t y, PCWSTR str, int size, PCWSTR eol) {
			if (set_position(y) && psi().EditorControl(-1, ECTL_INSERTSTRING, 0, 0)) {
				return set_string(y, str, size, eol);
			}
			return nullptr;
		}

		INT_PTR del_string(ssize_t y) {
			set_position(y);
			return psi().EditorControl(-1, ECTL_DELETESTRING, 0, nullptr);
		}

		INT_PTR unselect_block() {
			EditorSelect tmp = {BTYPE_NONE};
			return psi().EditorControl(-1, ECTL_SELECT, 0, &tmp);
		}

		INT_PTR start_undo() {
			EditorUndoRedo eur = {EUR_BEGIN, {0}};
			return psi().EditorControl(-1, ECTL_UNDOREDO, 0, &eur);
		}

		INT_PTR stop_undo() {
			EditorUndoRedo eur = {EUR_END, {0}};
			return psi().EditorControl(-1, ECTL_UNDOREDO, 0, &eur);
		}

		INT_PTR redraw() {
			return psi().EditorControl(-1, ECTL_REDRAW, 0, nullptr);
		}

		int get_info(EditorInfo & info) {
			return psi().EditorControl(-1, ECTL_GETINFO, 0, &info);
		}

		int get_string(size_t index, EditorGetString & egs) {
			egs.StringNumber = index;
			return psi().EditorControl(-1, ECTL_GETSTRING, 0, &egs);
		}
	}
}
