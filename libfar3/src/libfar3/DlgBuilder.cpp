#include <libfar3/helper.hpp>
#include <libfar3/DlgBuilder.hpp>

struct DialogAPIBinding: public DialogItemBinding_i {
protected:
	DialogAPIBinding(const PluginStartupInfo &aInfo, HANDLE * aHandle, int aId) :
		Info(aInfo),
		DialogHandle(aHandle),
		Id(aId) {
	}

	const PluginStartupInfo &Info;
	HANDLE *DialogHandle;
	int Id;
};

struct PluginCheckBoxBinding: public DialogAPIBinding {
	PluginCheckBoxBinding(const PluginStartupInfo &aInfo, HANDLE *aHandle, int aId, ssize_t * aValue, ssize_t aMask) :
		DialogAPIBinding(aInfo, aHandle, aId), Value(aValue), Mask(aMask) {
	}

	virtual void SaveValue(FarDialogItem */*Item*/, int /*RadioGroupIndex*/) {
		BOOL Selected = static_cast<BOOL>(Info.SendDlgMessage(*DialogHandle, DM_GETCHECK, Id, 0));
		if (!Mask) {
			*Value = Selected;
		} else {
			if (Selected)
				*Value |= Mask;
			else
				*Value &= ~Mask;
		}
	}

private:
	ssize_t * Value;
	ssize_t  Mask;
};

struct PluginRadioButtonBinding: public DialogAPIBinding {
	PluginRadioButtonBinding(const PluginStartupInfo &aInfo, HANDLE *aHandle, int aId, ssize_t * aValue) :
		DialogAPIBinding(aInfo, aHandle, aId), Value(aValue) {
	}

	virtual void SaveValue(FarDialogItem */*Item*/, int RadioGroupIndex) {
		if (Info.SendDlgMessage(*DialogHandle, DM_GETCHECK, Id, 0))
			*Value = RadioGroupIndex;
	}

private:
	ssize_t *Value;
};

struct PluginEditFieldBinding: public DialogAPIBinding {
	PluginEditFieldBinding(const PluginStartupInfo &aInfo, HANDLE *aHandle, int aId, wchar_t *aValue, int aMaxSize) :
		DialogAPIBinding(aInfo, aHandle, aId), Value(aValue), MaxSize(aMaxSize) {
	}

	virtual void SaveValue(FarDialogItem */*Item*/, int /*RadioGroupIndex*/) {
		const wchar_t *DataPtr = (const wchar_t *)Info.SendDlgMessage(*DialogHandle, DM_GETCONSTTEXTPTR, Id, 0);
		lstrcpynW(Value, DataPtr, MaxSize);
	}

private:
	wchar_t *Value;
	int MaxSize;
};

struct PluginIntEditFieldBinding: public DialogAPIBinding {
	PluginIntEditFieldBinding(const PluginStartupInfo & aInfo, HANDLE * aHandle, int aId, ssize_t * aValue, ssize_t Width) :
		DialogAPIBinding(aInfo, aHandle, aId), Value(aValue) {
		aInfo.FSF->sprintf(Buffer, L"%u", *aValue);
		int MaskWidth = Width < 31 ? Width : 31;
		for (int i = 1; i < MaskWidth; i++)
			Mask[i] = L'9';
		Mask[0] = L'#';
		Mask[MaskWidth] = L'\0';
	}

	virtual void SaveValue(FarDialogItem */*Item*/, int /*RadioGroupIndex*/) {
		const wchar_t *DataPtr = (const wchar_t *)Info.SendDlgMessage(*DialogHandle, DM_GETCONSTTEXTPTR, Id, 0);
		*Value = Info.FSF->atoi(DataPtr);
	}

	wchar_t *GetBuffer() {
		return Buffer;
	}

	const wchar_t *GetMask() {
		return Mask;
	}

private:
	ssize_t * Value;
	wchar_t Buffer[32];
	wchar_t Mask[32];
};


///=================================================================================================
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


///=================================================================================================
DialogBuilder_i::~DialogBuilder_i() {
}


///=================================================================================================
struct DialogBuilder_inst: public DialogBuilder_i {
	DialogBuilder_inst(const PluginStartupInfo & psi, const GUID & pluginId, const GUID & aId, int TitleLabelId, PCWSTR aHelpTopic = nullptr, FARWINDOWPROC aDlgProc = nullptr, void * aUserParam = nullptr);

