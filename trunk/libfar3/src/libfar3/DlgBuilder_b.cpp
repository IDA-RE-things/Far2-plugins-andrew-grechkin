#include "DlgBuilder_pvt.hpp"

namespace Far {

	PluginCheckBoxBinding::PluginCheckBoxBinding(HANDLE & aHandle, ssize_t aId, ssize_t * aValue, ssize_t aMask) :
		DialogItemBinding_i(aHandle, aId),
		Value(aValue),
		Mask(aMask)
	{
	}

	void PluginCheckBoxBinding::save_() const
	{
		intptr_t Selected = psi().SendDlgMessage(get_dlg(), DM_GETCHECK, get_index(), 0);
		if (!Mask) {
			*Value = Selected;
		} else {
			if (Selected)
				*Value |= Mask;
			else
				*Value &= ~Mask;
		}
	}

	ssize_t PluginCheckBoxBinding::get_width() const
	{
//		return ::lstrlenW(get_item()->Data) + 4;
		return 0;
	}

}
