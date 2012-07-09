#include "DlgBuilder_pvt.hpp"

namespace Far {

	PluginEditFieldBinding::PluginEditFieldBinding(HANDLE & aHandle, FarDialogItem * Item, ssize_t aId, PWSTR aValue, int aMaxSize):
		DialogItemBinding(aHandle, Item, aId),
		Value(aValue),
		MaxSize(aMaxSize) {
	}

	void PluginEditFieldBinding::save() const {
		PCWSTR DataPtr = (PCWSTR)psi().SendDlgMessage(get_dlg(), DM_GETCONSTTEXTPTR, get_index(), nullptr);
		lstrcpynW(Value, DataPtr, MaxSize);
	}

	ssize_t PluginEditFieldBinding::get_width() const {
		return 0;
	}

}