	DialogBuilder_inst(const PluginStartupInfo & psi, const GUID & pluginId, const GUID & aId, PCWSTR TitleLabel, PCWSTR aHelpTopic = nullptr, FARWINDOWPROC aDlgProc = nullptr, void * aUserParam = nullptr);

	virtual ~DialogBuilder_inst();

	virtual FarDialogItem * AddText_(int LabelId);

	virtual FarDialogItem * DoAddText(PCWSTR Label);

	virtual FarDialogItem * DoAddCheckbox(int TextLabelId, ssize_t * Value, ssize_t Mask, bool ThreeState);

	virtual void DoAddRadioButtons(ssize_t * Value, ssize_t OptionCount, const int LabelIds[], bool FocusOnSelected);

	virtual FarDialogItem * DoAddTextBefore(PCWSTR Label, FarDialogItem * RelativeTo);

	virtual FarDialogItem * DoAddTextAfter(PCWSTR Label, FarDialogItem * RelativeTo);

	virtual FarDialogItem * DoAddButtonAfter(PCWSTR Label, FarDialogItem * RelativeTo);

	virtual FarDialogItem * DoAddIntEditField(ssize_t * Value, ssize_t Width);

	virtual FarDialogItem * DoAddEditField(PWSTR Value, ssize_t MaxSize, ssize_t Width, PCWSTR HistoryId, bool UseLastHistory);

	virtual FarDialogItem * DoAddPasswordField(PWSTR Value, ssize_t MaxSize, ssize_t Width);

	virtual FarDialogItem * DoAddFixEditField(PWSTR Value, ssize_t MaxSize, ssize_t Width, PCWSTR Mask);

	virtual void DoStartColumns();

	virtual void DoColumnBreak();

	virtual void DoEndColumns();

	virtual void DoStartSingleBox(ssize_t Width, int LabelId, bool LeftAlign);

	virtual void DoEndSingleBox();

	virtual void DoAddEmptyLine();

	virtual void DoAddSeparator(int LabelId);

	virtual void DoAddOKCancel(int OKLabelId, int CancelLabelId, int ExtraLabelId, bool Separator);

	virtual int DoShowDialogEx();

	virtual int get_last_id_() const;

	virtual int DoShowDialog();

	virtual DialogItemBinding_i * DoCreateCheckBoxBinding(ssize_t * Value, ssize_t Mask);

	virtual DialogItemBinding_i * DoCreateRadioButtonBinding(ssize_t * Value);

private:
	PCWSTR get_lang_string(int LabelId) const;

	void InitDialogItem(FarDialogItem * NewDialogItem, PCWSTR Text);

	void AddBorder(PCWSTR TitleText);

	FarDialogItem * AddDialogItem(FARDIALOGITEMTYPES Type, PCWSTR Text);

	void SetNextY(FarDialogItem * Item);

	void SaveValues();

	DialogItemBinding_i * FindBinding(FarDialogItem * Item);

	int GetItemId(FarDialogItem * Item);

	void ReallocDialogItems();

	void SetLastItemBinding(DialogItemBinding_i * Binding);

	void UpdateSecondColumnPosition();

	int MaxTextWidth();

	void UpdateBorderSize();

private:
	const PluginStartupInfo & Info;
	GUID PluginId;
	GUID Id;
	PCWSTR HelpTopic;
	void * UserParam;
	FARWINDOWPROC DlgProc;

	HANDLE DialogHandle;
	FarDialogItem * DialogItems;
	DialogItemBinding_i ** Bindings;
	int DialogItemsCount;
	int DialogItemsAllocated;
	int NextY;
	int Indent;
	int SingleBoxIndex;
	int OKButtonId;
	int ColumnStartIndex;
	int ColumnBreakIndex;
	int ColumnStartY;
	int ColumnEndY;
	int ColumnMinWidth;

	static const int SECOND_COLUMN = -2;
};

DialogBuilder_inst::DialogBuilder_inst(const PluginStartupInfo & psi, const GUID & pluginId, const GUID & aId, int TitleLabelId, PCWSTR aHelpTopic, FARWINDOWPROC aDlgProc, void * aUserParam):
	Info(psi),
	PluginId(pluginId),
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
	SingleBoxIndex(-1),
	OKButtonId(-1),
	ColumnStartIndex(-1),
	ColumnBreakIndex(-1),
	ColumnStartY(-1),
	ColumnEndY(-1),
	ColumnMinWidth(0) {
	AddBorder(get_lang_string(TitleLabelId));
}

