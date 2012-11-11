#ifndef EDITOR_HPP_
#define EDITOR_HPP_

#include <libfar3/plugin.hpp>

namespace Far {

	///====================================================================================== Editor
	namespace Editor {
		ssize_t get_filename(PWSTR buf, ssize_t size);

		ssize_t get_string(ssize_t y, PCWSTR & str);

		INT_PTR set_position(ssize_t y, ssize_t x = -1);

		INT_PTR set_string(ssize_t y, PCWSTR str, size_t size, PCWSTR eol);

		INT_PTR insert_string(ssize_t y, PCWSTR str, int size, PCWSTR eol);

		INT_PTR del_string(ssize_t y);

		INT_PTR unselect_block();

		INT_PTR start_undo();

		INT_PTR stop_undo();

		INT_PTR redraw();

		int get_info(EditorInfo & info);

		int get_string(size_t index, EditorGetString & egs);
	}

}

#endif /* EDITOR_HPP_ */
