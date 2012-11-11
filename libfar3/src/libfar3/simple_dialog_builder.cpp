#include <libfar3/dialog_builder.hpp>

#include <libbase/pcstr.hpp>

#include "DlgBuilder_pvt.hpp"

namespace Far {

	ssize_t inline TextWidth(const FarDialogItem * Item)
	{
		return ::lstrlenW(Item->Data);
	}

	ssize_t ItemWidth(const FarDialogItem * Item)
	{
		switch (Item->Type) {
			case DI_TEXT:
				return TextWidth(Item);

			case DI_CHECKBOX:
			case DI_RADIOBUTTON:
			case DI_BUTTON:
				return TextWidth(Item) + 4;

			case DI_EDIT:
			case DI_FIXEDIT:
			case DI_COMBOBOX:
			case DI_PSWEDIT: {
				int Width = Item->X2 - Item->X1 + 1;
				// стрелка history занимает дополнительное место, но раньше она рисовалась поверх рамки???
				if (Item->Flags & DIF_HISTORY)
					Width++;
				return Width;
			}
				break;

			case DI_SINGLEBOX:
				return Item->X2 - Item->X1 + 1;

			default:
				break;
		}
		return 0;
	}

	///=============================================================================================
	SimpleDialogBuilder_i::~SimpleDialogBuilder_i()
	{
	}

	///---------------------------------------------------------------------------------------------
	SimpleDialogBuilder_impl::SimpleDialogBuilder_impl(const GUID & aId, PCWSTR Label, PCWSTR aHelpTopic, FARWINDOWPROC aDlgProc, void * aUserParam) :
		Id(aId),
		DlgProc(aDlgProc),
		HelpTopic(aHelpTopic),
		UserParam(aUserParam),
		DialogHandle(nullptr),
		DialogItemsAllocated(0),
		Indent(0),
		NextY(2),
		OKButtonId(-1)
	{
		add_border(Label);
	}

	SimpleDialogBuilder_impl::~SimpleDialogBuilder_impl()
	{
		psi().DialogFree(DialogHandle);

		std::for_each(DialogItems.begin(), DialogItems.end(), std::bind(&FarDialogItem_t::destroy, std::placeholders::_1));
	}

	FarDialogItem_t * SimpleDialogBuilder_impl::add_text_(PCWSTR Label)
	{
		FarDialogItem_t * Item = add_dialog_item(DI_TEXT, Label);
		set_next_y(Item);
		return Item;
	}

	FarDialogItem_t * SimpleDialogBuilder_impl::add_item_before_(FARDIALOGITEMTYPES Type, PCWSTR Label, FarDialogItem * RelativeTo)
	{
		FarDialogItem_t * Item = add_dialog_item(Type, Label);
		Item->Y1 = Item->Y2 = RelativeTo->Y1;
		Item->X1 = 5 + Indent;
		Item->X2 = Item->X1 + ItemWidth(Item) - 1;

		int RelativeToWidth = RelativeTo->X2 - RelativeTo->X1;
		RelativeTo->X1 = Item->X2 + 2;
		RelativeTo->X2 = RelativeTo->X1 + RelativeToWidth;

		return Item;
	}

	FarDialogItem_t * SimpleDialogBuilder_impl::add_item_after_(FARDIALOGITEMTYPES Type, PCWSTR Label, FarDialogItem * RelativeTo)
	{
		FarDialogItem_t * Item = add_dialog_item(Type, Label);
		Item->Y1 = Item->Y2 = RelativeTo->Y1;
		Item->X1 = RelativeTo->X1 + ItemWidth(RelativeTo) - 1 + 2;

		return Item;
	}

	FarDialogItem_t * SimpleDialogBuilder_impl::add_checkbox_(PCWSTR Label, ssize_t * Value, ssize_t Mask, bool ThreeState)
	{
		FarDialogItem_t * Item = add_dialog_item(DI_CHECKBOX, Label);
		DialogItemBinding_i * bind = new PluginCheckBoxBinding(DialogHandle, DialogItems.size() - 1, Value, Mask);
		Item->set_binding(bind);

		set_next_y(Item);
		Item->X2 = Item->X1 + Item->get_width();
		if (ThreeState && !Mask)
			Item->Flags |= DIF_3STATE;
		if (!Mask)
			Item->Selected = *Value;
		else
			Item->Selected = (*Value & Mask) ? TRUE : FALSE;
		return Item;
	}

	void SimpleDialogBuilder_impl::add_radiobuttons_(ssize_t * Value, ssize_t OptionCount, const AddRadioButton_t list[], bool FocusOnSelected)
	{
		ssize_t rg_index = 0;
		for (ssize_t i = 0; i < OptionCount; ++i) {
			FarDialogItem_t * Item = add_dialog_item(DI_RADIOBUTTON, get_msg(list[i].id), list[i].flags);
			DialogItemBinding_i * bind = new PluginRadioButtonBinding(DialogHandle, DialogItems.size() - 1, Value, rg_index++);
			set_next_y(Item);
			Item->X2 = Item->X1 + Item->get_width();
			if (!i)
				Item->Flags |= DIF_GROUP;
			if (*Value == i) {
				Item->Selected = TRUE;
				if (FocusOnSelected)
					Item->Flags |= DIF_FOCUS;
			}
			Item->set_binding(bind);
		}
	}

