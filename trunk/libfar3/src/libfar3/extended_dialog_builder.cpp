//#include <libfar3/helper.hpp>
#include <libfar3/dialog_builder_ex.hpp>

//#include <libbase/pcstr.hpp>
//#include <libbase/logger.hpp>

#include "DlgBuilder_pvt.hpp"

namespace Far {

	///=============================================================================================
	ExtendedDialogBuilder_i::~ExtendedDialogBuilder_i()
	{
	}

	///---------------------------------------------------------------------------------------------
//	FarDialogItem * DialogBuilder_inst::add_editfield_(PWSTR Value, ssize_t /*MaxSize*/, ssize_t /*Width*/, PCWSTR /*HistoryId*/, bool /*UseLastHistory*/)
//	{
//		FarDialogItem * Item = add_dialog_item(DI_EDIT, Value);
////		SetLastItemBinding(new PluginEditFieldBinding(DialogHandle, Item, DialogItemsCount - 1, Value, MaxSize));
////		set_next_y(Item);
////		if (Width == -1)
////			Width = MaxSize - 1;
////		Item->X2 = Item->X1 + Width - 1;
////		if (HistoryId) {
////			Item->History = HistoryId;
////			Item->Flags |= DIF_HISTORY;
////			if (UseLastHistory)
////				Item->Flags |= DIF_USELASTHISTORY;
////		}
//
//		return Item;
//	}
//
//	FarDialogItem * DialogBuilder_inst::add_fixeditfield_(PWSTR Value, ssize_t /*MaxSize*/, ssize_t /*Width*/, PCWSTR /*Mask*/)
//	{
//		FarDialogItem * Item = add_dialog_item(DI_FIXEDIT, Value);
////		SetLastItemBinding(new PluginEditFieldBinding(DialogHandle, Item, DialogItemsCount - 1, Value, MaxSize));
////		set_next_y(Item);
////		if (Width == -1)
////			Width = MaxSize - 1;
////		Item->X2 = Item->X1 + Width - 1;
////		if (Mask) {
////			Item->Mask = Mask;
////			Item->Flags |= DIF_MASKEDIT;
////		}
//
//		return Item;
//	}
//
//	FarDialogItem * DialogBuilder_inst::add_inteditfield_(ssize_t * /*Value*/, ssize_t /*Width*/)
//	{
//		FarDialogItem * Item = add_dialog_item(DI_FIXEDIT, L"");
////		PluginIntEditFieldBinding * Binding = new PluginIntEditFieldBinding(DialogHandle, Item, DialogItemsCount - 1, Value, Width);
////		SetLastItemBinding(Binding);
////		Item->Flags |= DIF_MASKEDIT;
////		Item->Data = Binding->GetBuffer();
////		Item->Mask = Binding->GetMask();
////		set_next_y(Item);
////		Item->X2 = Item->X1 + Width - 1;
//
//		return Item;
//	}
//
//	FarDialogItem * DialogBuilder_inst::add_passwordfield_(PWSTR Value, ssize_t /*MaxSize*/, ssize_t /*Width*/)
//	{
//		FarDialogItem * Item = add_dialog_item(DI_PSWEDIT, Value);
////		SetLastItemBinding(new PluginEditFieldBinding(DialogHandle, Item, DialogItemsCount - 1, Value, MaxSize));
////		set_next_y(Item);
////		Item->X2 = Item->X1 + Width - 1;
//
//		return Item;
//	}
//
//	void DialogBuilder_inst::start_column_()
//	{
//		ColumnStartIndex = DialogItemsCount;
//		ColumnStartY = NextY;
//	}
//
//	void DialogBuilder_inst::break_column_()
//	{
//		ColumnBreakIndex = DialogItemsCount;
//		NextY = ColumnStartY;
//	}
//
//	void DialogBuilder_inst::end_column_()
//	{
//		ssize_t colWidth = 0;
//		for (int i = ColumnStartIndex; i < ColumnBreakIndex; ++i) {
//			colWidth = std::max(colWidth, ItemWidth(&DialogItems[i]));
//		}
//		for (size_t i = ColumnBreakIndex; i < DialogItemsCount; ++i) {
//			DialogItems[i].X1 += (1 + colWidth);
//			DialogItems[i].X2 += (1 + colWidth);
//		}
//
//		ColumnStartIndex = -1;
//		ColumnBreakIndex = -1;
//	}
//
//	void DialogBuilder_inst::start_singlebox_(ssize_t Width, PCWSTR Label, bool LeftAlign)
//	{
//		FarDialogItem * SingleBox = add_dialog_item(DI_SINGLEBOX, Label);
//		SingleBox->Flags = LeftAlign ? DIF_LEFTTEXT : DIF_NONE;
//		SingleBox->X1 = 5;
//		SingleBox->X2 = SingleBox->X1 + Width;
//		SingleBox->Y1 = NextY++;
//		Indent = 2;
//		SingleBoxIndex = DialogItemsCount - 1;
//	}
//
//	void DialogBuilder_inst::end_singlebox_()
//	{
//		if (SingleBoxIndex) {
//			DialogItems[SingleBoxIndex].Y2 = NextY++;
//			SingleBoxIndex = Indent = 0;
//		}
//	}
//
//	//==============================================================================================
//	DialogBuilder_inst::DialogBuilder_inst(const GUID & aId, PCWSTR Label, PCWSTR aHelpTopic, FARWINDOWPROC aDlgProc, void * aUserParam) :
//		SimpleDialogBuilder_impl(aId, Label, aHelpTopic, aDlgProc, aUserParam),
//		SingleBoxIndex(0),
//		ColumnStartIndex(-1),
//		ColumnBreakIndex(-1),
//		ColumnStartY(-1)
//	{
//	}
//
//	ssize_t DialogBuilder_inst::get_last_id_() const
//	{
//		return DialogItemsCount - 1;
//	}
//
//	DialogItemBinding_i * DialogBuilder_inst::FindBinding(FarDialogItem * /*Item*/)
//	{
////		int Index = static_cast<int>(Item - DialogItems);
////		if (Index >= 0 && Index < DialogItemsCount)
////			return Bindings[Index];
//		return nullptr;
//	}
//
//	int DialogBuilder_inst::GetItemId(FarDialogItem * /*Item*/)
//	{
////		int Index = static_cast<int>(Item - DialogItems);
////		if (Index >= 0 && Index < DialogItemsCount)
////			return Index;
//		return -1;
//	}

	///=============================================================================================
	ExtendedDialogBuilder_i * get_extended_dialog_builder(const GUID & /*aId*/, PCWSTR /*TitleLabel*/, PCWSTR /*aHelpTopic*/, FARWINDOWPROC /*aDlgProc*/, void * /*aUserParam*/)
	{
//		return new DialogBuilder_inst(aId, TitleLabel, aHelpTopic, aDlgProc, aUserParam);
		return nullptr;
	}

}