DialogBuilder_inst::DialogBuilder_inst(const PluginStartupInfo & psi, const GUID & pluginId, const GUID & aId, PCWSTR TitleLabel, PCWSTR aHelpTopic, FARWINDOWPROC aDlgProc, void * aUserParam):
	Info(psi),
	PluginId(pluginId),
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
	SingleBoxIndex(-1),
	OKButtonId(-1),
	ColumnStartIndex(-1),
	ColumnBreakIndex(-1),
	ColumnStartY(-1),
	ColumnEndY(-1),
	ColumnMinWidth(0) {
	AddBorder(TitleLabel);
}

DialogBuilder_inst::~DialogBuilder_inst() {
	Info.DialogFree(DialogHandle);

	for (int i = 0; i < DialogItemsCount; i++) {
		if (Bindings[i])
			delete Bindings[i];
	}
	delete[] DialogItems;
	delete[] Bindings;
}

FarDialogItem * DialogBuilder_inst::AddText_(int LabelId) {
	FarDialogItem * Item = AddDialogItem(DI_TEXT, LabelId == -1 ? L"" : get_lang_string(LabelId));
	SetNextY(Item);
	return Item;
}

FarDialogItem * DialogBuilder_inst::DoAddText(PCWSTR Label) {
	FarDialogItem * Item = AddDialogItem(DI_TEXT, Label);
	SetNextY(Item);
	return Item;
}

FarDialogItem * DialogBuilder_inst::DoAddCheckbox(int TextLabelId, ssize_t * Value, ssize_t Mask, bool ThreeState) {
	FarDialogItem * Item = AddDialogItem(DI_CHECKBOX, get_lang_string(TextLabelId));
	if (ThreeState && !Mask)
		Item->Flags |= DIF_3STATE;
	SetNextY(Item);
	Item->X2 = Item->X1 + ItemWidth(Item);
	if (!Mask)
		Item->Selected = *Value;
	else
		Item->Selected = (*Value & Mask) ? TRUE : FALSE;
	SetLastItemBinding(DoCreateCheckBoxBinding(Value, Mask));
	return Item;
}

void DialogBuilder_inst::DoAddRadioButtons(ssize_t * Value, ssize_t OptionCount, const int LabelIds[], bool FocusOnSelected) {
	for (ssize_t i = 0; i < OptionCount; i++) {
		FarDialogItem *Item = AddDialogItem(DI_RADIOBUTTON, get_lang_string(LabelIds[i]));
		SetNextY(Item);
		Item->X2 = Item->X1 + ItemWidth(Item);
		if (!i)
			Item->Flags |= DIF_GROUP;
		if (*Value == i) {
			Item->Selected = TRUE;
			if (FocusOnSelected)
				Item->Flags |= DIF_FOCUS;
		}
		SetLastItemBinding(DoCreateRadioButtonBinding(Value));
	}
}

FarDialogItem * DialogBuilder_inst::DoAddTextBefore(PCWSTR Label, FarDialogItem * RelativeTo) {
	FarDialogItem * Item = AddDialogItem(DI_TEXT, Label);
	Item->Y1 = Item->Y2 = RelativeTo->Y1;
	Item->X1 = 5 + Indent;
	Item->X2 = Item->X1 + ItemWidth(Item) - 1;

	int RelativeToWidth = RelativeTo->X2 - RelativeTo->X1;
	RelativeTo->X1 = Item->X2 + 2;
	RelativeTo->X2 = RelativeTo->X1 + RelativeToWidth;

	DialogItemBinding_i * Binding = FindBinding(RelativeTo);
	if (Binding)
		Binding->BeforeLabelId = GetItemId(Item);

	return Item;
}

FarDialogItem * DialogBuilder_inst::DoAddTextAfter(PCWSTR Label, FarDialogItem * RelativeTo) {
	FarDialogItem *Item = AddDialogItem(DI_TEXT, Label);
	Item->Y1 = Item->Y2 = RelativeTo->Y1;
	Item->X1 = RelativeTo->X1 + ItemWidth(RelativeTo) - 1 + 2;

	DialogItemBinding_i *Binding = FindBinding(RelativeTo);
	if (Binding)
		Binding->AfterLabelId = GetItemId(Item);

	return Item;
}

