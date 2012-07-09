#include "DlgBuilder_pvt.hpp"

namespace Far {

	PluginIntEditFieldBinding::PluginIntEditFieldBinding(HANDLE & aHandle, FarDialogItem * Item, ssize_t aId, ssize_t * aValue, ssize_t Width) :
		DialogItemBinding(aHandle, Item, aId),
		Value(aValue) {
		fsf().sprintf(Buffer, L"%u", *aValue);
		int MaskWidth = Width < 31 ? Width : 31;
		for (int i = 1; i < MaskWidth; i++)
			Mask[i] = L'9';
		Mask[0] = L'#';
		Mask[MaskWidth] = L'\0';
	}

	void PluginIntEditFieldBinding::save() const {
		PCWSTR DataPtr = (PCWSTR)psi().SendDlgMessage(get_dlg(), DM_GETCONSTTEXTPTR, get_index(), 0);
		*Value = fsf().atoi(DataPtr);
	}

	ssize_t PluginIntEditFieldBinding::get_width() const {
		return 0;
	}

	PWSTR PluginIntEditFieldBinding::GetBuffer() {
		return Buffer;
	}

	PCWSTR PluginIntEditFieldBinding::GetMask() const {
		return Mask;
	}

}
