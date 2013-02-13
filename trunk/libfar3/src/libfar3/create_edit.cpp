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

	struct PluginEditFieldBinding: public DialogItemBinding_i {
		PluginEditFieldBinding(PWSTR value, ssize_t max_size);

		void save_() const override;

		ssize_t get_width_() const override;

	private:
		PWSTR Value;
		ssize_t MaxSize;
	};

	PluginEditFieldBinding::PluginEditFieldBinding(PWSTR value, ssize_t max_size) :
		Value(value),
		MaxSize(max_size - 1)
	{
		LogTrace();
	}

	void PluginEditFieldBinding::save_() const
	{
		PCWSTR DataPtr = (PCWSTR)psi().SendDlgMessage(get_dlg(), DM_GETCONSTTEXTPTR, get_index(), nullptr);
		lstrcpynW(Value, DataPtr, MaxSize);
		LogDebug(L"value: %s\n", Value);
	}

	ssize_t PluginEditFieldBinding::get_width_() const
	{
		LogTrace();
		return 0;
	}

	FarDialogItem_t * create_edit(PWSTR value, ssize_t max_size, ssize_t width, PCWSTR history_id, bool use_last_history, FARDIALOGITEMFLAGS flags)
	{
		LogTrace();

		auto ret = new FarDialogItem_t(new PluginEditFieldBinding(value, max_size), DI_EDIT, value, flags);
		if (width == -1 || width >= max_size)
			width = max_size - 1;

		ret->X2 = ret->X1 + width - 1;

		if (history_id) {
			ret->History = history_id;
			ret->Flags |= DIF_HISTORY;
			if (use_last_history)
				ret->Flags |= DIF_USELASTHISTORY;
		}

		return ret;
	}

	FarDialogItem_t * create_password(PWSTR value, ssize_t max_size, ssize_t width, FARDIALOGITEMFLAGS flags)
	{
		LogTrace();
		auto ret = new FarDialogItem_t(new PluginEditFieldBinding(value, max_size), DI_PSWEDIT, value, flags);
		if (width == -1 || width >= max_size)
			width = max_size - 1;

		ret->X2 = ret->X1 + width - 1;

		return ret;
	}
}