FarDialogItem * DialogBuilder_inst::DoAddButtonAfter(PCWSTR Label, FarDialogItem * RelativeTo) {
	FarDialogItem *Item = AddDialogItem(DI_BUTTON, Label);
	Item->Y1 = Item->Y2 = RelativeTo->Y1;
	Item->X1 = RelativeTo->X1 + ItemWidth(RelativeTo) - 1 + 2;

	DialogItemBinding_i * Binding = FindBinding(RelativeTo);
	if (Binding)
		Binding->AfterLabelId = GetItemId(Item);

	return Item;
}

FarDialogItem * DialogBuilder_inst::DoAddIntEditField(ssize_t * Value, ssize_t Width) {
	FarDialogItem *Item = AddDialogItem(DI_FIXEDIT, L"");
	Item->Flags |= DIF_MASKEDIT;
	PluginIntEditFieldBinding *Binding;
	Binding = new PluginIntEditFieldBinding(Info, &DialogHandle, DialogItemsCount - 1, Value, Width);
	Item->Data = Binding->GetBuffer();
	Item->Mask = Binding->GetMask();
	SetNextY(Item);
	Item->X2 = Item->X1 + Width - 1;
	SetLastItemBinding(Binding);
	return Item;
}

FarDialogItem * DialogBuilder_inst::DoAddEditField(PWSTR Value, ssize_t MaxSize, ssize_t Width, PCWSTR HistoryId, bool UseLastHistory) {
	FarDialogItem *Item = AddDialogItem(DI_EDIT, Value);
	SetNextY(Item);
	if (Width == -1)
		Width = MaxSize - 1;
	Item->X2 = Item->X1 + Width - 1;
	if (HistoryId) {
		Item->History = HistoryId;
		Item->Flags |= DIF_HISTORY;
		if (UseLastHistory)
			Item->Flags |= DIF_USELASTHISTORY;
	}

	SetLastItemBinding(new PluginEditFieldBinding(Info, &DialogHandle, DialogItemsCount - 1, Value, MaxSize));
	return Item;
}

FarDialogItem * DialogBuilder_inst::DoAddPasswordField(PWSTR Value, ssize_t MaxSize, ssize_t Width) {
	FarDialogItem *Item = AddDialogItem(DI_PSWEDIT, Value);
	SetNextY(Item);
	Item->X2 = Item->X1 + Width - 1;

	SetLastItemBinding(new PluginEditFieldBinding(Info, &DialogHandle, DialogItemsCount - 1, Value, MaxSize));
	return Item;
}

FarDialogItem * DialogBuilder_inst::DoAddFixEditField(PWSTR Value, ssize_t MaxSize, ssize_t Width, PCWSTR Mask) {
	FarDialogItem *Item = AddDialogItem(DI_FIXEDIT, Value);
	SetNextY(Item);
	Item->X2 = Item->X1 + Width - 1;
	if (Mask) {
		Item->Mask = Mask;
		Item->Flags |= DIF_MASKEDIT;
	}

	SetLastItemBinding(new PluginEditFieldBinding(Info, &DialogHandle, DialogItemsCount - 1, Value, MaxSize));
	return Item;
}

void DialogBuilder_inst::DoStartColumns() {
	ColumnStartIndex = DialogItemsCount;
	ColumnStartY = NextY;
}

void DialogBuilder_inst::DoColumnBreak() {
	ColumnBreakIndex = DialogItemsCount;
	ColumnEndY = NextY;
	NextY = ColumnStartY;
}

void DialogBuilder_inst::DoEndColumns() {
	for (int i = ColumnBreakIndex; i < DialogItemsCount; i++) {
		int Width = ItemWidth(&DialogItems[i]);
		if (Width > ColumnMinWidth)
			ColumnMinWidth = Width;
		if (i >= ColumnBreakIndex) {
			DialogItems[i].X1 = SECOND_COLUMN;
			DialogItems[i].X2 = SECOND_COLUMN + Width;
		}
	}

	ColumnStartIndex = -1;
	ColumnBreakIndex = -1;
}

void DialogBuilder_inst::DoStartSingleBox(ssize_t Width, int LabelId, bool LeftAlign) {
	FarDialogItem * SingleBox = AddDialogItem(DI_SINGLEBOX, LabelId == -1 ? L"" : get_lang_string(LabelId));
	SingleBox->Flags = LeftAlign ? DIF_LEFTTEXT : DIF_NONE;
	SingleBox->X1 = 5;
	SingleBox->X2 = SingleBox->X1 + Width;
	SingleBox->Y1 = NextY++;
	Indent = 2;
	SingleBoxIndex = DialogItemsCount - 1;
}

