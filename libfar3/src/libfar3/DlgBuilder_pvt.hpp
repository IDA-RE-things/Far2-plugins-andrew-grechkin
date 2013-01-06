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

#ifndef _LIBFAR_DLGBUILDER_PVT_HPP_
#define _LIBFAR_DLGBUILDER_PVT_HPP_

#include <libfar3/helper.hpp>
#include <libfar3/dialog_builder.hpp>
#include <libfar3/dialog_builder_ex.hpp>

#include <vector>

namespace Far {

	///=============================================================================================
	struct SimpleDialogBuilder_impl: public DialogBuilder_i {
		SimpleDialogBuilder_impl(const GUID & aId, PCWSTR Label, PCWSTR aHelpTopic = nullptr, FARWINDOWPROC aDlgProc = nullptr, void * aUserParam = nullptr);

		~SimpleDialogBuilder_impl() override;

		FarDialogItem_t * add_item_(FarDialogItem_t * item) override;

		FarDialogItem_t * add_item_before_(FarDialogItem_t * item) override;

		FarDialogItem_t * add_item_after_(FarDialogItem_t * item) override;

		void add_empty_line_() override;

		void add_OKCancel_(PCWSTR OKLabel, PCWSTR CancelLabel, PCWSTR ExtraLabel) override;

		void start_column_() override;

		void break_column_() override;

		void end_column_() override;

		void start_singlebox_(ssize_t Width, PCWSTR Label, bool LeftAlign) override;

		void end_singlebox_() override;

		int show_() override;

//		void add_radiobuttons_(ssize_t * Value, ssize_t OptionCount, const AddRadioButton_t list[], bool FocusOnSelected) override;

	protected:
		void create_border(PCWSTR Text);

		FarDialogItem_t * add_dialog_item(FARDIALOGITEMTYPES Type, PCWSTR Text, FARDIALOGITEMFLAGS flags = DIF_NONE);

		FarDialogItem_t * add_dialog_item(FarDialogItem_t * item);

		ssize_t GetMaxItemX2() const;

		void save();

		void set_next_y(FarDialogItem_t * Item);

		int show_dialog_();

		void UpdateBorderSize();

	protected:
		GUID Id;
		FARWINDOWPROC DlgProc;
		PCWSTR HelpTopic;
		void * UserParam;

		HANDLE DialogHandle;

		std::vector<FarDialogItem_t> DialogItems;
		size_t DialogItemsAllocated;

		ssize_t NextY;
		ssize_t Indent;

		ssize_t OKButtonId;
		ssize_t SingleBoxIndex;

	private:
		static const ssize_t DEFAULT_BORDER_INDENT_X = 3;
		static const ssize_t DEFAULT_BORDER_INDENT_Y = 1;
		static const ssize_t ZERO_X = 4;
		static const ssize_t ZERO_Y = 2;
		static const ssize_t DEFAULT_PADDING = 1;
	};

	///=============================================================================================
//	struct DialogBuilder_inst: public ExtendedDialogBuilder_i, private SimpleDialogBuilder_impl {
//		DialogBuilder_inst(const GUID & aId, PCWSTR Label, PCWSTR aHelpTopic = nullptr, FARWINDOWPROC aDlgProc = nullptr, void * aUserParam = nullptr);
//
//		using SimpleDialogBuilder_impl::add_text_;
//
//		~DialogBuilder_inst() override;
//
//		FarDialogItem * add_editfield_(PWSTR Value, ssize_t MaxSize, ssize_t Width, PCWSTR HistoryId, bool UseLastHistory) override;
//
//		FarDialogItem * add_fixeditfield_(PWSTR Value, ssize_t MaxSize, ssize_t Width, PCWSTR Mask) override;
//
//		FarDialogItem * add_inteditfield_(ssize_t * Value, ssize_t Width) override;
//
//		FarDialogItem * add_passwordfield_(PWSTR Value, ssize_t MaxSize, ssize_t Width) override;
//
//		void start_column_() override;
//
//		void break_column_() override;
//
//		void end_column_() override;
//
//		void start_singlebox_(ssize_t Width, PCWSTR Label, bool LeftAlign) override;
//
//		void end_singlebox_() override;
//
//	private:
//		ssize_t get_last_id_() const;
//
//		DialogItemBinding_i * FindBinding(FarDialogItem * Item);
//
//		int GetItemId(FarDialogItem * Item);
//
//	private:
//		int SingleBoxIndex;
//
//		int ColumnStartIndex;
//		int ColumnBreakIndex;
//		int ColumnStartY;
//	};

	///=============================================================================================
	struct PluginRadioButtonBinding: public DialogItemBinding_i {
		PluginRadioButtonBinding(HANDLE & aHandle, ssize_t aId, ssize_t * aValue, ssize_t RadioGroupIndex);

		void save_() const override;

		ssize_t get_width_() const override;

	private:
		ssize_t * Value;
		ssize_t m_rg_index;
	};

	struct PluginIntEditFieldBinding: public DialogItemBinding_i {
		PluginIntEditFieldBinding(HANDLE & aHandle, ssize_t aId, ssize_t * aValue, ssize_t Width);

		void save_() const override;

		ssize_t get_width_() const override;

		PWSTR GetBuffer();

		PCWSTR GetMask() const;

	private:
		ssize_t * Value;
		ssize_t m_width;
		wchar_t Buffer[32];
		wchar_t Mask[32];
	};

}

#endif