	void SimpleDialogBuilder_impl::add_empty_line_()
	{
		NextY++;
	}

	void SimpleDialogBuilder_impl::add_separator_(PCWSTR Label)
	{
		FarDialogItem * Separator = add_dialog_item(DI_TEXT, Label);
		Separator->Flags = DIF_SEPARATOR;
		Separator->X1 = 3;
		Separator->Y1 = Separator->Y2 = NextY++;
	}

	void SimpleDialogBuilder_impl::add_OKCancel_(PCWSTR OKLabel, PCWSTR CancelLabel, PCWSTR ExtraLabel, bool Separator)
	{
		if (Separator)
			add_separator();

		FarDialogItem * OKButton = add_dialog_item(DI_BUTTON, OKLabel);
		OKButton->Flags = DIF_CENTERGROUP | DIF_DEFAULTBUTTON;
		OKButton->Y1 = OKButton->Y2 = NextY++;
		OKButtonId = DialogItems.size() - 1;

		if (!Base::is_str_empty(CancelLabel)) {
			FarDialogItem * CancelButton = add_dialog_item(DI_BUTTON, CancelLabel);
			CancelButton->Flags = DIF_CENTERGROUP;
			CancelButton->Y1 = CancelButton->Y2 = OKButton->Y1;
		}

		if (!Base::is_str_empty(ExtraLabel)) {
			FarDialogItem * ExtraButton = add_dialog_item(DI_BUTTON, ExtraLabel);
			ExtraButton->Flags = DIF_CENTERGROUP;
			ExtraButton->Y1 = ExtraButton->Y2 = OKButton->Y1;
		}
	}

	int SimpleDialogBuilder_impl::show_()
	{
		UpdateBorderSize();
		int Result = show_dialog_();
		if (Result == OKButtonId) {
			save();
		}

		if (Result >= OKButtonId) {
			Result -= OKButtonId;
		}
		return Result;
	}

	///---------------------------------------------------------------------------------------------
	void SimpleDialogBuilder_impl::add_border(PCWSTR Text)
	{
		FarDialogItem_t * item = add_dialog_item(DI_DOUBLEBOX, Text);
		item->X1 = 3;
		item->Y1 = 1;
	}

	FarDialogItem_t * SimpleDialogBuilder_impl::add_dialog_item(FARDIALOGITEMTYPES Type, PCWSTR Text, FARDIALOGITEMFLAGS flags)
	{
		DialogItems.emplace_back(Type, Text, flags);
		return &DialogItems.back();
	}

	ssize_t SimpleDialogBuilder_impl::MaxTextWidth()
	{
		ssize_t MaxWidth = 0;
		for (size_t i = 1; i < DialogItems.size(); ++i) {
			ssize_t Width = ItemWidth(&DialogItems[i]) + DialogItems[i].X1 - 5;

			if (MaxWidth < Width)
				MaxWidth = Width;
		}
		return MaxWidth;
	}

	void SimpleDialogBuilder_impl::save()
	{
		std::for_each(DialogItems.begin(), DialogItems.end(), std::bind(&FarDialogItem_t::save, std::placeholders::_1));
	}

	void SimpleDialogBuilder_impl::set_next_y(FarDialogItem_t * Item)
	{
		Item->X1 = 5 + Indent;
		Item->Y1 = Item->Y2 = NextY++;
	}

	int SimpleDialogBuilder_impl::show_dialog_()
	{
		int Width = DialogItems[0].X2 + 4;
		int Height = DialogItems[0].Y2 + 2;
		DialogHandle = psi().DialogInit(get_plugin_guid(), &Id, -1, -1, Width, Height, HelpTopic, &DialogItems[0], DialogItems.size(), 0, 0, DlgProc, UserParam);
		return psi().DialogRun(DialogHandle);
	}

	void SimpleDialogBuilder_impl::UpdateBorderSize()
	{
		FarDialogItem * Title = &DialogItems[0];
		Title->X2 = Title->X1 + MaxTextWidth() + 3;
		Title->Y2 = DialogItems.back().Y2 + 1;
	}

	///=============================================================================================
	SimpleDialogBuilder_i * get_simple_dialog_builder(const GUID & aId, PCWSTR TitleLabel, PCWSTR aHelpTopic, FARWINDOWPROC aDlgProc, void * aUserParam)
	{
		return new SimpleDialogBuilder_impl(aId, TitleLabel, aHelpTopic, aDlgProc, aUserParam);
	}

}