void DialogBuilder_inst::DoEndSingleBox() {
	if (SingleBoxIndex != -1) {
		DialogItems[SingleBoxIndex].Y2 = NextY++;
		Indent = 0;
		SingleBoxIndex = -1;
	}
}

void DialogBuilder_inst::DoAddEmptyLine() {
	NextY++;
}

void DialogBuilder_inst::DoAddSeparator(int LabelId) {
	FarDialogItem * Separator = AddDialogItem(DI_TEXT, LabelId == -1 ? L"" : get_lang_string(LabelId));
	Separator->Flags = DIF_SEPARATOR;
	Separator->X1 = 3;
	Separator->Y1 = Separator->Y2 = NextY++;
}

void DialogBuilder_inst::DoAddOKCancel(int OKLabelId, int CancelLabelId, int ExtraLabelId, bool Separator) {
	if (Separator)
		AddSeparator();

	FarDialogItem * OKButton = AddDialogItem(DI_BUTTON, get_lang_string(OKLabelId));
	OKButton->Flags = DIF_CENTERGROUP | DIF_DEFAULTBUTTON;
	OKButton->Y1 = OKButton->Y2 = NextY++;
	OKButtonId = DialogItemsCount - 1;

	if (CancelLabelId != -1) {
		FarDialogItem * CancelButton = AddDialogItem(DI_BUTTON, get_lang_string(CancelLabelId));
		CancelButton->Flags = DIF_CENTERGROUP;
		CancelButton->Y1 = CancelButton->Y2 = OKButton->Y1;
	}

	if (ExtraLabelId != -1) {
		FarDialogItem * ExtraButton = AddDialogItem(DI_BUTTON, get_lang_string(ExtraLabelId));
		ExtraButton->Flags = DIF_CENTERGROUP;
		ExtraButton->Y1 = ExtraButton->Y2 = OKButton->Y1;
	}
}

int DialogBuilder_inst::DoShowDialogEx() {
	UpdateBorderSize();
	UpdateSecondColumnPosition();
	int Result = DoShowDialog();
	if (Result == OKButtonId) {
		SaveValues();
	}

	if (Result >= OKButtonId) {
		Result -= OKButtonId;
	}
	return Result;
}

int DialogBuilder_inst::get_last_id_() const {
	return DialogItemsCount - 1;
}

int DialogBuilder_inst::DoShowDialog() {
	int Width = DialogItems[0].X2 + 4;
	int Height = DialogItems[0].Y2 + 2;
	DialogHandle = Info.DialogInit(&PluginId, &Id, -1, -1, Width, Height, HelpTopic, DialogItems, DialogItemsCount, 0, 0, DlgProc, UserParam);
	return Info.DialogRun(DialogHandle);
}

DialogItemBinding_i * DialogBuilder_inst::DoCreateCheckBoxBinding(ssize_t * Value, ssize_t Mask) {
	return new PluginCheckBoxBinding(Info, &DialogHandle, DialogItemsCount - 1, Value, Mask);
}

DialogItemBinding_i * DialogBuilder_inst::DoCreateRadioButtonBinding(ssize_t * Value) {
	return new PluginRadioButtonBinding(Info, &DialogHandle, DialogItemsCount - 1, Value);
}

PCWSTR DialogBuilder_inst::get_lang_string(int LabelId) const {
	return Info.GetMsg(&PluginId, LabelId);
}

void DialogBuilder_inst::InitDialogItem(FarDialogItem * Item, PCWSTR Text) {
	memset(Item, 0, sizeof(FarDialogItem));
	Item->Data = Text;
}

void DialogBuilder_inst::AddBorder(PCWSTR TitleText) {
	FarDialogItem *Title = AddDialogItem(DI_DOUBLEBOX, TitleText);
	Title->X1 = 3;
	Title->Y1 = 1;
}

FarDialogItem * DialogBuilder_inst::AddDialogItem(FARDIALOGITEMTYPES Type, PCWSTR Text) {
	if (DialogItemsCount == DialogItemsAllocated) {
		ReallocDialogItems();
	}
	int Index = DialogItemsCount++;
	FarDialogItem * Item = &DialogItems[Index];
	InitDialogItem(Item, Text);
	Item->Type = Type;
	Bindings[Index] = nullptr;
	return Item;
}

void DialogBuilder_inst::SetNextY(FarDialogItem * Item) {
	Item->X1 = 5 + Indent;
	Item->Y1 = Item->Y2 = NextY++;
}

