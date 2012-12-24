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
#include <libbase/pcstr.hpp>

#include "DlgBuilder_pvt.hpp"

namespace Far {

	bool CompareX2_less(const FarDialogItem_t & a, const FarDialogItem_t & b)
	{
		return a.X2 < b.X2;
	}

	bool CompareWidth_less(const FarDialogItem_t & a, const FarDialogItem_t & b)
	{
		return a.get_width() < b.get_width();
	}

	///=============================================================================================
	DialogBuilder_i::~DialogBuilder_i()
	{
		LogTrace();
	}

	///=============================================================================================
	SimpleDialogBuilder_impl::SimpleDialogBuilder_impl(const GUID & aId, PCWSTR Label, PCWSTR aHelpTopic, FARWINDOWPROC aDlgProc, void * aUserParam) :
		Id(aId),
		DlgProc(aDlgProc),
		HelpTopic(aHelpTopic),
		UserParam(aUserParam),
		DialogHandle(nullptr),
		DialogItemsAllocated(0),
		NextY(ZERO_Y),
		OKButtonId(-1)
	{
		LogTrace();
		create_border(Label);
	}

	SimpleDialogBuilder_impl::~SimpleDialogBuilder_impl()
	{
		psi().DialogFree(DialogHandle);

//		std::for_each(DialogItems.begin(), DialogItems.end(), std::bind(&FarDialogItem_t::destroy, std::placeholders::_1));
		LogTrace();
	}

	FarDialogItem_t * SimpleDialogBuilder_impl::add_item_(FarDialogItem_t * item)
	{
		LogTrace();
		DialogItems.emplace_back(std::move(*item));
		delete item;

		FarDialogItem_t * Item = &DialogItems.back();
		Item->set_dlg(&DialogHandle);
		Item->set_index(DialogItems.size() - 1);
		set_next_y(Item);
		return Item;
	}

	FarDialogItem_t * SimpleDialogBuilder_impl::add_item_before_(FarDialogItem_t * /*item*/, FarDialogItem_t * /*RelativeTo*/)
	{
		LogTrace();
		FarDialogItem_t * Item = nullptr;//add_dialog_item(Type, Label);
//		Item->Y1 = Item->Y2 = RelativeTo->Y1;
//		Item->X1 = 5 + Indent;
//		Item->X2 = Item->X1 + Item->get_width() - 1;
//
//		int RelativeToWidth = RelativeTo->X2 - RelativeTo->X1;
//		RelativeTo->X1 = Item->X2 + 2;
//		RelativeTo->X2 = RelativeTo->X1 + RelativeToWidth;

		return Item;
	}

	FarDialogItem_t * SimpleDialogBuilder_impl::add_item_after_(FarDialogItem_t * /*item*/, FarDialogItem_t * /*RelativeTo*/)
	{
		LogTrace();
		FarDialogItem_t * Item = nullptr;//add_dialog_item(Type, Label);
//		Item->Y1 = Item->Y2 = RelativeTo->Y1;
//		Item->X1 = RelativeTo->X1 + RelativeTo->get_width() - 1 + 2;

		return Item;
	}

	void SimpleDialogBuilder_impl::add_empty_line_()
	{
		LogTrace();
		NextY++;
	}

	void SimpleDialogBuilder_impl::add_OKCancel_(PCWSTR OKLabel, PCWSTR CancelLabel, PCWSTR ExtraLabel)
	{
		LogTrace();
		FarDialogItem_t * OKButton = add_dialog_item(DI_BUTTON, OKLabel, DIF_CENTERGROUP | DIF_DEFAULTBUTTON);
		OKButton->Y1 = OKButton->Y2 = NextY++;
		OKButtonId = DialogItems.size() - 1;

		if (!Base::is_str_empty(CancelLabel)) {
			FarDialogItem_t * CancelButton = add_dialog_item(DI_BUTTON, CancelLabel, DIF_CENTERGROUP);
			CancelButton->Y1 = CancelButton->Y2 = OKButton->Y1;
		}

		if (!Base::is_str_empty(ExtraLabel)) {
			FarDialogItem_t * ExtraButton = add_dialog_item(DI_BUTTON, ExtraLabel, DIF_CENTERGROUP);
			ExtraButton->Y1 = ExtraButton->Y2 = OKButton->Y1;
		}
	}

