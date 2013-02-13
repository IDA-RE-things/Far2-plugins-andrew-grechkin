/**
	delstr: Delete strings in editor
	FAR3lua plugin

	© 2013 Andrew Grechkin

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

#include <process.hpp>
#include <lang.hpp>
#include <guid.hpp>
#include <globalinfo.hpp>

#include <libfar3/plugin.hpp>
#include <libfar3/editor.hpp>
#include <libfar3/dialog_builder.hpp>
#include <libbase/std.hpp>
#include <libbase/pcstr.hpp>
#include <libbase/logger.hpp>

#include <functional>
#include <cmath>
#include <vector>
#include <cstdlib>

void delete_string(size_t & index, size_t & total, size_t & current)
{
	LogNoise(L"index: %Iu, total: %Iu, current: %Iu\n", index, total, current);
	if (index < current)
		current--;
	Far::Editor::del_string(index--);
	total--;
}

bool Execute()
{
	LogTrace();
	if (!get_global_info()->get_total_lines())
		return false;

	size_t current = get_global_info()->get_current_line();
	size_t total = get_global_info()->get_total_lines();

	LogNoise(L"get_first_line(): %Iu\n", get_global_info()->get_first_line());
	LogNoise(L"get_block_type(): %Id\n", get_global_info()->get_block_type());
	LogNoise(L"get_current_line(): %Iu\n", current);
	LogNoise(L"get_total_lines(): %Iu\n", total);
	LogNoise(L"rbValue_operation: %Id\n", get_global_info()->rbValue_operation);
	Far::Editor::start_undo();

	EditorGetString egs;
	for (size_t i = get_global_info()->get_first_line(); i < total; ++i) {
		if (!Far::Editor::get_string(i, egs))
			break;

		if (i == (total - 1) && Base::Str::is_empty(egs.StringText))
			break;

		if (get_global_info()->get_block_type() != BTYPE_NONE && (egs.SelStart == -1 || egs.SelStart == egs.SelEnd))
			break;

		switch (get_global_info()->rbValue_operation) {
			case 0:
				if (Base::Str::is_empty(egs.StringText))
					delete_string(i, total, current);
				break;
			case 1:
				if (i > get_global_info()->get_first_line()) {
					static EditorGetString pred;
					Far::Editor::get_string(i - 1, pred);
					if (Base::Str::is_empty(pred.StringText) && Base::Str::is_empty(egs.StringText)) {
						delete_string(i, total, current);
					}
				}
				break;
			case 2:
				if (get_global_info()->cbValue_ismask) {
					if (Far::fsf().ProcessName((PWSTR)get_global_info()->text, (PWSTR)egs.StringText, 0, PN_CMPNAMELIST | PN_SKIPPATH)) {
						delete_string(i, total, current);
					}
				} else if (Base::Str::find(egs.StringText, get_global_info()->text)) {
					delete_string(i, total, current);
				}
				break;
			case 3:
				if (get_global_info()->cbValue_ismask) {
					if (!Far::fsf().ProcessName((PWSTR)get_global_info()->text, (PWSTR)egs.StringText, 0, PN_CMPNAMELIST | PN_SKIPPATH)) {
						delete_string(i, total, current);
					}
				} else if (!Base::Str::find(egs.StringText, get_global_info()->text)) {
					delete_string(i, total, current);
				}
				break;
		}
	}
	LogNoise(L"get_current_line(): %Iu\n", current);
	LogNoise(L"get_total_lines(): %Iu\n", total);
	Far::Editor::set_position(current, get_global_info()->get_current_column());
	Far::Editor::stop_undo();

	Far::Editor::redraw();

	return true;
}

enum {
	HEIGHT = 11,
	WIDTH = 70,

	indDelAll = 1,
	indDelRep,
	indDelWithText,
	indDelWithoutText,
	indText,
	indIsMask,
};

void process()
{
	LogTrace();
	get_global_info()->load_settings();
	get_global_info()->load_editor_info();

	using namespace Far;
	AddRadioButton_t buttons[] = {
		{rbDelAll, 0},
		{rbDelRepeated, 0},
		{rbDelWithText, 0},
		{rbDelWithoutText, 0},
	};

	auto Builder = create_dialog_builder(DialogGuid, get_msg(DlgTitle));
	Builder->add_radiobuttons(&get_global_info()->rbValue_operation, Base::lengthof(buttons), buttons, true);
	Builder->add_item(create_edit(get_global_info()->text, Base::lengthof(get_global_info()->text), 32, L"delstr.text"));
	Builder->add_item_after(create_checkbox(&get_global_info()->cbValue_ismask, cbMask));
	Builder->add_item(create_separator());
	Builder->add_OKCancel(get_msg(txtBtnOk), get_msg(txtBtnCancel));
	if (Builder->show()) {
		get_global_info()->save_settings();
		Execute();
	}
}