void DialogBuilder_inst::SaveValues() {
	int RadioGroupIndex = 0;
	for (int i = 0; i < DialogItemsCount; i++) {
		if (DialogItems[i].Flags & DIF_GROUP)
			RadioGroupIndex = 0;
		else
			RadioGroupIndex++;

		if (Bindings[i])
			Bindings[i]->SaveValue(&DialogItems[i], RadioGroupIndex);
	}
}

DialogItemBinding_i * DialogBuilder_inst::FindBinding(FarDialogItem * Item) {
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

void DialogBuilder_inst::ReallocDialogItems() {
	// реаллокация инвалидирует указатели на DialogItemEx, возвращённые из
	// AddDialogItem и аналогичных методов, поэтому размер массива подбираем такой,
	// чтобы все нормальные диалоги помещались без реаллокации
	// TODO хорошо бы, чтобы они вообще не инвалидировались
	DialogItemsAllocated += 32;
	if (!DialogItems) {
		DialogItems = new FarDialogItem[DialogItemsAllocated];
		Bindings = new DialogItemBinding_i *[DialogItemsAllocated];
	} else {
		FarDialogItem *NewDialogItems = new FarDialogItem[DialogItemsAllocated];
		DialogItemBinding_i **NewBindings = new DialogItemBinding_i *[DialogItemsAllocated];
		for (int i = 0; i < DialogItemsCount; i++) {
			NewDialogItems[i] = DialogItems[i];
			NewBindings[i] = Bindings[i];
		}
		delete[] DialogItems;
		delete[] Bindings;
		DialogItems = NewDialogItems;
		Bindings = NewBindings;
	}
}

void DialogBuilder_inst::SetLastItemBinding(DialogItemBinding_i * Binding) {
	Bindings[DialogItemsCount - 1] = Binding;
}

void DialogBuilder_inst::UpdateSecondColumnPosition() {
	int SecondColumnX1 = 6 + (DialogItems[0].X2 - DialogItems[0].X1 - 1) / 2;
	for (int i = 0; i < DialogItemsCount; i++) {
		if (DialogItems[i].X1 == SECOND_COLUMN) {
			int Width = DialogItems[i].X2 - DialogItems[i].X1;
			DialogItems[i].X1 = SecondColumnX1;
			DialogItems[i].X2 = DialogItems[i].X1 + Width;
		}
	}
}

int DialogBuilder_inst::MaxTextWidth() {
	int MaxWidth = 0;
	for (int i = 1; i < DialogItemsCount; i++) {
		if (DialogItems[i].X1 == SECOND_COLUMN)
			continue;
		int Width = ItemWidth(&DialogItems[i]);
		int Indent = DialogItems[i].X1 - 5;
		Width += Indent;

		if (MaxWidth < Width)
			MaxWidth = Width;
	}
	int ColumnsWidth = 2 * ColumnMinWidth + 1;
	if (MaxWidth < ColumnsWidth)
		return ColumnsWidth;
	return MaxWidth;
}

void DialogBuilder_inst::UpdateBorderSize() {
	FarDialogItem *Title = &DialogItems[0];
	Title->X2 = Title->X1 + MaxTextWidth() + 3;
	Title->Y2 = DialogItems[DialogItemsCount - 1].Y2 + 1;

	for (int i = 1; i < DialogItemsCount; i++) {
		if (DialogItems[i].Type == DI_SINGLEBOX) {
//				Indent = 2;
//				DialogItems[i].X2 = Title->X2;
		}
	}

	Title->X2 += Indent;
	Indent = 0;
}


///=================================================================================================
DialogBuilder_i * get_dialog_builder(const PluginStartupInfo & psi, const GUID & pluginId, const GUID & aId, PCWSTR TitleLabel, PCWSTR aHelpTopic, FARWINDOWPROC aDlgProc, void * aUserParam) {
	return new DialogBuilder_inst(psi, pluginId, aId, TitleLabel, aHelpTopic, aDlgProc, aUserParam);
}


DialogBuilder_i * get_dialog_builder(const PluginStartupInfo & psi, const GUID & pluginId, const GUID & aId, int TitleLabelId, PCWSTR aHelpTopic, FARWINDOWPROC aDlgProc, void * aUserParam) {
	return new DialogBuilder_inst(psi, pluginId, aId, TitleLabelId, aHelpTopic, aDlgProc, aUserParam);
}