	int SimpleDialogBuilder_impl::show_()
	{
		LogTrace();
		UpdateBorderSize();
		LogTrace();
		int Result = show_dialog_();
		LogTrace();
		if (Result == OKButtonId) {
			save();
		}
		LogTrace();
		if (Result >= OKButtonId) {
			Result -= OKButtonId;
		}
		return Result;
	}


//	void SimpleDialogBuilder_impl::add_radiobuttons_(ssize_t * Value, ssize_t OptionCount, const AddRadioButton_t list[], bool FocusOnSelected)
//	{
//		ssize_t rg_index = 0;
//		for (ssize_t i = 0; i < OptionCount; ++i) {
//			FarDialogItem_t * Item = add_dialog_item(DI_RADIOBUTTON, get_msg(list[i].id), list[i].flags);
//			DialogItemBinding_i * bind = new PluginRadioButtonBinding(DialogHandle, DialogItems.size() - 1, Value, rg_index++);
//			set_next_y(Item);
//			Item->X2 = Item->X1 + Item->get_width();
//			if (!i)
//				Item->Flags |= DIF_GROUP;
//			if (*Value == i) {
//				Item->Selected = TRUE;
//				if (FocusOnSelected)
//					Item->Flags |= DIF_FOCUS;
//			}
//			Item->set_binding(bind);
//		}
//	}

	///---------------------------------------------------------------------------------------------
	void SimpleDialogBuilder_impl::create_border(PCWSTR Text)
	{
		LogTrace();
		FarDialogItem_t * item = add_dialog_item(DI_DOUBLEBOX, Text);
		item->X1 = DEFAULT_BORDER_INDENT_X;
		item->X2 = item->X1 + item->get_width() + 2 + 2 - 1; // 2 for border and 2 for spaces in title
		item->Y1 = DEFAULT_BORDER_INDENT_Y;
		item->Y2 = item->Y1 + 2 - 1;
	}

	FarDialogItem_t * SimpleDialogBuilder_impl::add_dialog_item(FARDIALOGITEMTYPES Type, PCWSTR Text, FARDIALOGITEMFLAGS flags)
	{
		LogTrace();
		DialogItems.emplace_back(Type, Text, flags);
		FarDialogItem_t * ret = &DialogItems.back();
		ret->set_dlg(&DialogHandle);
		ret->set_index(DialogItems.size() - 1);
		return ret;
	}

	ssize_t SimpleDialogBuilder_impl::GetMaxItemWidth() const
	{
		LogTrace();
		auto it = std::max_element(++DialogItems.begin(), DialogItems.end(), &CompareWidth_less);
		LogDebug(L"it->X1: %Id\n", it->X1);
		LogDebug(L"it->Data: '%s'\n", it->Data);
		LogDebug(L"it->get_width(): %Id\n", it->get_width());
		return it->X1 + it->get_width() - 1 - ZERO_X;
	}

	void SimpleDialogBuilder_impl::save()
	{
		LogTrace();
		std::for_each(DialogItems.begin(), DialogItems.end(), std::bind(&FarDialogItem_t::save, std::placeholders::_1));
	}

	void SimpleDialogBuilder_impl::set_next_y(FarDialogItem_t * Item)
	{
		LogTrace();
		Item->X1 = ZERO_X + DEFAULT_PADDING;
		Item->Y1 = NextY++;
		Item->Y2 = Item->Y1;
//		Item->Y2 = Item->Y1 + Item->get_height() - 1;
//		NextY = Item->Y2 + 1;
	}

	int SimpleDialogBuilder_impl::show_dialog_()
	{
		LogTrace();
		int Width = DialogItems[0].X2 + (DEFAULT_BORDER_INDENT_X  + 1);
		int Height = DialogItems[0].Y2 + (DEFAULT_BORDER_INDENT_Y  + 1);
		LogDebug(L"Width: %d, Height: %d\n", Width, Height);
		LogDebug(L"DialogItems.size(): %Id\n", DialogItems.size());
		LogDebug(L"get_plugin_guid(): %p\n", DialogItems.size());
		DialogHandle = psi().DialogInit(get_plugin_guid(), &Id, -1, -1, Width, Height, HelpTopic, &DialogItems[0], DialogItems.size(), 0, 0, DlgProc, UserParam);
		LogTrace();
		return psi().DialogRun(DialogHandle);
	}

	void SimpleDialogBuilder_impl::UpdateBorderSize()
	{
		LogTrace();
		if (DialogItems.size() > 1) {
			FarDialogItem_t & border = DialogItems[0];
			auto borderX2 = GetMaxItemWidth() + DEFAULT_PADDING + 1;
			borderX2 -= (border.X2 - border.X1);
			if (borderX2 > 0)
				border.X2 += borderX2;
			border.Y2 = DialogItems.back().Y2 + 1;
		}
	}

	///=============================================================================================
	Base::shared_ptr<DialogBuilder_i> create_dialog_builder(const GUID & aId, PCWSTR TitleLabel, PCWSTR aHelpTopic, FARWINDOWPROC aDlgProc, void * aUserParam)
	{
		LogTrace();
		return Base::shared_ptr<DialogBuilder_i>(new SimpleDialogBuilder_impl(aId, TitleLabel, aHelpTopic, aDlgProc, aUserParam));
	}

}
