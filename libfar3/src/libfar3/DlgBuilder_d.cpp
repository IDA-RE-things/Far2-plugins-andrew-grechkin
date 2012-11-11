#include "DlgBuilder_pvt.hpp"

namespace Far {

	PluginEditFieldBinding::PluginEditFieldBinding(HANDLE & aHandle, ssize_t aId, PWSTR aValue, ssize_t aMaxSize) :
		DialogItemBinding_i(aHandle, aId),
		Value(aValue),
		MaxSize(aMaxSize)
	{
	}

	void PluginEditFieldBinding::save_() const
	{
		PCWSTR DataPtr = (PCWSTR)psi().SendDlgMessage(get_dlg(), DM_GETCONSTTEXTPTR, get_index(), nullptr);
		lstrcpynW(Value, DataPtr, MaxSize);
	}

	ssize_t PluginEditFieldBinding::get_width() const
	{
		return 0;
	}

}
