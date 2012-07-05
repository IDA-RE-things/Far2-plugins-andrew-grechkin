#include <libfar3/helper.hpp>
#include <libfar3/DlgBuilder.hpp>

DialogItemBinding_i::DialogItemBinding_i():
	BeforeLabelId(-1),
	AfterLabelId(-1) {
}

DialogItemBinding_i::~DialogItemBinding_i() {
}

void DialogItemBinding_i::SaveValue(FarDialogItem * /*Item*/, int /*RadioGroupIndex*/) {
}


///=================================================================================================
struct CheckBoxBinding: public DialogItemBinding_i {
	CheckBoxBinding(ssize_t * aValue, ssize_t aMask);

	virtual ~CheckBoxBinding();

	virtual void SaveValue(FarDialogItem * Item, int RadioGroupIndex);

private:
	ssize_t * Value;
	ssize_t Mask;
};

CheckBoxBinding::CheckBoxBinding(ssize_t *aValue, ssize_t aMask):
	Value(aValue),
	Mask(aMask) {
}

CheckBoxBinding::~CheckBoxBinding() {
}

void CheckBoxBinding::SaveValue(FarDialogItem * Item, int /*RadioGroupIndex*/) {
	if (!Mask) {
		*Value = Item->Selected;
	} else {
		if (Item->Selected)
			*Value |= Mask;
		else
			*Value &= ~Mask;
	}
}


///=================================================================================================
struct RadioButtonBinding: public DialogItemBinding_i {
	RadioButtonBinding(int *aValue) :
		Value(aValue) {
	}

	virtual ~RadioButtonBinding() {
	}

	virtual void SaveValue(FarDialogItem * Item, int RadioGroupIndex) {
		if (Item->Selected)
			*Value = RadioGroupIndex;
	}

private:
	int *Value;
};


///=================================================================================================
struct ComboBoxBinding: public DialogItemBinding_i {
	ComboBoxBinding(int * aValue, FarList * aList):
		Value(aValue),
		List(aList) {
	}

	virtual ~ComboBoxBinding() {
		delete[] List->Items;
		delete List;
	}

	virtual void SaveValue(FarDialogItem * /*Item*/, int /*RadioGroupIndex*/) {
//		FarListItem &ListItem = List->Items[Item->ListPos];
//		*Value = ListItem.Reserved[0];
	}

public:
	int * Value;
	FarList * List;
};


///=================================================================================================
