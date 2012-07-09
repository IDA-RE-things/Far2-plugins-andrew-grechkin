#include <libfar3/helper.hpp>
#include <libfar3/DlgBuilder.hpp>

#include <libbase/pcstr.hpp>

#include <libbase/logger.hpp>

#include "DlgBuilder_pvt.hpp"

namespace Far {

	///=============================================================================================
	ssize_t TextWidth(const FarDialogItem * Item) {
		return ::lstrlenW(Item->Data);
	}

	ssize_t ItemWidth(const FarDialogItem * Item) {
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
	DialogBuilder_i::~DialogBuilder_i() {
	}


	///=============================================================================================
	struct DialogBuilder_inst: public DialogBuilder_i {
		DialogBuilder_inst(const GUID & aId, PCWSTR Label, PCWSTR aHelpTopic = nullptr, FARWINDOWPROC aDlgProc = nullptr, void * aUserParam = nullptr);

		virtual ~DialogBuilder_inst();

		virtual FarDialogItem * add_text_(PCWSTR Label);

		virtual FarDialogItem * add_checkbox_(PCWSTR Label, ssize_t * Value, ssize_t Mask, bool ThreeState);

		virtual void add_radiobuttons_(ssize_t * Value, ssize_t OptionCount, const AddRadioButton_t list[], bool FocusOnSelected);

		virtual FarDialogItem * add_editfield_(PWSTR Value, ssize_t MaxSize, ssize_t Width, PCWSTR HistoryId, bool UseLastHistory);

		virtual FarDialogItem * add_fixeditfield_(PWSTR Value, ssize_t MaxSize, ssize_t Width, PCWSTR Mask);

		virtual FarDialogItem * add_inteditfield_(ssize_t * Value, ssize_t Width);

		virtual FarDialogItem * add_passwordfield_(PWSTR Value, ssize_t MaxSize, ssize_t Width);

		virtual FarDialogItem * add_item_before_(FARDIALOGITEMTYPES Type, PCWSTR Label, FarDialogItem * RelativeTo);

		virtual FarDialogItem * add_item_after_(FARDIALOGITEMTYPES Type, PCWSTR Label, FarDialogItem * RelativeTo);

		virtual void start_column_();

		virtual void break_column_();

		virtual void end_column_();

		virtual void start_singlebox_(ssize_t Width, PCWSTR Label, bool LeftAlign);

		virtual void end_singlebox_();

		virtual void add_empty_line_();

		virtual void add_separator_(PCWSTR Label);

		virtual void add_OKCancel_(PCWSTR OKLabel, PCWSTR CancelLabel, PCWSTR ExtraLabel, bool Separator);

		virtual int show_dialog_ex_();

		virtual int show_dialog_();

	private:
		void ReallocDialogItems();

		void AddBorder(PCWSTR Text);

		FarDialogItem * add_dialog_item(FARDIALOGITEMTYPES Type, PCWSTR Text, FARDIALOGITEMFLAGS flags = DIF_NONE);

		void set_next_y(FarDialogItem * Item);

		ssize_t get_last_id_() const;

		DialogItemBinding * SetLastItemBinding(DialogItemBinding * Binding);

		void save();

		DialogItemBinding * FindBinding(FarDialogItem * Item);

		int GetItemId(FarDialogItem * Item);

		ssize_t MaxTextWidth();

		void UpdateBorderSize();

	private:
		GUID Id;
		PCWSTR HelpTopic;
		void * UserParam;
		FARWINDOWPROC DlgProc;

		HANDLE DialogHandle;
		FarDialogItem * DialogItems;
		DialogItemBinding ** Bindings;
		int DialogItemsCount;
		int DialogItemsAllocated;
		int NextY;
		int Indent;
		int OKButtonId;

		int SingleBoxIndex;

		int ColumnStartIndex;
		int ColumnBreakIndex;
		int ColumnStartY;
	};

	DialogBuilder_inst::~DialogBuilder_inst() {
		psi().DialogFree(DialogHandle);

		for (int i = 0; i < DialogItemsCount; i++) {
			if (Bindings[i])
				delete Bindings[i];
		}
		delete[] DialogItems;
		delete[] Bindings;
	}

	FarDialogItem * DialogBuilder_inst::add_text_(PCWSTR Label) {
		FarDialogItem * Item = add_dialog_item(DI_TEXT, Label);
		set_next_y(Item);
		return Item;
	}

	FarDialogItem * DialogBuilder_inst::add_checkbox_(PCWSTR Label, ssize_t * Value, ssize_t Mask, bool ThreeState) {
		FarDialogItem * Item = add_dialog_item(DI_CHECKBOX, Label);
		DialogItemBinding * bind = SetLastItemBinding(new PluginCheckBoxBinding(DialogHandle, Item, DialogItemsCount - 1, Value, Mask));

		set_next_y(Item);
		Item->X2 = Item->X1 + bind->get_width();
		if (ThreeState && !Mask)
			Item->Flags |= DIF_3STATE;
		if (!Mask)
			Item->Selected = *Value;
		else
			Item->Selected = (*Value & Mask) ? TRUE : FALSE;
		return Item;
	}

	void DialogBuilder_inst::add_radiobuttons_(ssize_t * Value, ssize_t OptionCount, const AddRadioButton_t list[], bool FocusOnSelected) {
		ssize_t rg_index = 0;
		for (ssize_t i = 0; i < OptionCount; ++i) {
			FarDialogItem * Item = add_dialog_item(DI_RADIOBUTTON, get_msg(list[i].id), list[i].flags);
			DialogItemBinding * bind = SetLastItemBinding(new PluginRadioButtonBinding(DialogHandle, Item, DialogItemsCount - 1, Value, rg_index++));
			set_next_y(Item);
			Item->X2 = Item->X1 + bind->get_width();
			if (!i)
				Item->Flags |= DIF_GROUP;
			if (*Value == i) {
				Item->Selected = TRUE;
				if (FocusOnSelected)
					Item->Flags |= DIF_FOCUS;
			}
		}
	}

	FarDialogItem * DialogBuilder_inst::add_editfield_(PWSTR Value, ssize_t MaxSize, ssize_t Width, PCWSTR HistoryId, bool UseLastHistory) {
		FarDialogItem * Item = add_dialog_item(DI_EDIT, Value);
		SetLastItemBinding(new PluginEditFieldBinding(DialogHandle, Item, DialogItemsCount - 1, Value, MaxSize));
		set_next_y(Item);
		if (Width == -1)
			Width = MaxSize - 1;
		Item->X2 = Item->X1 + Width - 1;
		if (HistoryId) {
			Item->History = HistoryId;
			Item->Flags |= DIF_HISTORY;
			if (UseLastHistory)
				Item->Flags |= DIF_USELASTHISTORY;
		}

		return Item;
	}

	FarDialogItem * DialogBuilder_inst::add_fixeditfield_(PWSTR Value, ssize_t MaxSize, ssize_t Width, PCWSTR Mask) {
		FarDialogItem * Item = add_dialog_item(DI_FIXEDIT, Value);
		SetLastItemBinding(new PluginEditFieldBinding(DialogHandle, Item, DialogItemsCount - 1, Value, MaxSize));
		set_next_y(Item);
		Item->X2 = Item->X1 + Width - 1;
		if (Mask) {
			Item->Mask = Mask;
			Item->Flags |= DIF_MASKEDIT;
		}

		return Item;
	}

	FarDialogItem * DialogBuilder_inst::add_inteditfield_(ssize_t * Value, ssize_t Width) {
		FarDialogItem * Item = add_dialog_item(DI_FIXEDIT, L"");
		PluginIntEditFieldBinding * Binding = new PluginIntEditFieldBinding(DialogHandle, Item, DialogItemsCount - 1, Value, Width);
		SetLastItemBinding(Binding);
		Item->Flags |= DIF_MASKEDIT;
		Item->Data = Binding->GetBuffer();
		Item->Mask = Binding->GetMask();
		set_next_y(Item);
		Item->X2 = Item->X1 + Width - 1;

		return Item;
	}

	FarDialogItem * DialogBuilder_inst::add_passwordfield_(PWSTR Value, ssize_t MaxSize, ssize_t Width) {
		FarDialogItem * Item = add_dialog_item(DI_PSWEDIT, Value);
		SetLastItemBinding(new PluginEditFieldBinding(DialogHandle, Item, DialogItemsCount - 1, Value, MaxSize));
		set_next_y(Item);
		Item->X2 = Item->X1 + Width - 1;

		return Item;
	}

	FarDialogItem * DialogBuilder_inst::add_item_before_(FARDIALOGITEMTYPES Type, PCWSTR Label, FarDialogItem * RelativeTo) {
		FarDialogItem * Item = add_dialog_item(Type, Label);
		Item->Y1 = Item->Y2 = RelativeTo->Y1;
		Item->X1 = 5 + Indent;
		Item->X2 = Item->X1 + ItemWidth(Item) - 1;

		int RelativeToWidth = RelativeTo->X2 - RelativeTo->X1;
		RelativeTo->X1 = Item->X2 + 2;
		RelativeTo->X2 = RelativeTo->X1 + RelativeToWidth;

		return Item;
	}

	FarDialogItem * DialogBuilder_inst::add_item_after_(FARDIALOGITEMTYPES Type, PCWSTR Label, FarDialogItem * RelativeTo) {
		FarDialogItem * Item = add_dialog_item(Type, Label);
		Item->Y1 = Item->Y2 = RelativeTo->Y1;
		Item->X1 = RelativeTo->X1 + ItemWidth(RelativeTo) - 1 + 2;

		return Item;
	}

	void DialogBuilder_inst::start_column_() {
		ColumnStartIndex = DialogItemsCount;
		ColumnStartY = NextY;
	}

	void DialogBuilder_inst::break_column_() {
		ColumnBreakIndex = DialogItemsCount;
		NextY = ColumnStartY;
	}

	void DialogBuilder_inst::end_column_() {
		ssize_t colWidth = 0;
		for (int i = ColumnStartIndex; i < ColumnBreakIndex; ++i) {
			colWidth = std::max(colWidth, ItemWidth(&DialogItems[i]));
		}
		for (int i = ColumnBreakIndex; i < DialogItemsCount; ++i) {
			DialogItems[i].X1 += (1 + colWidth);
			DialogItems[i].X2 += (1 + colWidth);
		}

		ColumnStartIndex = -1;
		ColumnBreakIndex = -1;
	}

	void DialogBuilder_inst::start_singlebox_(ssize_t Width, PCWSTR Label, bool LeftAlign) {
		FarDialogItem * SingleBox = add_dialog_item(DI_SINGLEBOX, Label);
		SingleBox->Flags = LeftAlign ? DIF_LEFTTEXT : DIF_NONE;
		SingleBox->X1 = 5;
		SingleBox->X2 = SingleBox->X1 + Width;
		SingleBox->Y1 = NextY++;
		Indent = 2;
		SingleBoxIndex = DialogItemsCount - 1;
	}

	void DialogBuilder_inst::end_singlebox_() {
		if (SingleBoxIndex) {
			DialogItems[SingleBoxIndex].Y2 = NextY++;
			SingleBoxIndex = Indent = 0;
		}
	}

	void DialogBuilder_inst::add_empty_line_() {
		NextY++;
	}

	void DialogBuilder_inst::add_separator_(PCWSTR Label) {
		FarDialogItem * Separator = add_dialog_item(DI_TEXT, Label);
		Separator->Flags = DIF_SEPARATOR;
		Separator->X1 = 3;
		Separator->Y1 = Separator->Y2 = NextY++;
	}

	void DialogBuilder_inst::add_OKCancel_(PCWSTR OKLabel, PCWSTR CancelLabel, PCWSTR ExtraLabel, bool Separator) {
		if (Separator)
			add_separator();

		FarDialogItem * OKButton = add_dialog_item(DI_BUTTON, OKLabel);
		OKButton->Flags = DIF_CENTERGROUP | DIF_DEFAULTBUTTON;
		OKButton->Y1 = OKButton->Y2 = NextY++;
		OKButtonId = DialogItemsCount - 1;

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

	int DialogBuilder_inst::show_dialog_ex_() {
		LogTrace();
		UpdateBorderSize();
		int Result = show_dialog_();
		if (Result == OKButtonId) {
			save();
		}
		LogTrace();

		if (Result >= OKButtonId) {
			Result -= OKButtonId;
		}
		return Result;
	}


	//==============================================================================================
	DialogBuilder_inst::DialogBuilder_inst(const GUID & aId, PCWSTR Label, PCWSTR aHelpTopic, FARWINDOWPROC aDlgProc, void * aUserParam):
		Id(aId),
		HelpTopic(aHelpTopic),
		UserParam(aUserParam),
		DlgProc(aDlgProc),
		DialogHandle(nullptr),
		DialogItems(nullptr),
		Bindings(nullptr),
		DialogItemsCount(0),
		DialogItemsAllocated(0),
		NextY(2),
		Indent(0),
		OKButtonId(-1),
		SingleBoxIndex(0),
		ColumnStartIndex(-1),
		ColumnBreakIndex(-1),
		ColumnStartY(-1) {
		AddBorder(Label);
	}

	void DialogBuilder_inst::ReallocDialogItems() {
		// реаллокация инвалидирует указатели, возвращенные из add_dialog_item,
		// поэтому размер массива подбираем такой,
		// чтобы все нормальные диалоги помещались без реаллокации
		// TODO хорошо бы, чтобы они вообще не инвалидировались
		DialogItemsAllocated += 32;
		if (DialogItems) {
			FarDialogItem * NewDialogItems = new FarDialogItem[DialogItemsAllocated];
			DialogItemBinding ** NewBindings = new DialogItemBinding * [DialogItemsAllocated];
			for (int i = 0; i < DialogItemsCount; ++i) {
				NewDialogItems[i] = DialogItems[i];
				NewBindings[i] = Bindings[i];
			}
			delete[] DialogItems;
			delete[] Bindings;
			DialogItems = NewDialogItems;
			Bindings = NewBindings;
		} else {
			DialogItems = new FarDialogItem[DialogItemsAllocated];
			Bindings = new DialogItemBinding * [DialogItemsAllocated];
		}
	}

	void DialogBuilder_inst::AddBorder(PCWSTR Text) {
		FarDialogItem * item = add_dialog_item(DI_DOUBLEBOX, Text);
		item->X1 = 3;
		item->Y1 = 1;
	}

	FarDialogItem * DialogBuilder_inst::add_dialog_item(FARDIALOGITEMTYPES Type, PCWSTR Text, FARDIALOGITEMFLAGS flags) {
		if (DialogItemsCount == DialogItemsAllocated) {
			ReallocDialogItems();
		}
		int Index = DialogItemsCount++;
		Bindings[Index] = nullptr;
		FarDialogItem * Item = &DialogItems[Index];
		::memset(Item, 0, sizeof(*Item));
		Item->Type = Type;
		Item->Data = Text;
		Item->Flags = flags;
		return Item;
	}

	void DialogBuilder_inst::set_next_y(FarDialogItem * Item) {
		Item->X1 = 5 + Indent;
		Item->Y1 = Item->Y2 = NextY++;
	}

	ssize_t DialogBuilder_inst::get_last_id_() const {
		return DialogItemsCount - 1;
	}

	DialogItemBinding * DialogBuilder_inst::SetLastItemBinding(DialogItemBinding * Binding) {
		Bindings[Binding->get_index()] = Binding;
		return Binding;
	}



	int DialogBuilder_inst::show_dialog_() {
		int Width = DialogItems[0].X2 + 4;
		int Height = DialogItems[0].Y2 + 2;
		DialogHandle = psi().DialogInit(get_plugin_guid(), &Id, -1, -1, Width, Height, HelpTopic, DialogItems, DialogItemsCount, 0, 0, DlgProc, UserParam);
		return psi().DialogRun(DialogHandle);
	}

	void DialogBuilder_inst::save() {
		LogTrace();
		for (int i = 0; i < DialogItemsCount; ++i) {
			if (Bindings[i])
				Bindings[i]->save();
		}
	}

	DialogItemBinding * DialogBuilder_inst::FindBinding(FarDialogItem * Item) {
		int Index = static_cast<int>(Item - DialogItems);
		if (Index >= 0 && Index < DialogItemsCount)
			return Bindings[Index];
		return nullptr;
	}

	int DialogBuilder_inst::GetItemId(FarDialogItem * Item) {
		int Index = static_cast<int>(Item - DialogItems);
		if (Index >= 0 && Index < DialogItemsCount)
			return Index;
		return -1;
	}

	ssize_t DialogBuilder_inst::MaxTextWidth() {
		ssize_t MaxWidth = 0;
		for (ssize_t i = 1; i < DialogItemsCount; ++i) {
			ssize_t Width = ItemWidth(&DialogItems[i]) + DialogItems[i].X1 - 5;

			if (MaxWidth < Width)
				MaxWidth = Width;
		}
		return MaxWidth;
	}

	void DialogBuilder_inst::UpdateBorderSize() {
		FarDialogItem * Title = &DialogItems[0];
		Title->X2 = Title->X1 + MaxTextWidth() + 3;
		Title->Y2 = DialogItems[DialogItemsCount - 1].Y2 + 1;
	}


	///=================================================================================================
	DialogBuilder_i * get_dialog_builder(const GUID & aId, PCWSTR TitleLabel, PCWSTR aHelpTopic, FARWINDOWPROC aDlgProc, void * aUserParam) {
		return new DialogBuilder_inst(aId, TitleLabel, aHelpTopic, aDlgProc, aUserParam);
	}

}
